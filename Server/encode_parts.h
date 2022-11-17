#ifndef __SOURCE_H__
#define __SOURCE_H__


#include <queue>

#include <string>
#include <array>
#include <vector>
#include "lzw.h"
// #include <wolfssl/wolfcrypt/sha.h>
typedef int CHUNK_idx_t;  // index of unique chunk
typedef int CHUNK_pos_t;  // index of chunk end pos in packet buffer
typedef std::queue<std::array<int, 2>> IDXQ;    // {CHUNK_idx_t,CHUNK_pos_t}

//typedef std::array<unsigned char,HASH_SIZE> HASH;
typedef std::string HASH;
//function call inside CDC
uint64_t hash_func(unsigned char* input, unsigned int pos);
uint64_t hash_func2(unsigned char* input, unsigned int pos, uint64_t hash_res);

// CDC
/*
    @ buff： array acquired from get_packet()
    @ buff_size: buff size
    @ chunk_index: queue used to store pair of <chunk end position in the buff, unique chunk number>
*/
// void cdc(unsigned char* buff, unsigned int buff_size, queue<int>& chunk_index);
void cdc(unsigned char* buff, unsigned int buff_size, IDXQ& chunk_index);



// #define CHUNK_SIZE 64
// #define TOTAL_LEN_LEN 8
// uint32_t right_rot(uint32_t value, unsigned int count)
// void SHA_256(CHUNK_pos_t chunk_index, char* packet, unsigned int packet_size, HASH& hash_value)
/*
    @ begin: chunk start position
    @ end: chunk end position
    @ packet:   packet array
    @ packet_size: packet size
    @ hash_value: OUTPUT
*/

// void SHA_384_HW(CHUNK_pos_t begin,CHUNK_pos_t end, unsigned char* packet, unsigned int packet_size, HASH& hash_value);
void SHA_HW( uint8_t* message,CHUNK_pos_t  chunk_start,CHUNK_pos_t chunk_end,  HASH *digest_hash);


// deduplication
CHUNK_idx_t deduplication(CHUNK_idx_t chunk_index,HASH& hash_value);


// LZW
/*
    @ chunk_start： current chunk's start position
    @ chunk_end:    current chunk's end position
    @ s1 : chunk in string type
    @ packet_size: packet size
    @ output_code: output
*/

// void LZW(int chunk_start,int chunk_end,std::string &s_packet,unsigned int packet_size,std::vector<unsigned char> &output_code);
// void LZW(int chunk_start,int chunk_end,std::string &s1,unsigned int packet_size,unsigned char*output_code,size_t * outlen);


uint8_t encode(uint8_t * output_buf,uint8_t* input_buf, int inlength, int * outlength );

#endif
