## Project title
The JK Mixer — "Live" mixing with the Mango Pi

## Team members
Joseph Shull and Kevin Chen

## Project description
We will be adapting the speaker setup Chris demonstrated in the class by also
implementing a microphone. We hope to be able to efficiently transpose and
process live audio, and replay it through speakers.
Our project has the ability to change levels, compression, add a backing track,
modify the length of the recording, and add reverb.
There is also a handy UI application to modify the values of these features.

## Member contribution
Joseph worked on the breadboarding and the soldering. He programmed the main
functionality for levels, compression, backing track, length, and reverb. He
also was in charge of testing the audio, and making sure that everything sounds
great!

Kevin was in charge of designing and programming the UI. This included applying
and adding to our graphics library, adding keyboard functionality, swapping
between buffers, and combining the two components.

Ideas and concepts were contributed by both members.


## References
Cite any references/resources that inspired/influenced your project. 
If your submission incorporates external contributions such as adopting 
someone else's code or circuit into your project, be sure to clearly 
delineate what portion of the work was derivative and what portion is 
your original work.

Chris' microphone and speaker application! Very much saved the day and allowed
us to focus more on modifying the microphone input rather than painstakingly
figuring our how to capture it.
We were both interesting in Music production, and how that would work with I2S
microphones and speakers.
We used Logic Pro (Digital Audio Workstation) as a primary inspiration.

We also used the Midpoint circle algorithm for drawing the circles, which was
added into gl.c

Outside of these inspirations (+ Chris' hard work), all of our code was
original.

## Self-evaluation
How well was your team able to execute on the plan in your proposal?  
Any trying or heroic moments you would like to share? Of what are you particularly proud:
the effort you put into it? the end product? the process you followed?
what you learned along the way? Tell us about it!

I think we were able to execute our plan really well! We were struggling
initially with the Microphone Amplifyer and the Microphone, so Chris really came
in and saved the day for that.

We have a small list of short-comings and "failed" implementations that we
decided not to include in our final presentation:

Recording within session: A problem we faced was properly freeing space for our
converted_samples and reverb_buffer. In particular, we experienced a Valgrind
error when trying to free reverb_buffer. 

Compression: While we did include compression, it was one of the most finnicky
applications that we had. 

Next Steps:
- FFT Library for EQ
- Audio Visualizer
- Remixing Audio/Replaying Audio
- PS/2 Mouse adaptation

The above are all applications that we really wanted to do, but we unfortunately
didn't have the time.

One of the more "trying" and exciting moments was combining the UI and the
Mixing files. It took us a while to debug, with a non-functioning keyboard, and
super frustrating wiring, but once we got it working, it was truly exhilerating!

We both learned a lot through this project, both in terms of how sound is can be
visualized with I2S, and with how mixers potentially modify sound. One of the
most interesting functionalities was both reverb and backing track. Testing
sound, and slowly modifying the code to achieve a good sound was a fun process!

In terms of the UI, it was really fun to figure out how to update the config
structure while also displaying the appropriate knob values, as well as the
rectangles. While it may not been the most elegant solution, I felt like it
worked really well with what we were trying to achieve!

## Photos
You are encouraged to submit photos/videos of your project in action. 
Add the files and commit to your project repository to include along with your submission.

Video attached!
