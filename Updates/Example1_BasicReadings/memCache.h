
#include "Z_MemManagment.h"//lets us know what mem management we are using

#ifndef mydatacach_
#define mydatacach_
#define TA_SHIFT 8 //Default shift for MLX90640 in open air

uint8_t Analog_resolution;//this is cal at init() function
    float vdd ;
    float Ta; 

    float tr; 
    float emissivity; 
    //we place non floats above floats so we can switch float methods easier

#if    customSmallCacheForMemReads == true
uint8_t linecache=255;//we store current y line for caching. 255 for us means it is empty. if it is 255, first read will fill cache completely
uint16_t SmallMemCache_i2c_efficency[768];//this evetuanolly will use  128 bytes extra cach for improved performance
#endif
uint16_t worddata[32]; //used for data manipulation 1 word value or up to an entire row 32
        int16_t kVdd;
        int16_t vdd25;
        uint16_t vPTAT25;
        int16_t gainEE;
        uint8_t resolutionEE;
        uint8_t calibrationModeEE;
        int16_t ct[4];
        int16_t cpOffset[2];
        uint16_t brokenPixels[5];
        uint16_t outlierPixels[5]; 
        #if NEW_METHOD != true   
        float alpha[768]; 
        int16_t offset[768]; //we dont need this anymore
        float kta[768];    
        float kv[768];
        #endif

 #if use64bitfloatwith8bitdata != true //set false for arm, otherwise math will be slower!                            
        float KvPTAT;
        float KtPTAT;        
        float alphaPTAT;        
        float tgc;
        float cpKv;
        float cpKta;
        float KsTa;
        float ksTo[4];
        float cpAlpha[2];
        float ilChessC[3]; 

        //these are one time use per loop
#if NEW_METHOD == true 

    float sub_calc_vdd;
    float sub_calc_ta;
    float sub_calc_ta4;
    float sub_calc_tr4;
    float sub_calc_taTr;
    float sub_calc_gain;
    float sub_calc_irDataCP[2];
    float sub_calc_irData;
    float sub_calc_alphaCompensated;
    uint8_t sub_calc_mode;
    int8_t sub_calc_ilPattern;
    int8_t sub_calc_chessPattern;
    int8_t sub_calc_pattern;
    int8_t sub_calc_conversionPattern;
    float sub_calc_Sx;
    float sub_calc_To;
    float sub_calc_alphaCorrR[4];
    int8_t sub_calc_range;
    uint16_t sub_calc_subPage;

#endif
#endif//8bitmethodfor64bit math== false end
 #if use64bitfloatwith8bitdata == true //this might be an option for arm processors                           
        float KvPTAT;
        float KtPTAT;        
        float alphaPTAT;        
        float tgc;
        float cpKv;
        float cpKta;
        float KsTa;
        float ksTo[4];
        float cpAlpha[2];
        float ilChessC[3]; 

        //these are one time use per loop
#if NEW_METHOD == true   
    float sub_calc_vdd;
    float sub_calc_ta;
    float sub_calc_ta4;
    float sub_calc_tr4;
    float sub_calc_taTr;
    float sub_calc_gain;
    float sub_calc_irDataCP[2];
    float sub_calc_irData;
    float sub_calc_alphaCompensated;
    uint8_t sub_calc_mode;
    int8_t sub_calc_ilPattern;
    int8_t sub_calc_chessPattern;
    int8_t sub_calc_pattern;
    int8_t sub_calc_conversionPattern;
    float sub_calc_Sx;
    float sub_calc_To;
    float sub_calc_alphaCorrR[4];
    int8_t sub_calc_range;
    uint16_t sub_calc_subPage;
#endif
#endif//8bitmethodfor64bit math== false end
        
#endif
