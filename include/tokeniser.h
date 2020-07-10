#pragma once
#include "unit.h"
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

struct Tokeniser
{
    std::string expr;
    std::vector<Unit> tokens;

    Tokeniser(std::string _file);
    Tokeniser() = default;
    void Tokenise();
};