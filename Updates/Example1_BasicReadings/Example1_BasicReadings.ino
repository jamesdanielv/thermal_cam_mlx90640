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
// some of the setup functions have been removed or moved to the void InitSensor() in "MLX90640_API.h"

#include <Wire.h>

#define DoubleResolution true //this doubels resolution output to 64x48 to make seeing objects a little easier
#define do_system_rom_verify_check true //this verifies that data is copied using
#include "MLX90640_API.h"
#include "MLX90640_I2C_Driver.h"
#include "factoryCalData.h" //we use this to verify rom values
#include "Z_MemManagment.h"//lets us know what mem management we are using

#include "i2c_Address.h"
#define wireClockSpeed 800000 //we define here easier to set from here.800000 is fastest will work on arduino, 2mhz seems to work on teensy. not sure
#define SerialBaudRate 1000000 //this is speed of serial protocol. be sure enough time is set to allow messages to be sent thru before using i2c
#define continuousmode true//true is default when sensor is bought, however we want step mode. this is checked by the code and only written to if it is different than value here. it is here for experimentation.
#define hzMode 4//0=0.5hz,1=1hz,2=2hz,3=4hz,4=8hz,5=16hz,6=32hz,7=64hz 
#define adSensorResolution 3 //0=16bit,it 1=17bit, 2=18bit, 3=19b
#define MLX90640_mirror true //this flips direction of sensor in case used in camera mode
#define pixelmodeTrueTestModeFalse true//true outputs display image of sorts to terminal, false outputs raw sensor data in deg C/**
 * @copyright (C) 2017 Melexis N.V.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * files have been modified by james villeneuve
 */

#include "Z_MemManagment.h"//tells us what mem managment we are using, old or new more mem efficient
#include "factoryCalData.h"
#include "MLX90640_I2C_Driver.h"
#include "MLX90640_API.h"
#include <math.h> //no longer needed if using q_sqrt saves 5k of flash rom letting compiler decide if it is used or not. 
#include "i2c_Address.h" 
#include "memCache.h"
#include "pixelframemem.h"
#include "pixelCache.h"
void ExtractVDDParameters( );//we are removing the need for this
void ExtractPTATParameters();
void ExtractGainParameters();
void ExtractTgcParameters();
void ExtractResolutionParameters();
void ExtractKsTaParameters( );
void ExtractKsToParameters( );
void ExtractAlphaParameters( );
void ExtractOffsetParameters( );//we no longer need this as this is done real time now
void ExtractKtaPixelParameters( );
void ExtractKvPixelParameters();
void ExtractCPParameters();
void ExtractCILCParameters();
int ExtractDeviatingPixels( );
int CheckAdjacentPixels(uint16_t pix1, uint16_t pix2);
int CheckEEPROMValid(uint16_t *eeData);

uint16_t linecache;//we store current line on
//---------------- simple set

uint16_t eeDataGetStoredInLocalEPROM(uint16_t value){

return pgm_read_word_near(factoryCalData+ value);

  
}
uint16_t RamGetStoredInLocal(uint16_t value){
//400+
 MLX90640_I2CRead(MLX90640_address, 1024+value, 1,worddata);
return worddata[0];
  
}

uint16_t RamGetStoredInLocalManyatonce(uint16_t value){
//400+
// temp=value;
uint16_t temp=value & 31;//we make it so only lower bits are known
if (linecache !=(value &65505)) {//if we are on a different line

 MLX90640_I2CRead(MLX90640_address, 1024+value, 32,worddata);
linecache =value &65505;//we set line
}
return worddata[temp];
  
}

float Readmlx90640To(uint16_t value){
#if NEW_METHOD == true 
return  MLX90640_CalculateToRawPerPixel(value);
#endif
#if NEW_METHOD == false
return mlx90640To[value];//old method
#endif
}

void InitSensor(){
    vdd = MLX90640_GetVdd();
    Ta = MLX90640_GetTa();

    tr = Ta - TA_SHIFT; //Reflected temperature based on the sensor ambient temperature
     emissivity = 0.95;
}

float SimplePow(float base, uint8_t exponent)
{//we create or own low memory multiplier

 float tempbase=base;if (exponent >0){for (uint8_t i=1;i< exponent;i++){////we have base number,we start at 1 not 0, because 1 is already done
  tempbase=tempbase*base;}}else{tempbase=1;}//we multiply unless exponent is 0 then result is 1
 return tempbase;//we return result
}
/* not using this method officially yet. so it will be removed to avoid cross boundery errors (we are hacking a cast to a float from a long.)
float Q_rsqrt( float number ) //a good enough square root method.
{//https://en.wikipedia.org/wiki/Fast_inverse_square_root
  long i;float x2;float y; float threehalfs = 1.5F;
  x2 = number * 0.5F;
  y  = number;
  i  = * ( long * ) &y;                       // evil floating point bit level hacking
  i  = 0x5f3759df - ( i >> 1 );               // ######edited language###
  y  = * ( float * ) &i;
  y  = y * ( threehalfs - ( x2 * y * y ) );   // 1st iteration
  y  = y * ( threehalfs - ( x2 * y * y ) );   // 2nd iteration, this can be removed
  return y;
}*/
//**************************************************************************************
#if NEW_METHOD == true   //these are low mem methods of getting same data. whodda thought?
 

int16_t ExtractOffsetParametersRawPerPixel(uint16_t value )
{//we get row and collumb data and process from there
  uint8_t col= (value&31) ;//we take bottom 5 bits and add as we use 1-32
  uint8_t row=(value&65504)>>5;;//this should have upper bits only
  
    //int occRow[24];
    //int occColumn[32];
    //we only need to store one word each as conversion done on the fly
    int occRow;
    int occColumn;
   uint8_t p = 0;
    int16_t offsetRef;
   uint8_t occRowScale;
    uint8_t occColumnScale;
    uint8_t occRemScale;
    

    occRemScale = (eeDataGetStoredInLocalEPROM(16) & 0x000F);
    occColumnScale = (eeDataGetStoredInLocalEPROM(16) & 0x00F0) >> 4;
    occRowScale = (eeDataGetStoredInLocalEPROM(16) & 0x0F00) >> 8;
    offsetRef = eeDataGetStoredInLocalEPROM(17);//11hex
    if (offsetRef > 32767)
    {
        offsetRef = offsetRef - 65536;
    }
//we have raw number for offset now
p=row&3;//we have 0-3 for data bits
uint8_t i=row>>2;
   if (p==0) {       occRow = (eeDataGetStoredInLocalEPROM(18 +i) & 0x000F);}
   if (p==1) {       occRow = (eeDataGetStoredInLocalEPROM(18 +i) & 0x00F0) >> 4;}
   if (p==2) {       occRow = (eeDataGetStoredInLocalEPROM(18 +i) & 0x0F00) >> 8;}
   if (p==3) {       occRow = (eeDataGetStoredInLocalEPROM(18 +i) & 0xF000) >> 12;}
   

        if (occRow > 7)
        {
            occRow = occRow - 16;
        }
    
  p=col&3;//we have 0-3 for data bits  
  i=col>>2;
       
  if (p==0) {       occColumn = (eeDataGetStoredInLocalEPROM(24 +i) & 0x000F);}
  if (p==1) {       occColumn = (eeDataGetStoredInLocalEPROM(24 +i)& 0x00F0) >> 4;}
  if (p==2) {       occColumn = (eeDataGetStoredInLocalEPROM(24 +i) & 0x0F00) >> 8;}
  if (p==3) {       occColumn = (eeDataGetStoredInLocalEPROM(24 +i) & 0xF000) >> 12;}
   
 
        if (occColumn > 7)
        {
            occColumn = occColumn- 16;
        }
   

 //   for(int i = 0; i < 24; i++)
 //   {
 //       for(int j = 0; j < 32; j ++)
  //      {
   //         p = 32 * i +j;//this can be directly value
            ;//we have direct cell number

   
           // offset[p] = (eeDataGetStoredInLocalEPROM(64 +p) & 0xFC00) >> 10;

            int16_t temp=(eeDataGetStoredInLocalEPROM(64 +value) & 0xFC00) >> 10;
            if (temp > 31)
            {
               temp = temp- 64;
            }
           temp =temp*(1 << occRemScale);
           temp = (offsetRef + (occRow << occRowScale) + (occColumn << occColumnScale) + temp);
     //   }
  //  }
  return temp;
}

