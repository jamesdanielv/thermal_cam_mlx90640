/**
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
 */

#include "testing.h";//for optimization testing
#include "Z_MemManagment.h"//tells us what mem managment we are using, old or new
#include "factoryCalData.h"
#include "MLX90640_I2C_Driver.h"
#include "MLX90640_API.h"
#if USE_FAST_SQUARERT_METHOD == false
#include <math.h> //no longer needed saves 5k of rom
#endif
#include "i2c_Address.h" 
#include "memCache.h"
#include "pixelframemem.h"
#include "pixelCache.h"
#include "customMath.h";//this is some custom stuff to speed things up som values have been tablized and pre solved

void ExtractVDDParameters( );
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


//---------------- simple set

uint16_t eeDataGetStoredInLocalEPROM(uint16_t value){

return pgm_read_word_near(factoryCalData+ value);

  
}
uint16_t RamGetStoredInLocal(uint16_t value){

#if customSmallCacheForMemReads !=true //we read only 2 bytes at a time with a lot of address overhead
 MLX90640_I2CRead(MLX90640_address, 1024+value, 1,worddata);
return worddata[0];
#endif
#if customSmallCacheForMemReads ==true //we read only 2 bytes at a time with a lot of address overhead
//here is predictive loading. if y is different, we do different things. if y is different, we determin if y is incremental or further away. if further away we cache 64 values
uint8_t valueofy =(value>>6) ;//we are gettint the y line data /64
if (linecache !=valueofy ){//this means we have a cache miss.
//65408is 8bit //65408 is 7   6 bits wide65472 . 5bits wide is65504. this causes lower area to set itself to a mask
MLX90640_I2CRead(MLX90640_address, 1024+(value&65472),64,SmallMemCache_i2c_efficency);
   linecache =valueofy ;  }//we take the small time to update the cache and change the line buffered

 
//this part is alwasy ready on reads and if line is already cached data is instantly available. 
uint8_t valueofx =(value& 63);
return SmallMemCache_i2c_efficency[valueofx];//we return cached value most if time
#endif  //end of different cached or non cached methods

}




float Readmlx90640To(uint16_t value){
#if NEW_METHOD == true 
#if Replace_detailed_calc_with_image_data != true
return  MLX90640_CalculateToRawPerPixel(value);
#else
return  MLX90640_GetImageRawPerPixel(value);//let accurate but should be faster with less math
#endif
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
     emissivityInverted =1/ emissivity ;//this is done one time. 
Analog_resolution=MLX90640_GetCurResolution(MLX90640_address);//we get resolution of ad converter 16-19 bit resolution
Analog_resolution+=16;//this makes resolution 16-19


 
}

float SimplePow(float base, uint8_t exponent)
{//we create or own low memory multiplier

 float tempbase=base;if (exponent >0){for (uint8_t i=1;i< exponent;i++){////we have base number,we start at 1 not 0, because 1 is already done
  tempbase=tempbase*base;}}else{tempbase=1;}//we multiply unless exponent is 0 then result is 1
 return tempbase;//we return result
}

float SimplePow_ReturnFloat_Integer_operations(uint16_t base, uint8_t exponent)
{//we create or own low memory multiplier

float tempbase=base;if (exponent >0){for (uint8_t i=1;i< exponent;i++){////we have base number,we start at 1 not 0, because 1 is already done
  tempbase=tempbase*base;}}else{tempbase=1;}//we multiply unless exponent is 0 then result is 1
 return tempbase;//we return result
}






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
    
   uint16_t cacheromvalue=eeDataGetStoredInLocalEPROM(16);//with this we reduce the calls to eeprom
    occRemScale = (cacheromvalue& 0x000F);
    occColumnScale = (cacheromvalue & 0x00F0) >> 4;
    occRowScale = (cacheromvalue & 0x0F00) >> 8;
    offsetRef = eeDataGetStoredInLocalEPROM(17);//11hex
    if (offsetRef > 32767)
    {
        offsetRef = offsetRef - 65536;
    }
