#pragma once
#include <cassert>
#include "node.h"
#include "simplify.h"

namespace Differentiate
{

    Node *Differentiate(Node *source, std::string s);
    Node *Derivative(Node *source, std::string &s);

} // namespace Differentiate