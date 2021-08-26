#include "linker_l.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <cmath>
#include <map>

using namespace std;

std::map<std::string, std::string> label26;

void copyDataSection(ifstream &infile, ofstream &outfile){
    string line;
    while (getline(infile, line)){
        line.erase(0,line.find_first_not_of(" "));                        //after remove, delete the empty space
        line.erase(line.find_last_not_of(" ") + 1);                       //前面清除，后面清除，留下，便于寻找
        if(line.substr(0,5) == ".data"){
            outfile << ".data" << endl;
            break;
        }else continue;
    }
    //现在已经找到dataSection了，开始原封不动的复制了
    while (getline(infile, line)){
        line.erase(0,line.find_first_not_of(" "));                        //after remove, delete the empty space
        line.erase(line.find_last_not_of(" ") + 1);                       //前面清除，后面清除，留下，便于寻找
        if(line.substr(0,5) == ".text"){
            outfile << ".text" << endl;
            break;
        }else{
            outfile << line << endl;
        }
    }
}

int copyMainAndCount(ifstream &infile, ofstream &outfile){
    int countLine = 0;
    string line;
    while (getline(infile, line)){
        line.erase(0,line.find_first_not_of(" "));                        //after remove, delete the empty space
        line.erase(line.find_last_not_of(" ") + 1);                       //前面清除，后面清除，留下，便于寻找
        if(line != ""){
            outfile << line << endl;
            countLine++;
        }
    }
    return countLine;
}

int binaryStringToInt26(string s){
    reverse(s.begin(),s.end());
    int count = 0;
    for(unsigned i = 0; i < s.length(); i++){
        if(s[i] == '1'){
            count = count + pow(2,i);
        }
    }
    return count;
}

string decToBin26(int dec){
    string result;
    int temp = (dec >= 0) ? dec : abs(dec+1);
    if (temp==0) result="0";
    while(temp){
        result = to_string(temp % 2) + result;
        temp = temp / 2;
    }
    if (dec >= 0) {
        int count = result.length();
        for (int i = 0; i < (26-count); i++){
            result = "0"+result;
        }
    }
    else {       //dec<0
        for (unsigned i=0; i < result.length(); i++){
            result[i] = (result[i] == '0') ? '1' : '0';
        }
        int count = result.length();
        for (int i = 0; i < (26-count); i++){
            result = "1"+result;
        }
    }
    return result;
}

string processFuncLine(string line, int count){
    string result = "";
    int labelPos = line.find(':');
    if (line.find(':') != string::npos){                       //如果有冒号，就把冒号加上
        if(line.substr(0,2) == "if" || line.substr(0,5) == "endif" ){
            line.erase(0,labelPos+1);
        }else{
            result += line.substr(0,labelPos+1);
            line.erase(0,labelPos+1);
        }
    }
    //经过擦除以后就只剩下机器码了
    string op = line.substr(0,6);
    if (op == "000010" || op == "000011"){                  //如果他是j或者jal
        result += line.substr(0,6);
        line.erase(0,6);
        if(line[0]!='0' && line[0] != '1'){                //说明后面跟的是func而不是机器码
            result += line;
        }else{                                             //对应的是机器码，需要偏移
            int ori = binaryStringToInt26(line);
            int fin = ori + count;
            string str_fin = decToBin26(fin);
            result += str_fin;
        }

    }else{
        result += line;
    }
    return result;
}

