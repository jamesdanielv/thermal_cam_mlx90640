//this is eeprom data that we read from the program  geteepromdump https://github.com/jamesdanielv/thermal_cam_mlx90640/blob/master/getEEpromDump.zip
//just cut and paste data dumped and paste it into this table
//we reference this with #include "factoryCalData.h" 

#ifndef _MLX90460SensorCalData
#define _MLX90460SensorCalData
#include <stdint.h>  //needed to define values as teensy libraryt doesnt include it automatically
#include <avr/pgmspace.h> 

const  uint16_t factoryCalData[] PROGMEM=  {//new 55deg x35 deg sensor
//data goes here 0x2400 (9216) to 0x273f (10047)
165, 31135, 0, 8289, 4, 800, 992, 5641, 35011, 391, 1177, 0, 6401, 0, 0, 48691, 
16912, 65470, 514, 514, 514, 57842, 53729, 40896, 3599, 3857, 2, 61699, 61700, 57604, 53733, 37331, 
35221, 13963, 61148, 4111, 4369, 4385, 17, 65280, 56779, 255, 8737, 13107, 8755, 4386, 65280, 48094, 
5895, 12193, 8534, 41867, 30294, 405, 14646, 13621, 9298, 149, 5052, 1567, 62976, 38807, 38807, 11002, 
2158, 3024, 9182, 63582, 4030, 96, 7198, 62606, 1182, 1968, 5262, 61598, 14, 174, 5166, 64478, 
3074, 80, 5312, 61662, 64752, 62590, 5182, 60526, 64720, 63616, 3104, 60382, 61456, 62576, 3248, 61488, 
5088, 3918, 1854, 5056, 4898, 990, 910, 4096, 2064, 2878, 63500, 4096, 1920, 1070, 64428, 5984, 
6002, 1998, 64604, 4176, 1138, 65534, 65454, 4064, 1106, 2016, 65422, 3936, 880, 992, 63550, 6048, 
3054, 1952, 8190, 64446, 2942, 16, 8126, 64478, 2096, 2912, 6176, 62526, 1008, 112, 4112, 63486, 
2048, 2, 4318, 62494, 112, 63584, 5168, 62430, 64624, 1008, 3088, 61280, 64414, 63520, 4178, 62432, 
4162, 1054, 64622, 5152, 4066, 158, 78, 5200, 3234, 4078, 64654, 4256, 1138, 1278, 63614, 5216, 
3202, 1150, 63838, 5234, 2290, 192, 64718, 5216, 1250, 1150, 62604, 5088, 1026, 158, 63678, 4162, 
7136, 2994, 9152, 63534, 6126, 1104, 7232, 64544, 3312, 2048, 6238, 61662, 4080, 2064, 7120, 63614, 
2192, 1104, 5424, 62702, 382, 63664, 5248, 62560, 64752, 64720, 5056, 62398, 63534, 64592, 4224, 61504, 
6962, 1790, 814, 5008, 4928, 958, 65438, 6032, 3154, 1902, 65438, 2064, 4898, 2880, 65278, 4992, 
4064, 1936, 63630, 3152, 1202, 64512, 65534, 5056, 1090, 62, 65342, 3840, 880, 958, 64478, 4000, 
3166, 3024, 9200, 64576, 1118, 208, 7248, 63646, 1262, 3040, 6224, 63536, 992, 2066, 6064, 63518, 
1088, 1040, 6256, 62622, 210, 63666, 5216, 62606, 1170, 192, 6130, 62384, 65504, 48, 3280, 60560, 
2978, 816, 64334, 3970, 1954, 63520, 64446, 3040, 32, 814, 63372, 3936, 802, 1872, 63214, 3904, 
1906, 862, 64446, 4048, 1028, 64494, 64414, 4016, 3028, 30, 64350, 3840, 818, 910, 61502, 3040, 
2080, 2962, 8144, 63536, 3040, 1090, 9136, 63520, 1118, 2930, 8016, 63408, 912, 1968, 5008, 64304, 
992, 944, 4208, 61582, 64608, 63600, 3200, 63424, 82, 176, 3122, 63296, 65376, 64544, 4178, 61456, 
98, 64480, 61486, 1136, 18, 63584, 64494, 1088, 128, 928, 63358, 2000, 65458, 65520, 60382, 3936, 
1010, 65488, 61582, 1184, 64690, 61616, 61630, 4080, 148, 64766, 61550, 3952, 930, 64624, 62590, 2128, 
4224, 2050, 7216, 63616, 4208, 1202, 8160, 63630, 2208, 2880, 4112, 62528, 2000, 3024, 4944, 65392, 
64672, 64592, 5152, 62528, 128, 64592, 4224, 62528, 114, 64768, 2176, 60480, 64496, 64690, 1248, 61536, 
178, 62528, 60526, 160, 1170, 62688, 61454, 160, 64690, 65376, 59454, 64640, 64530, 65504, 61326, 2944, 
63696, 62592, 60494, 1138, 64690, 62608, 60590, 1136, 164, 63790, 60606, 112, 63538, 63712, 60718, 1168, 
192, 2066, 4304, 62656, 144, 258, 4224, 63536, 64720, 2034, 3104, 60448, 62464, 64, 3024, 61456, 
64530, 64528, 3200, 61584, 63616, 63584, 1296, 60544, 62672, 64722, 1184, 60448, 65456, 63680, 2288, 60560, 
65506, 64304, 61438, 2032, 930, 63520, 62366, 3922, 65522, 65296, 61246, 2880, 64274, 65376, 61182, 3856, 
1842, 65358, 63374, 4000, 1922, 65376, 61486, 5008, 2034, 1024, 63438, 4912, 3778, 2000, 63504, 5056, 
256, 5218, 6352, 64768, 224, 3298, 6240, 64670, 1216, 5090, 7120, 64496, 65504, 4080, 5040, 896, 
128, 2082, 5232, 64560, 2082, 1122, 5232, 64528, 64768, 3282, 5202, 63488, 32, 2226, 4336, 64608, 
63746, 64640, 60670, 2320, 64738, 64752, 62590, 3248, 1202, 2046, 63486, 3104, 994, 2064, 62398, 6032, 
1138, 1072, 63630, 5200, 3122, 1120, 63630, 6192, 2340, 3310, 64654, 6162, 3106, 3278, 240, 7296, 
2144, 5106, 4128, 64656, 64, 1122, 6016, 63536, 112, 2898, 2976, 63376, 800, 2976, 912, 65248, 
1904, 1970, 3072, 63472, 930, 978, 5104, 62528, 62560, 1122, 1070, 63376, 64416, 64720, 1168, 62576, 
63570, 64496, 59424, 64672, 63554, 62576, 60334, 98, 64626, 65390, 60334, 912, 818, 65456, 59310, 3824, 
1922, 65504, 60478, 3088, 2002, 65520, 63470, 3136, 64626, 1120, 61470, 3984, 1938, 208, 62608, 3200, 
58640, 1170, 1200, 61682, 62576, 64802, 1168, 61632, 60704, 1010, 3008, 60416, 60382, 34, 912, 63376, 
62480, 64578, 1136, 60576, 61536, 62624, 1090, 61440, 59664, 194, 64688, 62400, 59486, 146, 224, 61630, 
59666, 62592, 57518, 64736, 63570, 61712, 59550, 1232, 61744, 64512, 60368, 1056, 62416, 48, 60334, 4000, 
34, 64576, 60558, 3232, 82, 64686, 62542, 5152, 64786, 2272, 61614, 7088, 50, 3200, 63712, 7344, 
58448, 4018, 32, 62544, 62400, 1026, 2016, 62496, 61440, 1890, 2864, 61376, 61280, 2882, 1776, 63328, 
62368, 896, 48, 63408, 63408, 994, 2032, 64368, 61488, 2066, 1904, 65168, 63262, 1024, 2000, 63520, 
57376, 62336, 56318, 64560, 61346, 61440, 58350, 16, 63474, 64352, 60222, 960, 61296, 65376, 59150, 3936, 
64434, 65424, 59454, 5056, 928, 65518, 61438, 7008, 18, 4080, 63342, 7824, 2816, 3054, 64462, 6160, 
61536, 7010, 2976, 944, 65360, 2050, 2912, 64416, 63488, 4914, 862, 64368, 63342, 4866, 768, 784, 
65456, 4896, 4992, 65456, 65504, 4978, 5008, 1776, 63440, 6994, 878, 65280, 62270, 6016, 1022, 2032, 
58434, 64336, 56238, 928, 63298, 63456, 58222, 944, 64498, 65344, 58222, 880, 63344, 2832, 60190, 4912, 
962, 2864, 63374, 7104, 1008, 2960, 64414, 9968, 3010, 5982, 63326, 5904, 1840, 6046, 65534, 9184, 
//date ends here
};
//**********************end of factory calibration data********************************************************************************************



#endif