float ExtractAlphaParametersRawPerPixel(uint16_t value )
{//we get row and collumb data and process from there
  uint8_t col= (value&31) ;//we take bottom 5 bits and add as we use 1-32
  uint8_t row=(value&65504)>>5;;//this should have upper bits only
  
    int accRow;
    int accColumn;
    int p = 0;
    int alphaRef;
    uint8_t alphaScale;
    uint8_t accRowScale;
    uint8_t accColumnScale;
    uint8_t accRemScale;
    

    accRemScale = eeDataGetStoredInLocalEPROM(32) & 0x000F;
    accColumnScale = (eeDataGetStoredInLocalEPROM(32) & 0x00F0) >> 4;
    accRowScale = (eeDataGetStoredInLocalEPROM(32) & 0x0F00) >> 8;
    alphaScale = ((eeDataGetStoredInLocalEPROM(32) & 0xF000) >> 12) + 30;
    alphaRef = eeDataGetStoredInLocalEPROM(33);
    
p=row&3;//we have 0-3 for data bits
uint8_t i=row>>2;
   if (p==0) {       accRow = (eeDataGetStoredInLocalEPROM(34 +i) & 0x000F);}
   if (p==1) {       accRow = (eeDataGetStoredInLocalEPROM(34 +i) & 0x00F0) >> 4;}
   if (p==2) {       accRow = (eeDataGetStoredInLocalEPROM(34 +i) & 0x0F00) >> 8;}
   if (p==3) {       accRow = (eeDataGetStoredInLocalEPROM(34 +i) & 0xF000) >> 12;}


        if (accRow > 7)
        {
            accRow = accRow - 16;
        }

  
     p=col&3;//we have 0-3 for data bits  
  i=col>>2;
       
  if (p==0) {       accColumn = (eeDataGetStoredInLocalEPROM(40 +i) & 0x000F);}
  if (p==1) {       accColumn = (eeDataGetStoredInLocalEPROM(40 +i)& 0x00F0) >> 4;}
  if (p==2) {       accColumn = (eeDataGetStoredInLocalEPROM(40 +i) & 0x0F00) >> 8;}
  if (p==3) {       accColumn = (eeDataGetStoredInLocalEPROM(40 +i) & 0xF000) >> 12;}
   
 
    
        if (accColumn > 7)
        {
            accColumn =accColumn- 16;
        }

  
            
          //  alpha[p] = (eeDataGetStoredInLocalEPROM(64+ p) & 0x03F0) >> 4;
          float temp=(eeDataGetStoredInLocalEPROM(64 +value) & 0x03F0) >> 4;
            if (temp > 31)
            {
                temp =temp - 64;
            }
             temp=  temp*(1 << accRemScale);
             temp= (alphaRef + (accRow << accRowScale) + (accColumn << accColumnScale) + temp);
            temp= temp / SimplePow(2,(double)alphaScale);
            return temp;
}

float ExtractKtaPixelParametersRawPerPixel(uint16_t value )
{
    int p = 0;
    int8_t KtaRC[4];
    int8_t KtaRoCo;
    int8_t KtaRoCe;
    int8_t KtaReCo;
    int8_t KtaReCe;
    uint8_t ktaScale1;
    uint8_t ktaScale2;
    uint8_t split;

    KtaRoCo = (eeDataGetStoredInLocalEPROM(54) & 0xFF00) >> 8;
    if (KtaRoCo > 127)
    {
        KtaRoCo = KtaRoCo - 256;
    }
    KtaRC[0] = KtaRoCo;
    
    KtaReCo = (eeDataGetStoredInLocalEPROM(54)  & 0x00FF);
    if (KtaReCo > 127)
    {
        KtaReCo = KtaReCo - 256;
    }
    KtaRC[2] = KtaReCo;
      
    KtaRoCe = (eeDataGetStoredInLocalEPROM(55)  & 0xFF00) >> 8;
    if (KtaRoCe > 127)
    {
        KtaRoCe = KtaRoCe - 256;
    }
    KtaRC[1] = KtaRoCe;
      
    KtaReCe = (eeDataGetStoredInLocalEPROM(55)  & 0x00FF);
    if (KtaReCe > 127)
    {
        KtaReCe = KtaReCe - 256;
    }
    KtaRC[3] = KtaReCe;
  
    ktaScale1 = ((eeDataGetStoredInLocalEPROM(56)  & 0x00F0) >> 4) + 8;
    ktaScale2 = (eeDataGetStoredInLocalEPROM(56)  & 0x000F);

  //  for(int i = 0; i < 24; i++)
  //  {
  //      for(int j = 0; j < 32; j ++)
   //     {
            p = value;//we are getting for a single pixel
            split = 2*(((p>>5)&32768) - ((p>>6)&32768)*2) + p%2;//since number is an int not a uint16_t we have to fix the change of the sign
            float temp = (eeDataGetStoredInLocalEPROM(64+p)  & 0x000E) >> 1;
            if (temp > 3)
            {
                temp = temp - 8;
            }
            temp = temp * (1 << ktaScale2);
           temp = KtaRC[split] + temp;
            temp= temp / SimplePow(2,(double)ktaScale1);
            return temp;
   //     }
  //  }
}
float ExtractKvPixelParametersRawPerPixel(uint16_t value )
{
    int p = 0;
    int8_t KvT[4];
    int8_t KvRoCo;
    int8_t KvRoCe;
    int8_t KvReCo;
    int8_t KvReCe;
    uint8_t kvScale;
    uint8_t split;

    KvRoCo = (eeDataGetStoredInLocalEPROM(52)  & 0xF000) >> 12;
    if (KvRoCo > 7)
    {
        KvRoCo = KvRoCo - 16;
    }
    KvT[0] = KvRoCo;
    
    KvReCo = (eeDataGetStoredInLocalEPROM(52)  & 0x0F00) >> 8;
    if (KvReCo > 7)
    {
        KvReCo = KvReCo - 16;
    }
    KvT[2] = KvReCo;
      
    KvRoCe = (eeDataGetStoredInLocalEPROM(52)  & 0x00F0) >> 4;
    if (KvRoCe > 7)
    {
        KvRoCe = KvRoCe - 16;
    }
    KvT[1] = KvRoCe;
      
    KvReCe = (eeDataGetStoredInLocalEPROM(52) & 0x000F);
    if (KvReCe > 7)
    {
        KvReCe = KvReCe - 16;
    }
    KvT[3] = KvReCe;
  
    kvScale = (eeDataGetStoredInLocalEPROM(56) & 0x0F00) >> 8;


 //   for(int i = 0; i < 24; i++)
 //   {
 //       for(int j = 0; j < 32; j ++)
  //      {
            p =value;
            split = 2*(p/32 - (p/64)*2) + p%2;
            float temp = KvT[split];
             temp =  temp /SimplePow(2,(double)kvScale);
 //       }
 //   }
 return temp;
}

