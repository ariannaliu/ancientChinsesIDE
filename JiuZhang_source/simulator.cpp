#include "simulator.h"
#include "assember.h"
#include "GUI.h"
#include "number.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cstring>
#include <algorithm>
#include <iostream>
#include <fcntl.h>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <string>
#include <vector>
#include <cmath>
#include <map>
#include <windows.h>
#include <string.h>
#include <stdlib.h>
#include <QApplication>

using namespace std;


bool findData(ifstream & infile){
    string line;
    bool flag = false;
    while(getline(infile,line)){
        if (!line.empty()){
            line.erase(0,line.find_first_not_of(" "));     //remove empty space
            line.erase(line.find_last_not_of(" ") + 1);
            if (line.substr(0,5) == ".data") flag = true; break;
        }
    }
    return flag;
}

//把假地址变成真的地址
intptr_t fakeToReal(int fakeDecimalAddr, void* pBaseAddr){
    int offset = fakeDecimalAddr - 4194304;     //offset between fake addr and Ox00400000
    intptr_t realAddr = intptr_t(pBaseAddr) + offset;
    return realAddr;
}

int realToFake(intptr_t realDecimalAddr, void* pBaseAddr){
    intptr_t pBA = reinterpret_cast<intptr_t>(pBaseAddr);
    int offset = realDecimalAddr - pBA;
    int fakeAddr = 0x400000 + offset;
    return fakeAddr;
}

int binaryStringToInt(string s){              //two's complement binary string to signed integer
    reverse(s.begin(),s.end());
    int count = 0;
    for(int i = 0; i < s.length()-1; i++){
        if(s[i] == '1'){
            count = count + pow(2,i);
        }
    }
    if(s[31] == '1') count = count - pow(2,31);
    return count;
}

int binaryStringToInt16(string s){              //two's complement binary string to signed integer
    reverse(s.begin(),s.end());
    int count = 0;
    for(int i = 0; i < s.length()-1; i++){
        if(s[i] == '1'){
            count = count + pow(2,i);
        }
    }
    if(s[15] == '1') count = count - pow(2,15);
    return count;
}

string decCodeToBin(int dec){                        //int >>> bin 2's complement
    string result;
    int temp = (dec >= 0) ? dec : abs(dec+1);
    if (temp==0) result="0";
    while(temp){
        result = to_string(temp % 2) + result;
        temp = temp / 2;
    }
    if (dec >= 0) {
        int count = result.length();
        for (int i = 0; i < (32-count); i++){
            result = "0"+result;
        }
    }
    else {       //dec<0
        for (int i=0;i < result.length();i++){
            result[i] = (result[i] == '0') ? '1' : '0';
        }
        int count = result.length();
        for (int i = 0; i < (32-count); i++){
            result = "1"+result;
        }
    }
    return result;
}


string GbkToUtf8(const char *src_str)
{
    int len = MultiByteToWideChar(CP_ACP, 0, src_str, -1, NULL, 0);
    wchar_t* wstr = new wchar_t[len + 1];
    memset(wstr, 0, len + 1);
    MultiByteToWideChar(CP_ACP, 0, src_str, -1, wstr, len);
    len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
    char* str = new char[len + 1];
    memset(str, 0, len + 1);
    WideCharToMultiByte(CP_UTF8, 0, wstr, -1, str, len, NULL, NULL);
    string strTemp = str;
    if (wstr) delete[] wstr;
    if (str) delete[] str;
    return strTemp;
}

