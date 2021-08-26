#ifndef ASSEMBER_522_H
#define ASSEMBER_522_H

#include <string>
#include <map>
#include <vector>

/*
 * Function: split
 * ------------------------------
 * split string s into vector v by detecting element c
 */
void split(std::string s, std::vector<std::string>& v, std::string c);


/*
 * Function: scanForUndefined_func
 * ------------------------------
 * find the undefined function in the infile
 */
void scanForUndefined_func(std::ifstream & infile, std::map<std::string, std::string> & undefined_funcs);

/*
 * function: scanForLabel
 * ************************
 * this funciton is used to scan through the file
 * and get the label and its address, and map them by using map
 */
void scanForLabel(std::ifstream & infile, std::map<std::string, std::string> & labels);


/*
 * Function: copy_header
 * ------------------------------
 * copy header file of infile to outfile
 */
void copy_header(std::ifstream &infile, std::ofstream &outfile);


/*
 * Function: copy_data
 * ------------------------------
 * copy header file of infile to outfile
 */
void copy_data(std::ifstream &infile, std::ofstream &outfile);


/*
 * Function: assToBin
 * ------------------------------
 * convert mips code to binary code
 */
void assToBin(std::ifstream & infile, std::ofstream & outfile);


/*
 * Function: lineToString
 * ------------------------------
 * change each element of a vector to binary code and gather them to a string
 */
std::string lineToString(std::vector<std::string> line);


/*
 * Function: assembler
 * ------------------------------
 * this function realize what assembler did.
 * it will receive a vector with file path of each file in it.
 * it will convert each .asm file to .o file, with machine code
 * and deliver the vector to linker.
 */
std::vector<std::string> assembler(std::vector<std::string> compilerFile);

#endif // ASSEMBER_522_H
