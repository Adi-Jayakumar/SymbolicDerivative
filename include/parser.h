#pragma once
#include <iostream>
#include "node.h"
#include "tokeniser.h"

struct Parser
{
    Tokeniser tk;
    Parser(std::string _file);
    Node *Parse(int &i);
    Node *ParseMul(int &i);
    Node *ParseInd(int &i);
    Node *ParseUnit(int &i);
};