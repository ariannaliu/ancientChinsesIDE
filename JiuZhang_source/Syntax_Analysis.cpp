#include <stack>
#include "Syntax_Analysis.h"

using namespace std;

void size_test(int size, const vector<token> tokenbuff, int linenum) {
    if (tokenbuff.size() != size) {
        throw string("错误：")+to_string(linenum)+string("行：语法错误。");
    }
}

node* syntax_analysis(vector<token> token_list) {
    node* root = new node;
    node* main = new node;
    main->type = "main";
    root->subNode.push_back(main);
    root->type = "fathernode";
    node* fathernode = main;
    node* scopenode = main;
    stack<node*> nodebuff;
    vector<token> tokenbuff;
    int casenum = 0;
    int chunknum = 0;
    int index = 0;
    int max = token_list.size();
    int linenum = 0;

    while (index < max) {
        linenum++;
        // read line
        while (true) {
            string current_type = token_list[index].type;
            if (current_type == "sep") {
                index++;
                break;
            }
            else if (current_type == "init" && casenum != 13) {
                casenum = 1;
            }
            else if (current_type == "operator" || current_type == "relop" || current_type == "boolop") {
                if (token_list[index].value == "not") {
                    casenum = 2;
                }
                else {
                    casenum = 3;
                }
            }
            else if (current_type == "assign" && casenum != 2 && casenum != 3 && casenum != 12) {
                casenum = 4;
            }
            else if (current_type == "print") {
                casenum = 5;
            }
            else if (current_type == "input") {
                casenum = 6;
            }
            else if (current_type == "while") {
                casenum = 7;
            }
            else if (current_type == "break" || current_type == "continue") {
                casenum = 8;
            }
            else if (current_type == "end") {
                casenum = 9;
            }
            else if (current_type == "if") {
                casenum = 10;
            }
            else if (current_type == "else") {
                casenum = 11;
            }
            else if (current_type == "usefunction") {
                casenum = 12;
            }
            else if (current_type == "definefunction") {
                casenum = 13;
            }
            else if (current_type == "return") {
                casenum = 14;
            }
            tokenbuff.push_back(token_list[index++]);
        }

        switch (casenum) {
        // default
        case 0: break;
        // initial variable
        case 1:{
            size_test(2, tokenbuff, linenum);
            node* newnode = new node;
            newnode->type = "init";
            newnode->value = tokenbuff[0].value;
            newnode->value2 = tokenbuff[1].value;
            if (scopenode->id.count(newnode->value2) > 0) {
                throw string("错误：")+to_string(linenum)+string("行：重复定义变量。");
            }
            scopenode->id[newnode->value2] = newnode->value;
            fathernode->subNode.push_back(newnode);
            break;
        }
        // not operator
        case 2:{
            size_test(4, tokenbuff, linenum);
            node* newnode = new node;
            newnode->type = "not";
            newnode->subtoken.push_back(tokenbuff[1]);
            newnode->subtoken.push_back(tokenbuff[3]);
            fathernode->subNode.push_back(newnode);
            break;
        }
        // other operator
        case 3:{
            size_test(5, tokenbuff, linenum);
            node* newnode = new node;
            newnode->type = tokenbuff[1].value;
            newnode->subtoken.push_back(tokenbuff[0]);
            newnode->subtoken.push_back(tokenbuff[2]);
            newnode->subtoken.push_back(tokenbuff[4]);
            fathernode->subNode.push_back(newnode);
            break;
        }
        // assign value
        case 4:{
            size_test(3, tokenbuff, linenum);
            node* newnode = new node;
            newnode->type = "assign";
            newnode->subtoken.push_back(tokenbuff[0]);
            newnode->subtoken.push_back(tokenbuff[2]);
            fathernode->subNode.push_back(newnode);
            break;
        }
        // print
        case 5:{
            size_test(2, tokenbuff, linenum);
            node* newnode = new node;
            newnode->type = "print";
            newnode->subtoken.push_back(tokenbuff[1]);
            fathernode->subNode.push_back(newnode);
            break;
        }
        // input
        case 6:{
            size_test(2, tokenbuff, linenum);
            node* newnode = new node;
            newnode->type = "input";
            newnode->subtoken.push_back(tokenbuff[1]);
            fathernode->subNode.push_back(newnode);
            break;
        }
        // while
        case 7:{
            size_test(2, tokenbuff, linenum);
            chunknum++;
            node* newnode = new node;
            newnode->type = "while";
            newnode->value = tokenbuff[1].value;
            fathernode->subNode.push_back(newnode);
            nodebuff.push(fathernode);
            fathernode = newnode;
            break;
        }
        // break continue
        case 8:{
            size_test(1, tokenbuff, linenum);
            node* newnode = new node;
            newnode->type = tokenbuff[0].type;
            fathernode->subNode.push_back(newnode);
            break;
        }
        // end
        case 9:{
            size_test(1, tokenbuff, linenum);
            chunknum--;
            if (fathernode->type == "function") {
                fathernode = main;
                scopenode = main;
            }
            else {
                fathernode = nodebuff.top();
                nodebuff.pop();
            }
            break;
        }
        // if
        case 10:{
            size_test(2, tokenbuff, linenum);
            chunknum++;
            node* newnode = new node;
            newnode->type = "if";
            newnode->value = tokenbuff[1].value;
            fathernode->subNode.push_back(newnode);
            nodebuff.push(fathernode);
            fathernode = newnode;
            break;
        }
        // else
        case 11:{
            size_test(1, tokenbuff, linenum);
            if (fathernode->type != "if") {
                throw string("错误：")+to_string(linenum)+string("行：没有if的else。");
            }
            node* newnode = new node;
            newnode->type = "else";
            fathernode->elsenode = newnode;
            fathernode = newnode;
            break;
        }
        // use function
        case 12:{
            node* newnode = new node;
            newnode->type = "usefunction";
            newnode->value = tokenbuff[1].value;
            unsigned elementnum = 0;
            for (auto token : tokenbuff) {
                if (elementnum != 0 && elementnum != 1 && elementnum != (tokenbuff.size() - 2)) {
                    newnode->subtoken.push_back(token);
                }
                elementnum++;
              }
            fathernode->subNode.push_back(newnode);
            break;
        }
        // define function
        case 13:{
            if (tokenbuff.size() % 2 != 0) throw string("错误：")+to_string(linenum)+string("行：定义函数语法错误。");
            int variablenum = 0;
            chunknum++;
            node* newnode = new node;
            newnode->type = "function";
            newnode->value = tokenbuff[1].value;
            for (unsigned i = 2; i < tokenbuff.size(); i += 2) {
                newnode->id[tokenbuff[i+1].value] = tokenbuff[i].value;
                variablenum++;
            }
            newnode->value2 = to_string(variablenum);
            scopenode = newnode;
            fathernode = newnode;
            root->subNode.push_back(newnode);
            break;
        }
        // return value
        case 14:{
            if (fathernode->type != "function") throw string("错误：")+to_string(linenum)+string("行：函数外返回。");
            size_test(2, tokenbuff, linenum);
            node* newnode = new node;
            newnode->type = "return";
            newnode->value = tokenbuff[1].value;
            fathernode->subNode.push_back(newnode);
            break;
        }
        }
        casenum = 0;
        tokenbuff.clear();
    }
    if (chunknum != 0) {
        throw string("错误：文件缺乏end。");
    }
    return root;
}
