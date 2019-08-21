
#include "Z_MemManagment.h"//lets us know what mem management we are using
bool frame_0_or_1;//this has us choose frame
#if NEW_METHOD == false
uint16_t mlx90640Frame[834];//1668 bytes  mlx90640Frame[832] = controlRegister1;
#endif
#if NEW_METHOD == true
uint16_t mlx90640Frame[834-768];//1668 bytes  mlx90640Frame[832] = controlRegister1;
uint16_t mlx90640FrameCELLRAM[768];//1668 bytes  mlx90640Frame[832] = controlRegister1;
#endif
uint16_t statusRegister;//mlx90640Frame[833] these are values that we need to manage 
uint16_t controlRegister1;//mlx90640Frame[832] 
