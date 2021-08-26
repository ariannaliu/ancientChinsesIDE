#include "toMIPS.h"

/* Function prototypes */

struct info;
void executeNode(info* &currentInfo);
void j_init(info* &currentInfo);
void j_not(info* &currentInfo);
void j_arithmatic(info* &currentInfo);
void j_assign(info* &currentInfo);
void j_print(info* &currentInfo);
void j_input(info* &currentInfo);
void j_while(info* &currentInfo);
void j_continue(info* &currentInfo);
void j_break(info* &currentInfo);
void j_if(info* &currentInfo);
void j_else(info* &currentInfo);
void j_usefunction(info* &currentInfo);
void j_function(info* &currentInfo);
void j_return(info* &currentInfo);
void j_main(info* &currentInfo);
void j_fathernode(info* &currentInfo);
void varTest(info* &currentInfo,string var);
int stringToInt(string str);
std::string WString2String(const std::wstring& ws);


/* Class */

struct info
{
    int prt = 0;
    int loop = 0;
    int ifNum = 0;
    int funcflag = 0;
    int addressC = 0;
    int dataline = 0;
    int textline = -1;
    int funcline = 0;
    int memorySize = 80;
    string forName;
    string ifName;
    string funcName;
    string data = "\n.data\n";
    string text = "\n.text\n";
    string funcText = "";
    node* tree;
    map<string,int> funcD;
    map<string,int> funcU;
    map<string,int> varD;
    map<string,int> varU;
    stack<int> ifNumber;
    stack<int> loopNumber;
};

/* Main program */

void toMIPS(node* tree, string filename){
    info* currentInfo = new info;
    currentInfo->tree = tree;
    executeNode(currentInfo);
    string functionD = ".linkerHeader\n\nfunc_defined:\n";
    for(auto it : currentInfo->funcD){
        functionD = functionD + it.first + "\n";
    }
    string functionU = "\nfunc_undefined:\n";
    for(auto it : currentInfo->funcU){
        functionU = functionU + it.first + "@" + to_string(it.second) + "\n";
    }
    string variableD = "\nvar_defined:\n";
    ofstream ofs;
    const string outfilename = filename + ".asm";
    ofs.open(outfilename,ios::out);
    ofs << functionD;
    ofs << functionU;
    ofs << variableD;
    ofs << currentInfo->data;
    ofs << currentInfo->text;
    ofs << currentInfo->funcText;
    ofs.close();
    delete currentInfo;
}

/* Function */

void executeNode(info* &currentInfo){
    string type = currentInfo->tree->type;
    if (type == "init") j_init(currentInfo);
    else if (type == "not") j_not(currentInfo);
    else if (type == "assign") j_assign(currentInfo);
    else if (type == "print") j_print(currentInfo);
    else if (type == "input") j_input(currentInfo);
    else if (type == "while") j_while(currentInfo);
    else if (type == "continue") j_continue(currentInfo);
    else if (type == "break") j_break(currentInfo);
    else if (type == "if") j_if(currentInfo);
    else if (type == "else") j_else(currentInfo);
    else if (type == "usefunction") j_usefunction(currentInfo);
    else if (type == "function") j_function(currentInfo);
    else if (type == "return") j_return(currentInfo);
    else if (type == "main") j_main(currentInfo);
    else if (type == "fathernode") j_fathernode(currentInfo);
    else j_arithmatic(currentInfo);
    delete currentInfo->tree;
}

void j_init(info* &currentInfo){
    string var;
    if (currentInfo->funcflag == 0){
        var = "v" + currentInfo->tree->value2;
        currentInfo->varD[var] = currentInfo->addressC;
    }
    if (currentInfo->tree->value == "num"){
        currentInfo->data = currentInfo->data +  var + ":.int    # num" + "\n";
        currentInfo->addressC += 4;
    }
    else if (currentInfo->tree->value == "bool"){
        currentInfo->data = currentInfo->data +  var + ":.int     # bool" + "\n";
        currentInfo->addressC += 4;
    }
    (currentInfo->dataline)++;
    delete currentInfo->tree;
}

void j_not(info* &currentInfo){
    string var0 = "v" + currentInfo->tree->subtoken[0].value;
    string var1 = "v" + currentInfo->tree->subtoken[1].value;
    varTest(currentInfo,var0);
    varTest(currentInfo,var1);
    string id0 = to_string(currentInfo->varD[var0]) + "($sp)";
    string id1 = to_string(currentInfo->varD[var1]) + "($sp)";
    currentInfo->text = currentInfo->text
                      +"           lw $t1, " + id0 + "# not" + "\n"
                      +"           xori $t2,$t1,1\n"
                      +"           sw $t2, " + id1 + "\n";
    currentInfo->textline += 3;
    delete currentInfo->tree;
}

