#include "node.h"
#pragma once

namespace Simplify
{

    Node *Simplify(Node *n);

    /*----- Unary node simplifications -----*/

    // Simplifies ---+-+...(...) to -(...) or (...) accordingly
    Node *SimplifyNegation(Unary *source);
    // Simplfiies f(num) to a Num node which has the result of the function
    Node *EvaluateFunction(Unary *source);

    /*----- Multi node simplifications -----*/

    // Simplfifies a Multi node of multiplications and divisions into a Multi
    // node with one divide at the top and 2 children which are the numerators
    // and denominators which are Multi nodes whose operation is multiplication
    Node *MulCanonicalForm(Multi *source);

    // Simplfifies a Multi node of additions and subtractions into a Multi
    // node with one subtraction at the top and 2 children which are the additive
    // and subtractive parts which are Multi nodes operation is addition
    Node *AddCanoncialForm(Multi *source);

    Node *SimplifyAddition(Multi *soruce);
    Node *SimplifyMultiplication(Multi *source);
} // namespace Simplify