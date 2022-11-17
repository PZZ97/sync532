#ifndef __LZW1_H__
#define __LZW1_H__

#include <string>
#include <array>
#include <queue>
// LZW
/*
    @ chunk_start： current chunk's start position
    @ chunk_end:    current chunk's end position
    @ s1 : chunk in string type
    @ packet_size: packet size
    @ output_code: output
*/
typedef int CHUNK_idx_t;  // index of unique chunk
typedef int CHUNK_pos_t;  // index of chunk end pos in packet buffer
typedef std::queue<std::array<int, 2>> IDXQ;    // {CHUNK_idx_t,CHUNK_pos_t}

// void LZW(int chunk_start,int chunk_end,std::string &s_packet,unsigned int packet_size,std::vector<unsigned char> &output_code);
void LZW(CHUNK_pos_t chunk_start,CHUNK_pos_t chunk_end,std::string &s1,unsigned int packet_size,unsigned char*output_code,size_t * outlen);

#endif