void* dataSection(ifstream & infile,void* pBaseAddr){
    char *dataSegment = static_cast<char *>(pBaseAddr);
    dataSegment +=  1048576;                      //move the pointer to the bottom of the data sgment
    void *ptrVoid = static_cast<void *>(dataSegment);

    string line;
    int commentPos = 0;
    int labelPos = 0;
    vector<string> lineElements;
    while(getline(infile, line)){
        if (!line.empty()){
            commentPos = line.find('#');
            if (commentPos >= 0) line = line.substr(0, commentPos);                               //remove comment
            labelPos = line.find(':');
            if (labelPos >= 0) line = line.substr(labelPos + 1, line.length() - labelPos - 1);    //remove label
            line.erase(0,line.find_first_not_of(" "));                        //after remove, delete the empty space
            line.erase(line.find_last_not_of(" ") + 1);
            if (line.substr(0,5) == ".data") continue;
            if (line.substr(0,5) == ".text") break;

            if(line.substr(0,7) == ".asciiz"){
                line.erase(0,7);
                vector<string> lineElements;
                split(line, lineElements,"\"");
                string strInData = lineElements[1];     //remove latter "
                int space = ceil((strInData.size()+double(1))/4)*4;
                memcpy(ptrVoid, strInData.c_str(), strInData.size()+1);
                intptr_t ptrVoidInt = reinterpret_cast<intptr_t>(ptrVoid);
                ptrVoid = reinterpret_cast<void*>(ptrVoidInt+space);
            }
            else if(line.substr(0,6) == ".ascii"){
                line.erase(0,6);
                vector<string> lineElements;
                split(line, lineElements,"\"");
                string strInData = lineElements[1];
                int space = ceil((strInData.size())/double(4))*4;
                for (int i = 0; i < strInData.size(); i++){
                    char element = strInData[i];
                    memcpy(ptrVoid, &element, 1);
                    intptr_t ptrVoidInt = reinterpret_cast<intptr_t>(ptrVoid);
                    ptrVoid = reinterpret_cast<void*>(ptrVoidInt+1);

                }
                intptr_t ptrVI = reinterpret_cast<intptr_t>(ptrVoid);
                ptrVoid = reinterpret_cast<void*>(ptrVI+space-strInData.size());
            }
            else if(line.substr(0,5) == ".word"){

                line.erase(0,5);
                line.erase(0,line.find_first_not_of(" "));                        //after remove, delete the empty space
                line.erase(line.find_last_not_of(" ") + 1);
                while (true) {
                    int commaPos = line.find(',');
                    if (commaPos < 0) break;
                    line = line.replace(commaPos, 1, " ");
                }
                vector<string> lineElements;
                split(line, lineElements," ");
                int* ptrInt = static_cast<int *>(ptrVoid);
                for(int i = 0; i < lineElements.size(); i++){
                    int element = stoi(lineElements[i]);
                    *ptrInt = element;
                    ptrInt++;
                }
                ptrVoid = reinterpret_cast<void*>(ptrInt);
            }
            else if(line.substr(0,5) == ".half"){
                line.erase(0,5);
                line.erase(0,line.find_first_not_of(" "));                        //after remove, delete the empty space
                line.erase(line.find_last_not_of(" ") + 1);
                while (true) {
                    int commaPos = line.find(',');
                    if (commaPos < 0) break;
                    line = line.replace(commaPos, 1, " ");
                }
                vector<string> lineElements;
                split(line, lineElements," ");
                uint16_t * ptr = static_cast<uint16_t *>(ptrVoid);
                for(int i = 0; i < lineElements.size(); i++){
                    uint16_t element = stoi(lineElements[i]);
                    *ptr = element;
                    ptr++;
                }
                ptrVoid = static_cast<void*>(ptr);
            }
            else if(line.substr(0,5) == ".byte"){                       //byte被订正，没有赋值只保留内容位置
                line.erase(0,5);
                line.erase(0,line.find_first_not_of(" "));                        //after remove, delete the empty space
                line.erase(line.find_last_not_of(" ") + 1);

                uint8_t * ptr = static_cast<uint8_t *>(ptrVoid);
                ptr++;
                ptrVoid = static_cast<void*>(ptr);
            }else if(line.substr(0,8) == ".jiuzhang"){          //新增.jiuzhang文件格式
                line.erase(0,8);
                vector<string> lineElements;
                split(line, lineElements,"\"");
                string strInData = lineElements[1];     //remove latter "
                strInData = GbkToUtf8(strInData.c_str());
                int space = ceil((strInData.size()+double(1))/4)*4;
                memcpy(ptrVoid, strInData.c_str(), strInData.size()+1);
                intptr_t ptrVoidInt = reinterpret_cast<intptr_t>(ptrVoid);
                ptrVoid = reinterpret_cast<void*>(ptrVoidInt+space);
            }else if(line.substr(0,4) == ".int"){               //新增.int格式，没有赋值只保留内存位置
                int* ptrInt = static_cast<int *>(ptrVoid);
                ptrInt++;
                ptrVoid = reinterpret_cast<void*>(ptrInt);
            }
        }
    }
    return ptrVoid;
}

void putBin(ifstream & infile, int* &tptr){
    string line;
    while (getline(infile, line)){
        if( !line.empty() ){
            line.erase(0,line.find_first_not_of(" "));
            line.erase(line.find_last_not_of(" ") + 1);   //掐头去尾

            string binaryString = line;
            int mipsInt = binaryStringToInt(binaryString);
            *tptr = mipsInt;                                //mips code变成整数，放到模拟内存中
            tptr += 1;                                      //指针指向下一行
        }
    }
}





void add(int reg[34], int rs, int rt, int rd){
    bool flag = __builtin_add_overflow(reg[rs], reg[rt], &reg[rd]);
    if(flag == true){
        cout << "overflow error" << endl;
        exit(1);
    }

}

void addu(int reg[34], int rs, int rt, int rd){
    unsigned int zuo = reg[rs];
    unsigned int you = reg[rt];
    reg[rd] = zuo + you;
}

void andFun(int reg[34], int rs, int rt, int rd){
    string srs = decCodeToBin(reg[rs]);
    string srt = decCodeToBin(reg[rt]);
    string result;
    for (int i =0; i<32; i++){
        if(srs[i] == '1' && srt[i] == '1'){
            result = result +"1";
        }else{
            result = result + "0";
        }
    }
    reg[rd] = binaryStringToInt(result);
}

void norFun(int reg[34], int rs, int rt, int rd){
    string srs = decCodeToBin(reg[rs]);
    string srt = decCodeToBin(reg[rt]);
    string result;
    for (int i =0; i<32; i++){
        if(srs[i] == '0' && srt[i] == '0'){
            result = result +"1";
        }else{
            result = result + "0";
        }
    }
    reg[rd] = binaryStringToInt(result);
}