void j_arithmatic(info* &currentInfo){
    map<string, string> arithmaticMap = {
        {"add", "add"}, {"sub", "sub"},{"multi", "mult"},{"div", "div"},
        {"mod", "mod"},{"power", "pow"},{"less", "slt"},{"and", "and"},
        {"or", "or"}
    };
    string opreat = currentInfo->tree->type;
    string instrution = arithmaticMap[opreat];
    string type0 = currentInfo->tree->subtoken[0].type;
    string type1 = currentInfo->tree->subtoken[1].type;
    string num0 = currentInfo->tree->subtoken[0].value;
    string num1 = currentInfo->tree->subtoken[1].value;
    string num2 = currentInfo->tree->subtoken[2].value;
    string var0 = "v" + num0;
    string var1 = "v" + num1;
    string var2 = "v" + num2;
    varTest(currentInfo,var2);
    string id2 = to_string(currentInfo->varD[var2]) + "($sp)";
    string avar0 = "$a" + num0;
    string avar1 = "$a" + num1;
    string avar2 = "$a" + num2;
    if (type0 == "num") currentInfo->text = currentInfo->text + "           li $t1, " + num0 + "#" + opreat + "\n";
    else if (type0 == "id") {
        varTest(currentInfo,var0);
        string id0 = to_string(currentInfo->varD[var0]) + "($sp)";
        currentInfo->text = currentInfo->text + "           lw $t1, " + id0 + "#" + opreat + "\n";
    }
    if (type1 == "num") currentInfo->text = currentInfo->text + "           li $t2, " + num1 + "\n";
    else if (type1 == "id") {
        varTest(currentInfo,var1);
        string id1 = to_string(currentInfo->varD[var1]) + "($sp)";
        currentInfo->text = currentInfo->text + "           lw $t2, " + id1 + "\n";
    }
    if (opreat == "large"){
        currentInfo->text = currentInfo->text + "           slt $t3,$t2,$t1\n           sw $t3, " + id2 + "\n";
        currentInfo->textline += 4;
    }
    else if (opreat == "equal"){
        currentInfo->text = currentInfo->text
                          + "           slt $t3,$t2,$t1\n"
                          + "           slt $t4,$t1,$t2\n"
                          + "           or $t3,$t3,$t4\n"
                          + "           xori $t3,$t3,1\n"
                          + "           sw $t3, " + id2 + "\n";
        currentInfo->textline += 7;
    }
    else{
        currentInfo->text =  currentInfo->text + "           " + instrution + " $t3,$t1,$t2\n           sw $t3, " + id2 + "\n";
        currentInfo->textline += 4;
    }
}

void j_assign(info* &currentInfo){
     string assignValue = currentInfo->tree->subtoken[0].value;
     string var = "v" + currentInfo->tree->subtoken[1].value;
     varTest(currentInfo,var);
     string id = to_string(currentInfo->varD[var]) + "($sp)";
     if(assignValue == "true") assignValue = "1";
     else if(assignValue == "false") assignValue = "0";
     currentInfo->text = currentInfo->text
                       + "           li $t1, " + assignValue + "# assign \n"
                       + "           sw $t1, " + id +  "\n";
     currentInfo->textline += 2;
}

void j_print(info* &currentInfo){
    string strNum = "str" + to_string(currentInfo->prt);
    string strCnt = WString2String(currentInfo->tree->subtoken[0].wvalue);
    string strAdr = to_string(currentInfo->addressC);
    double tmp = strCnt.length();
    int strlength = ceil((tmp+1)/4)*4;
    if (currentInfo->tree->subtoken[0].type == "string"){
        currentInfo->data = currentInfo->data
                  + strNum + ":.asciiz \""  + strCnt + "\"\n";
        currentInfo->varD[strNum] = currentInfo->addressC;
        currentInfo->text = currentInfo->text
                  + "           ori $a0, $sp," + strAdr + "# print\n"
                  + "           addi $v0, $zero, 4  \n"
                  + "           syscall \n";
        currentInfo->prt += 1;
        currentInfo->addressC += strlength;
        currentInfo->textline += 3;
    }
    else if (currentInfo->tree->subtoken[0].type == "id"){
        string var = "v" + currentInfo->tree->subtoken[0].value;
        varTest(currentInfo,var);
        string id = to_string(currentInfo->varD[var]) + "($sp)";
        currentInfo->text = currentInfo->text
                  + "           lw $a0, " + id + "# print\n"
                  + "           addi $v0, $zero, 1  \n"
                  + "           syscall \n";
        currentInfo->textline += 3;
    }
}

