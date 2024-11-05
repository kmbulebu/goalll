# Research Log

As we research and experiment, our findings are recorded here.

## Score Sensing

Bridging the physical and virtual world is often the most challenging part for this type of project.
Our first challenge is to devise a way to sense when a goal has been scored. 
Within the plastic goal housing, the ball is guided down a ramp and through a narrow opening before dropping
into the ball return. We need a reliable method for detecting when the ball passes through this opening. Die hards may contend
that a bounce out is still a goal, but we can implement a workaround for this edge case in the user interface.

### Brain storm

A rough list of ideas for sensing the ball passing through the ball return:

- A microswitch with lever that the ball will push down.
- A momentary button that the ball will roll over. 
- Ultrasonic emitter and sensor to detect when something is closer to it than the wall.
- An IR reflectivity sensor to detect when something is closer to it than the wall.
- Separate IR emitter and sensor to detect when something interrupts the light.
- A microphone and sufficiently advanced AI inference to detect the sound of a score.
- A pressure plate inside the ball return to detect when the weight of returned balls increases.

Some of these ideas are a bit silly but let's look at some of the challenges:

- A ball can be added to the ball return outside of play. e.g. someone picks one up off the floor and returns it. 
- Balls, while not all regulation, could be of slightly different diameters and weights.
- Mechanically interfering with the ball invites reliability issues such as stuck balls. 
- A full ball return will let fast balls fall out. 

### Plan A: IR reflectivity sensor

IR reflectivity sensors are often used in amatuer robotics for obstacle avoidance. Infrared light is emitted, reflects off nearby surfaces, and the reflected light is measured by a sensor packaged next to the emitter. Measuring the strength of the received IR light can determine the relative proximity or presence of an object. 

We are making a few initial assumptions about this choice:

- Balls are reflective enough
- Goal and ball return enclosure will block IR light interference
- Sensors are fast enough to detect a dropping ball

#### Part Selection

From Amazon we purchased a TCRT5000 based sensor package: [HiLetgo 10pcs TCRT5000 Infrared Reflective Sensor](https://www.amazon.com/dp/B00LZV1V10)

Some things we found attractive:
- It's cheap. 10 pieces for $9 delivered.
- Supports 3.3-5V allowing for flexible prototyping
- Supported distance (25mm) has range for our ball drop
- Adjustable digital output from included comparator and potentiometer
- Analog output provides 'raw' measurement
- Integrated LEDs help with debug

#### Experiment 1: Breadboard with Multimeter

The IR reflectivity sensor was attached to a breadboard and powered with a 5 volt bench top power supply. A multimeter was
used to measure the voltage between ground and the analog or digital output, marked AO, and DO respectively. A ball was moved
in front of the sensor. Here's what we learned:

- Digital Output
  - Close Object = Low (0V)
  - Far Object = High (~4.85V)
  - ~2 centimeter distance to far cut off ambient light with OOB setting

- Analog Output
  - Voltage varies by object distance
    - Likely not linear
  - Far Object = ~5V
  - Close Object = ~0V 

**Conclusion: Digital output will be used for detecting balls.

##### Experiment 2: Testing Goals

The IR reflectivity sensor was attached to a long cable and hot glued in the ball return. An oscilloscope was setup to watch the digital output. 

In this configuration, the sensor nominally reads high. When a ball passes, it should drop low, and return to high again. To capture the full event, the oscilloscope trigger was setup for a one shot sweep with detection on the rising edge. 

Observations with ball drops:
- The digital output signal went low and returned to high again
- With red, rough ball, the signal was low for 15-19 milliseconds. 
- With white, shinier ball, the signal was low for 53-55 milliseconds. 
- Balls that fell along the front wall were often not detected.
  - The sensor was hot glued to the back wall.
- A shiny, black bottle cap was dropped repeatedly through and was never noticed by the sensor. 

Conclusions:
- The sensor will need mounted more towards the center of the space between the front and back wall to capture all balls.
- The software will need to allow for a wide range of low pulse lengths, perhaps from 10-100ms.
- Some balls, perhaps a very reflective or black ball may not be detectable.
- No issues with using a 3 meter length of cable between the sensor and the oscilloscope.


##### Experiment 3: IR Interference Testing

The IR emitter operates at 950nm wavelength. We will test using a consumer TV remote control for creating interference.

First, the IR reflectivity sensor was setup on the bench with oscilloscope attached to the digital output. A TV remote was pointed
at the sensor and buttons pressed. The signalling waveform of the TV remote was clearly read by the reflectivity sensor and displayed
on the Oscilloscope screen. 

We performed the same test again, but on the IR reflectivity sensor mounted within the foosball goal ball return. Despite pointing the TV remote
into the ball return and the goal from many angles, we could not register a signal on the Oscilloscope.

Conclusion:

The IR reflectivity sensor is prone to consumer IR remote interference when in close proximity however the foosball table goal sufficiently blocks the interference.