float MLX90640_CalculateToRawPerPixel(uint16_t pixelNumber )
{//this ligher code requires footwork per pixel
if (pixelNumber == 0){//we only do this on start of page
    
    //sub_calc_subPage = mlx90640Frame[833];//we dont care about page data individuallly that much anymore
    sub_calc_vdd = MLX90640_GetVdd();
    sub_calc_ta = MLX90640_GetTa();
    sub_calc_ta4 = SimplePow((sub_calc_ta + 273.15), (double)4);
    sub_calc_tr4 = SimplePow((tr + 273.15), (double)4);
    sub_calc_taTr = sub_calc_tr4 - (sub_calc_tr4-sub_calc_ta4)/emissivity;
    
    sub_calc_alphaCorrR[0] = 1 / (1 + ksTo[0] * 40);
    sub_calc_alphaCorrR[1] = 1 ;
    sub_calc_alphaCorrR[2] = (1 + ksTo[2] * ct[2]);
    sub_calc_alphaCorrR[3] = sub_calc_alphaCorrR[2] * (1 + ksTo[3] * (ct[3] - ct[2]));
    
//------------------------- Gain calculation -----------------------------------    
    sub_calc_gain = mlx90640Frame[778-768];//we use a smaller cache of page data now
    if(sub_calc_gain > 32767)
    {
        sub_calc_gain = sub_calc_gain - 65536;
    }
    
    sub_calc_gain = gainEE / sub_calc_gain; 

//------------------------- To calculation -------------------------------------    
  
    sub_calc_mode = (mlx90640Frame[832-768] & 0x1000) >> 5;
    sub_calc_irDataCP[0] = mlx90640Frame[776-768];  
    sub_calc_irDataCP[1] = mlx90640Frame[808-768];
  

 
    for( int i = 0; i < 2; i++)
    {

        if(sub_calc_irDataCP[i] > 32767)
        {
            sub_calc_irDataCP[i] = sub_calc_irDataCP[i] - 65536;
        }
        sub_calc_irDataCP[i] = sub_calc_irDataCP[i] * sub_calc_gain;
    }

    
    sub_calc_irDataCP[0] = sub_calc_irDataCP[0] - cpOffset[0] * (1 + cpKta * (sub_calc_ta - 25)) * (1 + cpKv * (sub_calc_vdd - 3.3));
    if( sub_calc_mode ==  calibrationModeEE)
    {
        sub_calc_irDataCP[1] = sub_calc_irDataCP[1] - cpOffset[1] * (1 + cpKta * (sub_calc_ta - 25)) * (1 + cpKv * (sub_calc_vdd - 3.3));
    }
    else
    {
      sub_calc_irDataCP[1] = sub_calc_irDataCP[1] - (cpOffset[1] + ilChessC[0]) * (1 + cpKta * (sub_calc_ta - 25)) * (1 + cpKv * (sub_calc_vdd - 3.3));
    }
    //to here data is same for every pixel
}//this is for one time per read check of data
   // for( int pixelNumber = 0; pixelNumber < 768; pixelNumber++)
   // {
        sub_calc_ilPattern = (pixelNumber >>5) - ((pixelNumber>>6) <<1); 
        sub_calc_chessPattern = sub_calc_ilPattern ^ (pixelNumber - ((pixelNumber/2)<<1)); 
        sub_calc_conversionPattern = (((pixelNumber + 2) >>2) - ((pixelNumber + 3) >>2) + ((pixelNumber + 1) >>2) - (pixelNumber >>2)) * (1 - 2 * sub_calc_ilPattern);
        
        if(sub_calc_mode == 0)
        {
          sub_calc_pattern = sub_calc_ilPattern; 
        }
        else 
        {
          sub_calc_pattern = sub_calc_chessPattern; 
        }  
     
        if(sub_calc_pattern == sub_calc_pattern)// mlx90640Frame[833-768])//if frame matches current data group then process further
        {  sub_calc_irData = RamGetStoredInLocal(pixelNumber) ;   //sub_calc_irData = mlx90640FrameCELLRAM[pixelNumber];
            if(sub_calc_irData > 32767)
            {
                sub_calc_irData = sub_calc_irData - 65536;
            }
        



          
            sub_calc_irData = sub_calc_irData * sub_calc_gain;
#if NEW_METHOD ==false  //old way
            irData = irData - offset[pixelNumber]*(1 + kta[pixelNumber]*(ta - 25))*(1 + kv[pixelNumber]*(vdd - 3.3));
#endif
#if NEW_METHOD ==true
           sub_calc_irData = sub_calc_irData - ExtractOffsetParametersRawPerPixel(pixelNumber)*(1 + ExtractKtaPixelParametersRawPerPixel(pixelNumber)*(sub_calc_ta - 25))*(1 + ExtractKtaPixelParametersRawPerPixel(pixelNumber)*(sub_calc_vdd - 3.3));
#endif

            
            if(sub_calc_mode !=  calibrationModeEE)
            {
              sub_calc_irData = sub_calc_irData + ilChessC[2] * (2 * sub_calc_ilPattern - 1) - ilChessC[1] * sub_calc_conversionPattern; 
            }
            
            sub_calc_irData = sub_calc_irData / emissivity;
    
            sub_calc_irData = sub_calc_irData - tgc * sub_calc_irDataCP[sub_calc_subPage];
            #if NEW_METHOD != true 
            alphaCompensated = (alpha[pixelNumber] - tgc * cpAlpha[subPage])*(1 + KsTa * (sub_calc_ta - 25));
            #endif
            #if NEW_METHOD == true 
            sub_calc_alphaCompensated = (ExtractAlphaParametersRawPerPixel(pixelNumber) - tgc * cpAlpha[sub_calc_subPage])*(1 + KsTa * (sub_calc_ta - 25));    
            #endif
            sub_calc_Sx = SimplePow((double)sub_calc_alphaCompensated, (double)3) * (sub_calc_irData + sub_calc_alphaCompensated * sub_calc_taTr);
          
            sub_calc_Sx = sqrt((sub_calc_Sx))       * ksTo[1];
            sub_calc_To = sqrt(sqrt(sub_calc_irData/(sub_calc_alphaCompensated * (1 - ksTo[1] * 273.15) + sub_calc_Sx) + sub_calc_taTr)) - 273.15;
           
            if(sub_calc_To < ct[1])
            {
                sub_calc_range = 0;
            }
            else if(sub_calc_To < ct[2])   
            {
                sub_calc_range = 1;            
            }   
            else if(sub_calc_To < ct[3])
            {
                sub_calc_range = 2;            
            }
            else
            {
                sub_calc_range = 3;            
            }      
            
            sub_calc_To =sqrt(sqrt(sub_calc_irData / (sub_calc_alphaCompensated * sub_calc_alphaCorrR[sub_calc_range] * (1 + ksTo[sub_calc_range] * (sub_calc_To - ct[sub_calc_range]))) + sub_calc_taTr)) - 273.15;
            
           
           
           return sub_calc_To;//we return value to main loop rather than do each pixel (all together)
        }
   // }
}
int MLX90640_GetFrameData(uint8_t slaveAddr)
{
    uint16_t dataReady = 1;
    uint16_t controlRegister1;
    uint16_t statusRegister;
    int error = 1;
    uint8_t cnt = 0;
    
    dataReady = 0;
    while(dataReady == 0)
    {
        error = MLX90640_I2CRead(slaveAddr, 0x8000, 1, &statusRegister);
        if(error != 0)
        {
            return error;
        }    
        dataReady = statusRegister & 0x0008;
    }       
        
    while(dataReady != 0 && cnt < 5)
    { 
        error = MLX90640_I2CWrite(slaveAddr, 0x8000, 0x0030);
        if(error == -1)
        {
            return error;
        }
            
/*
        error = MLX90640_I2CRead(slaveAddr, 1024, 768,  mlx90640FrameCELLRAM); 
        if(error != 0)
        {
            return error;
        }  
        */
        error = MLX90640_I2CRead(slaveAddr, 1024+768, 832-768,  mlx90640Frame); 
        if(error != 0)
        {
            return error;
        }


                         
        error = MLX90640_I2CRead(slaveAddr, 0x8000, 1, &statusRegister);
        if(error != 0)
        {
            return error;
        }    
        dataReady = statusRegister & 0x0008;
        cnt = cnt + 1;
    }
    
    if(cnt > 4)
    {
        return -8;
    }    
    
    error = MLX90640_I2CRead(slaveAddr, 0x800D, 1, &controlRegister1);
     mlx90640Frame[832-768] = controlRegister1;
     mlx90640Frame[833-768] = statusRegister & 0x0001;
    
    if(error != 0)
    {
        return error;
    }
    
    return  mlx90640Frame[833-768];    
}