//we have raw number for offset now
p=row&3;//we have 0-3 for data bits
uint8_t i=row>>2;     cacheromvalue=eeDataGetStoredInLocalEPROM(18 +i); 
   if (p==0) {       occRow = (cacheromvalue & 0x000F);}
   if (p==1) {       occRow = (cacheromvalue & 0x00F0) >> 4;}
   if (p==2) {       occRow = (cacheromvalue & 0x0F00) >> 8;}
   if (p==3) {       occRow = (cacheromvalue & 0xF000) >> 12;}
   

        if (occRow > 7)
        {
            occRow = occRow - 16;
        }
    
  p=col&3;//we have 0-3 for data bits  
  i=col>>2;
                    cacheromvalue=eeDataGetStoredInLocalEPROM(24 +i);
  if (p==0) {       occColumn = (cacheromvalue & 0x000F);}
  if (p==1) {       occColumn = (cacheromvalue& 0x00F0) >> 4;}
  if (p==2) {       occColumn = (cacheromvalue& 0x0F00) >> 8;}
  if (p==3) {       occColumn = (cacheromvalue & 0xF000) >> 12;}
   
 
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
  
    int accRow=0;
    int accColumn=0;
    int p = 0;
    int alphaRef;
    uint8_t alphaScale;
    uint8_t accRowScale;
    uint8_t accColumnScale;
    uint8_t accRemScale;
    
      uint16_t cacheromvalue=eeDataGetStoredInLocalEPROM(32);
    accRemScale =cacheromvalue & 0x000F;
    accColumnScale = (cacheromvalue & 0x00F0) >> 4;
    accRowScale = (cacheromvalue & 0x0F00) >> 8;
    alphaScale = ((cacheromvalue & 0xF000) >> 12) + 30;
    alphaRef = eeDataGetStoredInLocalEPROM(33);
    
p=row&3;//we have 0-3 for data bits
uint8_t i=row>>2;     cacheromvalue=eeDataGetStoredInLocalEPROM(34 +i);
   if (p==0) {       accRow = (cacheromvalue & 0x000F);}
   if (p==1) {       accRow = (cacheromvalue & 0x00F0) >> 4;}
   if (p==2) {       accRow = (cacheromvalue & 0x0F00) >> 8;}
   if (p==3) {       accRow = (cacheromvalue & 0xF000) >> 12;}


        if (accRow > 7)
        {
            accRow = accRow - 16;
        }

  
     p=col&3;//we have 0-3 for data bits  
  i=col>>2;
                    cacheromvalue=eeDataGetStoredInLocalEPROM(40 +i);
  if (p==0) {       accColumn = (cacheromvalue & 0x000F);}
  if (p==1) {       accColumn = (cacheromvalue& 0x00F0) >> 4;}
  if (p==2) {       accColumn = (cacheromvalue & 0x0F00) >> 8;}
  if (p==3) {       accColumn = (cacheromvalue & 0xF000) >> 12;}
   
 
    
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
             alphaScale_testing=alphaScale;
            temp= temp *SimplePowFast2sInverse(alphaScale);
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
           ktaScale1_testing=ktaScale1;//this will be removed
            temp= temp * SimplePowFast2sInverse(ktaScale1);
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
            kvScale_testing=kvScale;
             temp =  temp *SimplePowFast2sInverse(kvScale);
 //       }
 //   }
 return temp;
}

