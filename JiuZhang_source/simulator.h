#ifndef SIMULATOR_H
#define SIMULATOR_H
#include <iostream>
#include <GUI.h>



/*
 * Function: simulator
 * ------------------------------
 * this is the main function of a simulator
 * it will receive a string path from linker
 * simulate the machine code and connect to the GUI.
 */
void simulator(std::string simulatorAddr,myjz_519 *windows);


/*
 * Function: findData
 * ------------------------------
 * find whether the infile has a data section.
 */
bool findData(std::ifstream & infile);

/*
 * Function: fakeToReal
 * ------------------------------
 * turn the fake address to real address
 */
intptr_t fakeToReal(int fakeDecimalAddr, void* pBaseAddr);

/*
 * Function: putBin
 * ------------------------------
 * put binary code to memory.
 */
void putBin(std::ifstream & infile, int* &tptr);

/*
 * Function: binaryStringToInt
 * ------------------------------
 * change binary code into integer
 */
int binaryStringToInt(std::string s);

/*
 * Function: machineCycle
 * ------------------------------
 * doing the machine Cycle
 * including moving pc and execute binary code one line after another
 */
void machineCycle(int* &PC, int reg[34], void* baseAddr, void* &heap);

#endif // SIMULATOR_H
