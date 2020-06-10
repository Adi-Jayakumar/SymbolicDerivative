#include "node.h"
#include <cmath>

Num::Num(double _val)
{
    type = NodeType::NumNode;
    if (fabs(_val - (int)_val) < 1e-14)
        val = (int)_val;
    else
        val = _val;
}

std::string Num::Print()
{
    if (fabs(val - (int)val) < 1e-14)
        return std::to_string((int)val);
    return std::to_string(val);
}

Node *Num::Differentiate()
{
    return new Num(0);
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

std::string Var::Print()
{
    return id;
}

Node *Var::Differentiate()
{
    return new Num(1);
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

std::string Unary::Print()
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

    s += arg->Print();
    return s;
}

Node *Unary::Differentiate()
{
    if (op == Token::ADD)
        return arg->Differentiate();
    else if (op == Token::SUB)
        return new Unary(arg->Differentiate(), Token::SUB);
    else if (op == Token::LN)
        return new BinaryNode(arg->Differentiate(), arg, Token::DIV);
    else if (op == Token::EXP)
        return new BinaryNode(arg->Differentiate(), new Unary(arg, Token::EXP), Token::MUL);
    else if (op == Token::SIN)
        return new BinaryNode(arg->Differentiate(), new Unary(arg, Token::COS), Token::MUL);
    else if (op == Token::COS)
        return new BinaryNode(arg->Differentiate(), new Unary(new Unary(arg, Token::SIN), Token::SUB), Token::MUL);
    else if (op == Token::TAN)
        return new BinaryNode(arg->Differentiate(), new BinaryNode(new Unary(arg, Token::COS), new Num(-2), Token::POW), Token::MUL);
    return new Num(0);
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
        return new Unary(BaseArg(), Token::SUB);
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

BinaryNode::BinaryNode(Node *_lhs, Node *_rhs, Token _op)
{
    type = NodeType::Binary;
    lhs = _lhs;
    rhs = _rhs;
    op = _op;
}

std::string BinaryNode::Print()
{
    std::string s = "(";
    s += lhs->Print() + " ";
    if (op == Token::ADD)
        s += "+";
    else if (op == Token::SUB)
        s += "-";
    else if (op == Token::MUL)
        s += "*";
    else if (op == Token::DIV)
        s += "/";
    else if (op == Token::POW)
        s += "^";
    else if (op == Token::OP)
        s += "(";
    else if (op == Token::CP)
        s += ")";
    // else
    //     s += std::to_string(op);

    s += " " + rhs->Print() + ")";

    return s;
}

Node *BinaryNode::Differentiate()
{
    // (f +- g)' = f' +- g'
    if (op == Token::ADD || op == Token::SUB)
        return new BinaryNode(lhs->Differentiate(), rhs->Differentiate(), op);

    // (fg)' = f'g + g'f
    else if (op == Token::MUL)
        return new BinaryNode(new BinaryNode(lhs->Differentiate(), rhs, Token::MUL), new BinaryNode(lhs, rhs->Differentiate(), Token::MUL), Token::ADD);

    // (f/g)' = (f'g-g'f)/g*g
    else if (op == Token::DIV)
        return new BinaryNode(new BinaryNode(new BinaryNode(lhs->Differentiate(), rhs, Token::MUL), new BinaryNode(rhs->Differentiate(), lhs, Token::MUL), Token::SUB), new BinaryNode(rhs, rhs, Token::MUL), Token::DIV);

    // (f^g)' = (e^(f*ln(g)))' = ... = f^g * (f'*ln(g) + f*g'/g)
    else if (op == Token::POW)
        return new BinaryNode(new BinaryNode(lhs, rhs, Token::POW), new BinaryNode(new BinaryNode(rhs->Differentiate(), new Unary(lhs, Token::LN), Token::MUL), new BinaryNode(new BinaryNode(lhs->Differentiate(), rhs, Token::MUL), lhs, Token::DIV), Token::ADD), Token::MUL);
    return new Num(0);
}

Node *BinaryNode::Simplify()
{
    if (lhs->type == NodeType::NumNode)
    {
        Num *l = dynamic_cast<Num *>(lhs);
        if (fabs(l->val - 1) < 1e-14)
        {
            // case: 1 * (...) = (...)
            if (op == Token::MUL)
                return rhs;
        }
        else if (fabs(l->val) < 1e-14)
        {
            // case: 0 * (...) = 0
            if (op == Token::MUL)
                return new Num(0);
            // case: 0 + (...) = (...)
            if (op == Token::ADD)
                return rhs;
        }
    }
    if (rhs->type == NodeType::NumNode)
    {
        Num *r = dynamic_cast<Num *>(rhs);
        if (fabs(r->val - 1) < 1e-14)
        {
            // case: (...) * 1 = (...)
            if (op == Token::MUL)
                return lhs;
        }
        else if (fabs(r->val) < 1e-14)
        {
            // case: (...) * 0 = 0
            if (op == Token::MUL)
                return new Num(0);
            // case: (...) + 0 = (...)
            if (op == Token::ADD)
                return lhs;
        }
    }
    if (lhs->type == NodeType::NumNode && rhs->type == NodeType::NumNode)
    {
        Num *l = dynamic_cast<Num *>(lhs);
        Num *r = dynamic_cast<Num *>(rhs);
        if (op == Token::ADD)
            return new Num(l->val + r->val);
        else if (op == Token::SUB)
            return new Num(l->val - r->val);
        else if (op == Token::MUL)
            return new Num(l->val * r->val);
        else if (op == Token::DIV)
            return new Num(l->val / r->val);
        else if (op == Token::POW)
            return new Num(pow(l->val, r->val));
    }

    if (lhs->type == NodeType::VarNode && rhs->type == NodeType::NumNode)
    {
        Num* n = dynamic_cast<Num*>(rhs);
        if(fabs(n->val - 1) < 1e-14)
            return lhs;
    }

    return new BinaryNode(lhs->Simplify(), rhs->Simplify(), op);
}