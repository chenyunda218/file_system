#include <stdio.h>
#include <string.h>

#define BYTE unsigned char
#define BLOCK 16
BYTE AES_Sbox[] = {99,124,119,123,242,107,111,197,48,1,103,43,254,215,171,
  118,202,130,201,125,250,89,71,240,173,212,162,175,156,164,114,192,183,253,
  147,38,54,63,247,204,52,165,229,241,113,216,49,21,4,199,35,195,24,150,5,154,
  7,18,128,226,235,39,178,117,9,131,44,26,27,110,90,160,82,59,214,179,41,227,
  47,132,83,209,0,237,32,252,177,91,106,203,190,57,74,76,88,207,208,239,170,
  251,67,77,51,133,69,249,2,127,80,60,159,168,81,163,64,143,146,157,56,245,
  188,182,218,33,16,255,243,210,205,12,19,236,95,151,68,23,196,167,126,61,
  100,93,25,115,96,129,79,220,34,42,144,136,70,238,184,20,222,94,11,219,224,
  50,58,10,73,6,36,92,194,211,172,98,145,149,228,121,231,200,55,109,141,213,
  78,169,108,86,244,234,101,122,174,8,186,120,37,46,28,166,180,198,232,221,
  116,31,75,189,139,138,112,62,181,102,72,3,246,14,97,53,87,185,134,193,29,
  158,225,248,152,17,105,217,142,148,155,30,135,233,206,85,40,223,140,161,
  137,13,191,230,66,104,65,153,45,15,176,84,187,22};
BYTE AES_SboxInv[256];

BYTE AES_ShiftRowTab[] = {0,5,10,15,4,9,14,3,8,13,2,7,12,1,6,11};
BYTE h[16] = {0};
BYTE test[] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
BYTE AES_xtime[256];
char inputFile[100] = {0};
char outputFile[100] = {0};
BYTE key[32] = {0};
char mode[5] = "ECB";
char en = 1;

void init(){
    for(int i = 0 ;i<256;i++){
       AES_SboxInv[AES_Sbox[i]] = i;
    }
    for(int i = 0; i < 128; i++) {
    AES_xtime[i] = i << 1;
    AES_xtime[128 + i] = (i << 1) ^ 0x1b;
  }
}

void printTest(){
    for(int i=0;i<BLOCK;i++){
        printf("%5d",test[i]);
    }
}

void SubBytes(BYTE buffer[BLOCK]){
    for(int i=0;i<BLOCK;i++){
        buffer[i] = AES_Sbox[buffer[i]];
    }
}
void SubBytesInv(BYTE buffer[BLOCK]){
    for(int i=0;i<BLOCK;i++){
        buffer[i] = AES_SboxInv[buffer[i]];
    }
}

void readArg(int argc, char *argv[]){
    for(int i = 1; i < argc; i++) {
        char *arg = argv[i]; 
        if(arg[0] == '-'){
            switch(arg[1]){
                case 'k':
                    strcpy(key,argv[++i]);
                    break;
                case 'f':
                    strcpy(inputFile,argv[++i]);
                    break;
                case 'm':
                    strcpy(mode,argv[++i]);
                    break;
                case 'o':
                    strcpy(outputFile,argv[++i]);
                    break;
                case 'd':
                    en = 0;
                    break;
                case 'e':
                    en = 1;
                    break;
            }
        }
    }
}

void dataSize(FILE *fp,int *dataBody,int *dataTail){
	fseek(fp,0L,SEEK_END);
	int total = ftell(fp);
	*dataBody = total/BLOCK;
	*dataTail = total%BLOCK;
}

void copyBuff(BYTE from[BLOCK],BYTE to[BLOCK]){
    for(int i=0;i<BLOCK;i++){
        to[i] = from[i];
    }
}



