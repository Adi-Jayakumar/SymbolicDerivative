#pragma once
#include <string>
#include "token.h"

struct Unit
{
    Token t;
    std::string var, func;
    double val;

    Unit(Token _t, std::string _var, double _val, std::string _func);
};