//this just tells us mem method to use
#define NEW_METHOD true
#define USE_FAST_SQUARERT_METHOD false //if this is set to true uses a different method to roots. however it throws an error, but still compiles also typing fastest instead of true has an even faster method
#define customSmallCacheForMemReads true //if true this creates a small cache for ram data reads. only works in NEW_METHOD mode. 
#define limitRangeofMath true //this fixes some calculations to allow working on 8bit processors. no effect on 32bit
#define Replace_detailed_calc_with_image_data false//this switched out complex ta calc for one that is only good enough for image results but should be faster using less math
#define NormalizeImageValue 1/128 //1/16 -1/256  this centers image value. it just puts image within range of c temp.
