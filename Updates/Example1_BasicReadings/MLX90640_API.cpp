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
#include "factoryCalData.h"
#include "MLX90640_I2C_Driver.h"
#include "MLX90640_API.h"
#include <math.h>
#include "i2c_Address.h" 
#include "memCache.h"
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

//---------------- simple set

uint16_t eeDataGetStoredInLocalEPROM(uint16_t value){

return pgm_read_word_near(factoryCalData+ value);

  
}





//**************************************************************************************
#if NEW_METHOD == true   
 

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
            temp= temp / pow(2,(double)alphaScale);
            return temp;
}



#endif

//------------- old calcs
  
int MLX90640_DumpEE(uint8_t slaveAddr)
{ uint16_t eeData[1];
     return MLX90640_I2CRead(slaveAddr, 0x2400, 1, eeData);
}

int MLX90640_GetFrameData(uint8_t slaveAddr, uint16_t *frameData)
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
            
        error = MLX90640_I2CRead(slaveAddr, 0x0400, 832, frameData); 
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
    frameData[832] = controlRegister1;
    frameData[833] = statusRegister & 0x0001;
    
    if(error != 0)
    {
        return error;
    }
    
    return frameData[833];    
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
       #endif
        ExtractKtaPixelParameters();
        ExtractKvPixelParameters();
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

void MLX90640_CalculateTo(uint16_t *frameData,  float emissivity, float tr, float *result)
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
    
    subPage = frameData[833];
    vdd = MLX90640_GetVdd(frameData);
    ta = MLX90640_GetTa(frameData);
    ta4 = pow((ta + 273.15), (double)4);
    tr4 = pow((tr + 273.15), (double)4);
    taTr = tr4 - (tr4-ta4)/emissivity;
    
    alphaCorrR[0] = 1 / (1 + ksTo[0] * 40);
    alphaCorrR[1] = 1 ;
    alphaCorrR[2] = (1 + ksTo[2] * ct[2]);
    alphaCorrR[3] = alphaCorrR[2] * (1 + ksTo[3] * (ct[3] - ct[2]));
    
//------------------------- Gain calculation -----------------------------------    
    gain = frameData[778];
    if(gain > 32767)
    {
        gain = gain - 65536;
    }
    
    gain = gainEE / gain; 
  
//------------------------- To calculation -------------------------------------    
    mode = (frameData[832] & 0x1000) >> 5;
    
    irDataCP[0] = frameData[776];  
    irDataCP[1] = frameData[808];
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
        
        if(pattern == frameData[833])
        {    
            irData = frameData[pixelNumber];
            if(irData > 32767)
            {
                irData = irData - 65536;
            }
            irData = irData * gain;
#if NEW_METHOD ==false  //old way
            irData = irData - offset[pixelNumber]*(1 + kta[pixelNumber]*(ta - 25))*(1 + kv[pixelNumber]*(vdd - 3.3));
#endif
#if NEW_METHOD ==true
           irData = irData - ExtractOffsetParametersRawPerPixel(pixelNumber)*(1 + kta[pixelNumber]*(ta - 25))*(1 + kv[pixelNumber]*(vdd - 3.3));
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
            
            result[pixelNumber] = To;
        }
    }
}

//------------------------------------------------------------------------------

void MLX90640_GetImage(uint16_t *frameData,  float *result)
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
    
    subPage = frameData[833];
    vdd = MLX90640_GetVdd(frameData);
    ta = MLX90640_GetTa(frameData);
    
//------------------------- Gain calculation -----------------------------------    
    gain = frameData[778];
    if(gain > 32767)
    {
        gain = gain - 65536;
    }
    
    gain = gainEE / gain; 
  
//------------------------- Image calculation -------------------------------------    
    mode = (frameData[832] & 0x1000) >> 5;
    
    irDataCP[0] = frameData[776];  
    irDataCP[1] = frameData[808];
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
        
        if(pattern == frameData[833])
        {    
            irData = frameData[pixelNumber];
            if(irData > 32767)
            {
                irData = irData - 65536;
            }
            irData = irData * gain;
            
            irData = irData - 1111*(1 + kta[pixelNumber]*(ta - 25))*(1 + kv[pixelNumber]*(vdd - 3.3));
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

float MLX90640_GetVdd(uint16_t *frameData)
{
    float vdd;
    float resolutionCorrection;

    int resolutionRAM;    
    
    vdd = frameData[810];
    if(vdd > 32767)
    {
        vdd = vdd - 65536;
    }
    resolutionRAM = (frameData[832] & 0x0C00) >> 10;
    resolutionCorrection = pow(2, (double)resolutionEE) / pow(2, (double)resolutionRAM);
    vdd = (resolutionCorrection * vdd - vdd25) / kVdd + 3.3;
    
    return vdd;
}

//------------------------------------------------------------------------------

float MLX90640_GetTa(uint16_t *frameData)
{
    float ptat;
    float ptatArt;
    float vdd;
    float ta;
    
    vdd = MLX90640_GetVdd(frameData);
    
    ptat = frameData[800];
    if(ptat > 32767)
    {
        ptat = ptat - 65536;
    }
    
    ptatArt = frameData[768];
    if(ptatArt > 32767)
    {
        ptatArt = ptatArt - 65536;
    }
    ptatArt = (ptat / (ptat * alphaPTAT + ptatArt)) * pow(2, (double)18);
    
    ta = (ptatArt / (1 + KvPTAT * (vdd - 3.3)) - vPTAT25);
    ta = ta / KtPTAT + 25;
    
    return ta;
}

//------------------------------------------------------------------------------

int MLX90640_GetSubPageNumber(uint16_t *frameData)
{
    return frameData[833];    

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
    
    alphaPTAT = (eeDataGetStoredInLocalEPROM(16) & 0xF000) / pow(2, (double)14) + 8.0f;
    
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

//------------------------------------------------------------------------------

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
    alphaSP[0] = alphaSP[0] /  pow(2,(double)alphaScale);
    
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
    cpKta = cpKta / pow(2,(double)ktaScale1);
    
    cpKv = (eeDataGetStoredInLocalEPROM(59) & 0xFF00) >> 8;
    if (cpKv > 127)
    {
        cpKv = cpKv - 256;
    }
    kvScale = (eeDataGetStoredInLocalEPROM(56) & 0x0F00) >> 8;
    cpKv = cpKv / pow(2,(double)kvScale);
       
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

 