//end of low mem methods
#endif

//------------- old calcs
  
int MLX90640_DumpEE(uint8_t slaveAddr)
{ uint16_t eeData[1];
     return MLX90640_I2CRead(slaveAddr, 0x2400, 1, eeData);
}


int MLX90640_ExtractParameters( )
{ 
   // int error = 0;//if any data is zero 

  //  if(error == 0)
  //  {

        
        ExtractVDDParameters();
         
        ExtractPTATParameters( );
        ExtractGainParameters();
        ExtractTgcParameters( );
        ExtractResolutionParameters();
        ExtractKsTaParameters();
        ExtractKsToParameters();
         #if NEW_METHOD != true  
        ExtractAlphaParameters();
        
       ExtractOffsetParameters();//not needed. and full test at bootup
       
        ExtractKtaPixelParameters();
        
        ExtractKvPixelParameters();
        #endif
        ExtractCPParameters( );
        ExtractCILCParameters( );
       // error = ExtractDeviatingPixels(eeData, mlx90640);  
 //   }
    
    return 0;
//we already do this when we check the values at start up
}

//------------------------------------------------------------------------------

int MLX90640_SetResolution(uint8_t slaveAddr, uint8_t resolution)
{
    uint16_t controlRegister1;
    int value;
    int error;
    
    value = (resolution & 0x03) << 10;
    
    error = MLX90640_I2CRead(slaveAddr, 0x800D, 1, &controlRegister1);
    
    if(error == 0)
    {
        value = (controlRegister1 & 0xF3FF) | value;
        error = MLX90640_I2CWrite(slaveAddr, 0x800D, value);        
    }    
    
    return error;
}

//------------------------------------------------------------------------------

int MLX90640_GetCurResolution(uint8_t slaveAddr)
{
    uint16_t controlRegister1;
    int resolutionRAM;
    int error;
    
    error = MLX90640_I2CRead(slaveAddr, 0x800D, 1, &controlRegister1);
    if(error != 0)
    {
        return error;
    }    
    resolutionRAM = (controlRegister1 & 0x0C00) >> 10;
    
    return resolutionRAM; 
}

//------------------------------------------------------------------------------

int MLX90640_SetRefreshRate(uint8_t slaveAddr, uint8_t refreshRate)
{
    uint16_t controlRegister1;
    int value;
    int error;
    
    value = (refreshRate & 0x07)<<7;
    
    error = MLX90640_I2CRead(slaveAddr, 0x800D, 1, &controlRegister1);
    if(error == 0)
    {
        value = (controlRegister1 & 0xFC7F) | value;
        error = MLX90640_I2CWrite(slaveAddr, 0x800D, value);
    }    
    
    return error;
}

//------------------------------------------------------------------------------

int MLX90640_GetRefreshRate(uint8_t slaveAddr)
{
    uint16_t controlRegister1;
    int refreshRate;
    int error;
    
    error = MLX90640_I2CRead(slaveAddr, 0x800D, 1, &controlRegister1);
    if(error != 0)
    {
        return error;
    }    
    refreshRate = (controlRegister1 & 0x0380) >> 7;
    
    return refreshRate;
}

//------------------------------------------------------------------------------

int MLX90640_SetInterleavedMode(uint8_t slaveAddr)
{
    uint16_t controlRegister1;
    int value;
    int error;
    
    error = MLX90640_I2CRead(slaveAddr, 0x800D, 1, &controlRegister1);
    
    if(error == 0)
    {
        value = (controlRegister1 & 0xEFFF);
        error = MLX90640_I2CWrite(slaveAddr, 0x800D, value);        
    }    
    
    return error;
}

//------------------------------------------------------------------------------

int MLX90640_SetChessMode(uint8_t slaveAddr)
{
    uint16_t controlRegister1;
    int value;
    int error;
        
    error = MLX90640_I2CRead(slaveAddr, 0x800D, 1, &controlRegister1);
    
    if(error == 0)
    {
        value = (controlRegister1 | 0x1000);
        error = MLX90640_I2CWrite(slaveAddr, 0x800D, value);        
    }    
    
    return error;
}

//------------------------------------------------------------------------------

int MLX90640_GetCurMode(uint8_t slaveAddr)
{
    uint16_t controlRegister1;
    int modeRAM;
    int error;
    
    error = MLX90640_I2CRead(slaveAddr, 0x800D, 1, &controlRegister1);
    if(error != 0)
    {
        return error;
    }    
    modeRAM = (controlRegister1 & 0x1000) >> 12;
    
    return modeRAM; 
}

//------------------------------------------------------------------------------
#if NEW_METHOD ==false //this is old ram intensive method of getting ram data and calibrating it
void MLX90640_CalculateTo()
{
    float vdd;
    float ta;
    float ta4;
    float tr4;
    float taTr;
    float gain;
    float irDataCP[2];
    float irData;
    float alphaCompensated;
    uint8_t mode;
    int8_t ilPattern;
    int8_t chessPattern;
    int8_t pattern;
    int8_t conversionPattern;
    float Sx;
    float To;
    float alphaCorrR[4];
    int8_t range;
    uint16_t subPage;
    
    subPage = mlx90640Frame[833-768];
    vdd = MLX90640_GetVdd();
    ta = MLX90640_GetTa();
    ta4 = pow((ta + 273.15), (double)4);
    tr4 = pow((tr + 273.15), (double)4);
    taTr = tr4 - (tr4-ta4)/emissivity;
    
    alphaCorrR[0] = 1 / (1 + ksTo[0] * 40);
    alphaCorrR[1] = 1 ;
    alphaCorrR[2] = (1 + ksTo[2] * ct[2]);
    alphaCorrR[3] = alphaCorrR[2] * (1 + ksTo[3] * (ct[3] - ct[2]));
    
//------------------------- Gain calculation -----------------------------------    
    gain = mlx90640Frame[778];
    if(gain > 32767)
    {
        gain = gain - 65536;
    }
    
    gain = gainEE / gain; 
  
//------------------------- To calculation -------------------------------------    
    mode = (mlx90640Frame[832] & 0x1000) >> 5;
    
    irDataCP[0] = mlx90640Frame[776];  
    irDataCP[1] = mlx90640Frame[808];
    for( int i = 0; i < 2; i++)
    {
        if(irDataCP[i] > 32767)
        {
            irDataCP[i] = irDataCP[i] - 65536;
        }
        irDataCP[i] = irDataCP[i] * gain;
    }
    irDataCP[0] = irDataCP[0] - cpOffset[0] * (1 + cpKta * (ta - 25)) * (1 + cpKv * (vdd - 3.3));
    if( mode ==  calibrationModeEE)
    {
        irDataCP[1] = irDataCP[1] - cpOffset[1] * (1 + cpKta * (ta - 25)) * (1 + cpKv * (vdd - 3.3));
    }
    else
    {
      irDataCP[1] = irDataCP[1] - (cpOffset[1] + ilChessC[0]) * (1 + cpKta * (ta - 25)) * (1 + cpKv * (vdd - 3.3));
    }

    for( int pixelNumber = 0; pixelNumber < 768; pixelNumber++)
    {
        ilPattern = pixelNumber / 32 - (pixelNumber / 64) * 2; 
        chessPattern = ilPattern ^ (pixelNumber - (pixelNumber/2)*2); 
        conversionPattern = ((pixelNumber + 2) / 4 - (pixelNumber + 3) / 4 + (pixelNumber + 1) / 4 - pixelNumber / 4) * (1 - 2 * ilPattern);
        
        if(mode == 0)
        {
          pattern = ilPattern; 
        }
        else 
        {
          pattern = chessPattern; 
        }               
        
        if(pattern == mlx90640Frame[833])
        {    
            irData = mlx90640Frame[pixelNumber];
            if(irData > 32767)
            {
                irData = irData - 65536;
            }
            irData = irData * gain;
#if NEW_METHOD ==false  //old way
            irData = irData - offset[pixelNumber]*(1 + kta[pixelNumber]*(ta - 25))*(1 + kv[pixelNumber]*(vdd - 3.3));
#endif
#if NEW_METHOD ==true
           irData = irData - ExtractOffsetParametersRawPerPixel(pixelNumber)*(1 + ExtractKtaPixelParametersRawPerPixel(pixelNumber)*(ta - 25))*(1 + ExtractKtaPixelParametersRawPerPixel(pixelNumber)*(vdd - 3.3));
#endif

            
            if(mode !=  calibrationModeEE)
            {
              irData = irData + ilChessC[2] * (2 * ilPattern - 1) - ilChessC[1] * conversionPattern; 
            }
            
            irData = irData / emissivity;
    
            irData = irData - tgc * irDataCP[subPage];
            #if NEW_METHOD != true 
            alphaCompensated = (alpha[pixelNumber] - tgc * cpAlpha[subPage])*(1 + KsTa * (ta - 25));
            #endif
            #if NEW_METHOD == true 
            alphaCompensated = (ExtractAlphaParametersRawPerPixel(pixelNumber) - tgc * cpAlpha[subPage])*(1 + KsTa * (ta - 25));    
            #endif
            Sx = pow((double)alphaCompensated, (double)3) * (irData + alphaCompensated * taTr);
          //  Sx = sqrt(sqrt(Sx))       * ksTo[1];
            Sx = sqrt((Sx))       * ksTo[1];
            To = sqrt(sqrt(irData/(alphaCompensated * (1 - ksTo[1] * 273.15) + Sx) + taTr)) - 273.15;
                    
            if(To < ct[1])
            {
                range = 0;
            }
            else if(To < ct[2])   
            {
                range = 1;            
            }   
            else if(To < ct[3])
            {
                range = 2;            
            }
            else
            {
                range = 3;            
            }      
            
            To = sqrt(sqrt(irData / (alphaCompensated * alphaCorrR[range] * (1 + ksTo[range] * (To - ct[range]))) + taTr)) - 273.15;
          
         
            mlx90640To[pixelNumber]= To;//we update only if no data there!
        }
    }
}
#endif //end of old way of calc each pixel in a ram array of several types of data ~>20k ram required. speed of sensor and math Calc are factor in speed. mem might make less noise?

