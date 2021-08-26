#include "Lexical_Analysis.h"
#include "number.h"
#include <string>
using namespace std;


vector<token> lexical_analysis(const wstring text, map<wstring, int> &idList){

    map<wchar_t, string> typeMap = {
        {L'数', "init"}, {L'判', "init"}, {L'盈', "bool"}, {L'亏', "bool"}, {L'赋', "assign"}, {L'加', "operator"}, {L'减', "operator"},
        {L'乘', "operator"}, {L'除', "operator"}, {L'余', "operator"}, {L'幂', "operator"}, {L'盛', "relop"},  {L'等', "relop"}, {L'衰', "relop"},
        {L'且', "boolop"}, {L'或', "boolop"}, {L'非', "boolop"}, {L'若', "if"}, {L'否', "else"}, {L'止', "end"}, {L'当', "while"}, {L'继', "continue"},
        {L'离', "break"}, {L'法', "definefunction"}, {L'关', "null"}, {L'得', "return"}, {L'施', "usefunction"}, {L'言', "print"},
        {L'闻', "input"}, {L'。', "sep"}, {L'有', "null"}, {L'曰', "null"}, {L'乃', "null"}, {L'以', "null"}, {L'于', "null"}, {L'之', "null"},
        {L'乎', "null"}, {L'，', "null"}, {L'\n', "null"}, {L' ', "null"}
    };

    map<wchar_t, string> valueMap = {
        {L'数', "num"}, {L'判', "bool"}, {L'盈', "true"}, {L'亏', "false"}, {L'加', "add"}, {L'减', "sub"},
        {L'乘', "multi"}, {L'除', "div"}, {L'余', "mod"}, {L'幂', "power"}, {L'盛', "large"},  {L'等', "equal"}, {L'衰', "less"}, {L'且', "and"},
        {L'或', "or"}, {L'非', "not"}
    };

    wstring numbers = L"零一二三四五六七八九十百千万亿负";

    vector<token> token_list;
    wstring buff;
    wchar_t textbuff;
    int casenum = 0;
    int max = text.length();
    int rownum = 1;

    for (int i = 0; i < max; i++){
        textbuff = text[i];     // read the text
        switch(casenum){

        // default state
        case 0:{
            if (textbuff == L'“') {   // variable name
                casenum = 1;
            }
            else if (numbers.find(textbuff) != string::npos) {    // number
                buff = buff + textbuff;
                casenum = 2;
            }
            else if (typeMap.count(textbuff) > 0){
                if (typeMap[textbuff] == "null") break;
                else {
                    token tokenbuff;
                    tokenbuff.type = typeMap[textbuff];
                    tokenbuff.value = valueMap.count(textbuff) > 0? valueMap[textbuff] : "";
                    token_list.push_back(tokenbuff);
                    if (textbuff == L'言') {    // string
                        casenum = 3;
                    }
                    if (textbuff == L'。') {    // next statement
                        rownum++;
                    }
                }
            }
            else if (textbuff == L'注') {    // note
                casenum = 4;
            }
            else throw string("错误：")+to_string(rownum)+string("行：未识别的字符。");
            break;
        }

        // variable name
        case 1:{
            if (textbuff == L'”') {
                if (!(idList.count(buff) > 0)) {
                    idList[buff] = idList.size();
                }
                token tokenbuff;
                tokenbuff.type = "id";
                tokenbuff.value = to_string(idList[buff]);
                token_list.push_back(tokenbuff);
                buff = L"";
                casenum = 0;
            }
            else if (textbuff == L'。') {
                throw string("错误：")+to_string(rownum)+string("行：缺少” 。");
            }
            else if (textbuff == L'“') {
                throw string("错误：")+to_string(rownum)+string("行：“ 使用错误。");
            }
            else buff = buff + textbuff;
            break;
        }

        // number
        case 2:{
            if (numbers.find(textbuff) != string::npos) {
                buff = buff + textbuff;
            }
            else {
                token tokenbuff;
                tokenbuff.type = "num";
                tokenbuff.value = to_string(chineseToNum(buff));
                token_list.push_back(tokenbuff);
                i--;
                buff = L"";
                casenum = 0;
            }
            break;
        }

        // string
        case 3:{
            if (textbuff == L'。') {
                token tokenbuff;
                tokenbuff.type = "string";
                tokenbuff.wvalue = buff;
                token_list.push_back(tokenbuff);
                i--;
                buff = L"";
                casenum = 0;
            }
            else if (textbuff == L'“') {
                if (buff != L""){
                    token tokenbuff;
                    tokenbuff.type = "string";
                    tokenbuff.wvalue = buff;
                    token_list.push_back(tokenbuff);
                    buff = L"";
                }
                casenum = 1;
            }
            else {
                buff = buff + textbuff;
            }
            break;
        }
        // note
        case 4:{
            if (textbuff == L'。') {
                i--;
                casenum = 0;
            }
            break;
        }
        }
    }
    return token_list;
}