void j_input(info* &currentInfo){
    string var = "v" + currentInfo->tree->subtoken[0].value;
    varTest(currentInfo,var);
    string id = to_string(currentInfo->varD[var]) + "($sp)";
    currentInfo->text = currentInfo->text
                      + "           addi $v0, $zero, 5 #input \n"
                      + "           syscall \n"
                      + "           sw $v0, " + id + "\n";
    currentInfo->textline += 3;
}

void j_while(info* &currentInfo){
    string loop = "loop" + to_string(currentInfo->loop);
    string judge = "v" + currentInfo->tree->value;
    varTest(currentInfo,judge);
    string idJ = to_string(currentInfo->varD[judge]) + "($sp)";
    currentInfo->loopNumber.push(currentInfo->loop);
    (currentInfo->loop)++;
    currentInfo->text = currentInfo->text
                      + loop +":nop\n"
                      + "           li $t1, 0\n"
                      + "           lw $t2," + idJ +"\n"
                      + "           beq $t1, $t2, end" + loop + "\n";
    currentInfo->textline += 4;
    info* tep = new info;
    tep->tree = currentInfo->tree;
    unsigned int num = (currentInfo->tree->subNode).size();
    for(unsigned int i = 0; i < num ;i++){
        currentInfo->forName = loop;
        currentInfo->tree = tep->tree->subNode[i];
        executeNode(currentInfo);
    }
    int currentloop = currentInfo->loopNumber.top();
    currentInfo->loopNumber.pop();
    loop = "loop" + to_string(currentloop);
    currentInfo->text = currentInfo->text
                      +"           j " + loop + "\n"
                      + "end" + loop + ":nop\n";
    currentInfo->textline += 2;
    delete tep;
}

void j_continue(info* &currentInfo){
     currentInfo->text = currentInfo->text
                       + "           j " + currentInfo->forName + "\n";
     currentInfo->textline += 1;
}

void j_break(info* &currentInfo){
    currentInfo->text = currentInfo->text
            + "           j end" + currentInfo->forName + "\n";
    currentInfo->textline += 1;
}

void j_if(info* &currentInfo){
    string endif = "endif" + to_string(currentInfo->ifNum);
    string judge = "v" + currentInfo->tree->value;
    varTest(currentInfo,judge);
    string idJ = to_string(currentInfo->varD[judge]) + "($sp)";
    currentInfo->ifNumber.push(currentInfo->ifNum);
    currentInfo->ifNum++;
    currentInfo->text = currentInfo->text
                      + "           li $t1, 0 # if\n"
                      + "           lw $t2, " + idJ +"\n"
                      + "           beq $t1, $t2, " + endif + "\n";
    currentInfo->textline += 3;
    info* tep = new info;
    tep->tree = currentInfo->tree;
    unsigned int num = (currentInfo->tree->subNode).size();
    for(unsigned int i = 0; i < num ;i++){
        currentInfo->tree = tep->tree->subNode[i];
        executeNode(currentInfo);
    }
    int currentIf = currentInfo->ifNumber.top();
    currentInfo->ifNumber.pop();
    endif = "endif" + to_string(currentIf);
    string endif1 = "endif" + to_string(currentInfo->ifNum);
    currentInfo->tree = tep->tree;
    if (currentInfo->tree->elsenode != NULL){
        currentInfo->text = currentInfo->text
                      + "           j " + endif1 + "\n"
                      + endif + ":nop\n";
        currentInfo->textline += 2;
    }
    else{
        currentInfo->text = currentInfo->text
                      + endif + ":nop\n";
        currentInfo->textline += 1;

    }
    if(currentInfo->tree->elsenode != NULL){
        currentInfo->tree = currentInfo->tree->elsenode;
        currentInfo->tree->value = 1;
        executeNode(currentInfo);
    }
    delete tep;
}

void j_else(info* &currentInfo){
    string endif = "endif" + to_string(currentInfo->ifNum);
//    string judge = "v" + currentInfo->tree->value;
//    varTest(currentInfo,judge);
//    string idJ = to_string(currentInfo->varD[judge]) + "($sp)";
    currentInfo->ifNumber.push(currentInfo->ifNum);
    currentInfo->ifNum++;
    info* tep = new info;
    tep->tree = currentInfo->tree;
    unsigned int num = (currentInfo->tree->subNode).size();
    for(unsigned int i = 0; i < num ;i++){
        currentInfo->tree = tep->tree->subNode[i];
        executeNode(currentInfo);
    }
    int currentIf = currentInfo->ifNumber.top();
    currentInfo->ifNumber.pop();
    endif = "endif" + to_string(currentIf);
    string endif1 = "endif" + to_string(currentInfo->ifNum);
    currentInfo->text = currentInfo->text
                  + endif + ":nop\n";
    currentInfo->textline += 1;
    currentInfo->tree = tep->tree;
    delete tep;
}

