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

#### Sensor Experiment 1: Breadboard with Multimeter

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

##### Sensor Experiment 2: Testing Goals

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


##### Sensor Experiment 3: IR Interference Testing

The IR emitter operates at 950nm wavelength. We will test using a consumer TV remote control for creating interference.

First, the IR reflectivity sensor was setup on the bench with oscilloscope attached to the digital output. A TV remote was pointed
at the sensor and buttons pressed. The signalling waveform of the TV remote was clearly read by the reflectivity sensor and displayed
on the Oscilloscope screen. 

We performed the same test again, but on the IR reflectivity sensor mounted within the foosball goal ball return. Despite pointing the TV remote
into the ball return and the goal from many angles, we could not register a signal on the Oscilloscope.

Conclusion:

The IR reflectivity sensor is prone to consumer IR remote interference when in close proximity however the foosball table goal sufficiently blocks the interference.

## Microcontroller Selection and Development

We have started prototyping and development with a [ESP32-C3](https://www.espressif.com/en/products/socs/esp32-c3)
[devkit](https://wiki.luatos.org/chips/esp32c3/board.html#id5) bought from [Amazon](https://www.amazon.com/gp/product/B0BWN14X65/ref=ppx_yo_dt_b_search_asin_title?ie=UTF8&psc=1). 

Some reasons:

- I already it
- It's inexpensive
- System on Chip includes WiFi and Bluetooth connectivity
- PlatformIO and Arduino support
- Plenty of peripherals (UART, GPIO, ADC, SPI, I2C, etc)

The devkit seems to be compatible with or a clone of the AirM2M Core ESP32-C3 board and
supported by [PlatformIO](https://docs.platformio.org/en/latest/boards/espressif32/airm2m_core_esp32c3.html).

Our platformio.ini configuration:
```
[env:airm2m_core_esp32c3]
platform = espressif32
board = airm2m_core_esp32c3
framework = arduino
build_type = debug
upload_protocol = esp-builtin
debug_tool = esp-builtin
```

To validate our configuration, we copied the [Blinker sample](https://github.com/platformio/platform-espressif32/blob/develop/examples/arduino-blink/src/Blink.cpp) and verified the built-in LED flashed under control of our code. We then modified the code it to blink the LED based on a GPIO input. Using this we verified the Ardunio pin assignments matched the labels on the dev board. 

Other findings:

- Upload/Flash works correctly.
- Debugger mostly works but with some issues.
  - Lots of console errors (benign?) when attached.
  - Breakpoints within interrupt routines trigger but show no values.

## Prototyping PIN Assignments

| Pin | Logical PIN | Assignment | 
| --- | ----------- | ---------- |
| 05  | USB_D-      | USB-C Connector |
| 06  | USB_D+      | USB-C Connector |
| 19  | SPI_CK      | Display Home Pin 4 |
| 20  | SPI_MOSI    | Display Home Pin 5 |
| 21  | SPI_MISO    | |
| 23  | SPI_CS      | Display Home CS Pin 16 | 
| 04  | GPIO12      | Display Home DC Pin 14 |
| 10  | GPIO13      | Display Home RST Pin 15 |
| 30  | GPIO09      | Game Button |
| 27  | GPIO05      | Away Goal IR Sensor DO |
| 28  | GPIO04      | Home Goal IR Sensor DO |


## Display Selection

Teyleten Robot 3.12 inch OLED Display Module 256 * 64 OLED Display Module 7-Pin SPI Interface SSD1322 Serial Screen White on [Amazon](https://www.amazon.com/dp/B0C61Z6Q8V)

> From Tim Corcoran's very helpful review:
>This is a very nice 256*64 display based on the SSD1322 controller. The module can be configured to communicate via SPI (3 or 4 wire) or Parallel (80xx or 68xx). It >comes configured for 80xx so I needed to move the 0-ohm bridge from R6 to R5 for use in my ESP32 design. Strapping options are clearly printed on the back of the >module. I tested with a SEEED XIAO ESP32-S3 dev board. The easiest library to use with it is U8G2 and I used the following constructor line for the XIAO pinout:
>U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 6, /* dc=*/ 4, /* reset=*/ 5);
>
>The module came without any documentation and if you search for the pinout, there is a lot of confusion on other Amazon listings. For my SPI-7 setup I wired the >following pins of the 16 pin connector on the module to my XIAO controller.
>
>1: Ground
>2: VCC (3.3v, did not try 5V)
>4: SCLK (to GPIO7 on XIAO)
>5: MOSI (to GPIO9 on XIAO)
>14: DC. (to GPIO4 on XIAO)
>15: RST (to GPIO5 on XIAO)
>16: CS (to GPIO6 on XIAO)