void orFun(int reg[34], int rs, int rt, int rd){
    string srs = decCodeToBin(reg[rs]);
    string srt = decCodeToBin(reg[rt]);
    string result;
    for (int i =0; i<32; i++){
        if(srs[i] == '0' && srt[i] == '0'){
            result = result +"0";
        }else{
            result = result + "1";
        }
    }
    reg[rd] = binaryStringToInt(result);
}

void sllv(int reg[34], int rs, int rt, int rd){
    string srt = decCodeToBin(reg[rt]);
    rs = reg[rs];
    string result = srt.substr(rs, srt.length()-rs);
    for (int i =0; i < rs; i++){
        result = result + "0";
    }
    reg[rd] = binaryStringToInt(result);
}

void srav(int reg[34], int rs, int rt, int rd){
    string srt = decCodeToBin(reg[rt]);
    rs = reg[rs];
    string result = srt.substr(0, srt.length()-rs);
    if(srt[0] == '1'){
        for (int i =0; i < rs; i++){
            result = "1" + result;
        }
    }else{
        for (int i =0; i < rs; i++){
            result = "0" + result;
        }
    }
    reg[rd] = binaryStringToInt(result);
}

void srlv(int reg[34], int rs, int rt, int rd){
    string srt = decCodeToBin(reg[rt]);
    rs = reg[rs];
    string result = srt.substr(0, srt.length()-rs);
    for (int i =0; i < rs; i++){
        result ="0" + result;
    }
    reg[rd] = binaryStringToInt(result);
}

void sub(int reg[34], int rs, int rt, int rd){
    bool flag = __builtin_sub_overflow(reg[rs], reg[rt], &reg[rd]);
    if (flag == true){
        cout << "overflow error"<<endl;
        exit(1);
    }
}

void subu(int reg[34], int rs, int rt, int rd){
    unsigned int zuo = reg[rs];
    unsigned int you = reg[rt];
    reg[rd] = zuo - you;
}

void xorFun(int reg[34], int rs, int rt, int rd){
    string srs = decCodeToBin(reg[rs]);
    string srt = decCodeToBin(reg[rt]);
    string result;
    for (int i =0; i<32; i++){
        if(srs[i] == srt[i]){
            result = result +"0";
        }else{
            result = result + "1";
        }
    }
    reg[rd] = binaryStringToInt(result);
}

void slt(int reg[34], int rs, int rt, int rd){
    if(reg[rs] < reg[rt]){
        reg[rd] = 1;
    }else reg[rd] = 0;
}

void sltu(int reg[34], int rs, int rt, int rd){
    unsigned int zuo = reg[rs];
    unsigned int you = reg[rt];
    if(zuo < you){
        reg[rd] = 1;
    }else reg[rd] = 0;
}

void sll(int reg[34], int rt, int rd, int shamt){
    string srt = decCodeToBin(reg[rt]);
    string result = srt.substr(shamt, srt.length()-shamt);
    for (int i =0; i < shamt; i++){
        result = result + "0";
    }
    reg[rd] = binaryStringToInt(result);

}

void srl(int reg[34], int rt, int rd, int shamt){
    string srt = decCodeToBin(reg[rt]);
    string result = srt.substr(0, srt.length()-shamt);
    for (int i =0; i < shamt; i++){
        result = "0" + result;
    }
    reg[rd] = binaryStringToInt(result);
}

void sra(int reg[34], int rt, int rd, int shamt){
    string srt = decCodeToBin(reg[rt]);
    string result = srt.substr(0, srt.length()-shamt);
    if(srt[0] == '1'){
        for (int i =0; i < shamt; i++){
            result = "1" + result;
        }
    }else{
        for (int i =0; i < shamt; i++){
            result = "0" + result;
        }
    }
    reg[rd] = binaryStringToInt(result);
}

void div(int reg[34], int rs, int rt, int rd){          //除法更新
    reg[rd] = reg[rs]/reg[rt];
}

void divu(int reg[34], int rs, int rt,int rd){
    unsigned int beiChuShu = reg[rs];
    unsigned int chuShu = reg[rt];
    reg[rd] = beiChuShu / chuShu;
}

void mult(int reg[34], int rs, int rt, int rd){                 //mult修改更新
    reg[rd] = reg[rs]*reg[rt];
}

void multu(int reg[34], int rs, int rt, int rd){                //multu修改更新
    unsigned int zuo = reg[rs];
    unsigned int you = reg[rt];
    reg[rd] = zuo*you;
}

void teq(int reg[34], int rs, int rt){
    if(reg[rs] == reg[rt]){
        cout << "trap error" << endl;
        exit(1);
    }
}

void tne(int reg[34], int rs, int rt){
    if(reg[rs] != reg[rt]){
        cout << "trap error" << endl;
        exit(1);
    }
}

void tge(int reg[34], int rs, int rt){
    if(reg[rs] >= reg[rt]){
        cout << "trap error" << endl;
        exit(1);
    }
}

void tgeu(int reg[34], int rs, int rt){
    unsigned int zuo = reg[rs];
    unsigned int you = reg[rt];
    if(zuo >= you){
        cout << "trap error" << endl;
        exit(1);
    }
}

void tlt(int reg[34], int rs, int rt){
    if(reg[rs] < reg[rt]){
        cout << "trap error" << endl;
        exit(1);
    }
}

