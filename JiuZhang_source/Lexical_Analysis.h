/*
 * File: Lexical_Analysis.h
 * ------------------
 * This file contains the lexical analysis part of the compiler.
 */

#ifndef LEXICAL_ANALYSIS_H
#define LEXICAL_ANALYSIS_H

#include "Lcollection.h"

/*
 * Function: lexical_analysis
 * ------------------------------
 * Performing the lexical analysis.
 */
std::vector<token> lexical_analysis(const std::wstring text, std::map<std::wstring, int> &idList);

#endif // LEXICAL_ANALYSIS_H
