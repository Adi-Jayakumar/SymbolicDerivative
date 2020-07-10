#pragma once
#include "node.h"
#include <cassert>
#include <iostream>
#include <map>
#include <string>

namespace DOT
{
    struct Point
    {
        NodeType nt;
        std::string s;
    };
    void Tree(Node *n, std::map<int, Point> &map, int &label, std::string &DOT);
    std::string ToDOT(Node *n);
} // namespace DOT