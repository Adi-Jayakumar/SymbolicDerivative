#include "node.h"
#include <cassert>
#include <map>
#include <string>
#pragma once

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