void tltu(int reg[34], int rs, int rt){
    unsigned int zuo = reg[rs];
    unsigned int you = reg[rt];
    if(zuo < you){
        cout << "trap error" << endl;
        exit(1);
    }
}


void syscall(int reg[34], bool & flag, void* baseAddr, void* &heap,myjz_519 *windows)
{
    switch(reg[2])
    {
    case 1:{
        int integer_print = reg[4];
        string str_integer_print = numToChinese(integer_print);
        windows->edit_output(str_integer_print);
        break;
    }
    case 4:{                                                            //打印中文字符串
        int fakeAddr = reg[4];   //find the addr in $a0
        intptr_t realAddr = fakeToReal(fakeAddr, baseAddr);
        char * ptr = reinterpret_cast<char *> (realAddr);
        string str_hanzi;

        while(true){
            if (*ptr == '\0'){
                break;
            }
            str_hanzi += *ptr;
            ptr++;
        }

        string beg = GbkToUtf8(str_hanzi.c_str());
        if (beg == "毕"){
            windows->edit_output("\n");
        }
        else{
            windows->edit_output(beg);
        }
        break;
    }
    case 5:{
            windows->inputButton->setEnabled(true);
            windows->inputNum = 2147483640;
            while(true)
            {
                if(windows->inputNum != 2147483640) break;
                QCoreApplication::processEvents();
            }
            reg[2] = windows->inputNum;
            windows->inputButton->setDisabled(true);
            break;
    }

    case 8:{
        string input;
        cin >> input;
        int fakeAddr = reg[4];
        intptr_t realAddr = fakeToReal(fakeAddr, baseAddr);
        int len = reg[5];
        char * ptr = reinterpret_cast<char *> (realAddr);

        if(input.length() > len-1){
            input = input.substr(0, len-1);
        }
        memcpy(ptr, input.c_str(), input.length()+1);
        break;
    }
    case 9:{
        reg[2] = realToFake( intptr_t(heap), baseAddr );
        intptr_t h = reinterpret_cast<intptr_t>(heap);
        heap = reinterpret_cast<void*>(h + reg[4]);
        break;
    }
    case 10:{
        flag = false;
        break;
    }
    case 11:{
        cout << char(reg[4]) << endl;
        break;
    }
    case 12:{
        string input;
        cin >> input;
        const char * p = input.c_str();
        char ch = *p;
        reg[2] = ch;
        break;
    }
    case 13:{
        fstream infi;
        int fd;
        int fakeAddr = reg[4];
        intptr_t realAddr = fakeToReal(fakeAddr, baseAddr);
        char * ptr = reinterpret_cast<char *> (realAddr);
        string addr;
        while(*ptr != '\0'){
            string s(1, *ptr);
            addr = addr + s;
            ptr ++;
        }
        if (reg[5] == 0) {
            fd = open(addr.c_str(), O_CREAT|O_RDONLY,0777);
        } else if (reg[5] == 1) {
            fd = open(addr.c_str(), O_CREAT|O_WRONLY,0777);
        } else if (reg[5] == 2) {
            fd = open(addr.c_str(), O_CREAT|O_RDWR,0777);
        }
    if (fd < 0) {
        cout << "Cannot open this file";
            exit(1);
    }
    reg[4] = fd;
    break;
    }


    case 14:{
        int fd = reg[4];
        int fakeAddr = reg[5];
        intptr_t realAddr = fakeToReal(fakeAddr, baseAddr);
        char * ptr = reinterpret_cast<char *> (realAddr);
        int len = reg[6];
        int count;
        count = read(fd, ptr, len);
        reg[4] = count;
        break;
    }
    case 15:{
        int fd = reg[4];
        int fakeAddr = reg[5];
        intptr_t realAddr = fakeToReal(fakeAddr, baseAddr);
        char * ptr = reinterpret_cast<char *> (realAddr);
        int len = reg[6];
        int count;
        count = write(fd, ptr, len);
        reg[4] = count;
        break;
    }
    case 16:{
        int fd = reg[4];
        close(fd);
        break;
    }
    case 17:{
        flag = false;
        cout << reg[4] << endl;
        break;
    }
    }
}

void jalr(int reg[34], int rs, int rd, int* &PC, void * baseAddr){
    reg[rd] = realToFake(intptr_t(PC), baseAddr);
    PC = reinterpret_cast<int *>(fakeToReal(reg[rs],baseAddr));                                                 //assume things in rs is the absolute addr of mips
}

void jr(int reg[34], int rs, int* &PC, void * baseAddr){
    PC = reinterpret_cast<int *>(fakeToReal(reg[rs],baseAddr));
}

void mfhi(int reg[34], int rd){
    reg[rd] = reg[33];
}

void mflo(int reg[34], int rd){
    reg[rd] = reg[32];
}

void mthi(int reg[34], int rs){
    reg[33] = reg[rs];
}

void mtlo(int reg[34], int rs){
    reg[32] = reg[rs];
}

void j(int target, int* &PC, void * baseAddr){
    intptr_t b = reinterpret_cast<intptr_t>(baseAddr);
    PC = reinterpret_cast<int *>(target*4 + b);
}

