//simpler functions
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
#define emissivity  0.95 //some heat is reflected back at sensor
#define open_air_shift 8// this is how much cooler the air is to silicon from isolation of sensor Ta
#include <math.h>
#include "factoryCalData.h" //this contains data that we dumped from eeprom on the sensor and placed into progmem space
//I2C_BUFFER_LENGTH is defined in Wire.H
#define MLX90640_address 0x33 //Default 7-bit unshifted address of the MLX90640
//#define I2C_BUFFER_LENGTH BUFFER_LENGTH
uint8_t  rowcache=255;//if row data is not set it pulls a fresh row, some sort of simple cach line fetch for i2c
uint16_t ramstoreline[32];//this is a 32 byte at a time pull from sensor ram. more efficienct because of overhead;

#define I2C_BUFFER_LENGTH 32

int MLX90640_I2CWrite(uint8_t _deviceAddress, unsigned int writeAddress, uint16_t data)
{
  Wire.beginTransmission((uint8_t)_deviceAddress);
  Wire.write(writeAddress >> 8); //MSB
  Wire.write(writeAddress & 0xFF); //LSB
  Wire.write(data >> 8); //MSB
  Wire.write(data & 0xFF); //LSB
  
  if (Wire.endTransmission() != 0)
  {
    //Sensor did not ACK
    Serial.println("Error: Sensor did not ack");
    return (-1);
  }

  uint16_t dataCheck;
int   MLX90640_I2CRead(_deviceAddress, writeAddress, 1, &dataCheck);
  if (dataCheck != data)
  {
    //Serial.println("The write request didn't stick");
    return -2;
  }

  return (0); //Success
}





int MLX90640_I2CRead(uint8_t _deviceAddress, unsigned int startAddress, unsigned int nWordsRead, uint16_t *data)
{

  //Caller passes number of 'unsigned ints to read', increase this to 'bytes to read'
  uint16_t bytesRemaining = nWordsRead * 2;

  //It doesn't look like sequential read works. Do we need to re-issue the address command each time?

  uint16_t dataSpot = 0; //Start at beginning of array

  //Setup a series of chunked I2C_BUFFER_LENGTH byte reads
  while (bytesRemaining > 0)
  {
    Wire.beginTransmission(_deviceAddress);
    Wire.write(startAddress >> 8); //MSB
    Wire.write(startAddress & 0xFF); //LSB
    if (Wire.endTransmission(false) != 0) //Do not release bus
    {
      Serial.println("No ack read");
      return (0); //Sensor did not ACK
    }

    uint16_t numberOfBytesToRead = bytesRemaining;
    if (numberOfBytesToRead > I2C_BUFFER_LENGTH) numberOfBytesToRead = I2C_BUFFER_LENGTH;

    Wire.requestFrom((uint8_t)_deviceAddress, numberOfBytesToRead);
    if (Wire.available())
    {
      for (uint16_t x = 0 ; x < numberOfBytesToRead / 2; x++)
      {
        //Store data into array
        data[dataSpot] = Wire.read() << 8; //MSB
        data[dataSpot] |= Wire.read(); //LSB

        dataSpot++;
      }
    }

    bytesRemaining -= numberOfBytesToRead;

    startAddress += numberOfBytesToRead / 2;
  }

  return (0); //Success
}

int MLX90640_DumpEE(uint8_t slaveAddr, uint16_t *eeData)
{
     return MLX90640_I2CRead(slaveAddr, 0x2400, 832, eeData);
}



int MLX90640_GetFrameData(uint8_t slaveAddr, uint16_t *frameData)
{
    uint16_t dataReady = 1;
    uint16_t controlRegister1;
    uint16_t statusRegister;
    int error = 1;
    uint8_t cnt = 0;
    Serial.println(".");
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
    
    return frameData[10];    
}
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





//we save from orignal driver
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
void printBits(byte myByte){
 for(byte mask = 0x80; mask; mask >>= 1){
   if(mask  & myByte)
       Serial.print('1');
   else
       Serial.print('0');
 }
}
// useing temp instructions


