
#include "Z_MemManagment.h"//lets us know what mem management we are using
#ifndef mydatacach_
#define mydatacach_
#define TA_SHIFT 8 //Default shift for MLX90640 in open air
    float vdd ;
    float Ta; 

    float tr; 
    float emissivity; 
    #if use_ram_cache == true
    uint16_t cachedata[64];
    #endif
uint16_t worddata[32]; //used for data manipulation 1 word value or up to an entire row 32
        int16_t kVdd;
        int16_t vdd25;
        float KvPTAT;
        float KtPTAT;
        uint16_t vPTAT25;
        float alphaPTAT;
        int16_t gainEE;
        float tgc;
        float cpKv;
        float cpKta;
        uint8_t resolutionEE;
        uint8_t calibrationModeEE;
        float KsTa;
        float ksTo[4];
        int16_t ct[4];
        #if NEW_METHOD != true   
        float alpha[768]; 
        int16_t offset[768]; //we dont need this anymore
        #endif
        float kta[768];    
        float kv[768];
        float cpAlpha[2];
        int16_t cpOffset[2];
        float ilChessC[3]; 
        uint16_t brokenPixels[5];
        uint16_t outlierPixels[5]; 
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




        
#endif