void jal(int reg[34], int target, int* &PC, void * baseAddr){
    reg[31] = realToFake(intptr_t(PC), baseAddr);
    intptr_t b = reinterpret_cast<intptr_t>(baseAddr);
    PC = reinterpret_cast<int *>(target*4 + b);
}

void teqi(int reg[34], int rs, int imm){
    if(reg[rs] == imm){
        cout << "Trap Error"<< endl;
        exit(1);
    }
}

void tnei(int reg[34], int rs, int imm){
    if(reg[rs] != imm){
        cout << "Trap Error"<< endl;
        exit(1);
    }
}

void tgei(int reg[34], int rs, int imm){
    if(reg[rs] >= imm){
        cout << "Trap Error"<< endl;
        exit(1);
    }
}

void tgeiu(int reg[34], int rs, int imm){
    unsigned int zuo = reg[rs];
    unsigned int you = imm;
    if(zuo >= you){
        cout << "Trap Error"<< endl;
        exit(1);
    }
}

void tlti(int reg[34], int rs, int imm){
    if(reg[rs] < imm){
        cout << "Trap Error"<< endl;
        exit(1);
    }
}

void tltiu(int reg[34], int rs, int imm){
    unsigned int zuo = reg[rs];
    unsigned int you = imm;
    if(zuo < you){
        cout << "Trap Error"<< endl;
        exit(1);
    }
}

void bgez(int reg[34], int rs, int imm, int * &PC){
    if(reg[rs] >= 0){
        PC = PC + imm;
    }
}

void bgezal(int reg[34], int rs, int imm, int * &PC, void* baseAddr){
    reg[31] = realToFake(intptr_t(PC), baseAddr);
    if(reg[rs] >= 0){
        PC = PC + imm;
    }
}

void bltzal(int reg[34], int rs, int imm, int * &PC, void* baseAddr){
    reg[31] = realToFake(intptr_t(PC), baseAddr);
    if(reg[rs] < 0){
        PC = PC + imm;
    }
}

void bltz(int reg[34], int rs, int imm, int * &PC){
    if(reg[rs] < 0){
        PC = PC + imm;
    }
}

void addi(int reg[34], int rs, int rt, int imm){
    bool flag = __builtin_add_overflow(reg[rs], imm, &reg[rt]);
    if(flag == true){
        cout << "overflow error" << endl;
        exit(1);
    }
}

void addiu(int reg[34], int rs, int rt, int imm){
    unsigned int zuo = reg[rs];
    reg[rt] = zuo + imm;
}

void andi(int reg[34], int rs, int rt, int imm){
    string srs = decCodeToBin(reg[rs]);
    string srt = decCodeToBin(imm);
    string result;
    for (int i =0; i<32; i++){
        if(srs[i] == '1' && srt[i] == '1'){
            result = result +"1";
        }else{
            result = result + "0";
        }
    }
    reg[rt] = binaryStringToInt(result);
}

void xori(int reg[34], int rs, int rt, int imm){
    string srs = decCodeToBin(reg[rs]);
    string srt = decCodeToBin(imm);
    string result;
    for (int i =0; i<32; i++){
        if(srs[i] == srt[i]){
            result = result +"0";
        }else{
            result = result + "1";
        }
    }
    reg[rt] = binaryStringToInt(result);
}

void ori(int reg[34], int rs, int rt, int imm){
    string srs = decCodeToBin(reg[rs]);
    string srt = decCodeToBin(imm);
    string result;
    for (int i =0; i<32; i++){
        if(srs[i] == '0' && srt[i] == '0'){
            result = result +"0";
        }else{
            result = result + "1";
        }
    }
    reg[rt] = binaryStringToInt(result);
}

void slti(int reg[34], int rs, int rt, int imm){
    if(reg[rs] < imm){
        reg[rt] = 1;
    }else reg[rt] = 0;
}

void sltiu(int reg[34], int rs, int rt, int imm){
    unsigned int zuo = reg[rs];
    unsigned int you = imm;
    if(zuo < you){
        reg[rt] = 1;
    }else reg[rt] = 0;
}

void lui(int reg[34], int rt, int imm){
    string stp1 = decCodeToBin(imm);
    string lower = stp1.substr(16, 16);
    string result = lower + "0000000000000000";
    int addr = binaryStringToInt(result);              //signed 2's compliment
    reg[rt] = addr;
}

void beq(int reg[34], int rs, int rt, int imm, int * &PC){
    if(reg[rs] == reg[rt]){
        PC = PC + imm;
    }
}

void bne(int reg[34], int rs, int rt, int imm, int * &PC){
    if(reg[rs] != reg[rt]){
        PC = PC + imm;
    }
}

void bgtz(int reg[34], int rs, int imm, int * &PC){
    if(reg[rs] > 0){
        PC = PC + imm;
    }
}

void blez(int reg[34], int rs, int imm, int * &PC){
    if(reg[rs] < 0){
        PC = PC + imm;
    }
}

void lb(int reg[34], int rs, int rt, int imm, void* baseAddr){
    int fakeAddr = reg[rs] + imm;
    intptr_t realAddr = fakeToReal(fakeAddr, baseAddr);
    uint8_t * ptr = reinterpret_cast<uint8_t *>(realAddr);
    uint8_t oneByte = *ptr;
    int thing = int(oneByte);
    reg[rt] = thing;

}