//******************************* replace drivers ********************************************
//we also will have own noise reduction!
//we rebuild drivers for arduino
//we get kvdd, and vdd, as return values
//we need to get voltage and kvdd twice once for one frame, and the other for next frame. then data is lockeds and read sequetially.



float get_Kvdd(){//this returns value of cell vdd
float value;
  uint16_t temp= pgm_read_word_near(factoryCalData+0x0033);// 
temp =temp & 0xFF00;// we normalize and exclude data not used
temp=temp>>8;//we reduce value down to 8bits
if (temp>127){value=temp;value-=256;}
else{value=temp;}
value=value*2*2*2*2*2;//we now have k
return value;
}

uint8_t fast_2_to_powerof0_15( uint8_t x){
//since we know pow of 2^ is only 16 values we simplify
float value;//this stores value
switch (x){
  case 0: value=1;break;case 1: value=2;break;case 2: value=4;break;case 3: value=8;break;case 4: value=16;break;case 5: value=32;break;case 6: value=64;break;case 7: value=128;break;case 8: value=256;break;
  case 9: value=512;break;case 10: value=1024;break;case 11: value=2048;break;case 12: value=4096;break;case 13: value=8192;break;case 14: value=16384;break;case 15: value=32768;break;
}
return value;// done quick hopefully.  
}


float get_vdd25(){//this returns value of cell vdd
  float value;
 uint16_t temp= pgm_read_word_near(factoryCalData+0x0033);// 2433 but in eeprom
temp=temp & 0x00FF;//we normalize and exclude data not used
value=temp;
value-=256;
value=value* 2*2*2*2*2-2*2*2*2*2*2*2*2*2*2*2*2*2;//fast solve by compiler
//value=value- fast_2_to_powerof0_15(13);//this is -2^13 but it should be solved by compiler
return value;
}


float get_vbe_ram_value(){//vbe=ram[0x0700]
float value;

MLX90640_I2CRead(MLX90640_address,  0x0700,  1, worddata);//read VBE value.

uint16_t temp=worddata[0];//we get value
if (temp>32767){ value=temp;value-=65536;  }
else{value=temp;}
return value;
}
float get_VPTAT_ram_value(){//=ram[0x0720]
float value;
MLX90640_I2CRead(MLX90640_address,  0x0720,  1, worddata);
uint16_t temp=worddata[0];//we get value
if (temp>32767){ value=temp-65536;  }
else{value=temp;}
return value;
}


float get_KVPTAT_rom_value(){
float value;
uint16_t temp= pgm_read_word_near(factoryCalData+0x0032);//per 11.1.2 documentation 
temp=temp & 0xFC00 ;;//we normalize per 11.1.2 document
temp=temp>>10;//we lower this non signed number

if (temp>31){value=temp;value-=64;}//we unsign number
else{value=temp;}

value=value*0.00024414062;/// 2^12=4096. 1/4096 mult faster than divide/(2*2*2*2*2*2*2*2*2*2*2*2);//
return value;//we return usable float. may be converted later to int
}

float get_KTPTAT_rom_value(){
  float value;
uint16_t temp= pgm_read_word_near(factoryCalData+0x0032);//per 11.1.2 documentation 
temp=temp & 0x03FF ;//we normalize per 11.1.2 document

//we unsign number
if (temp>511){value=temp;value-=1024;}
else{value=temp;}

value=value*0.125 ;/// div by 2^3 or 8, or * 0.125 multi faster
return value;//we return usable float. may be converted later to int
}
//solve down to all that can be done with stored data. maybe even expand data to store in flash still?





float DELTA_VDD_calc_using_ram_and_rom_value( float KVDD, float VDD25){ //as oer 11.2.2 docs
 
MLX90640_I2CRead(MLX90640_address,  0x072A,  1, worddata);//read VBE value.
float value;//we structure for math. might make int later
uint16_t temp=worddata[0];//we get from device fresh ram value
if (temp>32767){value=temp;value-=65536;}
else{value=temp;}
//value=value-VDD25;//we subtract reference voltage
value=value/KVDD;//we then divide agains this number
return value;// in this case we return result as a float number
}

