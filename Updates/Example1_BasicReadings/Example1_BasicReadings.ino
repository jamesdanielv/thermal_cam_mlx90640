/*code modified by james villeneve to show in monitor on screen serial output an image of data 768 sensors
 * also will be modifing code to use new routines that use less memory, and add in a init routine to verify
 * that flash and thermopile data cal are the same. 
 * 
  Read the temperature pixels from the MLX90640 IR array
  By: Nathan Seidle
  SparkFun Electronics
  Date: May 22nd, 2018
  License: MIT. See license file for more information but you can
  basically do whatever you want with this code.

  Feel like supporting open source hardware?
  Buy a board from SparkFun! https://www.sparkfun.com/products/14769

  This example initializes the MLX90640 and outputs the 768 temperature values
  from the 768 pixels.

  This example will work with a Teensy 3.1 and above. The MLX90640 requires some
  hefty calculations and larger arrays. You will need a microcontroller with 20,000
  bytes or more of RAM.

  This relies on the driver written by Melexis and can be found at:
  https://github.com/melexis/mlx90640-library

  Hardware Connections:
  Connect the SparkFun Qwiic Breadboard Jumper (https://www.sparkfun.com/products/14425)
  to the Qwiic board
  Connect the male pins to the Teensy. The pinouts can be found here: https://www.pjrc.com/teensy/pinout.html
  Open the serial monitor at 9600 baud to see the output
*/
//if false we use old data sets


#include <Wire.h>

#define DoubleResolution true //this doubels resolution output to 64x48 to make seeing objects a little easier
#define do_system_rom_verify_check true //this verifies that data is copied using
#include "MLX90640_API.h"
#include "MLX90640_I2C_Driver.h"
#include "factoryCalData.h" //we use this to verify rom values
#include "Z_MemManagment.h"//lets us know what mem management we are using

#include "i2c_Address.h"
#define wireClockSpeed 400000 //we define here easier to set from here.800000 is fastest will work on arduino, 2mhz seems to work on teensy. not sure
#define SerialBaudRate 1000000 //this is speed of serial protocol. be sure enough time is set to allow messages to be sent thru before using i2c
#define continuousmode true//true is default when sensor is bought, however we want step mode. this is checked by the code and only written to if it is different than value here. it is here for experimentation.
#define hzMode 4//0=0.5hz,1=1hz,2=2hz,3=4hz,4=8hz,5=16hz,6=32hz,7=64hz 
#define adSensorResolution 3 //0=16bit,it 1=17bit, 2=18bit, 3=19b
#define MLX90640_mirror true //this flips direction of sensor in case used in camera mode
#define pixelmodeTrueTestModeFalse true//true outputs display image of sorts to terminal, false outputs raw sensor data in deg C
#define patternModecheckerInsteadOfscanline true //this is type of scan method
//thes are just some commands that we make sure are stored in flash instead of ram.
void serial_mxl90460 (){Serial.print(F("MLX90640"));} //we use the defined word as a reusabel variable
void serial_reset_message(){Serial.println(F("making changes to firmware. if nothing happens in 5-10 seconds reboot chip"));delay(5000);return;}
void serial_set_same_as_flash_msg(){Serial.println(F(" is set the same as #define settings in flash"));}
void serial_HZ_msg(){Serial.print(F(" HZ"));}
void serial_Data_should_now_be_changes_msg(){Serial.println(F("Data should now be changed."));}
void setting_is_different(){Serial.println(F("setting is different in firmware. we are going to write data now."));}
void Device_is_in_mode_of(){Serial.println(F(" Device is in mode of "));}
void reset_and_msg(){serial_Data_should_now_be_changes_msg();delay(200);serial_reset_message();delay(2000);}  //call reset
byte status =0;//we use for status
bool startupComplete= true;//this is used to loop or not loop

