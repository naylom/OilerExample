# OilerExample
Code to implement an oiler for a lathe using an Arduino Uno

This project is designed to implement an oiler for a metal lathe.

The Oiler consists of multiple motors to pump oil and for each motor there is a sensor that signals as drips of oil are delivered.

Additionally there can be inputs from the lathe that signal when the lathe is powered on/off and additionally for each revolution of its spindle. If these inputs are present the
system has additional options to drive when the oiling starts.

This is sketch consists of an example 'OilerExample.ino' sketch that demonstrates how to use the oiler codebase.

The code is implemted in an object orientated approach. The code base defines and instatiates an object to represent the oiler system, called TheOiler.  It also instantiates an object to represent the device being oiled, called TheMachine which
tracks machine units (number of spindle revolutions) and number of seconds of powered on time that has elapsed and finally there is an object called TheTimer that schedules work on requested basis.

When run, the first step is to add motor(s) to TheOiler and provide the pin(s) that signal when the motor generates output (referred to as a unit of work). Once done, TheOiler can be turned on. In this mode it waits for a configurable time
(in seconds) and then starts the motors. As the motors result in drips of oil, this is detected by the sensor and a count kept. The timer object regularly checks for the number of output 'units' and stops the related motor when a configurable
target is reached. When all motors are idle the Timer starts to monitors elasped time and restarts the motors when a configurable length of time has passed.

The functionality above can be enhanced by adding TheMachine object to the TheOiler. Once TheOiler 'knows' about TheMachine it can query TheMachine object about how many revolutions the spindle has done (described in the code as machine
work units) and also how long the lathe has been powered on. This information allows the Oiler to restart the motors on machine units (spindle revolutions) completed or on elapsed powered up time.

Currently the system supports two types of motors, a relay motor which uses the output of a single pin to drive a relay which in turn drives a dc motor. The second type is a four pin stepper driver & motor. This latter type has been tested
using a ULN2003 stepper driver and 28BYJ-48 stepper motor. This latter motor has additional features that the dc motor does not implement e.g. ability to programatically change the motor direction and the ability to alter the speed. For the
relay motor changing direction would be a wiring change at project setup.

Since the code uses a timer to monitor progress there is no need for a user to call any special function in the arduino loop function or worry that the loop is taking a long time and impacting the oiler processing. The loop is free for other
uses such as a user interface to monitor and control TheOiler.
The arduino setup function should be used to add motors to TheOiler, attach TheMachine to TheOiler, if TheMachine inputs are implemented in the project install, and finally turn TheOiler on.

In this example there is code to demonstrate adding a 4 pin stepper motor, starting the oiler, changing direction, using inputs from 'TheMachine' to start the oiler after spindle revolutions or elapsed powered on time. A rudimentary system monitor
and user interface is implemented in the loop function. This uses ANSI terminal control sequences (see https://en.wikipedia.org/wiki/ANSI_escape_code#Fe_Escape_sequences). To benefit from this connect to the Arduino serial port using a better terminal
monitor than the default one that comes with the arduino IDE. This was tested with the free version of PuTTY. Note to use this download the ketch to the arduino and take note of the port the arduino is on. Start your emaulator and connect to that port at 
the baud rate  used in the sketch (currently 19200) and off you go. Note that if you want to download the sketch again you will have to stop the terminal emulator so the arduino IDE can gain access.
