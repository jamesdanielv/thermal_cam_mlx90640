all i ask is you give credit where credit do. to MIT, to original authors and to me for my time and effort here. you might be doing the next big thing and the work here might help or save you time.

this is just a folder that contains rom, as originally modified, 
and will containt changes i bring over that reduce ram usage and so forth


im going to try to have #define NEW_METHOND true, or false for verifying works the same in high memory and low memory usage up to a point. this just changes the method of data storage from mostly ram to mostly on the sensor. in all cases i will remove the extreme stack needed for addressing of pointers.

setting pixelmodeTrueTestModeFalse false will output raw values in C at 32x24 resolution.

As i go foward remapping how code uses memory versus flash calibtration data:
so using old memory map method and procedures #define NEWMETHOND false,
using new memory methods should be done by changing to #define NEWMETHOND true

also i doubled sensor resolution without increasing memory usage. output to terminal in image mode outputs 64x48. but really it is 63x47 because left value has nothing to compare and bottom values have nothing to compare too, for simplicity i just chose to in this case make them the same as the nex value.


there may be some errors relating to get_image function. it currently is not used or made to work with NEW_METHOD. but it will be working by next update. it is required for faster read of pixels as it does not need to convert data to temp.
