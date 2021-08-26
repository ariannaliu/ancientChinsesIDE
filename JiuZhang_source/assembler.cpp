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
#include <assember.h>
using namespace std;

std::map<std::string, std::string> labels;
std::map<std::string, std::string> undefined_funcs;

map<string,string> registerNumber={                          //map register to binary code
    {"$zero", "00000"},{"$at", "00001"},{"$v0", "00010"},{"$v1", "00011"},
    {"$a0", "00100"},{"$a1", "00101"},{"$a2", "00110"},{"$a3", "00111"},
    {"$t0", "01000"},{"$t1", "01001"},{"$t2", "01010"},{"$t3", "01011"},{"$t4", "01100"},{"$t5", "01101"},{"$t6", "01110"},{"$t7", "01111"},
    {"$s0", "10000"},{"$s1", "10001"},{"$s2", "10010"},{"$s3", "10011"},{"$s4", "10100"},{"$s5", "10101"},{"$s6", "10110"},{"$s7", "10111"},
    {"$t8", "11000"},{"$t9", "11001"},{"$k0", "11010"},{"$k1", "11011"},{"$gp", "11100"},{"$sp", "11101"},{"$fp", "11110"},{"$ra", "11111"},

};

//R_1: 0*6 rs rt rd noShamt function                          //this map to its corresbonding function code
map<string,string> Rtype_1={
    {"add","100000"},{"addu","100001"},{"and","100100"},{"nor","100111"},{"or","100101"},{"sub","100010"},
    {"subu","100011"},{"xor","100110"},{"slt","101010"},{"sltu","101011"},
    {"div","011010"},{"divu","011011"},{"mult","011000"},{"multu","011001"},   //乘除法移动到这里来了
    {"mod","110111"},{"pow","111000"},
};

map<string,string> Rtype_1_5={
    {"sllv","000100"},{"srav","000111"},{"srlv","000110"},
};

//R_2: 0*12 rt rd  shamt function
map<string,string> Rtype_2={
    {"sll","000000"},{"sra","000011"},{"srl","000010"},
};

//R_3: 000000 rs rt 0*10 function
map<string,string> Rtype_3={
    {"teq","110100"},{"tne","110110"},{"tge","110000"},
    {"tgeu","110001"},{"tlt","110010"},{"tltu","110011"},
};

//I_1: op rs rt imm
map<string,string> Itype_1={
    {"addi","001000"},{"addiu","001001"},{"andi","001100"},{"ori","001101"},{"xori","001110"},{"slti","001010"},{"sltiu","001011"},
};

//I_2: 000001 rs ?*5 imm
map<string,string> Itype_2={
    {"teqi","01100"},{"tnei","01110"},{"tgei","01000"},{"tgeiu","01001"},{"tlti","01010"},{"tltiu","01011"},
};

//J: opcode target
map<string,string> Jtype={
    {"j","000010"},{"jal","000011"},
};

//LOAD STORE
map<string,string> LoadStore={
    {"lb","100000"},{"lbu","100100"},{"lh","100001"},{"lhu","100101"},{"lw","100011"},{"lwl","100010"},{"ll","110000"},{"lwr","100110"},
    {"sb","101000"},{"sh","101001"},{"sw","101011"},{"swl","101010"},{"swr","101110"},{"sc","111000"},
};


string decToBin(int dec){                        //this function convert decimal num to 2's com binary num
    string result;
    int temp = (dec >= 0) ? dec : abs(dec+1);
    if (temp==0) result="0";
    while(temp){
        result = to_string(temp % 2) + result;
        temp = temp / 2;
    }
    if (dec >= 0) return '0'+result;
    else {       //dec<0
        for (unsigned i=0;i < result.length();i++){
            result[i] = (result[i] == '0') ? '1' : '0';
        }
        return '1'+result;
    }
}

void split(string s, vector<string>& v, string c) {
    string::size_type pos1, pos2;
    pos2 = s.find(c);
    pos1 = 0;
    while(string::npos != pos2) {
        if (pos2 - pos1 != 0) v.push_back(s.substr(pos1, pos2-pos1));
        pos1 = pos2 + c.length();
        pos2 = s.find(c, pos1);
    }
    if(pos1 != s.length()) v.push_back(s.substr(pos1));
}



void scanForUndefined_func(ifstream & infile, map<string, string> & undefined_funcs){
    string line;
    while(getline(infile, line)){
        line.erase(0,line.find_first_not_of(" "));                   //拿到一行，去头去尾
        line.erase(line.find_last_not_of(" ") + 1);
        if (line.substr(0,15) == "func_undefined:") break;           //找到func-undefined的区域
    }

    while(getline(infile, line)){                                   //对func_undefined进行处理
        line.erase(0,line.find_first_not_of(" "));                   //拿到一行，去头去尾
        line.erase(line.find_last_not_of(" ") + 1);

        if(line.substr(0,12) == "var_defined:") break;                              //func_undefined区块结束
        if(line == "") continue;                                                    //如果是空行，进入下一个循环

        vector<string> func_lineElements;
        split(line, func_lineElements, "@");
        undefined_funcs[func_lineElements[0]] = func_lineElements[1];              //讲undefined函数和行数整合
    }
}


