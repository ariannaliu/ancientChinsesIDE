/*
 * File: Syntax_Analysis.h
 * ------------------
 * This file contains the syntax analysis part of the compiler.
 */

#ifndef SYNTAX_ANALYSIS_H
#define SYNTAX_ANALYSIS_H

#include "Lcollection.h"

/*
 * Function: syntax_analysis
 * ------------------------------
 * Performing the syntax analysis.
 */
node* syntax_analysis(std::vector<token> token_list);

/*
 * Function: size_test
 * ------------------------------
 * Test the size of the tokenbuff.
 */
void size_test(int size, const std::vector<token> tokenbuff, int linenum);

#endif // SYNTAX_ANALYSIS_H