void lbu(int reg[34], int rs, int rt, int imm, void* baseAddr){
    int fakeAddr = reg[rs] + imm;
    intptr_t realAddr = fakeToReal(fakeAddr, baseAddr);
    uint8_t * ptr = reinterpret_cast<uint8_t *>(realAddr);
    uint8_t oneByte = *ptr;
    unsigned int thing = int(oneByte);
    reg[rt] = thing;
}

void lh(int reg[34], int rs, int rt, int imm, void* baseAddr){
    int fakeAddr = reg[rs] + imm;
    intptr_t realAddr = fakeToReal(fakeAddr, baseAddr);
    uint16_t * ptr = reinterpret_cast<uint16_t *>(realAddr);
    uint16_t  half = *ptr;
    int thing = int(half);
    reg[rt] = thing;
}

void lhu(int reg[34], int rs, int rt, int imm, void* baseAddr){
    unsigned int fakeAddr = reg[rs] + imm;
    intptr_t realAddr = fakeToReal(fakeAddr, baseAddr);
    uint16_t * ptr = reinterpret_cast<uint16_t *>(realAddr);
    uint16_t  half = *ptr;
    unsigned int thing = int(half);
    reg[rt] = thing;
}

void lw(int reg[34], int rs, int rt, int imm, void* baseAddr){
    int fakeAddr = reg[rs] + imm;
    intptr_t realAddr = fakeToReal(fakeAddr, baseAddr);
    intptr_t * ptr = reinterpret_cast<intptr_t *>(realAddr);
    int word = *ptr;
    reg[rt] = word;
}

void sb(int reg[34], int rs, int rt, int imm, void* baseAddr){
    int fakeAddr = reg[rs] + imm;
    intptr_t realAddr = fakeToReal(fakeAddr, baseAddr);
    uint8_t * ptr = reinterpret_cast<uint8_t *>(realAddr);
    string s = decCodeToBin(reg[rt]);
    string lows = s.substr(24,8);
    int thing = binaryStringToInt(lows);
    *ptr = thing;
}

void sh(int reg[34], int rs, int rt, int imm, void* baseAddr){
    int fakeAddr = reg[rs] + imm;
    intptr_t realAddr = fakeToReal(fakeAddr, baseAddr);
    uint16_t * ptr = reinterpret_cast<uint16_t *>(realAddr);
    string s = decCodeToBin(reg[rt]);
    string lows = s.substr(16,16);
    int thing = binaryStringToInt(lows);
    *ptr = thing;
}

void sw(int reg[34], int rs, int rt, int imm, void* baseAddr){
    int fakeAddr = reg[rs] + imm;
    intptr_t realAddr = fakeToReal(fakeAddr, baseAddr);
    intptr_t * ptr = reinterpret_cast<intptr_t *>(realAddr);
    *ptr = reg[rt];
}

void ll(int reg[34], int rs, int rt, int imm, void* baseAddr){
    int fakeAddr = reg[rs] + imm;
    intptr_t realAddr = fakeToReal(fakeAddr, baseAddr);
    intptr_t * ptr = reinterpret_cast<intptr_t *>(realAddr);
    reg[rt] = *ptr;
}

void sc(int reg[34], int rs, int rt, int imm, void* baseAddr){
    int fakeAddr = reg[rs] + imm;
    intptr_t realAddr = fakeToReal(fakeAddr, baseAddr);
    intptr_t * ptr = reinterpret_cast<intptr_t *>(realAddr);
    *ptr = reg[rt];
}

void lwl(int reg[34], int rs, int rt, int imm, void* baseAddr){
    int fakeAddr = reg[rs] + imm;
    intptr_t realAddr = fakeToReal(fakeAddr, baseAddr);
    intptr_t num = realAddr % 4;
    intptr_t line = (num/4)*4;
    intptr_t* addr = reinterpret_cast<intptr_t *>(line);
    int lineInt = *addr;
    string lineCode = decCodeToBin(lineInt);
    string trans;
    switch(num){
    case 0: trans = lineCode; break;
    case 1: trans = lineCode.substr(8,24); break;
    case 2: trans = lineCode.substr(16, 16); break;
    case 3: trans = lineCode.substr(24, 8); break;
    }
    string srt = decCodeToBin(reg[rt]);
    string result = trans + srt.substr(trans.length(), 32-trans.length());
    int resultInt = binaryStringToInt(result);
    reg[rt] = resultInt;
}

void lwr(int reg[34], int rs, int rt, int imm, void* baseAddr){
    int fakeAddr = reg[rs] + imm;
    intptr_t realAddr = fakeToReal(fakeAddr, baseAddr);
    intptr_t num = realAddr % 4;
    intptr_t line = (num/4)*4;
    intptr_t* addr = reinterpret_cast<intptr_t *>(line);
    int lineInt = *addr;
    string lineCode = decCodeToBin(lineInt);
    string trans;
    switch(num){
    case 0: trans = lineCode.substr(0,0);break;
    case 1: trans = lineCode.substr(0,8); break;
    case 2: trans = lineCode.substr(0, 16); break;
    case 3: trans = lineCode.substr(0, 24); break;
    }
    string srt = decCodeToBin(reg[rt]);
    string result = srt.substr(0, 32-trans.length()) + trans;
    int resultInt = binaryStringToInt(result);
    reg[rt] = resultInt;
}

