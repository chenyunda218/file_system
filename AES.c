// 來源為 http://www.codedata.com.tw/social-coding/aes/，授權為 GNU GPL 授權。
// 本程式將來源程式擴充成5種加密模式

// gcc AES.c -o aes.exe 編譯
// 參數說明
// -f 輸入檔案 -o 輸出檔案 -k 密碼 -e 加密 -d 解密 -m 模式
// aes.exe -f test.7z -o out.en -k test -e -m ECB

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BYTE unsigned char //宣告BYTE為unsigned char的別名
#define BLOCK 16 //BLOCK size

void init(int argc , char *argv[]); //初始化
void readArg(int argc, char *argv[]); //讀取參數

void SubBytes(BYTE buffer[]); //SubBytes
void SubBytesInv(BYTE buffer[]); //inverse SubBytes
void ShiftRows(BYTE buffer[]); //Shift Rows
void ShiftRowsInv(BYTE buffer[]); //inverse Shift Rows
void MixColumns(BYTE state[]); //MixColumns
void MixColumnsInv(BYTE state[]); //inverse MixColumns
void AddRoundKey(BYTE buffer[],BYTE key[]); //Add Round Key
void KeyExpansion(BYTE rkey[],BYTE key[]); //Key Expansion


void copyBuff(BYTE from[],BYTE to[]); //復制buffer
void xor(BYTE *buffer,BYTE *last); //xor
void increase(BYTE counter[]); //CTR增量函數

void enBLOCK(BYTE buffer[]); //块加密器
void deBLOCK(BYTE buffer[]); //块解密器

void enECB(char in[],char out[]); //ECB加密
void deECB(char in[],char out[]); //ECB解密
void enCBC(char in[],char out[]); //CBC加密
void deCBC(char in[],char out[]); //CBC解密
void enCFB(char in[],char out[]); //CFB加密
void deCFB(char in[],char out[]); //CFB解密
void CTR(char in[],char out[]); //CTR加密和解密
void enOFB(char in[],char out[]); //OFB加密
void deOFB(char in[],char out[]); //OFB解密

BYTE AES_Sbox[] = { 99,124,119,123,242,107,111,197,48,1,103,43,254,215,171,
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
  137,13,191,230,66,104,65,153,45,15,176,84,187,22 };
BYTE AES_SboxInv[256];

BYTE shiftTable[BLOCK] = {0,5,10,15,4,9,14,3,8,13,2,7,12,1,6,11};
BYTE shiftTableInv[BLOCK] = {0,13,10,7,4,1,14,11,8,5,2,15,12,9,6,3};

BYTE AES_xtime[256]; //MixColumns所使用的陣列
BYTE keyLen = 4; //key長度
char inputFile[100] = {0}; //輸入檔案
char outputFile[100] = {0}; //輸出檔案
BYTE key[32] = {0}; //key
BYTE rkey[160] = {0}; //round key
char mode[5] = "ECB"; //加密模式
char en = 1; //加解密旗號
char n = 1; //偏移量

int main(int argc , char *argv[]){
    init(argc,argv); //初始化
    //模式選擇流
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
        if(en){
            CTR(inputFile,outputFile);
        }else{
            CTR(inputFile,outputFile);
        }
    }else if(strcmp(mode,"CFB") == 0){
        if(en){
            enCFB(inputFile,outputFile);
        }else{
            deCFB(inputFile,outputFile);
        }
    }else if(strcmp(mode,"OFB") == 0){
        if(en){
            enOFB(inputFile,outputFile);
        }else{
            deOFB(inputFile,outputFile);
        }
    }
    char crypt[10];
    if(en){
        strcpy(crypt,"encrypted");
    }else{
        strcpy(crypt,"decrypted");
    }
    printf("%s %s output to %s by %s mode",crypt,inputFile,outputFile,mode);
    
    return 0;
}

void init(int argc , char *argv[]){
    strcpy(outputFile,"output");
    readArg(argc,argv); //讀取參數
    for(int i = 0 ;i<256;i++){ //初始化inverse Sbox
        AES_SboxInv[AES_Sbox[i]] = i;
    }
    for(int i = 0; i < 128; i++) { //初始化AES_xtime
        AES_xtime[i] = i << 1;
        AES_xtime[128 + i] = (i << 1) ^ 0x1b;
    }
    KeyExpansion(rkey,key); //Key Expansion
}

void SubBytes(BYTE buffer[]){ 
    for(int i=0;i<BLOCK;i++){
        buffer[i] = AES_Sbox[buffer[i]];
    }
}
void SubBytesInv(BYTE buffer[]){
    for(int i=0;i<BLOCK;i++){
        buffer[i] = AES_SboxInv[buffer[i]];
    }
}

