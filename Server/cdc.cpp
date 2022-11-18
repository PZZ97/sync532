#include "encoder.h"
#include <math.h>
#include <hls_stream.h>

#define PRIME 3
#define WIN_SIZE 16
#define MODULUS 256
#define TARGET 0
using namespace std;
typedef int CHUNK_idx_t;  // index of unique chunk
typedef int CHUNK_pos_t;  // index of chunk end pos in packet buffer
// typedef queue<array<int, 2>> IDXQ; 
void read(unsigned char* buff, hls::stream<unsigned char> & buffstream, unsigned int buff_size){
	for(unsigned int i=0;i<buff_size;i++){
		buffstream.write(buff[i]);
	}
}
void exec0(hls::stream<unsigned char> & buffstream,hls::stream<int>& out, unsigned int buff_size){
		int buff[WIN_SIZE];
		//CHUNK_idx_t chunk_index=0;
		for(int j=0; j<WIN_SIZE; j++){
			buff[j]=buffstream.read();
		}
		for(int j=0; j<WIN_SIZE-1; j++){
			buff[j]=buffstream.read();
		}

		int offset=WIN_SIZE-1;
		for(int i=WIN_SIZE; i<buff_size-WIN_SIZE; i++){
			buff[(i+offset)%WIN_SIZE]=buffstream.read();
			offset++;
			offset%=WIN_SIZE;
			// hash_func
			uint64_t hash = 0;
			for(int j=i; j<WIN_SIZE; j++){
				hash += int(buff[WIN_SIZE - 1 - j]) * pow(PRIME, j + 1);
			}
			if ((hash % MODULUS) == TARGET) {
				out.write(i-1);
			}
		}
		out.write(buff_size-1);
		out.write(-1);
//		for(int i=WIN_SIZE; i<2 * WIN_SIZE-1; i++){
//			buff[i]=buffstream.read();
//		}
//
//		CHUNK_idx_t chunk_index=0;
//		uint64_t hash = 0;
//		for (int j = 0; j < WIN_SIZE; j++) {
//			hash += int(buff[2 * WIN_SIZE - 1 - j]) * pow(PRIME, j + 1);
//		}
//
//		int param = pow(PRIME, WIN_SIZE + 1);
//		if ((hash % MODULUS) == TARGET) {
//			chunk_q[chunk_index++] = (WIN_SIZE-1);
//		}
//
//
//		for (unsigned int i = WIN_SIZE+1; i < buff_size - WIN_SIZE; i+=WIN_SIZE) {
//			for(int i=WIN_SIZE; i<2 * WIN_SIZE-1; i++){
//				buff[i]=buffstream.read();
//			}
//			hash = hash * PRIME - int(buff[i-1]) * param + int(buff[i-1 + WIN_SIZE]) * PRIME;
//			if ((hash % MODULUS) == TARGET) {
//				chunk_q[chunk_index++] = (i-1);
//			}
//		}
//			chunk_q[chunk_index++] = buff_size-1;

}
void exec(hls::stream<unsigned char> & buffstream,hls::stream<int>& out, unsigned int buff_size){
		int buff[WIN_SIZE];
		//CHUNK_idx_t chunk_index=0;
		for(int j=0; j<WIN_SIZE; j++){
			buff[j]=buffstream.read();
		}
		for(int j=0; j<WIN_SIZE-1; j++){
			buff[j]=buffstream.read();
		}

		int offset=WIN_SIZE-1;
		for(int i=WIN_SIZE; i<buff_size-WIN_SIZE; i++){
			buff[(i+offset)%WIN_SIZE]=buffstream.read();
			offset++;
			offset%=WIN_SIZE;
			// hash_func
			uint64_t hash = 0;
			for(int j=i; j<WIN_SIZE; j++){
				hash += int(buff[WIN_SIZE - 1 - j]) * pow(PRIME, j + 1);
			}
			if ((hash % MODULUS) == TARGET) {
				out.write(i-1);
			}
		}
		out.write(buff_size-1);
		out.write(-1);
}

void write(hls::stream<int>& out, int*  chunk_q){
	unsigned int index=0;
	int value = out.read();
	while(value!=-1){
		chunk_q[index++]=value;
		//cout << index << endl;
		value = out.read();
	}

}
void cdc(unsigned char* buff, unsigned int buff_size, int * chunk_q){
	int chunk_index=0;
	uint64_t hash = 0;
	for (unsigned int i = WIN_SIZE; i < buff_size - WIN_SIZE; i++) {
#pragma HLS unroll
		hash = 0;
		for (int j = 0; j < WIN_SIZE; j++) {
#pragma HLS pipeline
			hash += int(buff[i + WIN_SIZE - 1 - j]) * pow(PRIME, j + 1);
		} //hash_func
		if ((hash % MODULUS) == TARGET) {
			chunk_q[chunk_index++] = (i-1);
		}
	}
	chunk_q[chunk_index++] = buff_size-1;  // the last chunk
}
void cdc0(unsigned char* buff, unsigned int buff_size, int*  chunk_q){
	hls::stream<unsigned char> inStream;
	hls::stream<int> outStream;
	read(buff, inStream, buff_size);
	exec(inStream, outStream, buff_size);
	write(outStream, chunk_q);
}
//void cdc(unsigned char* buff, unsigned int buff_size, int*  chunk_q)
//{
//#pragma HLS ARRAY_PARTITION variable=buff block factor=2
//    CHUNK_idx_t chunk_index=0;
//	uint64_t hash = 0;
//	int tmp1, tmp2;
//	for (int j = 0; j < WIN_SIZE; j++) {
//#pragma HLS unroll
//			hash += int(buff[2 * WIN_SIZE - 1 - j]) * pow(PRIME, j + 1);
//	}
//	int param = pow(PRIME, WIN_SIZE + 1);
//	if ((hash % MODULUS) == TARGET) {
//		chunk_q[chunk_index++] = (WIN_SIZE-1);
//	}
//	for (unsigned int i = WIN_SIZE+1; i < buff_size - WIN_SIZE; i++) {
//#pragma HLS pipeline
//		tmp1 = int(buff[i-1]) * param;
//		tmp2 = int(buff[i-1 + WIN_SIZE]) * PRIME;
//        hash = hash * PRIME - tmp1 + tmp2;
//		if ((hash % MODULUS) == TARGET) {
//			chunk_q[chunk_index++] = (i-1);
//		}
//	}
////    chunk_q.push({chunk_index++,buff_size-1});  // the last chunk
//		chunk_q[chunk_index++] = buff_size-1;
//}