BYTE shiftTable[BLOCK] = {0,5,10,15,4,9,14,3,8,13,2,7,12,1,6,11};
void ShiftRows(BYTE buffer[BLOCK]){
    BYTE temp[BLOCK];
    for(int i=0;i<BLOCK;i++){
        temp[i] = buffer[shiftTable[i]];
    }
    copyBuff(temp,buffer);
}
BYTE reshiftTable[BLOCK] = {0,13,10,7,4,1,14,11,8,5,2,15,12,9,6,3};

void ShiftRowsInv(BYTE buffer[BLOCK]){
    BYTE temp[BLOCK];
    for(int i=0;i<BLOCK;i++){
        temp[i] = buffer[reshiftTable[i]];
    }
    copyBuff(temp,buffer);
}

void printSub(){
    for(int i=0;i<256;i++){
        printf("%3x",AES_Sbox[i]);
        if(i % 16 == 15){ printf("\n");}
    }
}

void printReSub(){
    for(int i=0;i<256;i++){
        printf("%3x",AES_SboxInv[i]);
        if(i % 16 == 15){ printf("\n");}
    }
}
void MixColumns(BYTE state[]) {
  for(int i = 0; i < 16; i += 4) {
    BYTE s0 = state[i + 0], s1 = state[i + 1];
    BYTE s2 = state[i + 2], s3 = state[i + 3];
    BYTE h = s0 ^ s1 ^ s2 ^ s3;
    state[i + 0] ^= h ^ AES_xtime[s0 ^ s1];
    state[i + 1] ^= h ^ AES_xtime[s1 ^ s2];
    state[i + 2] ^= h ^ AES_xtime[s2 ^ s3];
    state[i + 3] ^= h ^ AES_xtime[s3 ^ s0];
  }
}
void MixColumnsInv(BYTE state[]) {
    for(int i = 0; i < 16; i += 4) {
        BYTE s0 = state[i + 0], s1 = state[i + 1];
        BYTE s2 = state[i + 2], s3 = state[i + 3];
        BYTE h = s0 ^ s1 ^ s2 ^ s3;
        BYTE xh = AES_xtime[h];
        BYTE h1 = AES_xtime[AES_xtime[xh ^ s0 ^ s2]] ^ h;
        BYTE h2 = AES_xtime[AES_xtime[xh ^ s1 ^ s3]] ^ h;
        state[i + 0] ^= h1 ^ AES_xtime[s0 ^ s1];
        state[i + 1] ^= h2 ^ AES_xtime[s1 ^ s2];
        state[i + 2] ^= h1 ^ AES_xtime[s2 ^ s3];
        state[i + 3] ^= h2 ^ AES_xtime[s3 ^ s0];
    }
}

void AddRoundKey(BYTE buffer[BLOCK]){
    for(int i=0;i<BLOCK;i++){
        buffer[i] = buffer[i] ^ key[i];
    }
}
void xor(BYTE *buffer,BYTE *last){
    for(int i=0;i<BLOCK;i++){
        buffer[i] = buffer[i] ^ last[i];
    }
}

void enECBBLOCK(BYTE buffer[BLOCK]){
    AddRoundKey(buffer);
    for(int i=0;i<9;i++){
        SubBytes(buffer);
        ShiftRows(buffer);
        MixColumns(buffer);
        AddRoundKey(buffer);
    }
    SubBytes(buffer);
    ShiftRows(buffer);
    AddRoundKey(buffer);
}
void deECBBLOCK(BYTE buffer[BLOCK]){
    AddRoundKey(buffer);
    ShiftRowsInv(buffer);
    SubBytesInv(buffer);
    for(int i=0;i<9;i++){
        AddRoundKey(buffer);
        MixColumnsInv(buffer);
        ShiftRowsInv(buffer);
        SubBytesInv(buffer);
    }
    AddRoundKey(buffer);
}


void enECB(char in[],char out[]){
    FILE * filer, * filew;
	int numr,numw;
	BYTE buffer[BLOCK];
    BYTE last[BLOCK];
    filer=fopen(in,"rb");
	filew=fopen(out,"wb");
	while(feof(filer)==0){
        if((numr=fread(buffer,1,BLOCK,filer)) == BLOCK){
            enECBBLOCK(buffer);
        }
        
        numw=fwrite(buffer,1,numr,filew);
    }
	fclose(filer);
	fclose(filew);
}

