//ZZZ_ heading just keeps this showing towards the right of tabs
#define nonfloatReturnNumbers true//if this is false it is more accurate. reduces buffer and speeds up output to serial display
//this is a small process that does not use much memory and increases resolution of data in
// however it requires fast ram reads of data in order to not store its own table
//it also requires modification of main structure so it can read data
//it just interpolates in between data by averaging creating a new pixel
//this is not my zoom method. this is just a quick method. some data resolution
// is lost on the edges. it is effectively a resdouble-1
// do not remove commends from this line and above. written james villeneuve
#include "MLX90640_API.h" //we need this so we can access the memory
#define XresolutionOriginal 32 //this is original resolution
#define YresolutionOriginal 24 //this is original resolution
//we use below as cache. we only update information 1/4 of time
//we need a line cache of 32 values to improve performance
//float DataCache[XresolutionOriginal];//this uses a small amount of cache
uint8_t bufferlineNumber =0;//this is a reference to y lines. x line is cached. and previous x line cached. 

#if nonfloatReturnNumbers != true
  float ReturnCache=0;//this is to make returns more readable. compiler hopefully will remove as a redundancy
  float prebufferCache[32];//we use this to reduce mem reads to 1 time.
  float CurrentbufferCache[32];//we use this to reduce mem reads to 1 time.
float DoubleResolutionValue(uint8_t XatNewresolution, uint8_t YatNewresolution) {
#endif

#if nonfloatReturnNumbers == true
  uint8_t ReturnCache=0;//this is to make returns more readable. compiler hopefully will remove as a redundancy
  uint8_t prebufferCache[32];//we use this to reduce mem reads to 1 time.
  uint8_t CurrentbufferCache[32];//we use this to reduce mem reads to 1 time.
uint8_t DoubleResolutionValue(uint8_t XatNewresolution, uint8_t YatNewresolution) {
#endif


  //this routine uses data from values it can get from sensor reads to double resolution of data without using much memory
  //we just need to know the ram we are reading, the x and y resolution, and pixel number
  //this is an over simplified setup that interpolates every other value in x and y
  //**** we need mem value to pull data from*****************************************************
  //****                                    *****************************************************
  //we need actual pixel values of non doubled data for ref

  uint8_t pix_X = XatNewresolution >> 1;
  uint8_t pix_Y = YatNewresolution >> 1;

  //we need to determin if new pixel is odd or even. odd numbers are interpolated with nearest left side value
  uint8_t  pix_x_is_it_odd = XatNewresolution & B00000001;
  uint8_t  pix_y_is_it_odd = YatNewresolution & B00000001;


  if ((YatNewresolution== 0) & (XatNewresolution==0)) {for (byte i = 0; i <XresolutionOriginal;i++) {prebufferCache[i]= (Readmlx90640To(i ));CurrentbufferCache[i]=(Readmlx90640To(i + XresolutionOriginal));};
  bufferlineNumber=pix_Y;} //we read 2 lines at start. also we set buffer to same line number
//here is predictive loading. if y is different, we do different things. if y is different, we determin if y is incremental or further away. if further away we cache 64 values
if (bufferlineNumber !=pix_Y){//this means we have a cache miss. to make it pull data better we only get a large pull at a time.
  
  for (byte i = 0; i <XresolutionOriginal;i++) {prebufferCache[i]= CurrentbufferCache[i];CurrentbufferCache[i]=(Readmlx90640To(i+pix_Y*XresolutionOriginal+XresolutionOriginal));};
   bufferlineNumber=pix_Y;  }
if (YatNewresolution<2){//if first two lines, we don't pull data. this is because there is no getting data below line zero
   #if nonfloatReturnNumbers != true
if (pix_x_is_it_odd==1 ){ReturnCache=(prebufferCache[pix_X] + prebufferCache[pix_X-1])*0.5;}
                                        else{ReturnCache=prebufferCache[pix_X];}//this is first and second line
                        }//1st 2 lines are done here. 
#endif
   #if nonfloatReturnNumbers == true
if (pix_x_is_it_odd==1 ){ReturnCache=(prebufferCache[pix_X] + prebufferCache[pix_X-1])>>1;}
                                        else{ReturnCache=prebufferCache[pix_X];}//this is first and second line
                        }//1st 2 lines are done here. 
 #endif                       
else{//this is for rest of data
if (XatNewresolution <1){ReturnCache=prebufferCache[pix_X];}//we have no date from left to get at zero or 1. so we make it the same data 
else{//if we are here we are not at pixel edges
#if nonfloatReturnNumbers != true
if (pix_x_is_it_odd==0){
if (pix_y_is_it_odd==1){ReturnCache=prebufferCache[pix_X];}
else{ReturnCache=(prebufferCache[pix_X]+CurrentbufferCache[pix_X])*0.5;}
}else{
if (pix_y_is_it_odd==1){ReturnCache=(prebufferCache[pix_X] + prebufferCache[pix_X-1])*0.5;} 
// belowe her is else of bufferlineNumber !=pix_Y. so below here we have cached data and we pull from it!
else{ReturnCache=(prebufferCache[pix_X] + prebufferCache[pix_X-1]+CurrentbufferCache[pix_X] + CurrentbufferCache[pix_X-1])*0.25;} 
} 
#endif
#if nonfloatReturnNumbers == true
if (pix_x_is_it_odd==0){//for numbers being bit shifted we add 1 in case value is in bettween
if ( pix_y_is_it_odd==1){ReturnCache=prebufferCache[pix_X];}
else{ReturnCache=(prebufferCache[pix_X]+CurrentbufferCache[pix_X]+1)>>1;}
}else{
if (pix_y_is_it_odd==1){ReturnCache=(prebufferCache[pix_X]+ prebufferCache[pix_X-1]+1)>>1;} 
// belowe her is else of bufferlineNumber !=pix_Y. so below here we have cached data and we pull from it!
else{ReturnCache=(prebufferCache[pix_X] + prebufferCache[pix_X-1]+CurrentbufferCache[pix_X] + CurrentbufferCache[pix_X-1]+2)>>2;} 
}
#endif

}//all data here can be interpolated
}//rest of data

return ReturnCache;//we send a newly calcualted float back if number is not float it 8 times bigger


}
