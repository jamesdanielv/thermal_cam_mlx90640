# thermal_cam_mlx90640
same as thermal cam project but with mlx90640 sensor. the idea is to get it working on arduino? why would i want to do that? it lowers the cost for trying out and testing devices, and allows more power and room for upgrade performance.
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
