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
    int leval;
    int compression_threshold;
    int compression_ratio;
    bool backing_track;
    int backing_track_leval;
    int length_of_recording;
    bool reverb;
} config;

uint16_t levals(uint16_t sample) {    
    if (config.leval == -6) {
        return 0;
    }

    int adjustment = config.leval * 3000;

    if ((sample + adjustment) < 0xffff) {
        if ((sample + adjustment) > 0x0) {
            sample += adjustment;
        }   
        else {
            sample = 0x0;
        }   
    }   
    else {
        sample = 0xffff;
    }   

    return sample;
}

uint16_t compression(int sample) {
    int threshold = config.compression_threshold * 3000;
    if (sample > threshold) {
        sample = threshold + ((sample - threshold) / config.compression_threshold);
    }
    return sample;
}

uint16_t backingtrack(uint16_t sample, int idx) {
    if (config.backing_track_leval == -6) {
        return sample;
    }
    
    int adjustment = config.backing_track_leval * 3000;
    int backing = pcm_data[idx];
    sample += (backing + adjustment);

    if (sample < 0x0) {
        return 0x0;
    }
    if (sample > 0xffff) {
        return 0xffff;
    }
    return ((uint16_t) sample);
}


void main ()
{
    gpio_init();
    uart_init();
    i2s_init();
    mic_init();
    
    // debug------------------------
    config.leval = 2;
    config.compression_threshold = 2;
    config.compression_ratio = 2;
    config.backing_track = true;
    config.backing_track_leval = -2;
    config.length_of_recording = 3;
    config.reverb = true;
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
            uint16_t *converted_samples = malloc(num_samples * sizeof(uint16_t) + 400);
            uint16_t *reverb_buffer = malloc(num_samples * sizeof(uint16_t) + 400);

            for (int i = 0; i < num_samples; i++) {
                converted_samples[i] = audio_samples[i] >> 16;
                // process
                converted_samples[i] = levals(converted_samples[i]);
                converted_samples[i] = compression(converted_samples[i]);
                // reverb
                if (config.reverb) {
                    reverb_buffer[i] = converted_samples[i];
                    reverb_buffer[i + (44 * 3)] = (int)(converted_samples[i] / 2);
                    reverb_buffer[i + (44 * 6)] = (int)(converted_samples[i] / 4);
                    reverb_buffer[i + (44 * 6)] = (int)(converted_samples[i] / 8);
                } 

                //backing track
                //converted_samples[i] = backingtrack(converted_samples[i], i);
                
                if (converted_samples[i] < 0x0) {
                    converted_samples[i] = 0x0;
                }
                if (converted_samples[i] > 0xffff) {
                    converted_samples[i] = 0xffff;
                }
            }
            free(audio_samples);
            if (config.reverb) {
                memcpy(converted_samples, reverb_buffer, (num_samples * sizeof(uint16_t) + 400));
                free(reverb_buffer);
            }
            i2s_init();

            audio_init(44100, 2, MONO);

            printf("1 second pause...\n");
            timer_delay_ms(1000);
            printf("starting play\n");
            audio_write_i16_dma(converted_samples, num_samples, 0);
            while (!dma_complete(0)) {
                printf("playing audio\n");
            }
            dma_disable(0);
            printf("done playing\n");
            free(converted_samples);
            return;
        }
    }
}
