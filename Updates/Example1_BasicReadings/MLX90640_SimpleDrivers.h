
uint8_t fast_2_to_powerof0_15( uint8_t x){
//since we know pow of 2^ is only 16 values we simplify
float value;//this stores value
switch (x){
  case 0: value=1;break;case 1: value=2;break;case 2: value=4;break;case 3: value=8;break;case 4: value=16;break;case 5: value=32;break;case 6: value=64;break;case 7: value=128;break;case 8: value=256;break;
  case 9: value=512;break;case 10: value=1024;break;case 11: value=2048;break;case 12: value=4096;break;case 13: value=8192;break;case 14: value=16384;break;case 15: value=32768;break;
}
return value;// done quick hopefully.  
}