void deECB(char in[],char out[]){
    FILE * filer, * filew;
	int numr,numw;
	BYTE buffer[BLOCK];
    filer=fopen(in,"rb");
    filew=fopen(out,"wb");
	while(feof(filer)==0){
        if((numr=fread(buffer,1,BLOCK,filer)) == BLOCK){
            deECBBLOCK(buffer);
        }
        numw=fwrite(buffer,1,numr,filew);
    }
	fclose(filer);
	fclose(filew);
}



void enCBC(char in[],char out[]){
    FILE * filer, * filew;
	int numr,numw;
	BYTE buffer[BLOCK];
    BYTE last[BLOCK];
    filer=fopen(in,"rb");
    filew=fopen(out,"wb");
    if(feof(filer) == 0){
        if((numr=fread(buffer,1,BLOCK,filer)) == BLOCK){
            MixColumns(buffer);
            AddRoundKey(buffer);
            copyBuff(buffer,last);
        }
        numw=fwrite(buffer,1,numr,filew);
    }
	while(feof(filer)==0){
        if((numr=fread(buffer,1,BLOCK,filer)) == BLOCK){
            xor(buffer,last);
            AddRoundKey(buffer);
            copyBuff(buffer,last);
        }
        numw=fwrite(buffer,1,numr,filew);
    }
	fclose(filer);
	fclose(filew);
}
void deCBC(char in[],char out[]){
    FILE * filer, * filew;
	int numr,numw;
	BYTE buffer[BLOCK];
    BYTE last[BLOCK];
    BYTE last2[BLOCK];
    filer=fopen(in,"rb");
    filew=fopen(out,"wb");
    if(feof(filer) == 0){
        if((numr=fread(buffer,1,BLOCK,filer)) == BLOCK){
            copyBuff(buffer,last);
            AddRoundKey(buffer);
            MixColumnsInv(buffer);
        }
        numw=fwrite(buffer,1,numr,filew);
    }
	while(feof(filer)==0){
        if((numr=fread(buffer,1,BLOCK,filer)) == BLOCK){
            copyBuff(buffer,last2);
            AddRoundKey(buffer);
            xor(buffer,last);
            copyBuff(last2,last);
        }
        numw=fwrite(buffer,1,numr,filew);
    }
	fclose(filer);
	fclose(filew);
}
void increase(BYTE counter[]){
    BYTE carry = 1;
    for(int i=0;i<16;i++){
        if(carry == 1 && counter[i] == 255){
            carry = 1;
            counter[i] = 0;
        }else{
            counter[i] += carry;
            carry = 0;
        }
    }
}
void CTR(char in[],char out[]){
    FILE * filer, * filew;
	int numr,numw;
	BYTE buffer[BLOCK];
    BYTE counter[BLOCK] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    BYTE last[BLOCK];
    filer=fopen(in,"rb");
	filew=fopen(out,"wb");
	while(feof(filer)==0){
        if((numr=fread(buffer,1,BLOCK,filer)) == BLOCK){
            copyBuff(counter,last);
            enECBBLOCK(last);
            xor(buffer,last);
            increase(counter);
        }
        numw=fwrite(buffer,1,numr,filew);
    }
	fclose(filer);
	fclose(filew);
}

int main(int argc , char *argv[]){
    init();
    readArg(argc,argv);
    if(strcmp(mode,"ECB") == 0){
        if(en){
            enECB(inputFile,outputFile);
        }else{
            deECB(inputFile,outputFile);
        }
    }else if(strcmp(mode,"CBC") == 0){
        if(en){
            enCBC(inputFile,outputFile);
        }else{
            deCBC(inputFile,outputFile);
        }
    }else if(strcmp(mode,"CTR") == 0){
        CTR(inputFile,outputFile);
    }
    
    

    return 0;
}
