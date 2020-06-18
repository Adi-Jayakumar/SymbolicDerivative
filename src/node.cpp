#include "node.h"
#include <algorithm>
#include <cmath>

Node *Node::Reduce()
{
    Node *prev = this;
    Node *next = nullptr;
    while (prev != next)
    {
        next = prev->Simplify();
        prev = next;
    }
    return prev;
}

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

std::string Num::SelfToString()
{
    return ToString();
}

Node *Num::Simplify()
{
    return this;
}

Var::Var(std::string _id)
{
    type = NodeType::VarNode;
    id = _id;
}

std::string Var::ToString()
{
    return SelfToString();
}

std::string Var::SelfToString()
{
    return id;
}

Node *Var::Simplify()
{
    return this;
}

Unary::Unary(Node *_arg, Token _op)
{
    type = NodeType::UnaryNode;
    arg = _arg;
    op = _op;
}

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

Node *Unary::BaseArg()
{
    if (op != Token::ADD && op != Token::SUB)
        return this;
    if (arg->type != NodeType::UnaryNode)
        return arg;
    else
    {
        Unary *u = dynamic_cast<Unary *>(arg);
        return u->BaseArg();
    }
}

int Unary::Parity(int &p)
{
    if (arg->type != NodeType::UnaryNode)
    {
        if (op == Token::SUB)
            p++;
        return p;
    }
    else
    {
        Unary *u = dynamic_cast<Unary *>(arg);
        if (op == Token::SUB)
        {
            p++;
            return u->Parity(p);
        }
        else
            return u->Parity(p);
    }
}

Node *Unary::SimplifyNegations()
{
    int p = 0;
    int parity = Parity(p);
    if (parity % 2 == 0)
        return BaseArg();
    else
        return new Multi(new Num(-1), BaseArg(), Token::MUL);
    return this;
}

Node *Unary::Simplify()
{
    if (op == Token::LN && arg->type == NodeType::NumNode)
    {
        Num *n = dynamic_cast<Num *>(arg);
        if (fabs(n->val - 1) < 1e-14)
            return new Num(0);
    }

    if ((op == Token::SIN || op == Token::TAN) && arg->type == NodeType::NumNode)
    {
        Num *n = dynamic_cast<Num *>(arg);
        if (fabs(n->val) < 1e-14)
            return new Num(0);
    }

    return SimplifyNegations();
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
    type = NodeType::Multi;
    args.emplace_back(arg);
}

Multi::Multi(Node *_lhs, Node *_rhs, Token _op)
{
    op = _op;
    type = NodeType::Multi;
    args.emplace_back(_lhs);
    args.emplace_back(_rhs);
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
            else if (op == Token::MUL)
                s += " * ";
            else if (op == Token::DIV)
                s += " / ";
            else if (op == Token::POW)
                s += " ^ ";
        }
        s += args[i]->ToString();
    }
    s += ")";
    return s;
}

std::vector<int> Multi::FindDivisons()
{
    std::vector<int> divs;
    for (long unsigned int i = 0; i < args.size(); ++i)
    {
        if (args[i]->type == NodeType::Multi)
        {
            Multi *cand = dynamic_cast<Multi *>(args[i]);
            if (cand->op == Token::DIV)
                divs.emplace_back(i);
        }
    }
    return divs;
}

Node *Multi::MulStdForm()
{
    std::vector<int> divs = FindDivisons();
    if (divs.size() == 0)
        return this;
    Multi *numerator = new Multi(Token::MUL);
    Multi *denominator = new Multi(Token::MUL);
    for (long unsigned int i = 0; i < args.size(); i++)
    {
        if (std::find(divs.begin(), divs.end(), i) != divs.end())
        {
            Multi *division = dynamic_cast<Multi *>(args[i]);
            denominator->AddArg(division->args[1]);
        }
        else
            numerator->AddArg(args[i]);
    }
    return new Multi(numerator, denominator, Token::DIV);
}

Node *Multi::Simplify()
{
    if (op == Token::ADD)
    {
        Multi *m = new Multi(Token::ADD);
        for (long unsigned int i = 0; i < args.size(); ++i)
        {
            m->AddArg(args[i]->Simplify());
        }
        return m;
    }
    else if (op == Token::MUL)
        return MulStdForm();

    return this;
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