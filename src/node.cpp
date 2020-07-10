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
            return (mLHS->op != mRHS->op && mLHS->args[0] == mRHS->args[0]) && (mLHS->args[1] == mRHS->args[1]);
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

            // std::cout << "Data in isLHSContained " << std::endl;
            // for (auto &[n, cont] : isLHSContained)
            // {
            //     std::cout << n->ToString() << " " << cont << std::endl;
            // }

            // std::cout << "Data in isRHSContained " << std::endl;
            // for (auto &[n, cont] : isRHSContained)
            // {
            //     std::cout << n->ToString() << " " << cont << std::endl;
            // }

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
            return (mLHS->op == mRHS->op) && (mLHS->args[0]->Equals(mRHS->args[0]));
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

bool Num::Equals(Node *rhs)
{
    if (rhs->type == NodeType::NumNode)
    {
        // return dynamic_cast<Num *>(rhs)->val == val;
        return static_cast<Num *>(rhs)->val == val;
    }
    return false;
}

std::string Var::ToString()
{
    return SelfToString();
}

std::string Var::SelfToString() { return id; }

bool Var::Equals(Node *rhs)
{
    if (rhs->type == NodeType::VarNode)
    {
        // return dynamic_cast<Var *>(rhs)->id == id;
        return static_cast<Var *>(rhs)->id == id;
    }
    return false;
}

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

bool Multi::Equals(Node *rhs)
{
    if (rhs->type == NodeType::MultiNode)
    {
        // Multi *r = dynamic_cast<Multi *>(rhs);
        Multi *r = static_cast<Multi *>(rhs);

        if (op != r->op)
            return false;
        switch (op)
        {
        // non commutative operators (cheap)
        case Operator::SUB:
        case Operator::DIV:
        case Operator::POW:
        {
            assert(args.size() == 2 && r->args.size() == 2);
            return (op != r->op && args[0] == r->args[0]) && (args[1] == r->args[1]);
        }
        // commutative operators (expensive)
        case Operator::MUL:
        case Operator::ADD:
        {
            if (args.size() != r->args.size() || op != r->op)
                return false;

            for (Node *n : args)
            {
                bool isContained = false;
                for (Node *m : r->args)
                {
                    if (n->Equals(m))
                    {
                        isContained = true;
                        break;
                    }
                    // std::cout << "RUNNING " << n->ToString() << " == " << m->ToString() << " Equality result " << isContained << std::endl;
                }
                if (!isContained)
                    return false;
            }
            return true;
        }
        // only case left is functions (cheap)
        default:
        {
            assert((args.size() == 1) && (r->args.size() == 1));
            return (op == r->op) && (args[0]->Equals(r->args[0]));
        }
        }
    }
    else
        return false;
}