void scanForLabel(ifstream & infile, map<string, string> & labels){
    string line;
    while (getline(infile, line)) {
        line.erase(0,line.find_first_not_of(" "));                        //after remove, delete the empty space
        line.erase(line.find_last_not_of(" ") + 1);                       //前面清除，后面清除，留下，便于寻找
        if (line.substr(0,5) == ".text") break;                           //找到.text的部分，跳出清除循环
    }
    int lineAddress = 0;
    int labelPos;
    string label;
    while (getline(infile, line)) {
        int indent = 0;
        while (true) {                                                     //while循环会清除全部的tab
            indent = line.find('\t', indent);
            if (indent < 0) break;
            line.erase(indent, 1);
        }
        vector<string> lineElements;
        split(line, lineElements, " ");
        if ((lineElements.size() > 0) && (lineElements[0] != "#")) {           //这一行是mips行
            if (lineElements.size() == 1 && lineElements[0].find(':') > 0  && lineElements[0][lineElements[0].length()-1] == ':') {
                label = lineElements[0].substr(0, lineElements[0].length()-1);
            } else {
                labelPos = line.find(':');
                if (labelPos > 0){                                              //表示存在label
                    label = line.substr(0, labelPos);
                    labels[label] = to_string(lineAddress);
                    label = "";
               } else if (label != "") {
                    labels[label] = to_string(lineAddress);
                    label = "";
                }
               lineAddress++;
            }
        }
    }
}

void copy_header(ifstream &infile, ofstream &outfile){
    string line;
    while (getline(infile, line)){
        line.erase(0,line.find_first_not_of(" "));                        //after remove, delete the empty space
        line.erase(line.find_last_not_of(" ") + 1);                       //前面清除，后面清除，留下，便于寻找
        if(line.substr(0,5) == ".data"){                                    //找到.data 跳出循环
            break;
        }
        //其他正常情况下
        outfile << line << endl;
    }

    while (getline(infile, line)){
        line.erase(0,line.find_first_not_of(" "));                        //after remove, delete the empty space
        line.erase(line.find_last_not_of(" ") + 1);                       //前面清除，后面清除，留下，便于寻找
        if(line.substr(0,5) == ".text"){                                    //找到.data 跳出循环
            break;
        }
    }
    int count_lineAddr = 0;
    while (getline(infile, line)){
        if(!line.empty()){
            line.erase(0,line.find_first_not_of(" "));                        //after remove, delete the empty space
            line.erase(line.find_last_not_of(" ") + 1);
            if(line != "") count_lineAddr++;
        }
    }
    outfile << "line:" << endl;
    outfile << to_string(count_lineAddr) << endl;
}

void copy_data(ifstream &infile, ofstream &outfile){
    string line;
    while (getline(infile, line)){
        line.erase(0,line.find_first_not_of(" "));                        //after remove, delete the empty space
        line.erase(line.find_last_not_of(" ") + 1);                       //前面清除，后面清除，留下，便于寻找
        if(line.substr(0,5) == ".data"){                                    //找到.data 跳出循环
            break;
        }
    }
    outfile << ".data" << endl;
    while (getline(infile, line)){
        line.erase(0,line.find_first_not_of(" "));                        //after remove, delete the empty space
        line.erase(line.find_last_not_of(" ") + 1);                       //前面清除，后面清除，留下，便于寻找
        if(line.substr(0,5) == ".text"){                                    //找到.data 跳出循环
            break;
        }
        //其他正常情况下
        outfile << line << endl;
    }
    outfile << ".text" << endl;

}

