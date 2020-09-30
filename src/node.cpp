#include "node.h"

bool operator==(const Node &lhs, const Node &rhs)
{
    if (lhs.type != rhs.type)
        return false;

    if (lhs.type == NodeType::NumNode)
    {
        const Num *nLHS = static_cast<const Num *>(&lhs);
        const Num *nRHS = static_cast<const Num *>(&rhs);
        return (nLHS->val == nRHS->val);
    }
    else if (lhs.type == NodeType::VarNode)
    {
        const Var *vLHS = static_cast<const Var *>(&lhs);
        const Var *vRHS = static_cast<const Var *>(&rhs);
        return (vLHS->id == vRHS->id);
    }
    else
    {
        const Multi *mLHS = static_cast<const Multi *>(&lhs);
        const Multi *mRHS = static_cast<const Multi *>(&rhs);

        if (mLHS->op != mRHS->op)
            return false;
        switch (mLHS->op)
        {
        // non-commutative operators
        case Operator::SUB:
        case Operator::DIV:
        case Operator::POW:
        {
            assert(mLHS->args.size() == 2 && mRHS->args.size() == 2);
            return (mLHS->op == mRHS->op) && (*mLHS->args[0] == *mRHS->args[0]) && (*mLHS->args[1] == *mRHS->args[1]);
        }
        // commutative operators
        case Operator::MUL:
        case Operator::ADD:
        {
            if (mLHS->args.size() != mRHS->args.size() || mLHS->op != mRHS->op)
                return false;

            std::unordered_map<Node *, bool> isLHSContained;
            std::unordered_map<Node *, bool> isRHSContained;

            for (Node *n : mLHS->args)
            {
                for (Node *m : mRHS->args)
                {
                    if (*n == *m && !isRHSContained[m])
                    {
                        isLHSContained[n] = true;
                        isRHSContained[m] = true;
                        break;
                    }
                }
            }

            for (Node *n : mLHS->args)
            {
                if (!isLHSContained[n])
                    return false;
            }
            for (Node *n : mRHS->args)
            {
                if (!isRHSContained[n])
                    return false;
            }
            return true;
        }
        // only case left is functions
        default:
        {
            assert((mLHS->args.size() == 1) && (mRHS->args.size() == 1));
            return (mLHS->op == mRHS->op) && (*mLHS->args[0] == *mRHS->args[0]);
        }
        }
    }
}

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

std::string Var::ToString()
{
    return id;
}

std::string Var::SelfToString() {return ToString();}


Multi::Multi(Operator _op)
{
    op = _op;
    type = NodeType::MultiNode;
}

Multi::Multi(Node *arg, Operator _op)
{
    op = _op;
    args.emplace_back(arg);
    type = NodeType::MultiNode;
}

Multi::Multi(Node *_lhs, Node *_rhs, Operator _op)
{
    op = _op;
    args.emplace_back(_lhs);
    args.emplace_back(_rhs);
    type = NodeType::MultiNode;
}

Multi::Multi(std::vector<Node *> _args, Operator _op)
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
            if (op == Operator::ADD)
                s += " + ";
            else if (op == Operator::SUB)
                s += " - ";
            else if (op == Operator::MUL)
                s += " * ";
            else if (op == Operator::DIV)
                s += " / ";
            else if (op == Operator::POW)
                s += " ^ ";
        }
        else
        {
            if (op == Operator::LN)
                s += "ln";
            else if (op == Operator::EXP)
                s += "exp";
            else if (op == Operator::SIN)
                s += "sin";
            else if (op == Operator::COS)
                s += "cos";
            else if (op == Operator::TAN)
                s += "tan";
        }
        s += args[i]->ToString();
    }
    s += ")";
    return s;
}

std::string Multi::SelfToString()
{
    if (op == Operator::ADD)
        return "+";
    else if (op == Operator::SUB)
        return " - ";
    else if (op == Operator::MUL)
        return "*";
    else if (op == Operator::DIV)
        return "/";
    else if (op == Operator::POW)
        return "^";
    if (op == Operator::LN)
        return "ln";
    else if (op == Operator::EXP)
        return "exp";
    else if (op == Operator::SIN)
        return "sin";
    else if (op == Operator::COS)
        return "cos";
    else if (op == Operator::TAN)
        return "tan";
    return "";
}
