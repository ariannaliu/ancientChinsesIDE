/*
 * File: Compiler.h
 * ------------------
 * This file defines a compiler.
 */

#ifndef COMPILER_H
#define COMPILER_H

#include <iostream>
#include "Lexical_Analysis.h"
#include "Syntax_Analysis.h"
#include "toMIPS.h"
#include <string>
#include <fstream>

/*
 * Function: Compiler
 * ------------------------------
 * Compile multiple .jiuzhang files into .asm files.
 */
vector<string> Compiler(vector<std::string> editorFilePath, vector<std::wstring> editorFileContent);

#endif // COMPILER_H
