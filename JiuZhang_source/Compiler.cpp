#include "Compiler.h"

using namespace std;

vector<string> Compiler(vector<std::string> editorFilePath, vector<std::wstring> editorFileContent)
{
    map<wstring, int> idList;
    vector<string> compilerFile;
    int count = editorFilePath.size();

    for (int i = 0; i < count; i++){
        vector<token>  testtoken = lexical_analysis(editorFileContent[i], idList);
        node* testnode = syntax_analysis(testtoken);
        toMIPS(testnode, editorFilePath[i]);
        compilerFile.push_back(editorFilePath[i]);
    }

    return compilerFile;
}