void readArg(int argc, char *argv[]){
    for(int i = 1; i < argc; i++) {
        char *arg = argv[i];
        if(arg[0] == '-'){
            switch(arg[1]){
                case 'k': //讀取key
                    strcpy(key,argv[++i]);
                    break;
                case 'f': //讀取輸入檔案路徑
                    strcpy(inputFile,argv[++i]);
                    break;
                case 'm':  //讀取模式
                    strcpy(mode,argv[++i]);
                    break;
                case 'o': //讀取輸出檔案路徑
                    strcpy(outputFile,argv[++i]);
                    break;
                case 'd': //設為解密旗號
                    en = 0;
                    break;
                case 'e': //設為加密旗號
                    en = 1;
                    break;
                case 'l': //設定rkey長度
                    keyLen = atoi(argv[++i]);
                    if(keyLen != 4 && keyLen != 6 && keyLen != 8){
                        keyLen = 4;
                    }
                    break;
                case 'n': //設定偏移量
                    n = atoi(argv[++i]);
                    if(n>16) n = 16;
                    break;
            }
        }
    }
}

void copyBuff(BYTE from[],BYTE to[]){
    for(int i=0;i<BLOCK;i++){
        to[i] = from[i];
    }
}

void ShiftRows(BYTE buffer[]){
    BYTE temp[BLOCK];
    for(int i=0;i<BLOCK;i++){
        temp[i] = buffer[shiftTable[i]];
    }
    copyBuff(temp,buffer);
}


