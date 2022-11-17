
#include "encode_parts.h"
#include <unordered_map>
#include <iostream>
#include <wolfssl/options.h>
#include <wolfssl/wolfcrypt/sha.h>
#include <math.h>

using namespace std;

#define HASH_SIZE SHA_DIGEST_SIZE
#define PRIME 3
#define WIN_SIZE 16
#define MODULUS 256
#define TARGET 0

uint64_t hash_func(unsigned char* input, unsigned int pos)
{
	uint64_t hash = 0;
	for (int i = 0; i < WIN_SIZE; i++) {
		hash += int(input[pos + WIN_SIZE - 1 - i]) * pow(PRIME, i + 1);
	}
	return hash;
}

uint64_t hash_func2(unsigned char* input, unsigned int pos, uint64_t hash_res)
{
	return hash_res * PRIME - int(input[pos-1]) * pow(PRIME, WIN_SIZE + 1) + int(input[pos-1 + WIN_SIZE]) * PRIME;
}

void cdc(unsigned char* buff, unsigned int buff_size, IDXQ& chunk_q)
{
    CHUNK_idx_t chunk_index=0; 
	uint64_t hash = 0;
	for (unsigned int i = WIN_SIZE; i < buff_size - WIN_SIZE; i++) {
		if (i == WIN_SIZE) {
			hash = hash_func(buff, i);
		}
		else {
			hash = hash_func2(buff, i, hash);
		}
		if ((hash % MODULUS) == TARGET) {
			chunk_q.push({chunk_index++,i-1});
		}
	}
    chunk_q.push({chunk_index++,buff_size-1});  // the last chunk
}

/*
void SHA_HW(char* message, char*digest){
    // https://edstem.org/us/courses/27305/discussion/2053707
    char shaSum[HASH_SIZE];
    Sha sha;
    wc_InitSha(&sha);
    wc_ShaUpdate(&sha, (const unsigned char*)message, strlen(message)); 
    wc_ShaFinal(&sha, (unsigned char*)digest);
}
*/
void SHA_HW( uint8_t* message,CHUNK_pos_t  chunk_start,CHUNK_pos_t chunk_end,  HASH *digest_hash){
    // https://edstem.org/us/courses/27305/discussion/2053707
    char digest[HASH_SIZE];
    Sha sha;
    wc_InitSha(&sha);
    wc_ShaUpdate(&sha, message+chunk_start,chunk_end-chunk_start+1 ); 
    wc_ShaFinal(&sha, (unsigned char*)digest);
    // cout<<"digest=";
    // for(int i=0;i<HASH_SIZE;i++)
    //     printf("%02x",digest[i]);
    // cout<<endl;
    *digest_hash=digest;
    
}

CHUNK_idx_t deduplication(CHUNK_idx_t chunk_index,HASH& hash_value){
    static unordered_map<HASH,CHUNK_idx_t> umap;
    CHUNK_idx_t idx= umap[hash_value];
    if(idx!=0)  // get value
        return idx-1;
    umap[hash_value] =chunk_index+1;
    return -1;  
}