void j_usefunction(info* &currentInfo){
    int size = (currentInfo->tree->subtoken).size()-1;
    string var;
    string id;
    string var1 = "v" + currentInfo->tree->subtoken[size].value;
    string id1 = to_string(currentInfo->varD[var1]) + "($sp)";
    string funcNum = "func" + currentInfo->tree->value;
    for(int i = 0; i < size; i++){
        var = "v" + currentInfo->tree->subtoken[i].value;
        id = to_string(currentInfo->varD[var]) + "($sp)";
        currentInfo->text = currentInfo->text
                + "           lw $a" + to_string(i) + ", " + id + "\n";
        currentInfo->textline = currentInfo->textline + 1;
    }
    if(currentInfo->funcD[funcNum] == NULL){
        currentInfo->funcD.erase(funcNum);
        currentInfo->funcU[funcNum] = currentInfo->textline + 1;

    }
    currentInfo->text = currentInfo->text
            + "           jal " + funcNum + " #usefunction\n"
            + "           lw $ra, 0($sp)\n"
            + "           sw $v0, " + id1 + " \n";

    currentInfo->textline = currentInfo->textline + size +3;
}


void j_function(info* &currentInfo){
    map<string,int> varAddr;
    int address = 4;
    int inputNum = stringToInt(currentInfo->tree->value2);
    string var;
    for(auto it :currentInfo->tree->id){
        var = "v"+it.first;
        varAddr[var] = address;
        address += 4;
    }
    string funcNum = "func" + currentInfo->tree->value;
    currentInfo->funcText = currentInfo->funcText
            +  funcNum + ":nop\n"
            + "           lui $t0, 1\n"
            + "           sub $sp, $sp, $t0\n"
            + "           sw $ra, 0($sp)\n";
    for (int i = 1; i <= inputNum; i++){
         currentInfo->funcText = currentInfo->funcText
                 + "           sw $a" + to_string(i-1) + ", " + to_string(4*i) + "($sp)\n";
    }
    currentInfo->textline = currentInfo->textline + inputNum + 4;
    info* tep = new info;
    tep->varD = varAddr;
    tep->text = "";
    tep->funcflag = 1;
    unsigned int num = (currentInfo->tree->subNode).size();
    for(unsigned int i = 0; i < num ;i++){
        tep->tree = currentInfo->tree->subNode[i];
        executeNode(tep);
    }
    for(auto it : tep->funcU){
        currentInfo->funcU[it.first] = 0;
    }
    currentInfo->funcText += tep->text;
    currentInfo->textline = currentInfo->textline + tep->textline;
    currentInfo->funcD[funcNum] = 0;
    currentInfo->funcU.erase(funcNum);
    delete tep;
}

void j_return(info* &currentInfo){
    string var = "v" + currentInfo->tree->value;
    varTest(currentInfo,var);
    string id = to_string(currentInfo->varD[var]) + "($sp)";
    currentInfo->text = currentInfo->text
            + "           lw $t0, " + id + "\n"
            + "           ori $v0, $t0, 0\n"
            + "           lui $t0, 1\n"
            + "           add $sp, $sp, $t0\n"
            + "           jr $ra\n";
    currentInfo->textline += 5;
}

void j_main(info* &currentInfo){
    int BeginPst = currentInfo->memorySize;
    currentInfo->text = currentInfo->text
                       + "main:nop\n"
                       + "           lui $sp," + to_string(BeginPst) + "\n";
    currentInfo->textline += 2;
    node* mainNode = currentInfo->tree;
    for(node* tree : mainNode->subNode){
        currentInfo->funcName = "main";
        currentInfo->tree = tree;
        executeNode(currentInfo);
    }
    currentInfo->text = currentInfo->text +
                        "           addi $v0, $zero, 10\n" +
                        "           syscall\n";
    currentInfo->textline += 2;
}

void j_fathernode(info* &currentInfo){
    node* fatherNode = currentInfo->tree;
    for(node* tree : fatherNode->subNode){
        currentInfo->tree = tree;
        executeNode(currentInfo);
    }
}

/* Assist Function */

void varTest(info* &currentInfo,string var){
    if (currentInfo->varD.count(var) == 0){
        throw string("错误：使用未定义变量");
    }
}

std::string WString2String(const std::wstring& ws){
    std::string strLocale = setlocale(LC_ALL, "");
    const wchar_t* wchSrc = ws.c_str();
    size_t nDestSize = wcstombs(NULL, wchSrc, 0) + 1;
    char *chDest = new char[nDestSize];
    memset(chDest, 0, nDestSize);
    wcstombs(chDest, wchSrc, nDestSize);
    std::string strResult = chDest;
    delete[]chDest;
    setlocale(LC_ALL, strLocale.c_str());
    return strResult;
}

int stringToInt(string str){
    int i;
    istringstream is (str);
    is >> i;
    return i;
}
