#include "node.h"
#include <algorithm>
#include <cassert>
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

bool Num::Equals(Node &rhs)
{
    if (rhs.type == NodeType::NumNode)
    {
        return dynamic_cast<Num *>(&rhs)->val == val;
    }
    return false;
}

std::string Var::ToString()
{
    return SelfToString();
}

std::string Var::SelfToString() { return id; }

bool Var::Equals(Node &rhs)
{
    if (rhs.type == NodeType::VarNode)
    {
        return dynamic_cast<Var *>(&rhs)->id == id;
    }
    return false;
}

Multi::Multi(Token _op)
{
    op = _op;
    type = NodeType::MultiNode;
}

Multi::Multi(Node *arg, Token _op)
{
    op = _op;
    args.emplace_back(arg);
    type = NodeType::MultiNode;
}

Multi::Multi(Node *_lhs, Node *_rhs, Token _op)
{
    op = _op;
    args.emplace_back(_lhs);
    args.emplace_back(_rhs);
    type = NodeType::MultiNode;
}

Multi::Multi(std::vector<Node *> _args, Token _op)
{
    args = _args;
    op = _op;
    type = NodeType::MultiNode;
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
    else if (op == Token::SUB)
        return " - ";
    else if (op == Token::MUL)
        return "*";
    else if (op == Token::DIV)
        return "/";
    else if (op == Token::POW)
        return "^";
    if (op == Token::LN)
        return "ln";
    else if (op == Token::EXP)
        return "exp";
    else if (op == Token::SIN)
        return "sin";
    else if (op == Token::COS)
        return "cos";
    else if (op == Token::TAN)
        return "tan";
    return "";
}

bool Multi::Equals(Node &rhs)
{
    if (rhs.type == NodeType::MultiNode)
    {
        Multi *r = dynamic_cast<Multi *>(&rhs);
        if (op != r->op)
            return false;
        switch (op)
        {
        case Token::SUB:
        case Token::DIV:
        case Token::POW:
        {
            assert(args.size() == 2 && r->args.size() == 2);
            return (op != r->op && args[0] == r->args[0]) && (args[1] == r->args[1]);
        }
        case Token::MUL:
        case Token::ADD:
        {
            if (args.size() != r->args.size() || op != r->op)
                return false;

            bool equal = true;
            for (Node *n : args)
            {
                bool isContained = false;
                for (Node *m : r->args)
                {
                    isContained = (n == m);
                    equal = equal || isContained;
                }
            }
            return equal;
        }
        default:
        {
            return false;
        }
        }
    }
    else
        return false;
    
}