//------------------------------------------------------------------------------


float MLX90640_GetVdd()
{
    float vdd;
    float resolutionCorrection;

    int resolutionRAM;    
     #if NEW_METHOD == true
    vdd = mlx90640Frame[810-768];
    #endif
    #if NEW_METHOD == false
    vdd = mlx90640Frame[810];
    #endif
    if(vdd > 32767)
    {
        vdd = vdd - 65536;
    }
    #if NEW_METHOD == true
    resolutionRAM = (mlx90640Frame[832-768] & 0x0C00) >> 10;
    #endif
        #if NEW_METHOD == false
    resolutionRAM = (mlx90640Frame[832] & 0x0C00) >> 10;
    #endif
    resolutionCorrection =  SimplePow(2, (double)resolutionEE) /  SimplePow(2, (double)resolutionRAM);
    vdd = (resolutionCorrection * vdd - vdd25) / kVdd + 3.3;
    
    return vdd;
}

//------------------------------------------------------------------------------

float MLX90640_GetTa()
{
    float ptat;
    float ptatArt;
    float vdd;
    float ta;
    
    vdd = MLX90640_GetVdd();
    #if NEW_METHOD == true
    ptat = mlx90640Frame[800-768];
    #endif
    #if NEW_METHOD == false
    ptat = mlx90640Frame[800];
    #endif
    if(ptat > 32767)
    {
        ptat = ptat - 65536;
    }
    #if NEW_METHOD == true
    ptatArt = mlx90640Frame[768-768];
    #endif
    #if NEW_METHOD == false
    ptatArt = mlx90640Frame[768];
     #endif
    if(ptatArt > 32767)
    {
        ptatArt = ptatArt - 65536;
    }
    ptatArt = (ptat / (ptat * alphaPTAT + ptatArt)) *  SimplePow(2, (double)18);
    
    ta = (ptatArt / (1 + KvPTAT * (vdd - 3.3)) - vPTAT25);
    ta = ta / KtPTAT + 25;
    
    return ta;
}

//------------------------------------------------------------------------------

int MLX90640_GetSubPageNumber()
{  
#if NEW_METHOD == true 
    return mlx90640Frame[833-768];    
#endif
 #if NEW_METHOD == false
    return mlx90640Frame[833];    
#endif
}    


void ExtractPTATParameters()
{
   //  KvPTAT;
   // KtPTAT;
    //vPTAT25;
    //alphaPTAT;
    
    KvPTAT = (eeDataGetStoredInLocalEPROM(50) & 0xFC00) >> 10;
    if(KvPTAT > 31)
    {
        KvPTAT = KvPTAT - 64;
    }
    KvPTAT = KvPTAT/4096;
    
    KtPTAT = eeDataGetStoredInLocalEPROM(50) & 0x03FF;
    if(KtPTAT > 511)
    {
        KtPTAT = KtPTAT - 1024;
    }
    KtPTAT = KtPTAT/8;
    
    vPTAT25 = eeDataGetStoredInLocalEPROM(49);
    
    alphaPTAT = (eeDataGetStoredInLocalEPROM(16) & 0xF000) /  SimplePow(2, (double)14) + 8.0f;
    
    KvPTAT = KvPTAT;
    KtPTAT = KtPTAT;    
    vPTAT25 = vPTAT25;
    alphaPTAT = alphaPTAT;   
}

//------------------------------------------------------------------------------

void ExtractGainParameters()
{
   // int16_t gainEE;
    
    gainEE = eeDataGetStoredInLocalEPROM(48);
    if(gainEE > 32767)
    {
        gainEE = gainEE -65536;
    }
    
    gainEE = gainEE;    
}

//------------------------------------------------------------------------------

void ExtractTgcParameters( )
{
    float tgc;
    tgc = eeDataGetStoredInLocalEPROM(60) & 0x00FF;
    if(tgc > 127)
    {
        tgc = tgc - 256;
    }
    tgc = tgc / 32.0f;
    
    tgc = tgc;        
}

//------------------------------------------------------------------------------

void ExtractResolutionParameters()
{
  //  uint8_t resolutionEE;
    resolutionEE = (eeDataGetStoredInLocalEPROM(56) & 0x3000) >> 12;    
    
    resolutionEE = resolutionEE;
}

//------------------------------------------------------------------------------

void ExtractKsTaParameters( )
{
    float KsTa;
    KsTa = (eeDataGetStoredInLocalEPROM(60) & 0xFF00) >> 8;
    if(KsTa > 127)
    {
        KsTa = KsTa -256;
    }
    KsTa = KsTa / 8192.0f;
    
    KsTa = KsTa;
}

//------------------------------------------------------------------------------

void ExtractKsToParameters( )
{
    int KsToScale;
    int8_t step;
    
    step = ((eeDataGetStoredInLocalEPROM(63) & 0x3000) >> 12) * 10;
    
    ct[0] = -40;
    ct[1] = 0;
    ct[2] = (eeDataGetStoredInLocalEPROM(63) & 0x00F0) >> 4;
    ct[3] = (eeDataGetStoredInLocalEPROM(63)& 0x0F00) >> 8;
    
    ct[2] = ct[2]*step;
    ct[3] = ct[2] + ct[3]*step;
    
    KsToScale = (eeDataGetStoredInLocalEPROM(63) & 0x000F) + 8;
    KsToScale = 1 << KsToScale;
    
    ksTo[0] = eeDataGetStoredInLocalEPROM(61) & 0x00FF;
    ksTo[1] = (eeDataGetStoredInLocalEPROM(61)& 0xFF00) >> 8;
    ksTo[2] = eeDataGetStoredInLocalEPROM(62) & 0x00FF;
    ksTo[3] = (eeDataGetStoredInLocalEPROM(62) & 0xFF00) >> 8;
    
    
    for(int i = 0; i < 4; i++)
    {
        if(ksTo[i] > 127)
        {
            ksTo[i] = ksTo[i] -256;
        }
        ksTo[i] = ksTo[i] / KsToScale;
    } 
}

