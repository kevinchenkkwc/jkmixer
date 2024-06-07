#include "gpio.h"
#include "i2s.h"
#include "audio.h"
#include "printf.h"
#include "timer.h"
#include "uart.h"
#include "malloc.h"
#include "dma.h"
#include "strings.h"
#include <stdint.h>
#include "THX.h"

static struct {
    int level;
    int compression_threshold;
    bool backing_track;
    int length_of_recording;
    bool reverb;
} config;

uint16_t levels(uint16_t sample) {    
    if (config.level == 0) {
        return 0;
    }
    if (config.level == -2) {
        sample = (uint16_t)(sample / 2);
    }
    else {
        sample = (uint16_t)(sample * config.level);
    }
    return sample;
}

uint16_t compression(int sample) {
    int threshold;
    if (config.compression_threshold == 20) {
        threshold = 0xf000;
    }
    if (config.compression_threshold == 15) {
       threshold = 0xa000;
    }
    if (config.compression_threshold == 10) {
        threshold = 0x6000;
    }
    if (config.compression_threshold == 5) {
        threshold = 0x1000;
    }
    if (config.compression_threshold == 0) {
        threshold = 0;
    }
    if (sample < 0xa000) {
        sample = threshold - (int)((sample - threshold) / 10);
    }
    return sample;
}

uint16_t backingtrack(uint16_t sample, int idx) {
    if (config.backing_track == false) {
        return sample;
    }
    int backing = pcm_data[idx];
    return ((uint16_t) (sample + backing));
}


void main ()
{
    gpio_init();
    uart_init();
    i2s_init();
    mic_init();
    
    // debug------------------------
    config.level = 1;
    config.compression_threshold = 10;
    config.backing_track = false;
    config.length_of_recording = 5;
    config.reverb = false;
    // debug------------------------

    printf("starting mic read\n");
    const int num_samples = (44100 * config.length_of_recording); // sameple rate = 44100
    
    uint32_t *audio_samples = malloc(num_samples * sizeof(uint32_t) + 400);
    
    mic_capture_dma(audio_samples, num_samples);
    dma_mic_start();

    int count = 0;

    while (1) {
        printf("doing other stuff count: %d\n", count);
        count++;
        if (dma_complete(1)) {
            dma_disable(1);
            printf("Collection finished!\n");
            // convert to 16-bit samples
            uint16_t *converted_samples = malloc(num_samples * sizeof(uint16_t));
            uint16_t *reverb_buffer = malloc(num_samples * sizeof(uint16_t) + 4800);
            memset(reverb_buffer, 0, (num_samples * sizeof(uint16_t) + 4800));
            for (int i = 0; i < num_samples; i++) {
                converted_samples[i] = audio_samples[i] >> 16;
                
                // make intro & outro clipping less awful
                if (i < 44100) {
                    converted_samples[i] = (uint16_t)(converted_samples[i] / 2);
                }
                if (i > (num_samples - 44100)) {
                    converted_samples[i] = (uint16_t)(converted_samples[i] / 2);
                }

                converted_samples[i] = levels(converted_samples[i]);
                converted_samples[i] = compression(converted_samples[i]);
                
                // reverb
                if (config.reverb) {
                    reverb_buffer[i] += converted_samples[i];
                    reverb_buffer[i + (44 * 36)] += (int)(converted_samples[i] / 2);
                    reverb_buffer[i + (44 * 72)] += (int)(converted_samples[i] / 4);
                    reverb_buffer[i + (44 * 108)] += (int)(converted_samples[i] / 8);
                }

                // backing track
                if (config.reverb) {
                    reverb_buffer[i] = backingtrack(reverb_buffer[i], i);
                }
                else {
                    converted_samples[i] = backingtrack(converted_samples[i], i);
                }
            }
            free(audio_samples);
            i2s_init();

            audio_init(44100, 2, MONO);
            // audio_init(44100, 2, STEREO); // want to test
            printf("1 second pause...\n");
            timer_delay_ms(1000);
            printf("starting play\n");
            if (config.reverb) {
                audio_write_i16_dma(reverb_buffer, num_samples, 0);
                while (!dma_complete(0)) {
                    printf("playing reverb audio\n");
                }
                dma_disable(0);
            }
            else {
                audio_write_i16_dma(converted_samples, num_samples, 0);
                while (!dma_complete(0)) {
                    printf("playing audio\n");
                }
                dma_disable(0);
            }
            printf("done playing\n");
            free(converted_samples);
            return;
        }
    }
}
