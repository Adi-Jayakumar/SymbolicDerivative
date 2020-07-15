#include "differentiate.h"

namespace Differentiate
{

    Node *Derivative(Node *source, std::string &s)
    {
        if (source->type == NodeType::NumNode)
            return new Num(0.);
        else if (source->type == NodeType::VarNode)
        {
            Var *vSource = static_cast<Var *>(source);
            if (vSource->id == s)
                return new Num(1);
            else
                return new Num(0.);
        }
        else
        {
            assert(source->type == NodeType::MultiNode);
            Multi *mSource = static_cast<Multi *>(source);
            Multi *res;
            switch (mSource->op)
            {
            case Operator::ADD:
            {
                res = new Multi(Operator::ADD);
                for (Node *n : mSource->args)
                    res->AddArg(Derivative(n, s));
                return res;
            }
            case Operator::SUB:
            {
                assert(mSource->args.size() == 2);
                return new Multi(Derivative(mSource->args[0], s), Derivative(mSource->args[1], s), Operator::SUB);
            }
            case Operator::MUL:
            {
                res = new Multi(Operator::ADD);
                for (long unsigned int i = 0; i < mSource->args.size(); i++)
                {
                    Multi *diffArg = new Multi(mSource);
                    diffArg->args[i] = Derivative(diffArg->args[i], s);
                    res->AddArg(diffArg);
                }
                return res;
            }
            case Operator::DIV:
            {
                assert(mSource->args.size() == 2);
                res = new Multi(Operator::DIV);
                Node *num = mSource->args[0];
                Node *denom = mSource->args[1];
                res->AddArg(new Multi(new Multi(Derivative(num, s), denom, Operator::MUL), new Multi(num, Derivative(denom, s), Operator::MUL), Operator::SUB));
                res->AddArg(new Multi(denom, new Num(2), Operator::POW));
                return res;
            }
            case Operator::POW:
            {
                assert(mSource->args.size() == 2);
                res = new Multi(Operator::MUL);
                res->AddArg(source);
                res->AddArg(new Multi(new Multi(new Multi(mSource->args[0], Operator::LN), Derivative(mSource->args[1], s), Operator::MUL), new Multi(new Multi(Derivative(mSource->args[0], s), mSource->args[1], Operator::MUL), mSource->args[0], Operator::DIV), Operator::ADD));
                return res;
            }
            case Operator::LN:
            {
                assert(mSource->args.size() == 1);
                return new Multi(Derivative(mSource->args[0], s), mSource->args[0], Operator::DIV);
            }
            case Operator::EXP:
            {
                assert(mSource->args.size() == 1);
                return new Multi(Derivative(mSource->args[0], s), mSource, Operator::MUL);
            }
            case Operator::SIN:
            {
                assert(mSource->args.size() == 1);
                return new Multi(Derivative(mSource->args[0], s), new Multi(mSource->args[0], Operator::COS), Operator::MUL);
            }
            case Operator::COS:
            {
                assert(mSource->args.size() == 1);
                return new Multi(Derivative(mSource->args[0], s), new Multi(new Num(-1), new Multi(mSource->args[0], Operator::SIN), Operator::MUL), Operator::MUL);
            }
            case Operator::TAN:
            {
                assert(mSource->args.size() == 1);
                return new Multi(Derivative(mSource->args[0], s), new Multi(new Multi(mSource->args[0], Operator::COS), new Num(2), Operator::POW), Operator::DIV);
            }
            default:
                return mSource;
            }
        }
    }

    Node *Differentiate(Node *source, std::string s)
    {
        return Derivative(source, s);
    }

} // namespace Differentiate