//------------------------------------------------------------------------------
#if NEW_METHOD !=true   
void ExtractAlphaParameters( )
{
    int accRow[24];
    int accColumn[32];
    int p = 0;
    int alphaRef;
    uint8_t alphaScale;
    uint8_t accRowScale;
    uint8_t accColumnScale;
    uint8_t accRemScale;
    

    accRemScale = eeDataGetStoredInLocalEPROM(32) & 0x000F;
    accColumnScale = (eeDataGetStoredInLocalEPROM(32) & 0x00F0) >> 4;
    accRowScale = (eeDataGetStoredInLocalEPROM(32) & 0x0F00) >> 8;
    alphaScale = ((eeDataGetStoredInLocalEPROM(32) & 0xF000) >> 12) + 30;
    alphaRef = eeDataGetStoredInLocalEPROM(33);
    
    for(int i = 0; i < 6; i++)
    {
        p = i * 4;
        accRow[p + 0] = (eeDataGetStoredInLocalEPROM(34 + i) & 0x000F);
        accRow[p + 1] = (eeDataGetStoredInLocalEPROM(34 + i) & 0x00F0) >> 4;
        accRow[p + 2] = (eeDataGetStoredInLocalEPROM(34 + i) & 0x0F00) >> 8;
        accRow[p + 3] = (eeDataGetStoredInLocalEPROM(34 + i)& 0xF000) >> 12;
    }
    
    for(int i = 0; i < 24; i++)
    {
        if (accRow[i] > 7)
        {
            accRow[i] = accRow[i] - 16;
        }
    }
    
    for(int i = 0; i < 8; i++)
    {
        p = i * 4;
        accColumn[p + 0] = (eeDataGetStoredInLocalEPROM(40 + i) & 0x000F);
        accColumn[p + 1] = (eeDataGetStoredInLocalEPROM(40 + i) & 0x00F0) >> 4;
        accColumn[p + 2] = (eeDataGetStoredInLocalEPROM(40 + i) & 0x0F00) >> 8;
        accColumn[p + 3] = (eeDataGetStoredInLocalEPROM(40 + i) & 0xF000) >> 12;
    }
    
    for(int i = 0; i < 32; i ++)
    {
        if (accColumn[i] > 7)
        {
            accColumn[i] = accColumn[i] - 16;
        }
    }

    for(int i = 0; i < 24; i++)
    {
        for(int j = 0; j < 32; j ++)
        {
            p = 32 * i +j;
            alpha[p] = (eeDataGetStoredInLocalEPROM(64+ p) & 0x03F0) >> 4;
            if (alpha[p] > 31)
            {
                alpha[p] = alpha[p] - 64;
            }
            alpha[p] = alpha[p]*(1 << accRemScale);
            alpha[p] = (alphaRef + (accRow[i] << accRowScale) + (accColumn[j] << accColumnScale) + alpha[p]);
            alpha[p] = alpha[p] / pow(2,(double)alphaScale);
        }
    }
}
#endif
//------------------------------------------------------------------------------
#if NEW_METHOD !=true   

void ExtractOffsetParameters( )
{
    int occRow[24];
    int occColumn[32];
    int p = 0;
    int16_t offsetRef;
    uint8_t occRowScale;
    uint8_t occColumnScale;
    uint8_t occRemScale;
    

    occRemScale = (eeDataGetStoredInLocalEPROM(16) & 0x000F);
    occColumnScale = (eeDataGetStoredInLocalEPROM(16) & 0x00F0) >> 4;
    occRowScale = (eeDataGetStoredInLocalEPROM(16) & 0x0F00) >> 8;
    offsetRef = eeDataGetStoredInLocalEPROM(17);
    if (offsetRef > 32767)
    {
        offsetRef = offsetRef - 65536;
    }
    
    for(int i = 0; i < 6; i++)
    {
        p = i * 4;
        occRow[p + 0] = (eeDataGetStoredInLocalEPROM(18 +i) & 0x000F);
        occRow[p + 1] = (eeDataGetStoredInLocalEPROM(18 +i) & 0x00F0) >> 4;
        occRow[p + 2] = (eeDataGetStoredInLocalEPROM(18 +i) & 0x0F00) >> 8;
        occRow[p + 3] = (eeDataGetStoredInLocalEPROM(18 +i) & 0xF000) >> 12;
    }
    
    for(int i = 0; i < 24; i++)
    {
        if (occRow[i] > 7)
        {
            occRow[i] = occRow[i] - 16;
        }
    }
    
    for(int i = 0; i < 8; i++)
    {
        p = i * 4;
        occColumn[p + 0] = (eeDataGetStoredInLocalEPROM(24 +i) & 0x000F);
        occColumn[p + 1] = (eeDataGetStoredInLocalEPROM(24 +i)& 0x00F0) >> 4;
        occColumn[p + 2] = (eeDataGetStoredInLocalEPROM(24 +i) & 0x0F00) >> 8;
        occColumn[p + 3] = (eeDataGetStoredInLocalEPROM(24 +i) & 0xF000) >> 12;
    }
    
    for(int i = 0; i < 32; i ++)
    {
        if (occColumn[i] > 7)
        {
            occColumn[i] = occColumn[i] - 16;
        }
    }

    for(int i = 0; i < 24; i++)
    {
        for(int j = 0; j < 32; j ++)
        {
            p = 32 * i +j;
            offset[p] = (eeDataGetStoredInLocalEPROM(64 +p) & 0xFC00) >> 10;
            if (offset[p] > 31)
            {
                offset[p] = offset[p] - 64;
            }
            offset[p] = offset[p]*(1 << occRemScale);
            offset[p] = (offsetRef + (occRow[i] << occRowScale) + (occColumn[j] << occColumnScale) + offset[p]);
        }
    }
}

#endif

//------------------------------------------------------------------------------
 #if NEW_METHOD != true 
void ExtractKtaPixelParameters( )
{
    int p = 0;
    int8_t KtaRC[4];
    int8_t KtaRoCo;
    int8_t KtaRoCe;
    int8_t KtaReCo;
    int8_t KtaReCe;
    uint8_t ktaScale1;
    uint8_t ktaScale2;
    uint8_t split;

    KtaRoCo = (eeDataGetStoredInLocalEPROM(54) & 0xFF00) >> 8;
    if (KtaRoCo > 127)
    {
        KtaRoCo = KtaRoCo - 256;
    }
    KtaRC[0] = KtaRoCo;
    
    KtaReCo = (eeDataGetStoredInLocalEPROM(54)  & 0x00FF);
    if (KtaReCo > 127)
    {
        KtaReCo = KtaReCo - 256;
    }
    KtaRC[2] = KtaReCo;
      
    KtaRoCe = (eeDataGetStoredInLocalEPROM(55)  & 0xFF00) >> 8;
    if (KtaRoCe > 127)
    {
        KtaRoCe = KtaRoCe - 256;
    }
    KtaRC[1] = KtaRoCe;
      
    KtaReCe = (eeDataGetStoredInLocalEPROM(55)  & 0x00FF);
    if (KtaReCe > 127)
    {
        KtaReCe = KtaReCe - 256;
    }
    KtaRC[3] = KtaReCe;
  
    ktaScale1 = ((eeDataGetStoredInLocalEPROM(56)  & 0x00F0) >> 4) + 8;
    ktaScale2 = (eeDataGetStoredInLocalEPROM(56)  & 0x000F);

    for(int i = 0; i < 24; i++)
    {
        for(int j = 0; j < 32; j ++)
        {
            p = 32 * i +j;
            split = 2*(p/32 - (p/64)*2) + p%2;
            kta[p] = (eeDataGetStoredInLocalEPROM(64+p)  & 0x000E) >> 1;
            if (kta[p] > 3)
            {
                kta[p] = kta[p] - 8;
            }
            kta[p] = kta[p] * (1 << ktaScale2);
            kta[p] = KtaRC[split] + kta[p];
            kta[p] = kta[p] / pow(2,(double)ktaScale1);
        }
    }
}
#endif
//------------------------------------------------------------------------------
#if NEW_METHOD !=true   
void ExtractKvPixelParameters()
{
    int p = 0;
    int8_t KvT[4];
    int8_t KvRoCo;
    int8_t KvRoCe;
    int8_t KvReCo;
    int8_t KvReCe;
    uint8_t kvScale;
    uint8_t split;

    KvRoCo = (eeDataGetStoredInLocalEPROM(52)  & 0xF000) >> 12;
    if (KvRoCo > 7)
    {
        KvRoCo = KvRoCo - 16;
    }
    KvT[0] = KvRoCo;
    
    KvReCo = (eeDataGetStoredInLocalEPROM(52)  & 0x0F00) >> 8;
    if (KvReCo > 7)
    {
        KvReCo = KvReCo - 16;
    }
    KvT[2] = KvReCo;
      
    KvRoCe = (eeDataGetStoredInLocalEPROM(52)  & 0x00F0) >> 4;
    if (KvRoCe > 7)
    {
        KvRoCe = KvRoCe - 16;
    }
    KvT[1] = KvRoCe;
      
    KvReCe = (eeDataGetStoredInLocalEPROM(52) & 0x000F);
    if (KvReCe > 7)
    {
        KvReCe = KvReCe - 16;
    }
    KvT[3] = KvReCe;
  
    kvScale = (eeDataGetStoredInLocalEPROM(56) & 0x0F00) >> 8;


    for(int i = 0; i < 24; i++)
    {
        for(int j = 0; j < 32; j ++)
        {
            p = 32 * i +j;
            split = 2*(p/32 - (p/64)*2) + p%2;
            kv[p] = KvT[split];
            kv[p] = kv[p] / pow(2,(double)kvScale);
        }
    }
}
#endif
//------------------------------------------------------------------------------

