update is coming soon, just testing serial compression, and trying new math improvements in To cacl. if i can i will include math improvements as well. still need to finish testing on teensy3.1/3.2 and teensy-lc for compatability. it also works on uno and nano. i will update soon. 9/13/19...... well almost that date. lol

code in this folder will start to require the file factoryCalData.h, you will want to run a program included in folder called 
'getEEpromDump' extract it and with sensor functional pull out the data from the thermal sensor. this data needs to be cut and past from serial output and put in brackets of factoryCalData.h


This is for updates while testing and verifying sensor. 
i am uploading a movie of sensor showing my hand count to five. 
this is the same output you would see if you upload the files in this folder

my goal is to after this verification start switching code out for my new routines, reducing memory footprint.
i have had the sensor working for some time at some level, but there are issues with the melexis documentation, 
so i got a 3.2 teensy processor with 64k of ram and have it outputing a screen equivilent of data.

instead of printing out C from mlx90640To[x], i switched output to be this
* this was before i modified mem pointers, now mem is accessed thru a function Readmlx90640To();
   float temp=mlx90640To[x+y*32];
   
   if (temp<26){Serial.print(". .");}
   
    if (temp>26 & temp<29){Serial.print(".-.");}
    
   if (temp>29 & temp<31){Serial.print(".+.");}
   
   if (temp>31 & temp<33){Serial.print(".O.");}
   
   if (temp>33 & temp<35){Serial.print(".#.");}
   
   if (temp>35){Serial.print(".@.");}
   this is sort of a pixel art map from ascii char art.
   
   this allows a denser display showing of information if a warm body is in front of sensor.
   
   anyways, this code is what i am modifying to reduce overhead.
   
   i suspect a few things will eventually happen
   
   1) reduce ram usage to about 1k from ~20k (most ram values are static floats, that are only calculated at init.
   2) store rom cal values in flash for faster access, than pulling from thermopile flash (this is why current store is ram)
   3) dramatically increase performance by removing redundant math. for example squaring in this contects is just 2^0 to 2^16. (found in some cases this goes up past 34, so need to expand the table), so not using this until table is redone
   a table will speed up math processes in this case by a guestimate of 100.
   4) a lot less ram thrashing of floats. for now floats still needed because of range of values
   
