#ifndef LINKER_L_H
#define LINKER_L_H
#include <iostream>
#include <fstream>
#include <vector>

/*
 * Function: copyDataSection
 * ------------------------------
 * copy the Data Section of infile to outfile.
 */
void copyDataSection(std::ifstream &infile, std::ofstream &outfile);


/*
 * Function: linker_l
 * ------------------------------
 * this function will link all the file from file path
 * stored in the vector fileAddrs
 */
std::string linker_l (std::vector<std::string> fileAddrs);

#endif // LINKER_L_H
