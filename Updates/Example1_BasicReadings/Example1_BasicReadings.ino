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

//change  Replace_detailed_calc_with_image_data true to show faster to output image data without To calculations. it is in Z_memManagment.

#include <Wire.h>

//here are the terminal output settings
#define pixelmodeTrueTestModeFalse true//true outputs display image of sorts to terminal, false outputs raw sensor data in deg C
#define DoubleResolution true//this doubels resolution output to 64x48 to make seeing objects a little easier
#define do_system_rom_verify_check true //this verifies that data is copied using, but can be anoying for several checks after rom dump is verified. if erorrs they should still show.
#define hzMode 3//0=0.5hz,1=1hz,2=2hz,3=4hz,4=8hz,5=16hz,6=32hz,7=64hz 
#define adSensorResolution 3 //0=16bit,it 1=17bit, 2=18bit, 3=19b
#define MLX90640_mirror false //this flips direction of sensor in case used in camera mode
#define troubleshoot_optimize false //if true frames show slower and output processing time per frame
#define serialbuffermode false //this is to test a serial of 128 bytes
#include "MLX90640_API.h"
#include "MLX90640_I2C_Driver.h"
#include "factoryCalData.h" //we use this to verify rom values
#include "Z_MemManagment.h"//lets us know what mem management we are using

#include "i2c_Address.h"
#include "ZZZ_doubleResolution.h" //this is a low memory resolution doubler. it relies on fast ram access to data.
#define wireClockSpeed 800000 //we define here easier to set from here.800000 is fastest will work on arduino
#define SerialBaudRate 1000000  //this is speed of serial protocol. be sure enough time is set to allow messages to be sent thru before using i2c
#define continuousmode true//true is default when sensor is bought, however we want step mode. this is checked by the code and only written to if it is different than value here. it is here for experimentation.