void ExtractCPParameters( )
{
    float alphaSP[2];
    int16_t offsetSP[2];
   // float cpKv;
   // float cpKta;
    uint8_t alphaScale;
    uint8_t ktaScale1;
    uint8_t kvScale;

    alphaScale = ((eeDataGetStoredInLocalEPROM(32) & 0xF000) >> 12) + 27;
    
    offsetSP[0] = (eeDataGetStoredInLocalEPROM(58) & 0x03FF);
    if (offsetSP[0] > 511)
    {
        offsetSP[0] = offsetSP[0] - 1024;
    }
    
    offsetSP[1] = (eeDataGetStoredInLocalEPROM(58) & 0xFC00) >> 10;
    if (offsetSP[1] > 31)
    {
        offsetSP[1] = offsetSP[1] - 64;
    }
    offsetSP[1] = offsetSP[1] + offsetSP[0]; 
    
    alphaSP[0] = (eeDataGetStoredInLocalEPROM(57)& 0x03FF);
    if (alphaSP[0] > 511)
    {
        alphaSP[0] = alphaSP[0] - 1024;
    }
    alphaSP[0] = alphaSP[0] /   SimplePow(2,(double)alphaScale);
    
    alphaSP[1] = (eeDataGetStoredInLocalEPROM(57) & 0xFC00) >> 10;
    if (alphaSP[1] > 31)
    {
        alphaSP[1] = alphaSP[1] - 64;
    }
    alphaSP[1] = (1 + alphaSP[1]/128) * alphaSP[0];
    
    cpKta = (eeDataGetStoredInLocalEPROM(59)& 0x00FF);
    if (cpKta > 127)
    {
        cpKta = cpKta - 256;
    }
    ktaScale1 = ((eeDataGetStoredInLocalEPROM(56) & 0x00F0) >> 4) + 8;    
    cpKta = cpKta /  SimplePow(2,(double)ktaScale1);
    
    cpKv = (eeDataGetStoredInLocalEPROM(59) & 0xFF00) >> 8;
    if (cpKv > 127)
    {
        cpKv = cpKv - 256;
    }
    kvScale = (eeDataGetStoredInLocalEPROM(56) & 0x0F00) >> 8;
    cpKv = cpKv /  SimplePow(2,(double)kvScale);
       
    cpAlpha[0] = alphaSP[0];
    cpAlpha[1] = alphaSP[1];
    cpOffset[0] = offsetSP[0];
    cpOffset[1] = offsetSP[1];  
}

//------------------------------------------------------------------------------

void ExtractCILCParameters( )
{
    float ilChessC[3];
    uint8_t calibrationModeEE;
    
    calibrationModeEE = (eeDataGetStoredInLocalEPROM(10) & 0x0800) >> 4;
    calibrationModeEE = calibrationModeEE ^ 0x80;

    ilChessC[0] = (eeDataGetStoredInLocalEPROM(53)& 0x003F);
    if (ilChessC[0] > 31)
    {
        ilChessC[0] = ilChessC[0] - 64;
    }
    ilChessC[0] = ilChessC[0] / 16.0f;
    
    ilChessC[1] = (eeDataGetStoredInLocalEPROM(53) & 0x07C0) >> 6;
    if (ilChessC[1] > 15)
    {
        ilChessC[1] = ilChessC[1] - 32;
    }
    ilChessC[1] = ilChessC[1] / 2.0f;
    
    ilChessC[2] = (eeDataGetStoredInLocalEPROM(53) & 0xF800) >> 11;
    if (ilChessC[2] > 15)
    {
        ilChessC[2] = ilChessC[2] - 32;
    }
    ilChessC[2] = ilChessC[2] / 8.0f;
    
    calibrationModeEE = calibrationModeEE;
    ilChessC[0] = ilChessC[0];
    ilChessC[1] = ilChessC[1];
    ilChessC[2] = ilChessC[2];
}

//------------------------------------------------------------------------------
void ExtractVDDParameters( )
{
    // kVdd;
    // vdd25;
    
    kVdd = eeDataGetStoredInLocalEPROM(51);
    
    kVdd = (eeDataGetStoredInLocalEPROM(51) & 0xFF00) >> 8;
    if(kVdd > 127)
    {
        kVdd = kVdd - 256;
    }
    kVdd = 32 * kVdd;
    vdd25 = eeDataGetStoredInLocalEPROM(51) & 0x00FF;
    vdd25 = ((vdd25 - 256) << 5) - 8192;
    
    kVdd = kVdd;
    vdd25 = vdd25; 
}
int ExtractDeviatingPixels( )
{
    uint16_t pixCnt = 0;
    uint16_t brokenPixCnt = 0;
    uint16_t outlierPixCnt = 0;
    int warn = 0;
    int i;
    
    for(pixCnt = 0; pixCnt<5; pixCnt++)
    {
        brokenPixels[pixCnt] = 0xFFFF;
        outlierPixels[pixCnt] = 0xFFFF;
    }
        
    pixCnt = 0;    
    while (pixCnt < 768 && brokenPixCnt < 5 && outlierPixCnt < 5)
    {
        if(eeDataGetStoredInLocalEPROM(pixCnt+64) == 0)
        {
            brokenPixels[brokenPixCnt] = pixCnt;
            brokenPixCnt = brokenPixCnt + 1;
        }    
        else if((eeDataGetStoredInLocalEPROM(pixCnt+64) & 0x0001) != 0)
        {
            outlierPixels[outlierPixCnt] = pixCnt;
            outlierPixCnt = outlierPixCnt + 1;
        }    
        
        pixCnt = pixCnt + 1;
        
    } 
    
    if(brokenPixCnt > 4)  
    {
        warn = -3;
    }         
    else if(outlierPixCnt > 4)  
    {
        warn = -4;
    }
    else if((brokenPixCnt + outlierPixCnt) > 4)  
    {
        warn = -5;
    } 
    else
    {
        for(pixCnt=0; pixCnt<brokenPixCnt; pixCnt++)
        {
            for(i=pixCnt+1; i<brokenPixCnt; i++)
            {
                warn = CheckAdjacentPixels(brokenPixels[pixCnt],brokenPixels[i]);
                if(warn != 0)
                {
                    return warn;
                }    
            }    
        }
        
        for(pixCnt=0; pixCnt<outlierPixCnt; pixCnt++)
        {
            for(i=pixCnt+1; i<outlierPixCnt; i++)
            {
                warn = CheckAdjacentPixels(outlierPixels[pixCnt],outlierPixels[i]);
                if(warn != 0)
                {
                    return warn;
                }    
            }    
        } 
        
        for(pixCnt=0; pixCnt<brokenPixCnt; pixCnt++)
        {
            for(i=0; i<outlierPixCnt; i++)
            {
                warn = CheckAdjacentPixels(brokenPixels[pixCnt],outlierPixels[i]);
                if(warn != 0)
                {
                    return warn;
                }    
            }    
        }    
        
    }    
    
    
    return warn;
       
}