float MLX90640_CalculateToRawPerPixel(uint16_t pixelNumber )
{//this ligher code requires footwork per pixel
// time till her is *******90,000 ********** 
if (pixelNumber == 0){//we only do this on start of page
    
    //sub_calc_subPage = mlx90640Frame[833];//we dont care about page data individuallly that much anymore
    sub_calc_vdd = MLX90640_GetVdd();
    
    sub_calc_ta = MLX90640_GetTa();
sub_calc_ta_MINUS_25=sub_calc_ta-25;//we cache this value. simple math but it is done 768 times or more
   //
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
float cachedForArea= (1 + cpKta * (sub_calc_ta_MINUS_25)) * (1 + cpKv * (sub_calc_vdd - 3.3));
    
    sub_calc_irDataCP[0] = sub_calc_irDataCP[0] - cpOffset[0] * cachedForArea;
    if( sub_calc_mode ==  calibrationModeEE)
    {
        sub_calc_irDataCP[1] = sub_calc_irDataCP[1] - cpOffset[1] * cachedForArea;
    }
    else
    {
      sub_calc_irDataCP[1] = sub_calc_irDataCP[1] - (cpOffset[1] + ilChessC[0]) * cachedForArea;
    }
    //to here data is same for every pixel
}//this is for one time per read check of data
// to this point ******* 95,000 ******************* so 5000 microseconds

        sub_calc_ilPattern = (pixelNumber >>5) - ((pixelNumber>>6) <<1); 
        sub_calc_chessPattern = sub_calc_ilPattern ^ (pixelNumber - ((pixelNumber>>1)<<1)); 
        sub_calc_conversionPattern = (((pixelNumber + 2) >>2) - ((pixelNumber + 3) >>2) + ((pixelNumber + 1) >>2) - (pixelNumber >>2)) * (1 - ( sub_calc_ilPattern<<1));
       //to here takes ******105,000********* so about 15,000 cycles total.
        if(sub_calc_mode == 0)
        {
          sub_calc_pattern = sub_calc_ilPattern; 
        }
        else 
        {
          sub_calc_pattern = sub_calc_chessPattern; 
        }  
     
        if(sub_calc_pattern == sub_calc_pattern)// mlx90640Frame[833-768])//if frame matches current data group then process further
        {  
          sub_calc_irData =RamGetStoredInLocal(pixelNumber) ;   //sub_calc_irData = mlx90640FrameCELLRAM[pixelNumber];768 reads of ram total 100000microseconds
            if(sub_calc_irData > 32767)
            {
                sub_calc_irData = sub_calc_irData - 65536;
            }
         
    // to here takes ******145,000 or funciton to this point takes 65,000

    //these values can be put into rom as well! modify math so the values from rom can be managed in rom!
         //   sub_calc_irData = sub_calc_irData * sub_calc_gain;//this is first slow down
             sub_calc_irData = sub_calc_irData* sub_calc_gain; //20,000us


#if NEW_METHOD ==false  //old way
            irData = irData - offset[pixelNumber]*(1 + kta[pixelNumber]*(ta - 25))*(1 + kv[pixelNumber]*(vdd - 3.3));
#endif
#if NEW_METHOD ==true    //***this is an intensive routine**350,000 micro seconds per frame
           sub_calc_irData = sub_calc_irData - ExtractOffsetParametersRawPerPixel(pixelNumber)*
           (1 + ExtractKtaPixelParametersRawPerPixel(pixelNumber)*(sub_calc_ta_MINUS_25))*
           (1 + ExtractKvPixelParametersRawPerPixel(pixelNumber)*(sub_calc_vdd - 3.3));
#endif

            // to here it takes ******315000*********  micro seconds so it used 210000

 if (pixelNumber == 0){//we only do this on start of page 
    sub_calc_ta4 = SimplePow((sub_calc_ta + 273.15), (double)4);
    sub_calc_tr4 = SimplePow((tr + 273.15), (double)4);
    sub_calc_taTr = sub_calc_tr4 - (sub_calc_tr4-sub_calc_ta4)*emissivityInverted;
 //float sub_calc_taTrB=SimplePow((tr + 273.15),4)-    (SimplePow((sub_calc_ta + 273.15),4)*emissivityInverted);
 }
// to this point only 316000 so total used is 211000 *****************************
            
            if(sub_calc_mode !=  calibrationModeEE)
            {
              sub_calc_irData = sub_calc_irData + ilChessC[2] * (2 * sub_calc_ilPattern - 1) - ilChessC[1] * sub_calc_conversionPattern; 
            }
         
            sub_calc_irData = sub_calc_irData *emissivityInverted;// in place of / emissivity;saves 20,000 microseconds over 768 reads
            
            sub_calc_irData = sub_calc_irData - tgc * sub_calc_irDataCP[sub_calc_subPage];
          // //******316000 to here. reason. tgc in most cases is 0.
            
            #if NEW_METHOD != true 
            alphaCompensated = (alpha[pixelNumber] - tgc * cpAlpha[subPage])*(1 + KsTa * (sub_calc_ta_MINUS_25));
            #endif
            #if NEW_METHOD == true 
            sub_calc_alphaCompensated = (ExtractAlphaParametersRawPerPixel(pixelNumber) - tgc * cpAlpha[sub_calc_subPage])*(1 + KsTa * (sub_calc_ta_MINUS_25));    
            #endif
            //*********370000 microseconds to here so loop takes 280000************* or 70,000 
           
            sub_calc_Sx = SimplePow(sub_calc_alphaCompensated, 3) * (sub_calc_irData + sub_calc_alphaCompensated * sub_calc_taTr);
            sub_calc_Sx = Q_rsqrt(sub_calc_Sx) ; 
            sub_calc_Sx =sub_calc_Sx  * ksTo[1];
            //  *********370000 microseconds to here .. the above line is not processing for whatever reason
            //ref
            //sub_calc_To = Q_rsqrt(Q_rsqrt(sub_calc_irData/(sub_calc_alphaCompensated * (1 - ksTo[1] * 273.15) + sub_calc_Sx) + sub_calc_taTr)) - 273.15;
  /*
    sub_calc_ta4 = SimplePow((sub_calc_ta + 273.15), (double)4);
    sub_calc_tr4 = SimplePow((tr + 273.15), (double)4);
    sub_calc_taTr = sub_calc_tr4 - (sub_calc_tr4-sub_calc_ta4)*emissivityInverted;
 float sub_calc_taTrB=SimplePow((tr + 273.15),4)-    (SimplePow((sub_calc_ta + 273.15),4)*emissivityInverted);
   */
  
   #define newMathMethod true //if true we use new method
//value stored in customMath.h   //float CachedValue[4]//this is the data cached
                                 //float CachedREf[4]//this is the referernce so we know if value has changed or not
   #if newMathMethod == true 
            float temp;
            sub_calc_To =sub_calc_irData;
            if (sub_calc_alphaCompensated != 0){
              float temp=  sub_calc_alphaCompensated;
             temp=temp* (1 - ksTo[1]) ;
              temp=temp * 273.15 ;
            }else{temp=0;}//since is usually zero dont do the calc.
             
              temp=temp + sub_calc_Sx;//this value is needed
            float   tempinv=1/temp;//we invert temp
            sub_calc_To =sub_calc_To *tempinv;
              
              temp=temp + sub_calc_Sx;
            sub_calc_To =sub_calc_To *tempinv;   // sub_calc_ta4 = SimplePow((sub_calc_ta + 273.15), (double)4);
                //tr=Ta - TA_SHIFT   //  sub_calc_tr4 = SimplePow((tr + 273.15), (double)4);
            
                //below is this: sub_calc_taTr;//sub_calc_taTr = sub_calc_tr4 - (sub_calc_tr4-sub_calc_ta4)*emissivityInverted;
            sub_calc_To =sub_calc_To + SimplePow(tr + 273.15, 4)- (SimplePow(tr + 273.15,4)-SimplePow(sub_calc_ta + 273.15,4))*emissivityInverted;      
            
            sub_calc_To = sub_calc_To -5566789756.3;//-74610.9225xx74610.9225=-5566789756.3
            sub_calc_To =   Q_rsqrt(sub_calc_To);
            // sub_calc_To = sub_calc_To  -74610.9225
            sub_calc_To =Q_rsqrt(sub_calc_To);//-273.15x-273.15=-74610.9225
            #else // below is old method


            
            sub_calc_To =sub_calc_irData;
              float temp=  sub_calc_alphaCompensated;
             temp=temp* (1 - ksTo[1]) ;
              temp=temp * 273.15 ;
              temp=temp + sub_calc_Sx;
            sub_calc_To =sub_calc_To /temp;
              
              temp=temp + sub_calc_Sx;
            sub_calc_To =sub_calc_To /temp;
                //tr=Ta - TA_SHIFT
            sub_calc_To =sub_calc_To + sub_calc_taTr;
            sub_calc_To =   Q_rsqrt(sub_calc_To);
            sub_calc_To =Q_rsqrt(sub_calc_To);
            sub_calc_To =sub_calc_To -273.15; 
            #endif
//to this point *******640,000 *******
//return sub_calc_To;
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
               
            sub_calc_To =sub_calc_irData ;
            temp=sub_calc_alphaCompensated ;
             temp= temp* sub_calc_alphaCorrR[sub_calc_range];
            temp=temp*(1 + ksTo[sub_calc_range] *      (sub_calc_To - ct[sub_calc_range]));
            
            sub_calc_To =sub_calc_To/ temp ;   

            sub_calc_To = sub_calc_To + sub_calc_taTr;  
            sub_calc_To =    Q_rsqrt(sub_calc_To);
           sub_calc_To =Q_rsqrt(sub_calc_To);
           sub_calc_To =sub_calc_To - 273.15;
           return sub_calc_To- TA_SHIFT;//we return value to main loop rather than do each pixel (all together)
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
    
    if(cnt > 4) //this retrys a few trimes
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

float MLX90640_GetImageRawPerPixel(uint16_t pixelNumber)
{/*
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
 */ 
if (pixelNumber == 0){//we only do this on start of page  
   // subPage =mlx90640Frame[833]; we dont care about sub page. we process data when it all is collected on sensor
    sub_calc_vdd = MLX90640_GetVdd();
    sub_calc_ta = MLX90640_GetTa();
sub_calc_ta_MINUS_25=sub_calc_ta-25;//we cache this value. simple math but it is done 768 times or more
    
//------------------------- Gain calculation -----------------------------------    
#if NEW_METHOD !=true
    sub_calc_gain = mlx90640Frame[778];
#else
    sub_calc_gain = mlx90640Frame[778-768];
#endif
    if(sub_calc_gain > 32767)
    {
        sub_calc_gain = sub_calc_gain - 65536;
    }
    
    sub_calc_gain = gainEE / sub_calc_gain; 
  
//------------------------- Image calculation -------------------------------------    
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
float cachedForArea= (1 + cpKta * ( sub_calc_ta_MINUS_25)) * (1 + cpKv * (sub_calc_vdd - 3.3));//we cach it is used several times
    
    sub_calc_irDataCP[0] = sub_calc_irDataCP[0] - cpOffset[0] * cachedForArea;
    if( sub_calc_mode ==  calibrationModeEE)
    {
        sub_calc_irDataCP[1] = sub_calc_irDataCP[1] - cpOffset[1] * cachedForArea;
    }
    else
    {
      sub_calc_irDataCP[1] = sub_calc_irDataCP[1] - (cpOffset[1] + ilChessC[0]) * cachedForArea;
    }


    //to here data is same for every pixel
}//this is for one time per read check of data

//    for( int pixelNumber = 0; pixelNumber < 768; pixelNumber++)
 //   {
        sub_calc_ilPattern = (pixelNumber >>5) - (((pixelNumber >>6)) <<1); 
        sub_calc_chessPattern = sub_calc_ilPattern ^ (pixelNumber - (pixelNumber>>1)>>1); 
        sub_calc_conversionPattern = (((pixelNumber + 2) >>2) - ((pixelNumber + 3) >>2) + 
        ((pixelNumber + 1) >>2) - (pixelNumber >> 2)) * (1 - ( sub_calc_ilPattern<<1));
        
        if(sub_calc_mode == 0)
        {
          sub_calc_pattern = sub_calc_ilPattern; 
        }
        else 
        {
          sub_calc_pattern = sub_calc_chessPattern; 
        }
        
       if(sub_calc_pattern == sub_calc_pattern)  // if(sub_calc_pattern == mlx90640Frame[833]) whe change because we get full frame at a time now
        {    
            sub_calc_irData = RamGetStoredInLocal(pixelNumber) ;// old method--> mlx90640Frame[pixelNumber];
            if(sub_calc_irData > 32767)
            {
                sub_calc_irData = sub_calc_irData - 65536;
            }
            sub_calc_irData = sub_calc_irData * sub_calc_gain;
            #if NEW_METHOD ==false  //old way
             irData = irData - offset[pixelNumber]*(1 + kta[pixelNumber]*( sub_calc_ta_MINUS_25))*(1 + kv[pixelNumber]*(vdd - 3.3));
            #endif
             #if NEW_METHOD ==true  //old way
             
            sub_calc_irData = sub_calc_irData - ExtractOffsetParametersRawPerPixel(pixelNumber)*
            (1 +ExtractKtaPixelParametersRawPerPixel(pixelNumber)*( sub_calc_ta_MINUS_25))*
            (1 + ExtractKvPixelParametersRawPerPixel(pixelNumber)*(sub_calc_vdd - 3.3));
            #endif
            if(sub_calc_mode !=  calibrationModeEE)
            {
              sub_calc_irData = sub_calc_irData + ilChessC[2] * (( sub_calc_ilPattern<<1) - 1) - ilChessC[1] * sub_calc_conversionPattern; 
            }
            
            sub_calc_irData = sub_calc_irData - tgc * sub_calc_irDataCP[sub_calc_subPage];
            #if NEW_METHOD != true 
            sub_calc_alphaCompensated = (sub_calc_alpha[pixelNumber] - tgc * sub_calc_cpAlpha[sub_calc_subPage])*(1 + KsTa * ( sub_calc_ta_MINUS_25));
            #endif
            #if NEW_METHOD == true 
            sub_calc_alphaCompensated = (ExtractAlphaParametersRawPerPixel(pixelNumber) - tgc * cpAlpha[sub_calc_subPage])*(1 + KsTa * ( sub_calc_ta_MINUS_25));
            #endif
            float image =sub_calc_irData/sub_calc_alphaCompensated;
            
            
            //cleans up non numbers +/- 25 deg and range of sensor we want to multiply to reduce cycles
            //1/32768=0.00003051757  16 bit
            //1/65536=0.00001525878  17 bit
            //1/131072=0.00000762939 18 bit
            //1/262144=0.00000381469 19 bit

            //we normalize
            if (Analog_resolution== 16){image=image*0.00003051757*NormalizeImageValue;}
            if (Analog_resolution== 17){image=image*0.00001525878*NormalizeImageValue;}
            if (Analog_resolution== 18){image=image*0.00000762939*NormalizeImageValue;}
            if (Analog_resolution== 19){image=image*0.00000381469*NormalizeImageValue;}
            return image+25;

            //now we take from 25 deg and make it center of range depending on data
        }
  //  }
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
    resolutionCorrection =  SimplePowFast2s(resolutionEE) *SimplePowFast2sInverse(resolutionRAM);
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
    ptatArt = (ptat / (ptat * alphaPTAT + ptatArt)) *   SimplePowFast2s(18);
    
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
    
    alphaPTAT = (eeDataGetStoredInLocalEPROM(16) & 0xF000) *SimplePowFast2sInverse(14) + 8.0f;
    
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
    
    gainEE = gainEE;       gainEE_testing=gainEE;//testing 
}

//------------------------------------------------------------------------------

void ExtractTgcParameters( )
{
    //float tgc;
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
    #if limitRangeofMath != true
    KsToScale = 1 << KsToScale;//this number works different on 32 bit int 
    #endif
    uint32_t KsToScalebig =   SimplePowFast2s(KsToScale );//this number works different on 32 bit int 
   // #endif
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
       #if limitRangeofMath != true
        ksTo[i] = ksTo[i] /KsToScale ;;/// KsToScale;
        #else
        ksTo[i] = ksTo[i] /KsToScalebig ;;/// KsToScale;
       #endif
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
    

alphaSP[0] = alphaSP[0] *SimplePowFast2sInverse(alphaScale);

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
    cpKta = cpKta *SimplePowFast2sInverse(ktaScale1);
    
    cpKv = (eeDataGetStoredInLocalEPROM(59) & 0xFF00) >> 8;
    if (cpKv > 127)
    {
        cpKv = cpKv - 256;
    }
    kvScale = (eeDataGetStoredInLocalEPROM(56) & 0x0F00) >> 8;
    cpKv = cpKv *SimplePowFast2sInverse(kvScale);
       
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
#if NEW_METHOD !=true
    gain = mlx90640Frame[778];
#else
    gain = mlx90640Frame[778-768];

#endif
    if(gain > 32767)
    {
        gain = gain - 65536;
    }
    
    gain = gainEE / gain; 
  
//------------------------- Image calculation -------------------------------------    
    mode = (mlx90640Frame[832] & 0x1000) >> 5;
#if NEW_METHOD !=true    
    irDataCP[0] = mlx90640Frame[776];  
    irDataCP[1] = mlx90640Frame[808];
#else
    irDataCP[0] = mlx90640Frame[776-768];  
    irDataCP[1] = mlx90640Frame[808-768];    
#endif
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
uint8_t Analog_resolutionValue(){
return Analog_resolution;

 
 }

 float alphaScale_testing_results(){return alphaScale_testing;}
float kvScale_testing_results(){return kvScale_testing;}
float ktaScale1_testing_results(){return ktaScale1_testing;}
float gainEE_testing_results(){return gainEE_testing;}