#define patternModecheckerInsteadOfscanline true //this is type of scan method
#if serialbuffermode == true
char SerialBUffer[64];// this is serial buffer so we can load up the buffer routine
uint8_t SBC =0;//this is Serial buffer count
#endif
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
bool startupComplete=true;
//paramsMLX90640 mlx90640;//we no longer need this as pointer and me structure changed

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
byte testtoggle=0;
#if troubleshoot_optimize == true 
uint32_t  delaycount;//used to troubleshoot math timing
float results_testing; // for test results calc
#endif
void loop()
{Serial.println(F("MainLoop Init ok"));
 //reset_ram();//this resets all ram values before we store them
#if NEW_METHOD == false
  for (byte x = 0 ; x < 1 ; x++){ //Read both subpages and store all in ram
#endif    
#if NEW_METHOD == true

  for (byte x = 0 ; x < 1 ; x++){ //Read sub pages data and store only refference data. we keep ram data
#endif 
  
 
    int status = MLX90640_GetFrameData(MLX90640_address);//this is capture

    if (status < 0)
    {
      Serial.print(F("GetFrame Error: "));
      Serial.println(status);
    }
InitSensor();//same code as old just managed in mlx90640_api now. we do before first run, and most likely before every run?

Serial.println(F("getting raw To values"));
#if NEW_METHOD == false //this means using old method  
    MLX90640_CalculateTo();//this is data pull from 
#endif//new method does not need everything all at once! we get it from mem cal
  }
  #if troubleshoot_optimize == true //this is just used to test how long a frame takes to process
Serial.print(F("time it takers per screen"));Serial.println(delaycount);

delay(1000);//this delay is only so we can see time it takes frame to process in troubleshooting only
#endif
delay(100);

delay(100);
  for (int y = 0 ; y< 60 ; y++){//we scroll for new data
 Serial.print("\r\n");
  }
  Serial.println("");

  
 
#if troubleshoot_optimize == true
delaycount=micros();//we start timer for troubleshooting performance. normally this is not used
#endif
  #if DoubleResolution !=true //this means we run at sensor res

 for (uint16_t  y = 0 ; y< 24 ; y++){for (uint16_t x = 0 ; x < 32 ; x++){//this needs to be 16bit because the number it processes is above 255
 #else 
  for (uint8_t y = 0 ; y< 48 ; y++){for (uint8_t x = 0 ; x < 64 ; x++){
    #endif

//here is where we show a sort of image to serial terminal display    
#if pixelmodeTrueTestModeFalse ==  true
// Serial.print("|");
   // Serial.print(x);
   // Serial.print(": ");
    #if DoubleResolution !=true //32=100000
     uint8_t temp=Readmlx90640To(x+(y<<5));//since y is unsigned byte we can shift bits for multiply
#else
   uint8_t temp=DoubleResolutionValue(x,y);//we return double we read x,y directly not mem location 
#endif

   #if serialbuffermode !=true
   if (temp< 26) {Serial.print(F(". "));}
   if ((temp>25) & (temp<29)) {Serial.print(F(".-"));}
   if ((temp>28) & (temp<30)) {Serial.print(F(".+"));}
   if ((temp>29) & (temp<31)) {Serial.print(F(".X"));}
   if ((temp>30) & (temp<32)) {Serial.print(F(".O"));}
   if ((temp>31) & (temp<33)) {Serial.print(F(".0"));}
   if ((temp>32) & (temp<36)) {Serial.print(F(".#"));} 
   if (temp> 35) {Serial.print(F(".@"));}
 
   #else
   if (temp<26){SerialBUffer[SBC]=32;SBC++;}
   if ((temp>25) & (temp<29)){SerialBUffer[SBC]=45;SBC++;}
   if ((temp>28) & (temp<30)){SerialBUffer[SBC]=43;SBC++;}
   if ((temp>29) & (temp<31)){SerialBUffer[SBC]=88;SBC++;}
   if ((temp>30) & (temp<32)){SerialBUffer[SBC]=79;SBC++;}
   if ((temp>31) & (temp<33)){SerialBUffer[SBC]=48;SBC++;}
   if ((temp>32) & (temp<36)){SerialBUffer[SBC]=35;SBC++;}
   if (temp>35){SerialBUffer[SBC]=64;SBC++;}
   #endif
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
  }//end of line
  #if serialbuffermode !=true
   Serial.print(":");
  
 Serial.println();
#else

while (SBC !=0){//while instead of for loop has shorter jmp routine
Serial.write(46);
Serial.write(SerialBUffer[64-SBC]);
SBC--;//we get it in a while loop and return it at zero!
}

Serial.println(":");//we do this beacuse it is the same as write manually.





 #endif
 // delay(1000);

}
#if troubleshoot_optimize == true
delaycount=micros()-delaycount;//we stop timer. this normally is not used, only when we are trying to optimize
Serial.print("alphaScale:");Serial.println(alphaScale_testing_results());
Serial.print("kvScale:");Serial.println(kvScale_testing_results());
Serial.print("ktaScale1:");Serial.println(ktaScale1_testing_results());
Serial.print("gain:");Serial.println(gainEE_testing_results());
Serial.print("last_pixelTemp=");Serial.println(DoubleResolutionValue(48,63));
#endif
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
 # if do_system_rom_verify_check == true  //this verifies rom dump.
    for (int i=0;i<832;i++){//gets 0x2400 (9216) to 0x273f (10047)
MLX90640_I2CRead(MLX90640_address, 0x2400+i, 1,wordstore);
delay(1);
Serial.print(wordstore[0]);
Serial.print((", "));delay(1);
if ((i&15)==15){Serial.print(F(" register location:"));Serial.print(i+0x2400-15,HEX);Serial.print("-");Serial.println(i+0x2400,HEX);}//every 16 data make a line
delay(2);
    }//next
    Serial.println(F("data above is from epprom. it will be used later on for calibration"));
#endif
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

//we ad this to resolve clock being to fast for i2c on arduino
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__) || defined(__AVR_ATmega1284__) || defined(__AVR_ATmega1284P__) || defined(__AVR_ATmega644__) || defined(__AVR_ATmega644A__) || defined(__AVR_ATmega644P__) || defined(__AVR_ATmega644PA__) ||defined(__AVR_ATmega328P__) 
#if wireClockSpeed > 800000 //above this will not work in testing so we want to flag it
#define ErrorObj("this processor can not reliably do over 800khz, change wireClockSpeed in sketch to no greater than 800000");
#endif
#endif
#if wireClockSpeed > 1000000
#define ErrorObj("the MLX90640 sensor only is in spec to 1mhz. look at wireClockSPeed and make changes. going over 1mhz can damage sensor");
#endif 
