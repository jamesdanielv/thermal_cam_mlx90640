//this is eeprom data that we read from the program  geteepromdump https://github.com/jamesdanielv/thermal_cam_mlx90640/blob/master/getEEpromDump.zip
//just cut and paste data dumped and paste it into this table
//we reference this with #include "factoryCalData.h" 

#ifndef _MLX90460SesorCalData
#define _MLX90460SesorCalData
uint16_t worddata[1]; //used for data manipulation 1 word value or up to an entire row
uint16_t mydata[32];//array for pulling data in 32 words 32(8bytex2) at a time
const PROGMEM uint16_t factoryCalData[] =  {
//data goes here 0x2400 (9216) to 0x273f (10047)
171, 10651, 0, 8289, 5, 800, 992, 6670, 41522, 389, 1165, 0, 6401, 0, 0, 48691, 
16912, 65471, 514, 514, 61954, 58098, 53729, 41152, 65038, 65039, 61168, 61424, 61442, 61700, 61957, 58116, 
31142, 11880, 65500, 8720, 13106, 8755, 65297, 48094, 60892, 255, 8721, 8738, 8755, 4386, 61184, 52446, 
5946, 12188, 8532, 40315, 21555, 63952, 27241, 26211, 9059, 62547, 5052, 829, 60416, 62208, 40645, 9560, 
1998, 1954, 6416, 63568, 2142, 144, 6206, 61534, 62, 64, 8094, 64430, 64622, 64638, 5200, 62542, 
910, 64336, 5152, 60430, 64448, 64446, 5072, 62366, 63568, 62480, 4128, 60654, 64576, 61680, 5074, 61120, 
2050, 2000, 63710, 4096, 3088, 94, 64510, 4112, 2, 1070, 65390, 6000, 78, 1102, 63550, 4112, 
1890, 816, 64510, 4048, 914, 896, 65454, 4944, 1058, 992, 65534, 4256, 3074, 192, 63504, 3840, 
1086, 1952, 5232, 63422, 3006, 64544, 7072, 62414, 1982, 2014, 7968, 63358, 2032, 30, 5136, 61470, 
65374, 64286, 6096, 61358, 64368, 64336, 5008, 61278, 65520, 62384, 5008, 59504, 912, 62560, 5170, 60288, 
2192, 2000, 62654, 4064, 4066, 64622, 64510, 4096, 2032, 1054, 894, 5054, 2112, 110, 63614, 4192, 
1952, 878, 63550, 4080, 65474, 65454, 64478, 4000, 1060, 65520, 65502, 3248, 4050, 160, 64640, 2048, 
3214, 1968, 6272, 64462, 6096, 1074, 8174, 63502, 2096, 1088, 7086, 64478, 3200, 144, 4302, 62622, 
2976, 63440, 4222, 60462, 64496, 64448, 6096, 63374, 34, 64464, 6082, 60590, 976, 63618, 5202, 58432, 
2096, 864, 63550, 4976, 5010, 992, 64414, 5040, 2032, 1038, 64382, 4014, 3152, 94, 61598, 4192, 
3954, 64430, 62526, 4064, 65458, 910, 65438, 4928, 2020, 926, 65406, 2128, 1938, 48, 64512, 992, 
2032, 1874, 8176, 65360, 3952, 1968, 6064, 63440, 1982, 1040, 7054, 64382, 1086, 110, 5246, 63550, 
65470, 64416, 4158, 61438, 64418, 65408, 5056, 61296, 962, 65408, 4992, 60496, 850, 64514, 6114, 62304, 
32, 65392, 63534, 2928, 1954, 65504, 62446, 2048, 992, 78, 63422, 4016, 112, 158, 60606, 3184, 
65522, 63470, 60526, 1056, 64482, 65456, 62462, 3984, 2020, 65440, 63392, 1152, 2948, 32, 63488, 2946, 
1088, 1936, 5248, 64432, 3054, 1058, 7152, 62558, 64622, 1138, 5150, 62510, 2208, 1250, 3390, 62718, 
46, 62496, 4256, 60528, 64514, 64512, 4112, 60416, 63570, 64498, 5090, 61536, 946, 64610, 5170, 61376, 
65506, 62256, 59438, 1872, 65408, 64432, 61358, 1008, 62464, 64542, 61374, 1984, 82, 63630, 58604, 2208, 
65474, 62430, 59486, 32, 65458, 63392, 62398, 1936, 64500, 65408, 62334, 2048, 1860, 65520, 62416, 1874, 
4082, 1858, 6144, 64352, 4992, 2016, 6080, 64448, 2048, 1072, 5072, 63470, 2190, 1168, 4270, 62638, 
2032, 62464, 3200, 60464, 65490, 64466, 3072, 62350, 64480, 65408, 2944, 61440, 64370, 63490, 4066, 60288, 
4098, 832, 63534, 3952, 5008, 2016, 64478, 4048, 3090, 1088, 64494, 5118, 4258, 2208, 63694, 4272, 
2064, 64528, 62638, 3136, 3058, 2016, 62494, 6032, 3060, 2960, 65424, 5136, 2946, 1040, 65520, 3984, 
992, 1858, 4096, 63296, 64414, 978, 5008, 62448, 63504, 64592, 4048, 62432, 64670, 160, 2256, 61614, 
65504, 63472, 2144, 59440, 62432, 64416, 4018, 61328, 64466, 65394, 2930, 61440, 64322, 63490, 3042, 59248, 
63472, 63280, 60430, 1856, 64400, 65488, 62366, 992, 63488, 64592, 61406, 3024, 64656, 144, 59614, 3232, 
992, 64480, 60526, 1056, 64482, 912, 63422, 3968, 2004, 1904, 62334, 6130, 2884, 2, 63456, 2930, 
62526, 1922, 1120, 63376, 64464, 18, 3040, 61456, 63518, 64624, 4048, 62430, 62672, 64704, 1232, 60638, 
62464, 61456, 2128, 59470, 63440, 64466, 3040, 61328, 61490, 64466, 1970, 59472, 64368, 63554, 4066, 60336, 
32, 65408, 60510, 3968, 1986, 2, 62414, 3072, 2032, 1104, 63422, 6048, 176, 2222, 61630, 5296, 
3042, 1008, 63536, 3104, 2994, 2992, 65486, 7024, 2066, 2992, 65454, 6208, 5988, 4128, 2016, 6050, 
4176, 5026, 7234, 1952, 4032, 3090, 6064, 64496, 4048, 4098, 5022, 65456, 3136, 3200, 3216, 63648, 
2976, 1936, 5120, 62464, 65504, 1986, 6080, 65408, 65522, 1984, 2992, 63584, 64432, 1122, 3122, 62448, 
64578, 64384, 60462, 1920, 928, 63488, 60318, 2000, 1970, 2000, 61294, 2944, 2082, 80, 59502, 2160, 
2930, 864, 63440, 4032, 64450, 928, 63376, 4962, 2004, 912, 63376, 4160, 1956, 1088, 63520, 3024, 
62688, 3106, 1264, 64512, 2, 2130, 5104, 62528, 63520, 1152, 4064, 65440, 1104, 1200, 2224, 63632, 
65472, 898, 2128, 61456, 64450, 1968, 2064, 63408, 62496, 2034, 1056, 62624, 62496, 194, 3234, 60528, 
61632, 63488, 57566, 4066, 994, 64560, 61390, 2048, 65506, 64, 61358, 7010, 3088, 1120, 60542, 5202, 
1922, 2880, 61470, 6096, 1938, 3952, 63470, 8064, 3058, 4032, 62464, 7296, 4082, 4256, 128, 6224, 
61632, 6130, 3248, 2016, 65504, 4130, 6032, 992, 65456, 6082, 5936, 848, 1008, 5088, 6112, 2, 
1824, 3858, 6048, 864, 65346, 3906, 6016, 816, 946, 5010, 4048, 96, 992, 4242, 4224, 64528, 
59648, 63568, 57630, 2098, 63538, 64624, 61422, 1072, 64496, 2048, 62334, 6032, 48, 2080, 62494, 6192, 
2898, 2880, 64478, 8096, 1922, 4976, 64462, 9090, 3076, 6096, 63534, 9394, 4162, 6386, 192, 8210, 
63664, 5220, 1360, 64640, 64624, 2226, 1056, 64560, 62464, 3104, 1920, 848, 1008, 6114, 1054, 2016, 
1840, 3858, 4080, 65440, 880, 4994, 4080, 65440, 62528, 3090, 1104, 288, 62608, 5426, 64640, 912, 
59410, 63426, 53536, 64594, 60482, 61568, 54272, 1008, 62416, 65520, 58190, 2832, 1968, 2976, 60382, 6048, 
2802, 2768, 61358, 5984, 1858, 2880, 62400, 7026, 1042, 4064, 61472, 4320, 2146, 5328, 64464, 7904 
//date ends here
};
//**********************end of factort calibration data********************************************************************************************
#endif
