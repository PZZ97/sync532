#include "encoder.h"

#define PRIME 3
#define WIN_SIZE 16
#define MODULUS 256
#define TARGET 0
typedef int CHUNK_idx_t;  // index of unique chunk
typedef int CHUNK_pos_t;  // index of chunk end pos in packet buffer
typedef std::queue<std::array<int, 2>> IDXQ; 

void cdc(unsigned char* buff, unsigned int buff_size, IDXQ& chunk_q)
{
    CHUNK_idx_t chunk_index=0; 
	uint64_t hash = 0;
	for (unsigned int i = WIN_SIZE; i < buff_size - WIN_SIZE; i++) {
		if (i == WIN_SIZE) {
	        for (int j = 0; j < WIN_SIZE; j++) {
		        hash += int(buff[i + WIN_SIZE - 1 - j]) * pow(PRIME, j + 1);
	        }
			//hash = hash_func(buff, i);
		}
		else {
            hash = hash * PRIME - int(buff[i-1]) * pow(PRIME, WIN_SIZE + 1) + int(buff[i-1 + WIN_SIZE]) * PRIME;
			//hash = hash_func2(buff, i, hash);
		}
		if ((hash % MODULUS) == TARGET) {
			chunk_q.push({chunk_index++,i-1});
		}
	}
    chunk_q.push({chunk_index++,buff_size-1});  // the last chunk
}