#pragma once
#include <string>
#include <vector>
#include "unit.h"


struct Tokeniser
{
    std::string expr;
    std::vector<Unit> tokens;

    Tokeniser(std::string _file);
    Tokeniser() = default;
    void Tokenise();
};