//------------------------------------------------------------------------------

 int CheckAdjacentPixels(uint16_t pix1, uint16_t pix2)
 {
     int pixPosDif;
     
     pixPosDif = pix1 - pix2;
     if(pixPosDif > -34 && pixPosDif < -30)
     {
         return -6;
     } 
     if(pixPosDif > -2 && pixPosDif < 2)
     {
         return -6;
     } 
     if(pixPosDif > 30 && pixPosDif < 34)
     {
         return -6;
     }
     
     return 0;    
 }
 
 //------------------------------------------------------------------------------
 
 int CheckEEPROMValid()  
 {
     int deviceSelect;
     deviceSelect = eeDataGetStoredInLocalEPROM(10) & 0x0040;
     if(deviceSelect == 0)
     {
         return 0;
     }
     
     return -7;    
 }  
 #if NEW_METHOD != true       
int MLX90640_GetFrameData(uint8_t slaveAddr)
{
    uint16_t dataReady = 1;
    uint16_t controlRegister1;
    uint16_t statusRegister;
    int error = 1;
    uint8_t cnt = 0;
    
    dataReady = 0;
    while(dataReady == 0)
    {
        error = MLX90640_I2CRead(slaveAddr, 0x8000, 1, &statusRegister);
        if(error != 0)
        {
            return error;
        }    
        dataReady = statusRegister & 0x0008;
    }       
        
    while(dataReady != 0 && cnt < 5)
    { 
        error = MLX90640_I2CWrite(slaveAddr, 0x8000, 0x0030);
        if(error == -1)
        {
            return error;
        }
            
        error = MLX90640_I2CRead(slaveAddr, 0x0400, 832,  mlx90640Frame); 
        if(error != 0)
        {
            return error;
        }
                   
        error = MLX90640_I2CRead(slaveAddr, 0x8000, 1, &statusRegister);
        if(error != 0)
        {
            return error;
        }    
        dataReady = statusRegister & 0x0008;
        cnt = cnt + 1;
    }
    
    if(cnt > 4)
    {
        return -8;
    }    
    
    error = MLX90640_I2CRead(slaveAddr, 0x800D, 1, &controlRegister1);
     mlx90640Frame[832] = controlRegister1;
     mlx90640Frame[833] = statusRegister & 0x0001;
    
    if(error != 0)
    {
        return error;
    }
    
    return  mlx90640Frame[833];    
}

void MLX90640_GetImage( float *result)
{
    float vdd;
    float ta;
    float gain;
    float irDataCP[2];
    float irData;
    float alphaCompensated;
    uint8_t mode;
   int8_t ilPattern;
    int8_t chessPattern;
    int8_t pattern;
    int8_t conversionPattern;
    float image;
    uint16_t subPage;
    
    subPage =mlx90640Frame[833];
    vdd = MLX90640_GetVdd();
    ta = MLX90640_GetTa();
    
//------------------------- Gain calculation -----------------------------------    
    gain = mlx90640Frame[778];
    if(gain > 32767)
    {
        gain = gain - 65536;
    }
    
    gain = gainEE / gain; 
  
//------------------------- Image calculation -------------------------------------    
    mode = (mlx90640Frame[832] & 0x1000) >> 5;
    
    irDataCP[0] = mlx90640Frame[776];  
    irDataCP[1] = mlx90640Frame[808];
    for( int i = 0; i < 2; i++)
    {
        if(irDataCP[i] > 32767)
        {
            irDataCP[i] = irDataCP[i] - 65536;
        }
        irDataCP[i] = irDataCP[i] * gain;
    }
    irDataCP[0] = irDataCP[0] - cpOffset[0] * (1 + cpKta * (ta - 25)) * (1 + cpKv * (vdd - 3.3));
    if( mode ==  calibrationModeEE)
    {
        irDataCP[1] = irDataCP[1] - cpOffset[1] * (1 + cpKta * (ta - 25)) * (1 + cpKv * (vdd - 3.3));
    }
    else
    {
      irDataCP[1] = irDataCP[1] - (cpOffset[1] + ilChessC[0]) * (1 + cpKta * (ta - 25)) * (1 + cpKv * (vdd - 3.3));
    }

    for( int pixelNumber = 0; pixelNumber < 768; pixelNumber++)
    {
        ilPattern = pixelNumber / 32 - (pixelNumber / 64) * 2; 
        chessPattern = ilPattern ^ (pixelNumber - (pixelNumber/2)*2); 
        conversionPattern = ((pixelNumber + 2) / 4 - (pixelNumber + 3) / 4 + (pixelNumber + 1) / 4 - pixelNumber / 4) * (1 - 2 * ilPattern);
        
        if(mode == 0)
        {
          pattern = ilPattern; 
        }
        else 
        {
          pattern = chessPattern; 
        }
        
        if(pattern == mlx90640Frame[833])
        {    
            irData = mlx90640Frame[pixelNumber];
            if(irData > 32767)
            {
                irData = irData - 65536;
            }
            irData = irData * gain;
            #if NEW_METHOD ==false  //old way
             irData = irData - offset[pixelNumber]*(1 + kta[pixelNumber]*(ta - 25))*(1 + kv[pixelNumber]*(vdd - 3.3));
            #endif
             #if NEW_METHOD ==true  //old way
            irData = irData - ExtractOffsetParametersRawPerPixel(pixelNumber)*(1 +ExtractKtaPixelParametersRawPerPixel(pixelNumber)*(ta - 25))*(1 + ExtractKtaPixelParametersRawPerPixel(pixelNumber)*(vdd - 3.3));
            #endif
            if(mode !=  calibrationModeEE)
            {
              irData = irData + ilChessC[2] * (2 * ilPattern - 1) - ilChessC[1] * conversionPattern; 
            }
            
            irData = irData - tgc * irDataCP[subPage];
            #if NEW_METHOD != true 
            alphaCompensated = (alpha[pixelNumber] - tgc * cpAlpha[subPage])*(1 + KsTa * (ta - 25));
            #endif
            #if NEW_METHOD == true 
            alphaCompensated = (ExtractAlphaParametersRawPerPixel(pixelNumber) - tgc * cpAlpha[subPage])*(1 + KsTa * (ta - 25));
            #endif
            image = irData/alphaCompensated;
            
            result[pixelNumber] = image;
        }
    }
}
//------------------------------------------------------------------------------


#endif
  
void resetForNewFrameSample(){

  MLX90640_I2CWrite(MLX90640_address, 0x8000, 48);//step mode start, data overwrite disabled,no new data in ram,no pages read 
}

  
void SetSubPageForFrameSample(uint16_t value){//        16|8|0|2|1=27
if (value==0){
  MLX90640_I2CWrite(MLX90640_address, 0x800D,(controlRegister1|9));//step mode start, data overwrite disabled,no new data in ram,no pages read 
            }
if (value==1){
  MLX90640_I2CWrite(MLX90640_address, 0x800D,(controlRegister1|25));//step mode start, data overwrite disabled,no new data in ram,no pages read 
            }
}


 void reset_ram(){
 for (int i=0;i<768;i++){
  mlx90640To[i]= 0;
 }
 }
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