float get_VPTAT25_rom_value(){
  float value=0;
uint16_t temp= pgm_read_word_near(factoryCalData+0x0031);//per 11.1.2 documentation 
//we sign number
if (temp>32767){value=temp;value-=65536;}
else{value=temp;}
return value;//we return usable float. may be converted later to int
}

float get_GAIN_rom_value(){
  float value;
uint16_t temp= pgm_read_word_near(factoryCalData+0x0030);//per 11.1.2 documentation 
//we unsign number
if (temp>32767){value=temp;value-=65536;}
else{value=temp;}
return value;//we return usable float. may be converted later to int
}

float get_KGAIN_RAM_AND_ROM_calc_value(){
  float gain=get_GAIN_rom_value();
  float value;

MLX90640_I2CRead(MLX90640_address,  0x072A,  1, worddata);//read VBE value. 
uint16_t temp=worddata[0];//we get from device fresh ram value
//we unsign number
if (temp>32767){value=temp;value-=65536;}
else{value=temp;}

value=gain/value;//we get kgain
return value;//we return usable float. may be converted later to int
}


float VPTATart_calc_using_ram_and_rom_values (float VPTAT, float alphaPTAT, float VBE){

float VPTATart=(VPTAT /(VPTAT*alphaPTAT+VBE)) * 262144; //2^18;// this is how calc is don on papter
return VPTATart;//we return this number as a float


}


float ALPHA_PTAT_EE_rom_value(){
  float value;
uint16_t temp= pgm_read_word_near(factoryCalData+0x0010);//per 11.1.2 documentation 
temp=temp & 0x0FFF ;//we normalize as per doc 11.1.2
value=temp;// we store unsined int in float
value=value*0.00024414062; ;//this was /2^12 or 4096, but *0.00024414062; is 1/4096. mult faster
return value;//we return usable float. may be converted later to int
}

float ALPHA_PTAT_value(){// per doc 11.1.2
float value;  
value=ALPHA_PTAT_EE_rom_value()*0.25;
value+=8;// we add 8
return value;//we return corrected value
}

//solve down to all that can be done with stored data. maybe even expand data to store in flash still?


float pix_os_average_from_rom(){// doc Table 10 Calibration parameters memory (EEPROM - bits)
float value;
uint16_t temp= pgm_read_word_near(factoryCalData+0x0011);// 0x2411 per 11.1.2 documentation and table
//we unsign number
if (temp>32767){value=temp;value-=65536;}
else{value=temp;}
return value;//we return usable float. may be converted later to int
}

float ooc_row_rom_results( uint8_t row){//row data but we return float value
  float value;
uint8_t row_minus_one_div_by4=((row-1)>>2)&B00111111  ;//equivilent of dividing by 4
uint16_t value_end_result_1_of_4_sets_of_nibbles= pgm_read_word_near(factoryCalData+0x0012+row_minus_one_div_by4);// 0x2412 per 11.1.2 documentation and table
uint8_t setofnibblestograb= (row-1) & 3;// last 2 bits are order of nible to look, we ignore all but tha last 2 bits
value_end_result_1_of_4_sets_of_nibbles<<setofnibblestograb;//we rotate bits left from 0-3 depending on value
value_end_result_1_of_4_sets_of_nibbles= value_end_result_1_of_4_sets_of_nibbles &15;// bits active are now moved to lowest, we exclude all but the bottom 4 bits
if (row>24){ value_end_result_1_of_4_sets_of_nibbles=0;}//we return nothing if value out of range
if (value_end_result_1_of_4_sets_of_nibbles>7){value=value_end_result_1_of_4_sets_of_nibbles;}//we send back 4 bits of row cal data
else{value=value_end_result_1_of_4_sets_of_nibbles-16;}
return value;// return value as a float. might be int later on
}


