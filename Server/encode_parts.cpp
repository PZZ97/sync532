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
//	unordered_map <int,int> uma2;
	// put your hash function implementation here
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
            // printf("->");
            // for(int jj=-10;jj<=0;jj++){
            //     printf("%c",buff[i-1+jj]);
            // }
		}
	}
    chunk_q.push({chunk_index++,buff_size-1});  // the last chunk


}

// unsigned int k[64] = {
// 	0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
// 	0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
// 	0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
// 	0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
// 	0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
// 	0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
// 	0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
// 	0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
// };

// uint32_t right_rot(uint32_t value, unsigned int count)
// {
// 	return value >> count | value << (32 - count);
// }

void SHA_HW(char* message, char*digest){
    // https://edstem.org/us/courses/27305/discussion/2053707
    char shaSum[HASH_SIZE];
    Sha sha;
    wc_InitSha(&sha);
    wc_ShaUpdate(&sha, (const unsigned char*)message, strlen(message)); 
    wc_ShaFinal(&sha, (unsigned char*)digest);

}


CHUNK_idx_t deduplication(CHUNK_idx_t chunk_index,HASH& hash_value){
    static unordered_map<HASH,CHUNK_idx_t> umap;
    CHUNK_idx_t idx= umap[hash_value];
    if(idx!=0)  // get value
        return idx-1;

    umap[hash_value] =chunk_index+1;
    return -1;  
}

void LZW(int chunk_start,int chunk_end,string &s1,int packet_size,unsigned char*output_code,size_t * outlen){
    memset(output_code,0,(chunk_end-chunk_start+1)*2);
    unordered_map<string, int> table;
    // build the original table 
    for (int i = 0; i <= 255; i++) {
        string ch = "";
        ch += char(i);
        table[ch] = i;
    }
    string p = "", c = "";
    p += s1[chunk_start];
    unsigned int code = 256;

    *outlen=0;
    int appeartimes=0;
    for (int i = chunk_start; i <=chunk_end; i++) {
        if (i != chunk_end)
            c += s1[i + 1];
        if (table.find(p + c) != table.end()) {
            p = p + c;
        }
        else {
            
            if(appeartimes==0){
                output_code[(*outlen)++] |=( table[p]  &0b1111111100000)>>5;
                output_code[(*outlen)] |= (table[p]    &0b0000000011111)<<3;
            }
            else if(appeartimes==1){
                output_code[(*outlen)++]|=(table[p]  &0b1110000000000)>>10;
                output_code[(*outlen)++]|=(table[p]  &0b0001111111100)>>2;
                output_code[(*outlen)]|=(table[p]    &0b0000000000011)<<6;
            }
            else if(appeartimes==2){
                output_code[(*outlen)++]|=(table[p]  &0b1111110000000)>>7;
                output_code[(*outlen)]|=(table[p]    &0b0000001111111)<<1;
            }
            else if(appeartimes==3){
                output_code[(*outlen)++]|=(table[p]  &0b1000000000000)>>12;
                output_code[(*outlen)++]|=(table[p]  &0b0111111110000)>>4;
                output_code[(*outlen)]|=(table[p]    &0b0000000001111)<<4;
            }
            else if(appeartimes==4){
                output_code[(*outlen)++]|=(table[p]  &0b1111000000000)>>9;
                output_code[(*outlen)++]|=(table[p]  &0b0000111111110)>>1;
                output_code[(*outlen)]|=(table[p]    &0b0000000000001)<<7;
            }
            else if(appeartimes==5){
                output_code[(*outlen)++]|=(table[p]  &0b1111111000000)>>6;
                output_code[(*outlen)]|=(table[p]  &0b0000000111111)<<2;
            }
            else if(appeartimes==6){
                output_code[(*outlen)++]|=(table[p]  &0b1100000000000)>>11;
                output_code[(*outlen)++]|=(table[p]  &0b0011111111000)>>3;
                output_code[(*outlen)]|=(table[p]    &0b0000000000111)<<5;
            }                
            else if(appeartimes==7){
                output_code[(*outlen)++]|=(table[p]    &0b1111100000000)>>8;
                output_code[(*outlen)++]|=(table[p]    &0b0000011111111);
            }           
            appeartimes++;
            appeartimes%=8;
            
            table[p + c] = code;
            code++;
            p = c;
        }
        c = "";
    }
    if(appeartimes==0){
        output_code[(*outlen)++] |=( table[p]  &0b1111111100000)>>5;
        output_code[(*outlen)] |= (table[p]    &0b0000000011111)<<3;
    }
    else if(appeartimes==1){
        output_code[(*outlen)++]|=(table[p]  &0b1110000000000)>>10;
        output_code[(*outlen)++]|=(table[p]  &0b0001111111100)>>2;
        output_code[(*outlen)]|=(table[p]    &0b0000000000011)<<6;
    }
    else if(appeartimes==2){
        output_code[(*outlen)++]|=(table[p]  &0b1111110000000)>>7;
        output_code[(*outlen)]|=(table[p]    &0b0000001111111)<<1;
    }
    else if(appeartimes==3){       
        output_code[(*outlen)++]|=(table[p]  &0b1000000000000)>>12;
        output_code[(*outlen)++]|=(table[p]  &0b0111111110000)>>4;
        output_code[(*outlen)]|=(table[p]    &0b0000000001111)<<4;
    }
    else if(appeartimes==4){
        output_code[(*outlen)++]|=(table[p]  &0b1111000000000)>>9;
        output_code[(*outlen)++]|=(table[p]  &0b0000111111110)>>1;
        output_code[(*outlen)]|=(table[p]    &0b0000000000001)<<7;
    }
    else if(appeartimes==5){
        output_code[(*outlen)++]|=(table[p]  &0b1111111000000)>>6;
        output_code[(*outlen)]|=(table[p]  &0b0000000111111)<<2;
    }
    else if(appeartimes==6){
        output_code[(*outlen)++]|=(table[p]  &0b1100000000000)>>11;
        output_code[(*outlen)++]|=(table[p]  &0b0011111111000)>>3;
        output_code[(*outlen)]|=(table[p]    &0b0000000000111)<<5;
    }                
    else if(appeartimes==7){
        output_code[(*outlen)++]|=(table[p]    &0b1111100000000)>>8;
        output_code[(*outlen)]|=(table[p]    &0b0000011111111);
    }
    if((*outlen)%13!=0)
        output_code[++(*outlen)]=0;

}