int copyFunction(ifstream &infile, ofstream &outfile, int count){
    string line;
    int funcNum = 0;
    while (getline(infile, line)){
        line.erase(0,line.find_first_not_of(" "));                        //after remove, delete the empty space
        line.erase(line.find_last_not_of(" ") + 1);                       //前面清除，后面清除，留下，便于寻找
        if(line.substr(0,5) == ".text"){
            break;
        }else continue;
    }
    //下面全是机器码了,找到main：的结束部分
    while (getline(infile, line)) {
        line.erase(0,line.find_first_not_of(" "));                        //after remove, delete the empty space
        line.erase(line.find_last_not_of(" ") + 1);                       //前面清除，后面清除，留下，便于寻找

        if (line.find(':') != string::npos){                        //找到了冒号“：”
            if (line.substr(0,5) != "main:" && line.substr(0,2) != "if" && line.substr(0,5)!= "endif"){                       //找到了func部分,把第一行打出来了
                line = processFuncLine(line, count);
                outfile << line << endl;
                funcNum ++;
                break;
            }
        }
    }
    while (getline(infile,line)){
        line.erase(0,line.find_first_not_of(" "));                        //after remove, delete the empty space
        line.erase(line.find_last_not_of(" ") + 1);                       //前面清除，后面清除，留下，便于寻找
        if (line != ""){
            line = processFuncLine(line, count);
            outfile << line << endl;
            funcNum ++;
        }
    }
    return funcNum;
}


void scanLabel(ifstream & infile, map<string, string> & labels){
    string line;
    while (getline(infile, line)) {
        line.erase(0,line.find_first_not_of(" "));                        //after remove, delete the empty space
        line.erase(line.find_last_not_of(" ") + 1);                       //前面清除，后面清除，留下，便于寻找
        if (line.substr(0,5) == ".text") break;                           //找到.text的部分，跳出清除循环
    }

    int lineAddress = 0;
    int labelPos;
    string label;
    while (getline(infile, line)){
        line.erase(0,line.find_first_not_of(" "));                        //after remove, delete the empty space
        line.erase(line.find_last_not_of(" ") + 1);                       //前面清除，后面清除，留下，便于寻找
        if (line != ""){
            if (line.find(':') != string::npos){
                labelPos = line.find(':');
                label = line.substr(0,labelPos);
                labels[label] = to_string(lineAddress);
            }
        }
        lineAddress++;
    }
}

void finalCopy(ifstream &infile, ofstream &outfile){
    string line;
    int labelPos;
    while (getline(infile, line)){
        line.erase(0,line.find_first_not_of(" "));                        //after remove, delete the empty space
        line.erase(line.find_last_not_of(" ") + 1);                       //前面清除，后面清除，留下，便于寻找
        if(line != ""){
            if(line.find(':') != string::npos){             //label删除术
                labelPos = line.find(':');
                line.erase(0,labelPos+1);
            }
            if(line[6]!='0' && line[6] != '1'){             //说明有function
                outfile << line.substr(0,6);
                string function = line.substr(6,line.size()-6);
                string func_code = decToBin26(atoi(label26[function].c_str()));
                outfile << func_code << endl;
            }else{
                outfile << line << endl;
            }
        }
    }
}



string linker_l (vector<string> fileAddrs){
    ifstream infile;
    ofstream outfile;
    string i_path;
    string o_path;
    o_path = fileAddrs[0] + ".trans";
    outfile.open(o_path.c_str());


    //处理主函数文件
    i_path = fileAddrs[0] + ".o";
    infile.open(i_path.c_str());

    copyDataSection(infile, outfile);                           //将.data全部放入，同时加入.text

    int mainLine = copyMainAndCount(infile, outfile);
    infile.close();

    int count = mainLine;

    //主函数处理完成，处理其他函数
    for(unsigned fileNum = 1; fileNum < fileAddrs.size(); fileNum++){
        i_path = fileAddrs[fileNum]+".o";
        infile.open(i_path.c_str());            //打开了库文件
        count += copyFunction(infile, outfile, count);               //count加上了copy上的function
        infile.close();
    }

    outfile.close();    //main.trans 完成，开始func定位

    i_path = fileAddrs[0] + ".trans";
    o_path = fileAddrs[0] + ".jexe";

    infile.open(i_path.c_str());
    outfile.open(o_path.c_str());

    scanLabel(infile, label26);
    infile.clear(ios::goodbit);
    infile.seekg(ios::beg);

    copyDataSection(infile, outfile);                   //将.data全部放入，同时加入.text
    finalCopy(infile, outfile);

    infile.close();
    outfile.close();
    return o_path;
}
