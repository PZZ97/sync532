#ifndef __LZW_H__
#define __LZW_H__

#include <string>
#include <array>

// LZW
/*
    @ chunk_start： current chunk's start position
    @ chunk_end:    current chunk's end position
    @ s1 : chunk in string type
    @ packet_size: packet size
    @ output_code: output
*/
                // LZW(chunk_start_pos,chunk_end_pos,s_packet,inlength,output_code,&outlen);

// void LZW(int chunk_start,int chunk_end,std::string &s_packet,unsigned int packet_size,std::vector<unsigned char> &output_code);
void LZW(int chunk_start,int chunk_end,std::string &s1,unsigned int packet_size,unsigned char*output_code,size_t * outlen);

#endif
