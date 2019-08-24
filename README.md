# thermal_cam_mlx90640
all i ask is you give credit where credit do. to MIT, to original authors and to me for my time and effort here. you might be doing the next big thing and the work here might help or save you time.


//all of these are still a work in progress. but there is enough here to see what is going on and get sensor working on arduino, but with the calibrations settings the data is currently out of range for the arduino in the updates folder. the main folder pulls data from rom and seems to measure, but it only gets the temp of the silicone and errors from there. i rebuilt the software and had it so i can compare old and new methods side by side. there are math errors on arduino, however the code does take a lot less memory and work on the teensy 3.1/3.2. look in updates folder. i will be making changes to code that allow a 64 bit float library to be used. this should allow the uno to function ok, also there is the possibility of simplifying the equations to reduce the POW (base, exponent) and the sqrt requirements that make great resulution detail nessisary for calibrated results. however the examples the uno will work ok, because there is no detailed calibration just a sensativity adjustment at start up.

look in updates folder for progress of code being converted to use less memory. as a bonus it doubles resolution and creates an ouput that is image like on serial monitor terminal.

a few notes:

you will want to run getEEpromDump.zip to get eeprom data from mlx60640, this allows all calibration data from factory to be on eeprom of arduino, so it saves time and data space doing it this way. run the data dump, then cut and paste into factoryCalData.h folder on other programs

MLX906040_1tempcell.zip program is for temp calc of silicon. silicon temp is compared to temp of thermopile to adjust for ambient temp for later calculations.
it does do a single pixel reference as well, but sometimes the cell it is on is not able to be calibtrated
so change these lines in main sketch folder to another pixel.
Getpixeldata(12,10); 
CalculateTo(12,10);

MLX906040_example32ataTime.zip	to test in terminal program if a row of sensors is working, it shows a pattern when you move your hands across sensor.

to find out more information i would recommend looking at my notes here https://hackaday.io/project/161499-mlx90640-sensor-works-w-800-bytes it might be a little confusing but should fill in the gaps not mentioned in the notes here.


//

this is similar, or same as thermal cam project but with mlx90640 sensor. the idea is to get it working on arduino? why would i want to do that? it lowers the cost for trying out and testing devices, and allows more power and room for upgrade performance.
these sensors work differently and have different resolution, so process is different in many areas
one of the things that is different is paging of data and built in ram locations.
i have here an example that reads the first bank of 16 sensors.
at start up it calibrates to heat levels, and then anything above the heat level at start up registers on the terminal.

example showing it is is here:
https://youtu.be/r1J1AwunTps


MLX906040_example32ataTime.zip 
shows how to access status registers and memory page info, also as long as you only read sensor after enough time for mode has passed (2hz) means at least 500ms per page, then data will be available for both pages in ram,
also there is a single shot mode, which i will talke advantage of that fills all ram, 2 passes and then allows you to read it even 1 byte at a time.


fullsensorreadToterminal.zip reads entire sensor to screen, as a dot and dash to see sensor data.all of it. 

what will be needed going foward is the files that dump the calibration data from the mlx90460 sensor. this data will be used and stored in progmem (system flash) to save ram, and increase speed as we only need to read i2c buss for temp data mostly.