void swl(int reg[34], int rs, int rt, int imm, void* baseAddr){
    int fakeAddr = reg[rs] + imm;
    intptr_t realAddr = fakeToReal(fakeAddr, baseAddr);
    intptr_t num = realAddr % 4;
    intptr_t line = (num/4)*4;
    intptr_t* addr = reinterpret_cast<intptr_t *>(line);
    int lineInt = *addr;
    string lineCode = decCodeToBin(lineInt);
    string trans;
    string srt = decCodeToBin(reg[rt]);
    switch(num){
    case 0: trans = srt.substr(0,32);break;
    case 1: trans = srt.substr(0,24); break;
    case 2: trans = srt.substr(0,16); break;
    case 3: trans = srt.substr(0, 8); break;
    }
    string result = lineCode.substr(0,32-trans.length())+trans;
    int resultInt = binaryStringToInt(result);
    *addr = resultInt;
}

void swr(int reg[34], int rs, int rt, int imm, void* baseAddr){
    int fakeAddr = reg[rs] + imm;
    intptr_t realAddr = fakeToReal(fakeAddr, baseAddr);
    intptr_t num = realAddr % 4;
    intptr_t line = (num/4)*4;
    intptr_t* addr = reinterpret_cast<intptr_t *>(line);
    int lineInt = *addr;
    string lineCode = decCodeToBin(lineInt);
    string trans;
    string srt = decCodeToBin(reg[rt]);
    switch(num){
    case 0: trans = srt.substr(24,8);break;
    case 1: trans = srt.substr(16,16); break;
    case 2: trans = srt.substr(8,24); break;
    case 3: trans = srt.substr(0, 32); break;
    }
    string result = trans + lineCode.substr(trans.length(),32-trans.length());
    int resultInt = binaryStringToInt(result);
    *addr = resultInt;
}

void nop(){
    //do nothing
}

void li(int reg[34], int rs, int imm){
    string stp1 = decCodeToBin(imm);
    string lower = stp1.substr(16, 16);
    char signBit = lower[0];
    string upper;
    for (int i=0; i<16; i++){
        upper = upper + signBit;
    }
    string result = upper + lower;
    int addr = binaryStringToInt(result);              //signed 2's compliment
    reg[rs] = addr;
}

void mod(int reg[34], int rs, int rt, int rd){
    reg[rd] = reg[rs] % reg[rt];
}

void power(int reg[34], int rs, int rt, int rd){
    reg[rd] = pow(reg[rs], reg[rt]);
}





