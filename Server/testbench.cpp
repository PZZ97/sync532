#include "encode_parts.h"
#include <iostream>
#include<string>
#include <unordered_map>
#include <math.h>
using namespace std;
void generate_str(string &s,size_t length){
    for(int i=0;i<length;i++)
        s+=(char)(rand()%128);        
}

void LZW_HW(int chunk_start,int chunk_end,string &s1,unsigned char*output_code,size_t * outlen){
    LZW(chunk_start, chunk_end,s1,0,output_code, outlen);
}
void LZW_SW(int chunk_start,int chunk_end,string &s1,unsigned char*output_code,size_t * outlen){
    
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
bool compare(unsigned char* code1,unsigned char*code2,size_t length){
    for(size_t i=0;i<length;i++){
        if(code1[i]!=code2[i])
            return false;
    }
    return true;
}
#define INPUT_SIZE 200
 int main(){
    string input="";
    generate_str(input,200);

    unsigned char* code1=(unsigned char*)malloc(sizeof(unsigned char)*INPUT_SIZE*2);
    unsigned char* code2=(unsigned char*)malloc(sizeof(unsigned char)*INPUT_SIZE*2);
    memset(code1,0,(INPUT_SIZE)*2);
    memset(code2,0,(INPUT_SIZE)*2);
    size_t len1,len2;
    LZW_HW(0,INPUT_SIZE-1,input,code1,&len1);
    LZW_SW(0,INPUT_SIZE-1,input,code2,&len2);
    if(len1!=len2)
    cout<<"length incorrect, failed"<<endl;
    if(compare(code1,code2,len1))
    cout<<"Passed!"<<endl;
    else 
    cout<<"Failed"<<endl;

 }