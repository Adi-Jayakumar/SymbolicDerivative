#pragma once
#include <unordered_map>

enum class Token
{
    NUM,
    VAR,
    ADD,
    SUB,
    MUL,
    DIV,
    POW,
    OP,
    CP,
    LN,
    EXP,
    SIN,
    COS,
    TAN,
    EoF
};

enum class Operator
{
    ADD,
    SUB,
    MUL,
    DIV,
    POW,
    LN,
    EXP,
    SIN,
    COS,
    TAN,
    NONE
};

namespace Utility
{
    inline Operator TokenToOperator(Token t)
    {

        switch (t)
        {
        case Token::ADD:
            return Operator::ADD;
        case Token::SUB:
            return Operator::SUB;
        case Token::MUL:
            return Operator::MUL;
        case Token::DIV:
            return Operator::DIV;
        case Token::POW:
            return Operator::POW;
        case Token::LN:
            return Operator::LN;
        case Token::EXP:
            return Operator::EXP;
        case Token::SIN:
            return Operator::SIN;
        case Token::COS:
            return Operator::COS;
        case Token::TAN:
            return Operator::TAN;
        default:
            return Operator::NONE;
        }
    }
} // namespace Utility