float ooc_col_rom_results( uint8_t col){//row data but we return float value
   float value;
uint8_t col_minus_one_div_by4=((col-1)>>2) &B00111111;//equivilent of dividing by 4
uint16_t value_end_result_1_of_4_sets_of_nibbles= pgm_read_word_near(factoryCalData+0x0018+col_minus_one_div_by4);// 0x2412 per 11.1.2 documentation and table
uint8_t setofnibblestograb= (col-1) & 3;// last 2 bits are order of nible to look, we ignore all but tha last 2 bits
value_end_result_1_of_4_sets_of_nibbles<<setofnibblestograb;//we rotate bits left from 0-3 depending on value
value_end_result_1_of_4_sets_of_nibbles= value_end_result_1_of_4_sets_of_nibbles &15;// bits active are now moved to lowest, we exclude all but the bottom 4 bits
if (col>32){ value_end_result_1_of_4_sets_of_nibbles=0;}//we return nothing if value out of range

if (value_end_result_1_of_4_sets_of_nibbles>7){value=value_end_result_1_of_4_sets_of_nibbles;}//we send back 4 bits of row cal data
else{value=value_end_result_1_of_4_sets_of_nibbles-16;}
return value;// return value as a float. might be int later on
}


float ooc_row_scale_rom_results( uint8_t col){//row data but we return float value
float value;
uint16_t temp= pgm_read_word_near(factoryCalData+0x0010);//we get rom value
temp=temp & 0x0F00;// we normalize 11.1.2
temp=temp>>8;// we divide by 2^8
value=temp;// unsigned 0 to 15
return value;// return value as a float. might be int later on
}




float ooc_col_scale_rom_results(){//row data but we return float value
float value;
uint16_t temp= pgm_read_word_near(factoryCalData+0x0010);//we get rom value
temp=temp & 0x00F0;// we normalize 11.1.2
temp=temp>>4;// we divide by 2^8
value=temp;// unsigned 0 to 15
return value;
}




float ooc_scale_reminance_rom_results(){//row data but we return float value
float value;
uint16_t temp= pgm_read_word_near(factoryCalData+0x0010);//we get rom value
temp=temp & 0x000F;// we normalize 11.1.2
value=temp;//this number remains unsigned
return value;
}

float ooc_row_scale_rom_results(){//row data but we return float value
float value;
uint16_t temp= pgm_read_word_near(factoryCalData+0x0010);//we get rom value
temp=temp & 0x0F00;// we normalize 11.1.2
temp=temp>>8;// we divide by 2^8
//this number remains unsigned
value=temp;
return value;
}


float offset_calc_recovery(uint8_t row,uint8_t col){//this is specific to row and colum data but it just pulls values from eeprom of arduino that are stored
//row has 24, col has 32
float value;
col--;row--;//we correct it to 0,0 as the format is for 1,1 to 24,32
row<<5;//we make this x32 by rotating left 5 for 32 
uint16_t temp= pgm_read_word_near(factoryCalData+0x0040+col+row);//we get rom value from location

temp=temp & 0x0FC00;//this normalizes as we only use top most 6 bits
temp>>10;//we rotate value to make lower 6 bits have value
if (temp>31){value=temp;value-=64;}
else{value=temp;}
 return value; 
}

float ram_vdd_sample(){
MLX90640_I2CRead(MLX90640_address,  0x072A,  1, worddata);//read VBE value.
float value;//we structure for math. might make int later
uint16_t temp=worddata[0];//we get from device fresh ram value

  
}

