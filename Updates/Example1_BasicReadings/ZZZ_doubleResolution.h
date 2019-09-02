//ZZZ_ heading just keeps this showing towards the right of tabs
//this is a small process that does not use much memory and increases resolution of data in
// however it requires fast ram reads of data in order to not store its own table
//it also requires modification of main structure so it can read data
//it just interpolates in between data by averaging creating a new pixel 
//this is not my zoom method. this is just a quick method. some data resolution
// is lost on the edges. it is effectively a resdouble-1
// do not remove commends from this line and above. written james villeneuve
#define XresolutionOriginal 32 //this is original resolution
#define YresolutionOriginal 24 //this is original resolution
//we use below as cache. we only update information 1/4 of time

float DoubleResolutionValue(uint8_t XatNewresolution,uint8_t YatNewresolution){
float  Originaltemp;
float   PixeltoLEFTtemp;
float  PreviousRowtemp;
//this routine uses data from values it can get from sensor reads to double resolution of data without using much memory
//we just need to know the ram we are reading, the x and y resolution, and pixel number
//this is an over simplified setup that interpolates every other value in x and y
//**** we need mem value to pull data from*****************************************************
//****                                    *****************************************************
//we need actual pixel values of non doubled data for ref
#include "MLX90640_API.h" //we need this so we can access the memory
uint8_t pix_X=XatNewresolution>>1;
uint8_t pix_Y=YatNewresolution>>1;
//we need to determin if new pixel is odd or even. odd numbers are interpolated with nearest left side value
uint8_t  pix_x_is_it_odd =XatNewresolution &(uint8_t)1;
uint8_t  pix_y_is_it_odd =YatNewresolution &(uint8_t)1;


Originaltemp=Readmlx90640To(pix_X+pix_Y*XresolutionOriginal);//this is original
if (XatNewresolution !=0){PixeltoLEFTtemp=Readmlx90640To(pix_X-1+pix_Y*XresolutionOriginal); }else{PixeltoLEFTtemp=Readmlx90640To(pix_X+pix_Y*XresolutionOriginal);}
if (YatNewresolution !=0){PreviousRowtemp=Readmlx90640To(pix_X+(pix_Y-1)*XresolutionOriginal);}else{PreviousRowtemp=Readmlx90640To(pix_X+pix_Y*XresolutionOriginal);}
//****                                    *****************************************************
//********************************************************************************************* 
//below assigns interpolation of odd pixels only. it is not the best method but effective 
if ((pix_x_is_it_odd =0) & (pix_y_is_it_odd =0)){return Originaltemp;} //we return wiht pixel calc
if ((pix_x_is_it_odd =1) & (pix_y_is_it_odd =0)){return (Originaltemp+PixeltoLEFTtemp)*0.5;} //we return wiht pixel calc
if ((pix_x_is_it_odd =0) & (pix_y_is_it_odd =1)){return (Originaltemp+PreviousRowtemp)*0.5;} //we return wiht pixel calc
if ((pix_x_is_it_odd =1) & (pix_y_is_it_odd =1)){return (Originaltemp+Originaltemp+PreviousRowtemp+PixeltoLEFTtemp)*0.25;} //we return wiht pixel calc
}
