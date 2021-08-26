/*
 * File: Lcollection.h
 * ------------------
 * This file defines low-level data structures for compiler.
 */

#ifndef LCOLLECTION_H
#define LCOLLECTION_H

#include <string>
#include <vector>
#include <map>

/*
 * Type: token
 * ---------
 * This type represents an individual token for each lexical element.
 */
struct token {
    std::string type = "";
    std::string value = "";
    std::wstring wvalue = L"";
};

/*
 * Type: node
 * ---------
 * This type represents a node as a syntax tree.
 */
struct node {
    int line;
    std::string type = "";
    std::string value = "";
    std::string value2 = "";
    std::vector<node*> subNode;
    std::vector<token> subtoken;
    node* elsenode;
    std::map<std::string, std::string> id;
};

#endif // LCOLLECTION_H