void machineCycle(int* &PC, int reg[34], void* baseAddr, void* &heap, myjz_519 *windows){
    bool flag = true;
    while(flag == true){
        int intMachineCode = *PC;
        PC = PC + 1;
        string maCode = decCodeToBin(intMachineCode);
        int op, rd, rs, rt, shamt, imm, func, target;
        op = stoi(maCode.substr(0,6), nullptr, 2);

        if(op == 0){                            //Rtype
            rs = stoi(maCode.substr(6,5), nullptr, 2);
            rt = stoi(maCode.substr(11,5), nullptr, 2);
            rd = stoi(maCode.substr(16,5), nullptr, 2);
            shamt = stoi(maCode.substr(21,5), nullptr, 2);
            func = stoi(maCode.substr(26,6), nullptr, 2);
            switch(func){
            case 0x20: add(reg, rs, rt, rd); break;         //add
            case 0x21: addu(reg, rs, rt, rd); break;         //addu
            case 0x24: andFun(reg, rs, rt, rd); break;      //and
            case 0x27: norFun(reg, rs, rt, rd); break;      //nor
            case 0x25: orFun(reg, rs, rt, rd); break;      //or
            case 4: sllv(reg, rs, rt, rd); break;       //sllv
            case 7: srav(reg, rs, rt, rd); break;       //srav
            case 6: srlv(reg, rs, rt, rd); break;       //srlv
            case 0x22: sub(reg, rs, rt, rd); break;         //sub
            case 0x23: subu(reg, rs, rt, rd); break;         //subu
            case 0x26: xorFun(reg, rs, rt, rd); break;         //xor
            case 0x2a: slt(reg, rs, rt, rd); break;       //slt
            case 0x2b: sltu(reg, rs, rt, rd); break;       //sltu
            case 0: sll(reg, rt, rd, shamt); break;       //sll
            case 2: srl(reg, rt, rd, shamt); break;       //srl
            case 3: sra(reg, rt, rd, shamt); break;       //sra
            case 0x1a: div(reg, rs, rt, rd); break;         //div
            case 0x1b: divu(reg, rs, rt, rd); break;         //divu
            case 0x18: mult(reg, rs, rt, rd); break;        //mult
            case 0x19: multu(reg, rs, rt, rd); break;        //multu
            case 0x34: teq(reg, rs, rt); break;         //teq
            case 0x36: tne(reg, rs, rt); break;         //tnq
            case 0x30: tge(reg, rs, rt); break;         //tge
            case 0x31: tgeu(reg, rs, rt); break;         //tgeu
            case 0x32: tlt(reg, rs, rt); break;         //tlt
            case 0x33: tltu(reg, rs, rt); break;         //tltu
            case 0xc: syscall(reg, flag, baseAddr, heap, windows); break;              //syscall
            case 9: jalr(reg, rs, rd, PC, baseAddr); break;       //jalr
            case 8: jr(reg, rs, PC, baseAddr); break;             //jr
            case 0x10: mfhi(reg, rd); break;         //mfhi
            case 0x12: mflo(reg, rd); break;         //mflo
            case 0x11: mthi(reg, rs); break;         //mthi
            case 0x13: mtlo(reg, rs); break;         //mtlo
            case 0x37: mod(reg, rs, rt, rd); break;         //mod
            case 0x38: power(reg, rs, rt, rd); break;         //mod
            }
        }
        else if (op == 2 || op == 3){               //j or jal
            target = stoi(maCode.substr(6,26), nullptr, 2);
            switch(op){
            case 2: j(target, PC, baseAddr); break;
            case 3: jal(reg, target, PC, baseAddr); break;
            }
        }
        else{                                               //I type only
            rs = stoi(maCode.substr(6,5), nullptr, 2);
            rt = stoi(maCode.substr(11,5), nullptr, 2);
            imm = binaryStringToInt16(maCode.substr(16,16));
            switch(op){
            case 1:{
                switch(rt){
                case 0xc: teqi(reg, rs, imm); break;
                case 0xe: tnei(reg, rs, imm); break;
                case 8: tgei(reg, rs, imm); break;
                case 9: tgeiu(reg, rs, imm); break;
                case 0xa: tlti(reg, rs, imm); break;
                case 0xb: tltiu(reg, rs, imm); break;
                case 1: bgez(reg, rs, imm, PC); break;
                case 0x11: bgezal(reg, rs, imm, PC, baseAddr); break;
                case 0x10: bltzal(reg, rs, imm, PC, baseAddr); break;
                case 0: bltz(reg, rs, imm, PC); break;
                }
                break;
            }
            case 8: addi(reg, rs, rt, imm); break;
            case 9: addiu(reg, rs, rt, imm); break;
            case 0xc: andi(reg, rs, rt, imm); break;
            case 0xe: xori(reg, rs, rt, imm); break;
            case 0xd: ori(reg, rs, rt, imm); break;
            case 0xa: slti(reg, rs, rt, imm); break;
            case 0xb: sltiu(reg, rs, rt, imm); break;
            case 0xf: lui(reg, rt, imm); break;
            case 4: beq(reg, rs, rt, imm, PC); break;
            case 5: bne(reg, rs, rt, imm, PC); break;
            case 7: bgtz(reg, rs,imm, PC); break;
            case 6: blez(reg, rs, imm, PC); break;
            case 0x20: lb(reg, rs, rt, imm, baseAddr); break;
            case 0x24: lbu(reg, rs, rt, imm, baseAddr); break;
            case 0x21: lh(reg, rs, rt, imm, baseAddr); break;
            case 0x25: lhu(reg, rs, rt, imm, baseAddr); break;
            case 0x23: lw(reg, rs, rt, imm, baseAddr); break;
            case 0x22: lwl(reg, rs, rt, imm, baseAddr); break;
            case 0x26: lwr(reg, rs, rt, imm, baseAddr); break;
            case 0x30: ll(reg, rs, rt, imm, baseAddr); break;
            case 0x31: li(reg, rs, imm); break;
            case 0x28: sb(reg, rs, rt, imm, baseAddr); break;
            case 0x29: sh(reg, rs, rt, imm, baseAddr); break;
            case 0x2b: sw(reg, rs, rt, imm, baseAddr); break;
            case 0x2a: swl(reg, rs, rt, imm, baseAddr); break;
            case 0x2e: swr(reg, rs, rt, imm, baseAddr); break;
            case 0x38: sc(reg, rs, rt, imm, baseAddr); break;
            case 0x3f: nop();break;
            }
        }
    }
}

void simulator(string simulatorAddr, myjz_519 *windows){

    ifstream infile;

    string i_path = simulatorAddr;

    infile.open(i_path.c_str());

    //模拟记忆模块
    void *pBaseAddr = malloc(6291456);
    int reg[34]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};                          //initialize register;
    reg[29] = 10485760;      // initialize sp to the top of the stack
    reg[30] = 10485760;      // initialize fp to the bottom of the stack
    int *PC = (int *)pBaseAddr;           //initialize PC pointer

    //判断是否存在.datasection
    bool haveData = findData(infile);
    infile.clear(ios::goodbit);
    infile.seekg(ios::beg);           //back to the top of infile

    void *heap = (void*)(fakeToReal(0x500000, pBaseAddr));
    //.data section塞入内存中
    if(haveData == true) heap = dataSection(infile, pBaseAddr);     //if we have data section
    int *tptr = reinterpret_cast<int *>(pBaseAddr);   //把指针指到最底部，开始把机器码放进去

    putBin(infile, tptr);                   //把机器码塞到记忆内存中

    machineCycle(PC, reg, pBaseAddr, heap, windows);

    infile.close();
    free(pBaseAddr);

}
