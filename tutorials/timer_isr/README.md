# Timer Update ISR Blink

This is the first of 3 timer LED blink tutorials. In this one, we'll set up a timer to generate an interrupt every 500ms. In that interrupt routine, we'll toggle our LED. This improves upon our previous LED blink demo by allowing us to toggle the LED at exactly 500ms regardless of whatever else we might need to do in our FW. In fact, at the end of this tutorial, our main while loop will be empty - but we could utilize almost all of our CPU cycles here to do something, and it wouldn't impact the LED blinking (we just need a few cycles to run the timer ISR every 500ms).

Then we'll see how we can further improve our LED blink demo using the Output Capture Compare feature of STM32 timers. This will allow us to toggle the LED at exactly 500ms intervals even without an ISR.

Finally, we'll build on the Output Capture Compare mode to drive the LED with PWM output. This is convenient for controlling the LED's brightness.