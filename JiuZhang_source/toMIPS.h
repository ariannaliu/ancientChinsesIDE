#ifndef TOMIPS_H
#define TOMIPS_H

#include "Lcollection.h"
#include <fstream>
#include <string>
#include <windows.h>
#include <iostream>
#include <map>
#include <sstream>
#include <math.h>
#include <stack>

using namespace std;


/*
 * Function: toMIPS
 * ------------------------------
 * recognize nodes to mips codes.
 */
void toMIPS(node* j_tree,std::string filename);

#endif // TOMIPS_H
