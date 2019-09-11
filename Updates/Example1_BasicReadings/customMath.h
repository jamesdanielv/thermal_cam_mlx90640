#include "Z_MemManagment.h"//tells us what mem managment we are using, old or new
//this takes results of 2^0 to 2^64 and then applies a 1/result it allows multiply to be applied when it normally would be a divide. and it avoids the cost of POW
//this takes half the time if it is a table. so it will switch to that.

const float inverse_math[] PROGMEM = { 
1,
0.5,
0.25,
0.125,
0.0625,
0.03125,
0.015625,
0.0078125,
0.00390625,
0.001953125,
0.0009765625,
0.00048828125,
0.000244140625,
0.0001220703125,
0.00006103515625,
0.000030517578125,
0.0000152587890625,
0.00000762939453125,
0.000003814697265625,
0.0000019073486328125,
9.5367431640625e-7,
4.76837158203125e-7,
2.384185791015625e-7,
1.1920928955078125e-7,
5.960464477539063e-8,
2.9802322387695312e-8,
1.4901161193847656e-8,
7.450580596923828e-9,
3.725290298461914e-9,
1.862645149230957e-9,
9.313225746154785e-10,
4.656612873077393e-10,
2.3283064365386963e-10,
1.1641532182693481e-10,
5.820766091346741e-11,
2.9103830456733704e-11,
1.4551915228366852e-11,
7.275957614183426e-12,
3.637978807091713e-12,
1.8189894035458565e-12,
9.094947017729282e-13,
4.547473508864641e-13,
2.2737367544323206e-13,
1.1368683772161603e-13,
5.684341886080802e-14,
2.842170943040401e-14,
1.4210854715202004e-14,
7.105427357601002e-15,
3.552713678800501e-15,
1.7763568394002505e-15,
8.881784197001252e-16,
4.440892098500626e-16,
2.220446049250313e-16,
1.1102230246251565e-16,
5.551115123125783e-17,
2.7755575615628914e-17,
1.3877787807814457e-17,
6.938893903907228e-18,
3.469446951953614e-18,
1.734723475976807e-18,
8.673617379884035e-19,
4.336808689942018e-19,
2.168404344971009e-19,
1.0842021724855044e-19,
};
const float power_of2table[] PROGMEM = { 
1,
2,
4,
8,
16,
32,
64,
128,
256,
512,
1024,
2048,
4096,
8192,
16384,
32768,
65536,
131072,
262144,
524288,
1048576,
2097152,
4194304,
8388608,
16777216,
33554432,
67108864,
134217728,
268435456,
536870912,
1073741824,
2147483648,
4294967296,
8589934592,
17179869184,
34359738368,
68719476736,
137438953472,
274877906944,
549755813888,
1099511627776,
2199023255552,
4398046511104,
8796093022208,
17592186044416,
35184372088832,
70368744177664,
140737488355328,
281474976710656,
562949953421312,
1125899906842624,
2251799813685248,
4503599627370496,
9007199254740992,
18014398509481984,
36028797018963970,
72057594037927940,
144115188075855870,
288230376151711740,
576460752303423500,
1152921504606847000,
2305843009213694000,
4611686018427388000,
9223372036854776000,
};


float SimplePowFast2sInverse(uint8_t x){//we cause  multiply instead of division will move this into PROGMEM later
//we need to do 2^48 so at least 49 values, but we can go to 64. table will be generated from javascript and in project folder  
//float value;

return pgm_read_float_near(inverse_math+ x);
}
float SimplePowFast2s(uint8_t x){//we cause 2^x
return pgm_read_float_near(power_of2table+ x);

}

float Q_rsqrt( float number ) //a good enough square root method. if not enabled in Z_memManagment then it will work as regular sqrt and use math library
{
  #if USE_FAST_SQUARERT_METHOD == true
  //https://en.wikipedia.org/wiki/Fast_inverse_square_root
  long i;float x2, y;const float threehalfs = 1.5F;
  x2 = number * 0.5F;
  y  = number;
  i  = * ( long * ) &y;                       // evil floating point bit level hacking
  i  = 0x5f3759df - ( i >> 1 );               // what the #!$% (deleted bad language)
  y  = * ( float * ) &i;
  y  = y * ( threehalfs - ( x2 * y * y ) );   // 1st iteration
  //y  = y * ( threehalfs - ( x2 * y * y ) );   // 2nd iteration, this can be removed
  return 1/y;//we hate to use div.. this is slower than the bult in method..

  #else 

  return sqrt(number);
  #endif
}
