#include "encode_parts.h"
#include <unordered_map>
#include <iostream>
#include <wolfssl/options.h>
#include <wolfssl/wolfcrypt/sha3.h>
#include <math.h>
using namespace std;
#define HASH_SIZE SHA3_384_DIGEST_SIZE
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
		}
	}

}

unsigned int k[64] = {
	0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
	0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
	0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
	0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
	0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
	0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
	0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
	0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

uint32_t right_rot(uint32_t value, unsigned int count)
{
	return value >> count | value << (32 - count);
}
/*
void SHA_256(CHUNK_idx_t q_chunk_index, char* packet, unsigned int packet_size, HASH* hash_value)
{
	int h[8] = { 0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a, 0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19 };
	unsigned i, j;

    string chunk = packet;
    chunk.append(1);
    while (chunk.length < 512){
        chunk.append(0);
    }

    uint32_t ah[8];

    const uint8_t *p = chunk;

   //  Initialize working variables to current hash value: 
    for (i = 0; i < 8; i++)
        ah[i] = h[i];

     //Compression function main loop: 
    for (i = 0; i < 4; i++) {
        uint32_t w[16];

        for (j = 0; j < 16; j++) {
            if (i == 0) {
                w[j] = (uint32_t) p[0] << 24 | (uint32_t) p[1] << 16 |
                    (uint32_t) p[2] << 8 | (uint32_t) p[3];
                p += 4;
            } else {
                 Extend the first 16 words into the remaining 48 words w[16..63] of the message schedule array: 
                const uint32_t s0 = right_rot(w[(j + 1) & 0xf], 7) ^ right_rot(w[(j + 1) & 0xf], 18) ^ (w[(j + 1) & 0xf] >> 3);
                const uint32_t s1 = right_rot(w[(j + 14) & 0xf], 17) ^ right_rot(w[(j + 14) & 0xf], 19) ^ (w[(j + 14) & 0xf] >> 10);
                w[j] = w[j] + s0 + w[(j + 9) & 0xf] + s1;
            }
            const uint32_t s1 = right_rot(ah[4], 6) ^ right_rot(ah[4], 11) ^ right_rot(ah[4], 25);
            const uint32_t ch = (ah[4] & ah[5]) ^ (~ah[4] & ah[6]);
            const uint32_t temp1 = ah[7] + s1 + ch + k[i << 4 | j] + w[j];
            const uint32_t s0 = right_rot(ah[0], 2) ^ right_rot(ah[0], 13) ^ right_rot(ah[0], 22);
            const uint32_t maj = (ah[0] & ah[1]) ^ (ah[0] & ah[2]) ^ (ah[1] & ah[2]);
            const uint32_t temp2 = s0 + maj;

            ah[7] = ah[6];
            ah[6] = ah[5];
            ah[5] = ah[4];
            ah[4] = ah[3] + temp1;
            ah[3] = ah[2];
            ah[2] = ah[1];
            ah[1] = ah[0];
            ah[0] = temp1 + temp2;
        }
    }

    // Add the compressed chunk to the current hash value: 
    for (i = 0; i < 8; i++)
        h[i] += ah[i];
	

// Produce the final hash value: 
	for (i = 0, j = 0; i < 8; i++)
	{
		hash_value[j++] = (uint8_t) (h[i] >> 24);
		hash_value[j++] = (uint8_t) (h[i] >> 16);
		hash_value[j++] = (uint8_t) (h[i] >> 8);
		hash_value[j++] = (uint8_t) h[i];
	}
}
*/
// void SHA_384_HW(CHUNK_pos_t begin,CHUNK_pos_t end, unsigned char* packet, unsigned int packet_size, HASH& hash_value){
//     // https://edstem.org/us/courses/27305/discussion/2053707
//     //
//     unsigned char shaSum[SHA3_384_DIGEST_SIZE];
//     wc_Sha3 sha3_384;
//     wc_InitSha3_384(&sha3_384,NULL,INVALID_DEVID);
//     // printf("")
//     wc_Sha3_384_Update(&sha3_384, (packet+begin), end-begin+1);  // not sure about boundary
//     wc_Sha3_384_Final(&sha3_384, shaSum);
//     printf("shaSum=%s\n",shaSum);

//     for(int i=0;i<SHA3_384_DIGEST_SIZE;i+=2){
// //        hash_value[i]=shaSum[i];
//         hash_value+=shaSum[i];
//     }
// }

void SHA_384_HW_2(CHUNK_pos_t begin,CHUNK_pos_t end, unsigned char* packet, unsigned int packet_size, HASH& hash_value){
    // https://edstem.org/us/courses/27305/discussion/2053707
    //
    unsigned char shaSum[SHA3_384_DIGEST_SIZE];
    wc_Sha3 sha3_384;
    wc_InitSha3_384(&sha3_384,NULL,INVALID_DEVID);
    // printf("")
    wc_Sha3_384_Update(&sha3_384, (const unsigned char*)packet, end-begin+1);  // not sure about boundary
    wc_Sha3_384_Final(&sha3_384, shaSum);
    // printf("shaSum=%s\n",shaSum);

    for(int i=0;i<SHA3_384_DIGEST_SIZE;i+=2){
//        hash_value[i]=shaSum[i];
    printf("%x",shaSum[i]);
        hash_value+=shaSum[i];
    }
}
CHUNK_idx_t deduplication(CHUNK_idx_t chunk_index,HASH& hash_value){
    static unordered_map<HASH,CHUNK_idx_t> umap;
    CHUNK_idx_t idx= umap[hash_value];
    // index ranges from 0 to MAX, but we do not store 0, 
    // so make all idx ++ before stroe it
    if(idx!=0)  // get value
        return idx-1;

    umap[hash_value] =chunk_index+1;
    return -1;  
}

void LZW(int chunk_start,int chunk_end,string &s1,int packet_size,unsigned char*output_code,size_t * outlen){

    unordered_map<string, int> table;
    // build the original table 
    for (int i = 0; i <= 255; i++) {
        string ch = "";
        ch += char(i);
        table[ch] = i;
    }
    string p = "", c = "";
    p += s1[0];
    int code = 256;
    int length = chunk_end-chunk_start+1;
    *outlen=0;
    for (int i = 0; i <length; i++) {
        if (i != s1.length() - 1)
            c += s1[chunk_start+i + 1];
        if (table.find(p + c) != table.end()) {
            p = p + c;
        }
        else {
            // cout << p << "\t" << table[p] << "\t\t"
            //      << p + c << "\t" << code << endl;
            // output_code.push_back(table[p]);
            output_code[(*outlen)++] = table[p];
            table[p + c] = code;
            code++;
            p = c;
        }
        c = "";
    }
    // cout << p << "\t" << table[p] << endl;
    output_code[(*outlen)++]=table[p];
    // return output_code;
}


#ifdef __TESTMAIN__
IDXQ q; // queue for  chunk index & chunk end pos
int main(){


    // while(get buffer from packet): 
    // {
        //get_packet()
        CDC( packet,packet_size,  q);   // queue<pair<CHUNK_idx_t,int>> q;
        std::string s_packet(reinterpret_cast<char*>(packet));
        int start_pos= 0;
        int prev_end_pos=0;
        while(q.size()>0){
            pair<CHUNK_idx_t,CHUNK_pos_t> index =q.front();
            prev_end_pos= index.second;
            q.pop();
            if(start_pos!=0){
                start_pos=prev_end_pos+1;
            }
            
            // arrary<unsigned char,256bits>
            HASH hash_value;
            SHA_256(index.first, packet, packet_size,  hash_value);
            
            // return value of deduplication is unique chunk index if find index, else -1
            CHUNK_idx_t sent =deduplication(index.first,hash_value);
            if(sent ==-1 ){

                vector<unsigned char> output_code;
                LZW(start_pos,index.second,s_packet,packet_size,output_code);

                //send (output_code);
            }
            else{
                //send(sent);
            }
        }
    // }

}

#endif

/* &file[offset]->output_buf */

uint8_t encode(uint8_t * output_buf, uint8_t* input_buf, int inlength, int * outlength ){

    *outlength =0;  // initialize output length
    IDXQ q_chunk;   // q_chunk: queue saves each chunk by identifing each chunk end position and chunk ID

    cdc(input_buf,inlength,q_chunk); 
    std::string s_packet(reinterpret_cast<char*>(input_buf));  // convert input(unsigned char) to string saving in  $s_packet$
    CHUNK_pos_t chunk_start_pos= 0;
    CHUNK_pos_t chunk_end_pos=-1;
    while(q_chunk.size()>0){    // pop out each chunk and manipulate each chunk in order 
            // if(chunk_start_pos!=0){
                chunk_start_pos=chunk_end_pos+1;
            // }
            cout<<"chunk_start_pos="<<chunk_start_pos<<endl;
            // uint32_t header=0;  // LZW header
            array<CHUNK_idx_t,2> index =q_chunk.front();
            CHUNK_idx_t chunk_unique_id = index[0];
            chunk_end_pos= index[1];
            q_chunk.pop();

            cout<<"chunk id="<<chunk_unique_id<<"\tchunk size="<<chunk_end_pos-chunk_start_pos<<endl;

            // typedef std::array<unsigned char,HASH_SIZE> HASH
            HASH hash_value;
            for(int i=0;i<HASH_SIZE;i++)
                printf("%x",hash_value[i]);
            printf("\n") ;  
            char message[inlength];
            for(int i=chunk_start_pos;i<chunk_end_pos+1;i++)
                message[i-chunk_start_pos]=input_buf[i];

            // SHA_384_HW(chunk_start_pos,chunk_end_pos,input_buf,inlength,hash_value);
            SHA_384_HW_2(chunk_start_pos,chunk_end_pos,message,inlength,hash_value);
            
            // cout<<"HASH value="<<hash_value<<endl;
            for(int i=0;i<HASH_SIZE;i++)
                printf("%x",hash_value[i]);
            printf("\n") ;   
            // return value of deduplication is unique chunk index if find index, else -1
            CHUNK_idx_t sent =deduplication(chunk_unique_id,hash_value);
            if(sent ==-1 ){
                
                // vector<unsigned char> output_code;
                unsigned char* output_code = (unsigned char*) malloc(sizeof(unsigned char)*(chunk_end_pos-chunk_start_pos+1));
                cout<<"#generate LZW"<<endl;//, output code[0:5]="<<output_code[0]<<output_code[1]<<output_code[2]<<output_code[3]<<output_code[4]<<endl;
            
                size_t outlen;
                LZW(chunk_start_pos,chunk_end_pos,s_packet,inlength,output_code,&outlen);
                //send (output_code);
                outlen--;
                union {
                    uint32_t header;
                    uint8_t arr[4];
                }u;
                u.header = (uint32_t)outlen<<1;
                memcpy(&output_buf[*outlength],u.arr,4);
                (*outlength)+=4;
                cout <<"LZWheader ="<< u.header <<"\t"<<"arr[0]="<<u.arr[0]<<"\tarr[3]="<<u.arr[3]<< endl;
              
                memcpy(&output_buf[*outlength],output_code,outlen);
                *outlength+=outlen;
            }
            else{
                union {
                    uint32_t header;
                    uint8_t arr[4];
                }u;
                u.header = sent<<1;
                u.header|=1;
                // u.header=0x80000000|sent;
                cout <<"\t#repeat chunk, chunk id="<< (u.header>>1) <<"\t"<< endl;
                memcpy(&output_buf[*outlength],u.arr,4);
                (*outlength)+=4;
                //send(sent);
                // output_buf[(*outlength)++]=(unsigned char)sent;
                // memcpy(&output_buf[*outlength],output_code,outlen);
            }
        }
    return 0;
}