string lineToString(vector<string> line){
    stringstream ss;
    if (Rtype_1.count(line[0]) != 0 ){
        ss << "000000"<<registerNumber[line[2]]<<registerNumber[line[3]]<<registerNumber[line[1]]<<"00000"<<Rtype_1[line[0]];
    }
    else if (Rtype_1_5.count(line[0]) != 0 ){
         ss << "000000"<<registerNumber[line[3]]<<registerNumber[line[2]]<<registerNumber[line[1]]<<"00000"<<Rtype_1_5[line[0]];
    }
    else if (Rtype_2.count(line[0]) != 0 ){
        string shamt = decToBin(atoi(line[3].c_str()));
        if (shamt.length() > 5) shamt = shamt.substr(shamt.length()-5,5);
        ss << "00000000000"<<registerNumber[line[2]]<<registerNumber[line[1]]<< setfill(shamt[0]) <<setw(5)<<shamt<<Rtype_2[line[0]];
    }
    else if (Rtype_3.count(line[0]) != 0 ){
        ss << "000000"<<registerNumber[line[1]]<<registerNumber[line[2]]<<"0000000000"<<Rtype_3[line[0]];
    }
    else if (Itype_1.count(line[0]) != 0 ){
        string imm = decToBin(atoi(line[3].c_str()));
        if (imm.length() > 16) imm = imm.substr(imm.length()-16,16);
        ss << Itype_1[line[0]]<<registerNumber[line[2]]<<registerNumber[line[1]]<< setfill(imm[0]) <<setw(16) << imm;
    }
    else if (line[0] == "lui"){
        string imm = decToBin(atoi(line[2].c_str()));
        if (imm.length() > 16) imm = imm.substr(imm.length()-16,16);
        ss << "00111100000" << registerNumber[line[1]] << setfill(imm[0]) <<setw(16) << imm;
    }
    else if (Itype_2.count(line[0]) != 0 ){
        string imm =decToBin(atoi(line[2].c_str()));
        if (imm.length() > 16) imm = imm.substr(imm.length()-16,16);
        ss << "000001"<<registerNumber[line[1]] << Itype_2[line[0]] << setfill(imm[0]) <<setw(16) << imm;
    }
    else if (line[0] == "jalr"){
        ss << "000000" << registerNumber[line[1]] << "00000" << registerNumber[line[2]] <<"00000001001";
    }
    else if (line[0] == "jr"){
        ss << "000000" << registerNumber[line[1]] <<"000000000000000001000";
    }
    else if (line[0] == "mfhi"){
        ss << "0000000000000000"<<registerNumber[line[1]]<<"00000"<<"010000";
    }
    else if (line[0] == "mflo"){
        ss << "0000000000000000"<<registerNumber[line[1]]<<"00000"<<"010010";
    }
    else if (line[0] == "mthi"){
        ss << "000000" << registerNumber[line[1]] << "000000000000000"<<"010001";
    }
    else if (line[0] == "mtlo"){
        ss << "000000" << registerNumber[line[1]] << "000000000000000"<<"010011";
    }
    else if (Jtype.count(line[0]) != 0 ){
        if (undefined_funcs.count(line[1]) != 0){               //在j或者jal的时候，出现了没有定义的函数了
            ss<<Jtype[line[0]]<<line[1];
        }else{                                                  //情况正常，没得问题
            ss << Jtype[line[0]] << setfill('0') <<setw(26) << decToBin(atoi(labels[line[1]].c_str())) ;
        }
        //ss << Jtype[line[0]] << setfill('0') <<setw(26) << decToBin(atoi(labels[line[1]].c_str())) ;
    }
    else if (line[0] == "beq"){
        string imm = decToBin(atoi(labels[line[3]].c_str()) - atoi(line[4].c_str()) - 1);
        ss <<"000100"<<registerNumber[line[1]]<<registerNumber[line[2]]<< setfill(imm[0]) <<setw(16) << imm;
    }
    else if (line[0] == "bne"){
        string imm = decToBin(atoi(labels[line[3]].c_str()) - atoi(line[4].c_str()) - 1);
        ss <<"000101"<<registerNumber[line[1]]<<registerNumber[line[2]]<< setfill(imm[0]) <<setw(16) << imm;
    }
    else if (line[0] == "bgez"){
        string imm = decToBin(atoi(labels[line[2]].c_str()) - atoi(line[3].c_str()) - 1);
        ss <<"000001"<<registerNumber[line[1]]<<"00001"<< setfill(imm[0]) <<setw(16) << imm;
    }
    else if (line[0] == "bgezal"){
        string imm = decToBin(atoi(labels[line[2]].c_str()) - atoi(line[3].c_str()) - 1);
        ss <<"000001"<<registerNumber[line[1]]<<"10001"<< setfill(imm[0]) <<setw(16) << imm;
    }
    else if (line[0] == "bgtz"){
        string imm = decToBin(atoi(labels[line[2]].c_str()) - atoi(line[3].c_str()) - 1);
        ss <<"000111"<<registerNumber[line[1]]<<"00000"<< setfill(imm[0]) <<setw(16) << imm;
    }
    else if (line[0] == "blez"){
        string imm = decToBin(atoi(labels[line[2]].c_str()) - atoi(line[3].c_str()) - 1);
        ss <<"000110"<<registerNumber[line[1]]<<"00000"<< setfill(imm[0]) <<setw(16) << imm;
    }
    else if (line[0] == "bltzal"){
        string imm = decToBin(atoi(labels[line[2]].c_str()) - atoi(line[3].c_str()) - 1);
        ss <<"000001"<<registerNumber[line[1]]<<"10000"<< setfill(imm[0]) <<setw(16) << imm;
    }
    else if (line[0] == "bltz"){
        string imm = decToBin(atoi(labels[line[2]].c_str()) - atoi(line[3].c_str()) - 1);
        ss <<"000001"<<registerNumber[line[1]]<<"00000"<< setfill(imm[0]) <<setw(16) << imm;
    }
    else if (LoadStore.count(line[0]) != 0){
        int braPos = line[2].find("("), ketPos = line[2].find(")");
        string rs = line[2].substr(braPos+1, ketPos-braPos-1);
        string imm_dec = line[2].substr(0,braPos);
        string imm =decToBin(atoi(imm_dec.c_str()));
        if (imm.length() > 16) imm = imm.substr(imm.length()-16,16);
        ss << LoadStore[line[0]] << registerNumber[rs] << registerNumber[line[1]] << setfill(imm[0])<<setw(16)<<imm;

    }
    else if (line[0] == "nop"){                                 //新增nop
        ss << "11111100000000000000000000000000";
    }
    else if(line[0] == "li"){                                   //新增li $t1, 3
        string imm =decToBin(atoi(line[2].c_str()));
        if (imm.length() > 16) imm = imm.substr(imm.length()-16,16);
        ss << "110001" << registerNumber[line[1]] << "00000" << setfill(imm[0])<<setw(16)<< imm;
    }
    else if (line[0] == "syscall"){
        ss << "00000000000000000000000000"<<"001100";
    }


    return ss.str();
}




