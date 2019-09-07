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

#ifndef _MLX640_API_H_
#define _MLX640_API_H_
  
  //typedef struct
  //  { //this is paramsMLX90640(,0,0,0,0,0,0,0,0)....


   // } paramsMLX90640;
    
    int MLX90640_DumpEE(uint8_t slaveAddr);
    int MLX90640_GetFrameData(uint8_t slaveAddr);
    int MLX90640_ExtractParameters();
    float MLX90640_GetVdd();
    float MLX90640_GetTa();
    void MLX90640_GetImage(float *result);
    float MLX90640_GetImageRawPerPixel(uint16_t pixelNumber);//used for new_method
    void MLX90640_CalculateTo();
    float MLX90640_CalculateToRawPerPixel(uint16_t pixelNumber );
    int MLX90640_SetResolution(uint8_t slaveAddr, uint8_t resolution);
    int MLX90640_GetCurResolution(uint8_t slaveAddr);
    int MLX90640_SetRefreshRate(uint8_t slaveAddr, uint8_t refreshRate);   
    int MLX90640_GetRefreshRate(uint8_t slaveAddr);  
    int MLX90640_GetSubPageNumber();
    int MLX90640_GetCurMode(uint8_t slaveAddr); 
    int MLX90640_SetInterleavedMode(uint8_t slaveAddr);
    int MLX90640_SetChessMode(uint8_t slaveAddr);
    void SetSensorAtStart();
    float Readmlx90640To(uint16_t value);
    float emissivityvalue();
    float trvalue();
    void setframedata();
    void InitSensor();//we initialize values for first read vdd,Ta, shift...
    void resetForNewFrameSample();//this erases data in frame
  void    setmemdataforAllPixelsTogether();
  uint16_t readmlx90640To(uint16_t value);
  void SetSubPageForFrameSample(uint16_t value);
  void  reset_ram();
uint8_t Analog_resolutionValue();
void cachloadram();//used temp to learn about caching of data
float alphaScale_testing_results();
float kvScale_testing_results();
float ktaScale1_testing_results();
float gainEE_testing_results();
#endif
