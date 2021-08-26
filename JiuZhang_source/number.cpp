#include "number.h"
#include <map>
#include <string>
using namespace std;


map<wchar_t, int> toNumber = {
    {L'零', 0},{L'一', 1}, {L'二', 2}, {L'三', 3}, {L'四', 4}, {L'五', 5}, {L'六', 6},
    {L'七', 7}, {L'八', 8}, {L'九', 9}, {L'十', 10},  {L'百', 100}, {L'千', 1000}, {L'万', 10000}, {L'亿', 100000000}
};


int chineseToNum(wstring wstr){
    int index = 0;
    int result = 0;
    bool neg = false;
    int max = wstr.length();
    if (wstr[index] == L'负'){
        neg = true;
        index++;
    }
    if (wstr[index] == L'十'){
        result += 10;
        index++;
    }
    while (index < max) {
        if (wstr[index] == L'亿' || wstr[index] == L'万') {
            result *= toNumber[wstr[index]];
            index++;
        }
        else if (max - index == 1) {
            if (wstring(L"零一二三四五六七八九").find(wstr[index]) != string::npos){
                result += toNumber[wstr[index]];
                index++;
            }else{
                result = 2147483640;
                break;
            }
        }
        else if (wstr[index] == L'零') {
            index++;
        }
        else if (wstring(L"零一二三四五六七八九").find(wstr[index]) != string::npos) {
            if (wstr[index+1] == L'十' || wstr[index+1] == L'百' || wstr[index+1] == L'千') {
                result += toNumber[wstr[index]] * toNumber[wstr[index+1]];
                index += 2;
            }
            else {
                result += toNumber[wstr[index]];
                index ++;
            }
        }
        else{
            result = 2147483640;
            break;
        }
    }
    if (neg) result *= -1;
    return result;
}


string unite[5]={"","十","百","千","万"}; //单位
string num[10]={"零","一","二","三","四","五","六","七","八","九"}; //一个汉字占两个字节,另外再加一个'\0'字符.
string func(int a){
    int flag=0,tmp;
    string strtmp;
    string result;
    int atemp=a;     //设定a的临时存储值，防止每次进入循环都进行末尾零的判断
    while(a!=0)
    {

        while(atemp%10==0)
        {
            flag++;
            atemp/=10;
            a/=10;
        }

        tmp=a%10;
        if(tmp!=0)
        strtmp=num[tmp]+unite[flag];
        else if(tmp==0)
        strtmp="零";
        result=strtmp+result;
        a/=10;
        flag++;
    }
    return result;
}
string numToChinese(int a){
    string result,temp;
	bool neg = false;
	if (a < 0){
        neg = true;
		a = -a;
    }
    if (a == 0){
        result = "零";
    }
    else if(a<100000 && a != 0){
    result=func(a);
    }
    else
    {
    temp=func(a/10000);
    result=temp+ "万"+ func(a-a/10000*10000);
    }
	if (neg) result = "负"+result;
    return result;
}