//these below are the more complex calcs
float ta_sensor_paramaters(){//this complex stuff is needed to solve to silicone temp. however this will only need to change 1 time a second, as silicone changes temp slowly. 
  //and most calcs only need to run once at start up if recodede that way, but other than ram values most data is pulled from arduino eeprom so quickly. the math can be broken up to be solved partially.
//these values can be static and only calculated 1 time 
float VPTAT= get_VPTAT_ram_value();//? //this is ram 
float VDD25= get_vdd25();// ROM value;
float KVDD=get_Kvdd();//ROM value;
float KVPTAT=get_KVPTAT_rom_value();//we get data extrapulated and solved from rom
float VPTAT25= get_VPTAT25_rom_value();//we get data extrapulated and solved from rom
float KTPTAT=get_KTPTAT_rom_value();////we get data extrapulated and solved from rom

float Delta_Vdd= DELTA_VDD_calc_using_ram_and_rom_value(KVDD,VDD25);//pulled from equations from data in rom
//these values may need to be reread depending...
float VBE=get_vbe_ram_value();
float alphaPTAT=ALPHA_PTAT_value();
float VPTATart=VPTATart_calc_using_ram_and_rom_values (VPTAT,alphaPTAT, VBE);//we do math to solve for vptatart
float Ta=
(((VPTATart/   (1+KVPTAT*Delta_Vdd))
-VPTAT25)/KTPTAT)
+25;//this is temp in degrees c
//#define emissivity  0.95 //some heat is reflected back at sensor
//#define open_air_shift 8// this is how much cooler the air is to silicon from isolation of sensor Ta
return Ta*emissivity-open_air_shift;
}


float offest_corrections_calc_(uint8_t row,uint8_t col){//this is used to restore offset of sensor. needs to be done every time but data is all in eeprom of arduino, so it is quick
//these calcs are mainly memory reads, binary rotate rights and lefts, and some conversions to float afterwards. it would be nice to remove convert back to float
float offset_avg=pix_os_average_from_rom();
float ooc_row=ooc_row_rom_results(row);
float ooc_col=ooc_col_rom_results(col);//we use row data to extrapulate row specific calibration
float offset=offset_calc_recovery(row,col);
float ooc_row_scale=ooc_row_scale_rom_results();//0-15
float ooc_col_scale=ooc_col_scale_rom_results();//0-15
float ooc_scale_reminance=ooc_scale_reminance_rom_results();////0-15
float value=offset_avg+ooc_row* fast_2_to_powerof0_15(ooc_row_scale)+ooc_col*fast_2_to_powerof0_15(ooc_col_scale)+offset_calc_recovery(row,col)*fast_2_to_powerof0_15(ooc_scale_reminance);
return value;//
}



//this needs to be optimized to pull an entire 32 bytes at a time! it is a lot faster if more than 1 word sent at once, 
pull_entire_col_of_data_store_in_ramstoreline(uint8_t row){
  row--;//we lower column by 1, it allows mem map of 0,0-23,31 where name is 1,1-24,32
MLX90640_I2CRead(MLX90640_address,  0x0400+row*32,  32,mydata);
for (byte i=0;i<32;i++){//we copy all data when sent to other value
ramstoreline[i]=mydata[i];
}
  
}
float pix_gain(uint8_t row,uint8_t col){//this gives us pixel mem value and gain
  col--;//we need to offset col number
if (row ==rowcache){}
else{pull_entire_col_of_data_store_in_ramstoreline(row);rowcache=row;};//we compare to system variable on if this data row has been seen before, if it has we have data cached
//we dont need to worry about row data any more it will be current, we use col data within row 
float value=ramstoreline[col];//we got data imediatly from cached value loaded from sensor 
value=value*get_KGAIN_RAM_AND_ROM_calc_value();
return value;  
}


float resolutionEE(){
float value;
uint16_t temp= pgm_read_word_near(factoryCalData+0x0038);//we get rom value

temp=temp& 0x3000;
temp=temp>>12;
  value=temp;
  return value;
}

float first_sesnsor_calc_step_IR_data_compensation(uint8_t row,uint8_t col){//11.2.2.5.3 documentations
//we have calculations for cached raw value,gain, offset, VDD, and Ta (ambient temp of entire back of sensor i think)
float value=pix_gain(row,col);
return value;  
}

//0x0708=CP(SP 0)
//offset of data from 2440 (each one until 24x32= value 1,31=245e, 1,32=245f, -->0x273f(24,32);
//this data might not all be needed to calculate ta
