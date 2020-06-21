#include "node.h"
#include <algorithm>
#include <cmath>

Node::~Node() {}

Num::Num(double _val)
{
    type = NodeType::NumNode;
    if (fabs(_val - (int)_val) < 1e-14)
        val = (int)_val;
    else
        val = _val;
}

std::string Num::ToString()
{
    if (fabs(val - (int)val) < 1e-14)
        return std::to_string((int)val);
    return std::to_string(val);
}

std::string Num::SelfToString() { return ToString(); }

Var::Var(std::string _id)
{
    id = _id;
    type = NodeType::VarNode;
}

std::string Var::ToString() { return SelfToString(); }

std::string Var::SelfToString() { return id; }

Unary::Unary(Node *_arg, Token _op)
{
    arg = _arg;
    op = _op;
    type = NodeType::UnaryNode;
}

Unary::~Unary() { delete arg; }

std::string Unary::ToString()
{
    std::string s = "";
    if (op == Token::ADD)
        s += "+";
    else if (op == Token::SUB)
        s += "-";
    else if (op == Token::LN)
        s += "ln";
    else if (op == Token::EXP)
        s += "exp";
    else if (op == Token::SIN)
        s += "sin";
    else if (op == Token::COS)
        s += "cos";
    else if (op == Token::TAN)
        s += "tan";

    s += arg->ToString();
    return s;
}

std::string Unary::SelfToString()
{
    if (op == Token::SUB)
        return "-";
    else if (op == Token::SIN)
        return "sin";
    else if (op == Token::COS)
        return "cos";
    else if (op == Token::TAN)
        return "tan";
    else if (op == Token::EXP)
        return "exp";
    else if (op == Token::LN)
        return "ln";
    return "";
}

Multi::Multi(Token _op)
{
    op = _op;
    type = NodeType::Multi;
}

Multi::Multi(Node *arg, Token _op)
{
    op = _op;
    args.emplace_back(arg);
    type = NodeType::Multi;
}

Multi::Multi(Node *_lhs, Node *_rhs, Token _op)
{
    op = _op;
    args.emplace_back(_lhs);
    args.emplace_back(_rhs);
    type = NodeType::Multi;
}

Multi::Multi(std::vector<Node *> _args, Token _op)
{
    args = _args;
    op = _op;
    type = NodeType::Multi;
}

Multi::~Multi()
{
    for (size_t i = 0; i < args.size(); i++)
    {
        delete args[i];
    }
    args.clear();
}

void Multi::AddArg(Node *a)
{
    args.emplace_back(a);
}

std::string Multi::ToString()
{
    std::string s = "(";
    for (long unsigned int i = 0; i < args.size(); ++i)
    {
        if (i != 0)
        {
            if (op == Token::ADD)
                s += " + ";
            else if (op == Token::SUB)
                s += " - ";
            else if (op == Token::MUL)
                s += " * ";
            else if (op == Token::DIV)
                s += " / ";
            else if (op == Token::POW)
                s += " ^ ";
        }
        else
        {
            if (op == Token::LN)
                s += "ln";
            else if (op == Token::EXP)
                s += "exp";
            else if (op == Token::SIN)
                s += "sin";
            else if (op == Token::COS)
                s += "cos";
            else if (op == Token::TAN)
                s += "tan";
        }
        s += args[i]->ToString();
    }
    s += ")";
    return s;
}

std::string Multi::SelfToString()
{
    if (op == Token::ADD)
        return "+";
    else if (op == Token::MUL)
        return "*";
    else if (op == Token::DIV)
        return "/";
    else if (op == Token::POW)
        return "^";
    return "";
}