void assToBin(ifstream & infile, ofstream & outfile){
    string line;
    while(getline(infile,line)){
        line.erase(0,line.find_first_not_of(" "));                        //after remove, delete the empty space
        line.erase(line.find_last_not_of(" ") + 1);                       //前面清除，后面清除，留下，便于寻找
        if(line.substr(0,5) == ".text") break;                             //找到了.text，现在开始转机器码
    }

    int lineAddress = 0;
    while(getline(infile, line)){
        int commentPos;         //找#
        int labelPos;           //找冒号“：”
        int commaPos = 0;       //找逗号“，”
        int indent = 0;         //找tab

        if(!line.empty()){              //不是空行
            line.erase(0,line.find_first_not_of(" "));                      //掐头去尾
            line.erase(line.find_last_not_of(" ") + 1);

            //去备注
            commentPos = line.find('#');
            if (commentPos >= 0) line = line.substr(0, commentPos);

            //把label保留到第二个文件中，并在此文件中去除掉
            labelPos = line.find(':');
            if (labelPos >= 0){
                outfile << line.substr(0,labelPos+1);
                line = line.substr(labelPos + 1, line.length() - labelPos - 1);

            }
            //去除文件中所有的tab
            while (true) {
                indent = line.find('\t', indent);
                if (indent < 0) break;
                line.erase(indent, 1);
            }
            //将逗号变为空格
            while (true) {
                commaPos = line.find(',');
                if (commaPos < 0) break;
                line = line.replace(commaPos, 1, " ");
            }


            if(!line.empty()){
                vector<string> lineElements;
                split(line, lineElements, " ");
                lineElements.push_back(to_string(lineAddress));
                outfile << lineToString(lineElements) << endl;
                lineAddress++;
            }
        }


    }
}


vector<std::string> assembler(vector<std::string> compilerFile){
    ifstream infile;
    ofstream outfile;
    string i_path;
    string o_path;

    for(unsigned ass_time = 0; ass_time<compilerFile.size(); ass_time++){
        i_path = compilerFile[ass_time]+".asm";
        o_path = compilerFile[ass_time]+".o";

        //i_path = StringToUtf(i_path);
        //o_path = StringToUtf(o_path);

        infile.open(i_path.c_str());
        outfile.open(o_path.c_str());

        scanForUndefined_func(infile, undefined_funcs);         //扫描一遍未定义的函数（对源文件没有处理）
        infile.clear(ios::goodbit);
        infile.seekg(ios::beg);                                 //返回去


        scanForLabel(infile, labels);                           //扫描一遍func地址（对源文件没有处理）
        infile.clear(ios::goodbit);
        infile.seekg(ios::beg);                                 //返回去

        copy_header(infile, outfile);                           //将header原封不动放到输出文件中
        infile.clear(ios::goodbit);
        infile.seekg(ios::beg);                                 //返回去

        copy_data(infile, outfile);                           //将header原封不动放到输出文件中
        infile.clear(ios::goodbit);
        infile.seekg(ios::beg);                                 //返回去

        assToBin(infile, outfile);                              //assembler主要的功能
        infile.close();
        outfile.close();
    }
    return compilerFile;
}
