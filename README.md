# thermal_cam_mlx90640
there is a file mlx90640_st77xx.zip that has support for lcd output. 

st77xx 128x128 display. at this time this is the only display supported. it outputs about 2 fps at 64x48 resolution scalled to 128x96. more fps will be possible possibly later

if you want a little more speed Z_MemManagment change Replace_detailed_calc_with_image_data false to true
the display will start out saturated but sensor will adjust within a few seconds.

st77xx vin arudino 5v pin

st77xx gnd arudino gng pin

st77xx clk arudino 13 pin

st77xx miso arudino 12 pin

st77xx mosi arudino 11 pin

st77xx CS arudino 10 pin

st77xx rst arudino 9 pin

st77xx d/c arudino 8 pin



sensor mlx90640
mlx90640 3v to 3v st77xx pin
mlx90640 gnd to gnd arduino pin
mlx90640 SDA to 4 pin arduino
mlx90640 SLC to 5 pin arduino


all i ask is you give credit where credit do. to MIT, melexis,and to original authors and to me for my time and effort here. you might be doing the next big thing and the work here might help or save you time.

in /updates folder is working code that calibrates to using arduino 8 bit micro controllers. 

also code exists that doubles output resolution. it is a wrappper that does interpolations math and data caching so all you 

need to do is assume 64,48 and it will take care of resolution details automatically.

 float temp=DoubleResolutionValue(x,y);
 
//all of these are still a work in progress, however there is fully functional code in the /updates folder. there is code that works calibrated for the arudino, and it has been verfied to the original test code. it uses about 1k on 8bit processors. also there are some cache methods being implimented. so far only doubleResolution uses a 256 byte cache.

look in the /updates folder for code. this code will be moved into the main directory sometime after end of sept.

look in updates folder for progress of code being converted to use less memory. as a bonus it doubles resolution and creates an ouput that is image like on serial monitor terminal.

a few notes:

you will want to run getEEpromDump.zip to get eeprom data from mlx60640, this allows all calibration data from factory to be on eeprom of arduino, so it saves time and data space doing it this way. run the data dump, then cut and paste into factoryCalData.h folder on other programs

MLX90640_1tempcell.zip program is for temp calc of silicon. silicon temp is compared to temp of thermopile to adjust for ambient temp for later calculations.
it does do a single pixel reference as well, but sometimes the cell it is on is not able to be calibtrated
so change these lines in main sketch folder to another pixel.
Getpixeldata(12,10); 
CalculateTo(12,10);

MLX90640_example32ataTime.zip	to test in terminal program if a row of sensors is working, it shows a pattern when you move your hands across sensor.

to find out more information i would recommend looking at my notes here https://hackaday.io/project/161499-mlx90640-sensor-works-w-800-bytes it might be a little confusing but should fill in the gaps not mentioned in the notes here.


//

this is similar, or same as thermal cam project but with mlx90640 sensor. the idea is to get it working on arduino? why would i want to do that? it lowers the cost for trying out and testing devices, and allows more power and room for upgrade performance.
these sensors work differently and have different resolution, so process is different in many areas
one of the things that is different is paging of data and built in ram locations.
i have here an example that reads the first bank of 16 sensors.
at start up it calibrates to heat levels, and then anything above the heat level at start up registers on the terminal.

example showing it is is here:
https://youtu.be/r1J1AwunTps


MLX90640_example32ataTime.zip 
shows how to access status registers and memory page info, also as long as you only read sensor after enough time for mode has passed (2hz) means at least 500ms per page, then data will be available for both pages in ram,
also there is a single shot mode, which i will talke advantage of that fills all ram, 2 passes and then allows you to read it even 1 byte at a time.


fullsensorreadToterminal.zip reads entire sensor to screen, as a dot and dash to see sensor data.all of it. 

what will be needed going foward is the files that dump the calibration data from the mlx90460 sensor. this data will be used and stored in progmem (system flash) to save ram, and increase speed as we only need to read i2c buss for temp data mostly.