void ShiftRowsInv(BYTE buffer[]){
    BYTE temp[BLOCK];
    for(int i=0;i<BLOCK;i++){
        temp[i] = buffer[shiftTableInv[i]];
    }
    copyBuff(temp,buffer);
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

void AddRoundKey(BYTE buffer[],BYTE key[]){
    for(int i=1;i<BLOCK;i++){
        buffer[i] = buffer[i] ^ key[i];
    }
}

void xor(BYTE *buffer,BYTE *last){
    for(int i=0;i<BLOCK;i++){
        buffer[i] = buffer[i] ^ last[i];
    }
}

void KeyExpansion(BYTE rkey[],BYTE key[]){
    for(int i=0;i<16;i++){
        rkey[i] = key[i];
    }
    for(int i=16;i<160;i+=4){
        rkey[i] = AES_Sbox[rkey[i-3]] ^ rkey[i-16];
        rkey[i+1] = AES_Sbox[rkey[i+1-3]] ^ rkey[i+1-16];
        rkey[i+2] = AES_Sbox[rkey[i+2-3]] ^ rkey[i+2-16];
        rkey[i+2] = AES_Sbox[rkey[i-7]] ^ rkey[i+3-16];
    }
}

void enBLOCK(BYTE buffer[]){
    AddRoundKey(buffer,rkey);
    for(int i=1;i<10;i++){
        SubBytes(buffer);
        ShiftRows(buffer);
        MixColumns(buffer);
        AddRoundKey(buffer,&rkey[i*16]);
    }
    SubBytes(buffer);
    ShiftRows(buffer);
    AddRoundKey(buffer,&rkey[144]);
}

void deBLOCK(BYTE buffer[]){
    AddRoundKey(buffer,&rkey[144]);
    ShiftRowsInv(buffer);
    SubBytesInv(buffer);
    for(int i=9;i>=1;i--){
        AddRoundKey(buffer,&rkey[i*16]);
        MixColumnsInv(buffer);
        ShiftRowsInv(buffer);
        SubBytesInv(buffer);
    }
    AddRoundKey(buffer,rkey);
}

void enECB(char in[],char out[]){
    FILE * filer, * filew;
    BYTE buffer[BLOCK];
    filer=fopen(in,"rb");
    filew=fopen(out,"wb");
    fseek(filer,0,SEEK_END);
    int size = ftell(filer);
    int body = size / BLOCK;
    int tail = size % BLOCK;
    fwrite(&tail,1,1,filew);
    rewind(filer);
	for(int i=0;i<body;i++){
        fread(buffer,1,BLOCK,filer);
        enBLOCK(buffer);
        fwrite(buffer,1,BLOCK,filew);
    }
    if(tail > 0){
        fread(buffer,1,BLOCK,filer);
        enBLOCK(buffer);
        fwrite(buffer,1,BLOCK,filew);
    }
    fclose(filer);
    fclose(filew);
}

void deECB(char in[],char out[]){
    FILE * filer, * filew;
    BYTE buffer[BLOCK];
    filer=fopen(in,"rb");
    filew=fopen(out,"wb");
    fseek(filer,0,SEEK_END);
    int size = ftell(filer);
    int body = size / BLOCK - 1;
    int tail;
    rewind(filer);
    fread(&tail,1,1,filer);
    for(int i=0;i<body;i++){
        fread(buffer,1,BLOCK,filer);
        deBLOCK(buffer);
        fwrite(buffer,1,BLOCK,filew);
    }
    if(tail > 0){
        fread(buffer,1,BLOCK,filer);
        deBLOCK(buffer);
        fwrite(buffer,1,tail,filew);
    }
    fclose(filer);
    fclose(filew);
}

void enCBC(char in[],char out[]){
    FILE * filer, * filew;
    int numr;
    BYTE buffer[BLOCK];
    BYTE last[BLOCK] = {2,3,1,1,1,2,3,1,1,1,2,3,3,1,1,2};
    filer=fopen(in,"rb");
    filew=fopen(out,"wb");
    while(feof(filer)==0){
        if((numr=fread(buffer,1,BLOCK,filer)) == BLOCK){
            xor(buffer,last);
            enBLOCK(buffer);
            copyBuff(buffer,last);
        }
        fwrite(buffer,1,numr,filew);
    }
    fclose(filer);
    fclose(filew);
}

void deCBC(char in[],char out[]){
    FILE * filer, * filew;
    int numr,numw;
    BYTE buffer[BLOCK];
    BYTE last[BLOCK]={2,3,1,1,1,2,3,1,1,1,2,3,3,1,1,2};
    BYTE next[BLOCK]={2,3,1,1,1,2,3,1,1,1,2,3,3,1,1,2};
    filer=fopen(in,"rb");
    filew=fopen(out,"wb");
    while(feof(filer)==0){
        if((numr=fread(buffer,1,BLOCK,filer)) == BLOCK){
            copyBuff(buffer,next);
            deBLOCK(buffer);
            xor(buffer,last);
            copyBuff(next,last);
        }
        fwrite(buffer,1,numr,filew);
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
    BYTE counter[BLOCK] = {2,3,1,1,1,2,3,1,1,1,2,3,3,1,1,2};
    BYTE last[BLOCK];
    filer=fopen(in,"rb");
	filew=fopen(out,"wb");
	while(feof(filer)==0){
        if((numr=fread(buffer,1,BLOCK,filer)) == BLOCK){
            copyBuff(counter,last);
            enBLOCK(last);
            xor(buffer,last);
            increase(counter);
        }
        numw=fwrite(buffer,1,numr,filew);
    }
    fclose(filer);
    fclose(filew);
}

void push(BYTE *buffer,BYTE *iv){
    BYTE temp[BLOCK];
    copyBuff(buffer, temp);
    for(int j=0;j<n;j++){
        for(int i=0;i<15;i++){
            iv[i] = iv[i+1];
        }
        iv[15] = buffer[0];
        for(int i=0;i<15;i++){
            temp[i] = temp[i+1];
        }
    }
}

void enCFB(char in[],char out[]){
    FILE * filer, * filew;
    int numr,numw;
    BYTE buffer[BLOCK];
    BYTE iv[BLOCK] = {2,3,1,1,1,2,3,1,1,1,2,3,3,1,1,2};
    filer=fopen(in,"rb");
    filew=fopen(out,"wb");
	while(feof(filer)==0){
        if((numr=fread(buffer,1,BLOCK,filer)) == BLOCK){
            enBLOCK(iv);
            xor(buffer, iv);
            push(buffer, iv);
        }
        fwrite(buffer,1,numr,filew);
    }
    fclose(filer);
    fclose(filew);
}

void deCFB(char in[],char out[]){
    FILE * filer, * filew;
    int numr,numw;
    BYTE buffer[BLOCK];
    BYTE last[BLOCK] = {2,3,1,1,1,2,3,1,1,1,2,3,3,1,1,2};
    BYTE iv[BLOCK] = {2,3,1,1,1,2,3,1,1,1,2,3,3,1,1,2};
    filer=fopen(in,"rb");
    filew=fopen(out,"wb");
	while(feof(filer)==0){
        if((numr=fread(buffer,1,BLOCK,filer)) == BLOCK){
            enBLOCK(iv);
            copyBuff(buffer,last);
            xor(buffer, iv);
            push(last,iv);
        }
        fwrite(buffer,1,numr,filew);
    }
    fclose(filer);
    fclose(filew);
}

void enOFB(char in[],char out[]){
    FILE * filer, * filew;
    int numr,numw;
    BYTE buffer[BLOCK];
    BYTE iv[BLOCK] = {2,3,1,1,1,2,3,1,1,1,2,3,3,1,1,2};
    filer=fopen(in,"rb");
    filew=fopen(out,"wb");
	while(feof(filer)==0){
        if((numr=fread(buffer,1,BLOCK,filer)) == BLOCK){
            enBLOCK(iv);
            xor(buffer, iv);
            push(iv, iv);
        }
        numw=fwrite(buffer,1,numr,filew);
    }
    fclose(filer);
    fclose(filew);
}

void deOFB(char in[],char out[]){
    FILE * filer, * filew;
    int numr,numw;
    BYTE buffer[BLOCK];
    BYTE iv[BLOCK] = {2,3,1,1,1,2,3,1,1,1,2,3,3,1,1,2};
    filer=fopen(in,"rb");
    filew=fopen(out,"wb");
	while(feof(filer)==0){
        if((numr=fread(buffer,1,BLOCK,filer)) == BLOCK){
            enBLOCK(iv);
            xor(buffer, iv);
            push(iv, iv);
        }
        numw=fwrite(buffer,1,numr,filew);
    }
    fclose(filer);
    fclose(filew);
}
