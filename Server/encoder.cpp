#include "encoder.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include "server.h"
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "stopwatch.h"
#include "encode_parts.h"
#include <array>
// using namespace ::std;
#define NUM_PACKETS 8
#define pipe_depth 4
#define DONE_BIT_L (1 << 7)
#define DONE_BIT_H (1 << 15)

int offset = 0;
unsigned char* file;

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
            SHA_HW(input_buf,chunk_start_pos,chunk_end_pos, &hash_value);
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
void handle_input(int argc, char* argv[], int* blocksize) {
	int x;
	extern char *optarg;

	while ((x = getopt(argc, argv, ":b:")) != -1) {
		switch (x) {
		case 'b':
			*blocksize = atoi(optarg);
			printf("blocksize is set to %d optarg\n", *blocksize);
			break;
		case ':':
			printf("-%c without parameter\n", optopt);
			break;
		}
	}
}

int main(int argc, char* argv[]) {
	stopwatch ethernet_timer;
	unsigned char* input[NUM_PACKETS];
	int writer = 0;
	int done = 0;
	int length = 0;
	int count = 0;
	ESE532_Server server;

	// default is 2k
	int blocksize = BLOCKSIZE;

	// set blocksize if decalred through command line
	handle_input(argc, argv, &blocksize);

	file = (unsigned char*) malloc(sizeof(unsigned char) * 70000000);
	if (file == NULL) {
		printf("help\n");
	}

	for (int i = 0; i < NUM_PACKETS; i++) {
		input[i] = (unsigned char*) malloc(
				sizeof(unsigned char) * (NUM_ELEMENTS + HEADER));
		if (input[i] == NULL) {
			std::cout << "aborting " << std::endl;
			return 1;
		}
	}

	server.setup_server(blocksize);

	writer = pipe_depth;
	server.get_packet(input[writer]);
	count++;

	// get packet
	unsigned char* buffer = input[writer];

	// decode
	done = buffer[1] & DONE_BIT_L;
	length = buffer[0] | (buffer[1] << 8);
	length &= ~DONE_BIT_H;
	// printing takes time so be weary of transfer rate
	//printf("length: %d offset %d\n",length,offset);

	// we are just memcpy'ing here, but you should call your
	// top function here.
	// memcpy(&file[offset], &buffer[HEADER], length);
	int output_length=0;	
	encode(&file[offset],&buffer[HEADER],length,&output_length);
	offset += output_length;
	writer++;

	//last message
	while (!done) {
		// reset ring buffer
		if (writer == NUM_PACKETS) {
			writer = 0;
		}

		ethernet_timer.start();
		server.get_packet(input[writer]);
		ethernet_timer.stop();

		count++;
		// get packet
		unsigned char* buffer = input[writer];

		// decode
		done = buffer[1] & DONE_BIT_L;
		length = buffer[0] | (buffer[1] << 8);
		length &= ~DONE_BIT_H;
		//printf("length: %d offset %d\n",length,offset);
		// memcpy(&file[offset], &buffer[HEADER], length);
		encode(&file[offset],&buffer[HEADER],length,&output_length);
		offset += output_length;
		writer++;
	}

	// write file to root and you can use diff tool on board
	FILE *outfd = fopen("output_cpu.bin", "wb");
	int bytes_written = fwrite(&file[0], 1, offset, outfd);
	printf("write file with %d\n", bytes_written);
	fclose(outfd);

	for (int i = 0; i < NUM_PACKETS; i++) {
		free(input[i]);
	}

	free(file);
	std::cout << "--------------- Key Throughputs ---------------" << std::endl;
	float ethernet_latency = ethernet_timer.latency() / 1000.0;
	float input_throughput = (bytes_written * 8 / 1000000.0) / ethernet_latency; // Mb/s
	std::cout << "Input Throughput to Encoder: " << input_throughput << " Mb/s."
			<< " (Latency: " << ethernet_latency << "s)." << std::endl;

	return 0;
}