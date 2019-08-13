
#define NEW_METHOD true
#ifndef mydatacach_
#define mydatacach_
uint16_t worddata[1]; //used for data manipulation 1 word value or up to an entire row
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
#endif
