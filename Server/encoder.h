#ifndef _ENCODER_H_
#define _ENCODER_H_

// max number of elements we can get from ethernet
#define NUM_ELEMENTS 16384
#define HEADER 2

#include<string>
void LZW(int chunk_start,int chunk_end,std::string &s1,int packet_size,unsigned char*output_code,size_t * outlen);
#endif
