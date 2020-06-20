#pragma once
#include "unit.h"
#include <string>
#include <vector>

struct Tokeniser
{
    std::string expr;
    std::vector<Unit> tokens;

    Tokeniser(std::string _file);
    Tokeniser() = default;
    void Tokenise();
};