//paramsMLX90640 mlx90640;//we no longer need this as pointer and me structure changed
#if DoubleResolution ==true
float tempCache;//we use to up the resolution for show along x axis
float temp1;
#endif
uint16_t wordstore[1];//used to manupuated data
void setup()
{
  Wire.begin();
  Wire.setClock(wireClockSpeed); //Increase I2C clock speed to 400kHz

  Serial.begin(SerialBaudRate);
  while (!Serial); //Wait for user to open terminal
   for (byte i=0;i<64;i++){Serial.println();}//we clear screen in terminal in case reset.
  serial_mxl90460(); Serial.println(F(" IR Array Example Modified and changed by James Villeneuve"));
  Serial.println(F("BE PREPARED TO RESET CHIP EVERY TIME A FLASH PARAMITER IS CHANGED SUCH AS AD SENSATIVITY OR HZ rate, for example!"));
 delay(1500);//make sure serial done
//old check and needs to get ram values
  //Get device parameters - We only have to do this once

  //Once params are extracted, we can release eeMLX90640 array
  if (isConnected() == false)
  {
    Serial.println(F("MLX90640 not detected at default I2C address. Please check wiring. Freezing."));
   // while (1);
  }
 serial_mxl90460(); Serial.println(F(" online!"));//we use print routine for word mxl90460, and then add word online!
  delay(1500);//make sure serial done
//here we run system check and compare to thermopile

while (startupComplete ){//we set to false if it passes
  for (byte i=0;i<64;i++){Serial.println();}//we clear screen in terminal in case reset.
 testFlashVsThermopileFlashAndCheckOtherSettings();
 delay(500);//make sure serial done
}
//  int status;
 // uint16_t eeMLX90640[832];
//  status = MLX90640_DumpEE(MLX90640_address, eeMLX90640);//we no longer need this as data is stored in on board epprom!
 
  status = MLX90640_ExtractParameters();//this will go away soon!!
 Serial.println(F("done old extraction. this will eventually go away"));

Serial.println("paramiters data");





}
void printBits(byte myByte){
 for(byte mask = 0x80; mask; mask >>= 1){
   if(mask  & myByte)
       Serial.print('1');
   else
       Serial.print('0');
 }
}
void loop()
{Serial.println(F("MainLoop Init ok"));
 //reset_ram();//this resets all ram values before we store them
#if NEW_METHOD == false
  for (byte x = 0 ; x < 2 ; x++){ //Read both subpages and store all in ram
#endif    
#if NEW_METHOD == true

  for (byte x = 0 ; x < 1 ; x++){ //Read 1 time as all data is stored on ram of sensor. so it can all be read at once
#endif 
  
 
    int status = MLX90640_GetFrameData(MLX90640_address);

    if (status < 0)
    {
      Serial.print(F("GetFrame Error: "));
      Serial.println(status);
    }
InitSensor();//same code as old just managed in mlx90640_api now. we do before first run, and most likely before every run?

Serial.println(F("getting raw To values"));
#if NEW_METHOD == false //this means using old method  
    MLX90640_CalculateTo();
#endif//new method does not need everything all at once! we get it from mem cal
  }
 
  delay(200);
  for (int y = 0 ; y< 60 ; y++){//we scroll for new data
 Serial.print("\r\n");
  }
  Serial.println("");
 for (int y = 0 ; y< 24 ; y++){
  #if DoubleResolution ==true //this is where we double y data, buy averaging lines
//here is where we show all thermopile cells as individual temp cells in deg C   
 for (int ydouble = 0 ; ydouble <2 ; ydouble++){//this doubles y resolution
#endif
  for (int x = 0 ; x < 32 ; x++)
  {
//here is where we show a sort of image to serial terminal display    
#if pixelmodeTrueTestModeFalse ==  true
// Serial.print("|");
   // Serial.print(x);
   // Serial.print(": ");
   float temp=Readmlx90640To(x+y*32);
#if DoubleResolution ==true
   if (ydouble==0){//this is raw data 
   // temp=(Readmlx90640To(x+y*32));//this is redundant and causes more reads than needed
    temp1=(tempCache+temp)*0.5;//we use old value 
   }

      if (ydouble==1){//this we average ram between data for doubling y res
        if (y<31){ temp=(Readmlx90640To(x+y*32)+Readmlx90640To(x+(y+1)*32))*0.5;}
        else{temp=(Readmlx90640To(x+y*32));}//we cant go above 768. 
          temp1=(tempCache+temp)*0.5;//we use old value 
   }
   //first read 
    if (temp1<26){Serial.print(". ");}
    if ((temp1>26) & (temp1<29)){Serial.print(".-");}
   if ((temp1>29) & (temp1<30)){Serial.print(".+");}
   if ((temp1>30) & (temp1<31)){Serial.print(".X");}
   if ((temp1>31) & (temp1<32)){Serial.print(".O");}
   if ((temp1>32) & (temp1<33)){Serial.print(".0");}
   if ((temp1>33) & (temp1<35)){Serial.print(".#");}
   if (temp1>35){Serial.print(".@");}
    tempCache=temp;//we use to average out res data
//second read
#endif
   if (temp<26){Serial.print(". ");}
    if ((temp>26) & (temp<29)){Serial.print(".-");}
   if ((temp>29) & (temp<30)){Serial.print(".+");}
   if ((temp>30) & (temp<31)){Serial.print(".X");}
   if ((temp>31) & (temp<32)){Serial.print(".O");}
   if ((temp>32) & (temp<33)){Serial.print(".0");}
   if ((temp>33) & (temp<35)){Serial.print(".#");}
   if (temp>35){Serial.print(".@");}

    //Serial.print("C");
   // Serial.println();
  
#endif









#if pixelmodeTrueTestModeFalse !=  true
 Serial.print("|");
   // Serial.print(x);
   // Serial.print(": ");
   Serial.print(Readmlx90640To(x+y*32));

    Serial.print("C");

#endif
  }
   Serial.print(":");
  
 Serial.println();
 // delay(1000);
   #if DoubleResolution ==true //this is where we double y data, buy averaging lines
//here is where we show all thermopile cells as individual temp cells in deg C   
 };//this ends loop if y res doubled
#endif
}


}

//Returns true if the MLX90640 is detected on the I2C bus
boolean isConnected()
{
  Wire.beginTransmission((uint8_t)MLX90640_address);
  if (Wire.endTransmission() != 0)
    return (false); //Sensor did not ACK
  return (true);
}

//we have testing to verify settings and other data are there
void  testFlashVsThermopileFlashAndCheckOtherSettings()
{
 Serial.println();
 Serial.println(F("This shows a single image from entire sensor. since we have the start count delay, the memory will already be filled with sensor data"));
 Serial.println();
 delay(200);//serial data to clear
//uint16_t startvalue[8];//array
#if patternModecheckerInsteadOfscanline == true
MLX90640_SetChessMode(MLX90640_address);//set chess pattern read
#else
MLX90640_SetInterleavedMode(MLX90640_address);//set scan line mode read
#endif
 delay(200);//serial data to clear
//float ta ;//we use 4 corner sensors to determin chip temp at start up, then we take lowest pixel on screen and use that.

//we check flash values
    for (int i=0;i<832;i++){//gets 0x2400 (9216) to 0x273f (10047)
MLX90640_I2CRead(MLX90640_address, 0x2400+i, 1,wordstore);
delay(1);
Serial.print(wordstore[0]);
Serial.print((", "));delay(1);
if ((i&15)==15){Serial.print(F(" register location:"));Serial.print(i+0x2400-15,HEX);Serial.print("-");Serial.println(i+0x2400,HEX);}//every 16 data make a line
delay(2);
    }//next
    Serial.println(F("data above is from epprom. it will be used later on for calibration"));
Serial.println(F("We will now test the eeprom of the sensor with the flash data. if it does not match it will error but keep working using the included settings."));

    for (int i=0;i<832;i++){
MLX90640_I2CRead(MLX90640_address, 0x2400+i, 1,wordstore);
if (wordstore[0] !=pgm_read_word_near(factoryCalData+i)){Serial.print(F("Error at (in HEXMODE):"));
Serial.print(0x2400+i,HEX);Serial.print("sensor:");Serial.print(wordstore[0],HEX);
Serial.print("PROGMEMVALUE:");Serial.println(pgm_read_word_near(factoryCalData+i),HEX);delay(2);//allows serial to finish before i2c read
}
    }//next
Serial.print(F("Flash memory check complete. if errors troubleshoot and fix for best results"));

byte  testMode=MLX90640_GetCurMode(MLX90640_address);
  Serial.print(F("Device is in "));
  if (testMode==0){Serial.print(F("interleaved"));}
  if (testMode==1){Serial.print(F("checkered"));}
  Serial.println(F(" pattern mode"));
 testMode=MLX90640_GetCurResolution(MLX90640_address);//we get resolution of ad converter 16-19 bit resolution
Serial.print(F("Device ad resoltuion set to ....")); 
 Serial.print(16+testMode);Serial.println(F(".... bit"));
if (adSensorResolution==testMode){Serial.println(F("analog resolution")); serial_set_same_as_flash_msg();}
else{//if resolution is different we change resolution
MLX90640_I2CRead(MLX90640_address,  0x800D,  1,wordstore);//we read control register
delay(1);  
wordstore[0]=wordstore[0]&62463;//1111 0011 1111 1111 //we clear bits so we can just add changes
wordstore[0]=wordstore[0]+1024*adSensorResolution;  
MLX90640_I2CWrite(MLX90640_address, 0x800D, wordstore[0]);//we write modified data back to register and make it single mode
delay(5);// we add delay soit can finish write
reset_and_msg();
}
 testMode=MLX90640_GetRefreshRate(MLX90640_address);//get refresh in hz
Serial.print(F("HZ is set to "));
if (testMode==0){Serial.print(F("0.5"));}
if (testMode>0){
if (testMode==1){Serial.print(F("1"));}
if (testMode==2){Serial.print(F("2"));}
if (testMode==3){Serial.print(F("4"));}
if (testMode==4){Serial.print(F("8"));}
if (testMode==5){Serial.print(F("16"));}
if (testMode==6){Serial.print(F("32"));}
if (testMode==7){Serial.print(F("64"));}
Serial.print(F(".0"));
}
serial_HZ_msg();
if (hzMode==testMode){serial_HZ_msg(); serial_set_same_as_flash_msg();}
else{//this means setting is different
serial_HZ_msg();setting_is_different();

MLX90640_I2CRead(MLX90640_address,  0x800D,  1,wordstore);//we read control register
delay(1);
wordstore[0] = wordstore[0]&64639 ;//we make 1111110001111111 the zeros are the control registers for refresh rate. we set zero here for ease of or of 1's later
wordstore[0]+=128*hzMode;
//ok we have data changes to matcht the new time change, now we need to write it
MLX90640_I2CWrite(MLX90640_address, 0x800D,wordstore[0]);//we write modified data back to register and make it single mode
delay(5);// we add delay soit can finish write
reset_and_msg();
}//end of else
delay(200);
Serial.print(F("register:"));
delay(10);
MLX90640_I2CRead(MLX90640_address,  0x8000,  1,wordstore); 
delay(10);  
MLX90640_I2CRead(MLX90640_address,  0x800D,  1,wordstore);//we get status of step mode. default it is not in step mode, but we only want to write to eeprom if it is not in step mode
delay(5);//we wait ample time for i2c to complete
printBits(highByte(wordstore[0]));
printBits(lowByte(wordstore[0]));
if ((wordstore[0]&2) ==0){Device_is_in_mode_of();Serial.println(F("'continueous'"));
if (!continuousmode){//here is where we change it if it is set different
setting_is_different();
delay(5);//we wait ample time for i2c to complete

wordstore[0]=wordstore[0]|2;//we make this and 0000000000000010; bit2 is now high this is needed at 800D HEX location
delay(5);
Serial.println("new values:");
printBits(highByte(wordstore[0]));
printBits(lowByte(wordstore[0]));
Serial.println();
delay(5000);
 MLX90640_I2CWrite(MLX90640_address, 0x800D, wordstore[0]);//we write modified data back to register and make it single mode
  delay(5);//we wait ample time for i2c to complete
 serial_Data_should_now_be_changes_msg();
 delay(5);//we wait ample time for i2c to complete
}
}

else
{Device_is_in_mode_of();Serial.println(F("'step'"));
if (continuousmode){//here is where we change it if it is set different
setting_is_different();
delay(5);//we wait ample time for i2c to complete
wordstore[0]=wordstore[0]&65533;//we make this and 1111111111111101; bit2 is now low this is needed at 800D HEX location
delay(5);
Serial.println(F("new values:"));
printBits(highByte(wordstore[0]));
printBits(lowByte(wordstore[0]));
Serial.println();
delay(5000);
 MLX90640_I2CWrite(MLX90640_address, 0x800D, wordstore[0]);//we write modified data back to register and make it single mode
  delay(5);//we wait ample time for i2c to complete
 serial_Data_should_now_be_changes_msg();
 delay(5);//we wait ample time for i2c to complete
}
}
delay(10);
Serial.print(F("mirrored mode is :"));
#if MLX90640_mirror == false
Serial.println(F("off"));
#else
Serial.println(F("on"));
#endif
//this prints out register data
printBits(highByte(wordstore[0]));
printBits(lowByte(wordstore[0]));
Serial.print(F(". This means that page is on:"));
Serial.println(1&wordstore[0]);//it can be zero or 1
 Serial.print(F("seconds before startup:"));
 for (byte i=7;i>0;i--){Serial.print(i);Serial.print(F(".."));delay(500);}//7 seconds to read message before start
/*
//we force two scans so we get preuse data
if (!continuousmode){//if we are in step mode we need to tell sensor to scan page
MLX90640_I2CRead(MLX90640_address,  0x8000,  1,worddata);//we read sensor 
delay(600);
worddata[0]=  worddata[0]|32;//we set 000000000100000 wich is register that starts measurement
MLX90640_I2CWrite(MLX90640_address, 0x8000, worddata[0]);//we write modified values
delay(600);
worddata[0]=  worddata[0]|32;//we set 000000000100000 wich is register that starts measurement
MLX90640_I2CWrite(MLX90640_address, 0x8000, worddata[0]);//we write modified values


}
*/
Serial.print(F("completed main routines. ready for loops"));

startupComplete=false;//we completed, so we no longer need to do this again    





}
