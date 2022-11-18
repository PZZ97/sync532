#define CL_HPP_CL_1_2_DEFAULT_BUILD
#define CL_HPP_TARGET_OPENCL_VERSION 120
#define CL_HPP_MINIMUM_OPENCL_VERSION 120
#define CL_HPP_ENABLE_PROGRAM_CONSTRUCTION_FROM_ARRAY_COMPATIBILITY 1
#define CL_USE_DEPRECATED_OPENCL_1_2_APIS

#include <CL/cl2.hpp>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <vector>
#include "Utilities.h"
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
using namespace ::std;
#define NUM_PACKETS 8
#define pipe_depth 4
#define DONE_BIT_L (1 << 7)
#define DONE_BIT_H (1 << 15)

int offset = 0;
unsigned char* file;
uint8_t encode(string binaryFile,uint8_t * output_buf, uint8_t* input_buf2, int inlength, int * outlength ){
    *outlength =0;  // initialize output length
	int q_index[inlength]={-1};	// -1 indicates unwrite value
	unsigned int index=0;
    unsigned char input_buf[inlength];
    for(int i=0;i<inlength;i++)
        input_buf[i]=input_buf2[i];
    ////////////////////////////////////////
   std::cout<<"start encode"<<endl;
    // ------------------------------------------------------------------------------------
    // Step 1: Initialize the OpenCL environment
     // ------------------------------------------------------------------------------------
    cl_int err;
    // std::string binaryFile = argv[1];
    unsigned fileBufSize;
    std::vector<cl::Device> devices = get_xilinx_devices();
    devices.resize(1);
    cl::Device device = devices[0];
    cl::Context context(device, NULL, NULL, NULL, &err);
    char *fileBuf = read_binary_file(binaryFile, fileBufSize);
    cl::Program::Binaries bins{{fileBuf, fileBufSize}};
    cl::Program program(context, devices, bins, NULL, &err);
    cl::CommandQueue q(context, device, CL_QUEUE_PROFILING_ENABLE, &err);
    cl::Kernel krnl_cdc(program, "cdc", &err);
    // ------------------------------------------------------------------------------------
    // Step 2: Create buffers and initialize test values
    // ------------------------------------------------------------------------------------

// void cdc(unsigned char* buff, unsigned int buff_size, int * chunk_q){

    cl::Buffer a_buf;
    // cl::Buffer b_buf[NUM_MAT];
    cl::Buffer c_buf;

    a_buf = cl::Buffer(context, CL_MEM_READ_ONLY, (inlength*sizeof(unsigned char)), NULL, &err);

    c_buf = cl::Buffer(context, CL_MEM_WRITE_ONLY,( inlength*sizeof(int)), NULL, &err);
    

    // float *a[NUM_MAT];

    // float *b[NUM_MAT];
    // float *c[NUM_MAT];
    // unsigned char* buf 
    // int* q_index;
    // 
    for(int i=0;i<inlength;i++){
        q_index[i]=0;
    }
    input_buf = (unsigned char*)q.enqueueMapBuffer(a_buf, CL_TRUE, CL_MAP_WRITE, 0, (inlength*sizeof(unsigned char)));
    q_index = (int*)q.enqueueMapBuffer(c_buf, CL_TRUE, CL_MAP_READ, 0, (inlength*(sizeof(int))));

    // ------------------------------------------------------------------------------------
    // Step 3: Run the kernel
    // ------------------------------------------------------------------------------------

    std::vector<cl::Event> write_events;

    std::vector<cl::Event> exec_events, read_events;
    cl::Event write_ev, exec_ev, read_ev;

    krnl_cdc.setArg(0, a_buf);
    krnl_cdc.setArg(1, inlength);
    krnl_cdc.setArg(2, c_buf);

    // if(i == 0)
    // {
        q.enqueueMigrateMemObjects({a_buf, c_buf}, 0 /* 0 means from host*/, NULL, &write_ev);
    // }
    // else
    // {
    //     q.enqueueMigrateMemObjects({a_buf[i%NUM_MAT], b_buf[i%NUM_MAT]}, 0 /* 0 means from host*/, &write_events,
    //     &write_ev);
    //     write_events.pop_back();
    // }

    write_events.push_back(write_ev);
    q.enqueueTask(krnl_cdc, &write_events, &exec_ev);
    exec_events.push_back(exec_ev);
    q.enqueueMigrateMemObjects({c_buf}, CL_MIGRATE_MEM_OBJECT_HOST, &exec_events, &read_ev);
    read_events.push_back(read_ev);

    q.finish();

    // ------------------------------------------------------------------------------------
    // Step 4: Release Allocated Resources
    // ------------------------------------------------------------------------------------

    std::cout << "--------------- Total time ---------------"<<endl;

    ///////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////


    // cdc(input_buf,inlength,q_index); 
    
    std::string s_packet(reinterpret_cast<char*>(input_buf));  // convert input(unsigned char) to string saving in  $s_packet$
    CHUNK_pos_t chunk_start_pos= 0;
    CHUNK_pos_t chunk_end_pos=-1;
    while(q_index[index]!=-1){    // pop out each chunk and manipulate each chunk in order 
            chunk_start_pos=chunk_end_pos+1;
            // cout<<"chunk_start_pos="<<chunk_start_pos<<endl;
			chunk_end_pos=q_index[index];
			CHUNK_idx_t chunk_unique_id = index++;
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
	encode(argv[1],&file[offset],&buffer[HEADER],length,&output_length);
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
		encode(argv[1],&file[offset],&buffer[HEADER],length,&output_length);
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