/* &file[offset]->output_buf */

uint8_t encode(uint8_t * output_buf, uint8_t* input_buf, int inlength, int * outlength ){

    *outlength =0;  // initialize output length
    IDXQ q_chunk;   // q_chunk: queue saves each chunk by identifing each chunk end position and chunk ID

    cdc(input_buf,inlength,q_chunk); 
    std::string s_packet(reinterpret_cast<char*>(input_buf));  // convert input(unsigned char) to string saving in  $s_packet$
    CHUNK_pos_t chunk_start_pos= 0;
    CHUNK_pos_t chunk_end_pos=-1;
    while(q_chunk.size()>0){    // pop out each chunk and manipulate each chunk in order 
            chunk_start_pos=chunk_end_pos+1;
            // cout<<"chunk_start_pos="<<chunk_start_pos<<endl;
            array<CHUNK_idx_t,2> index =q_chunk.front();
            CHUNK_idx_t chunk_unique_id = index[0];
            chunk_end_pos= index[1];
            q_chunk.pop();
            HASH hash_value;
            char message[inlength];
            for(int i=chunk_start_pos;i<=chunk_end_pos;i++){
                message[i-chunk_start_pos]=input_buf[i];
            }
            char tmp[HASH_SIZE+1];
            strcpy(tmp,hash_value.c_str());
            SHA_HW(message,tmp);
            hash_value=tmp;
        
            CHUNK_idx_t sent =deduplication(chunk_unique_id,hash_value);
            if(sent ==-1 ){
                unsigned char* output_code = (unsigned char*) malloc(sizeof(unsigned char)*((chunk_end_pos-chunk_start_pos+1)*2));
                size_t outlen;
                LZW(chunk_start_pos,chunk_end_pos,s_packet,inlength,output_code,&outlen);
                // printf("\noutlen=%08x",outlen); /* outlen: length includes padding*/
                union {
                    uint32_t header;
                    uint8_t arr[4];
                }u;
                u.header = (uint32_t)outlen<<1;
                // printf("\nencode Header:%08x",u.header);
                memcpy(&output_buf[*outlength],u.arr,4);
                (*outlength)+=4;
                memcpy(&output_buf[*outlength],output_code,outlen);
                (*outlength)+=outlen;
                free(output_code);
            }
            else{
                // printf("\ndedup");
                union {
                    uint32_t header;
                    uint8_t arr[4];
                }u;
                u.header = sent<<1;
                u.header|=1;
                memcpy(&output_buf[*outlength],u.arr,4);
                (*outlength)+=4;
            }
        }
    return 0;
}
