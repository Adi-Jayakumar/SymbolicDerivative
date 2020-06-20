#include "simplify.h"
#include <cassert>
#include <cmath>

namespace Simplify
{

    Node *Simplify(Node *source)
    {
        std::cout << "Simplify " << source->ToString() << std::endl;
        if (source->type == NodeType::NumNode || source->type == NodeType::VarNode)
        {
            return source;
        }
        // Node *sourceSimp = Simplify(source);
        if (source->type == NodeType::UnaryNode)
        {
            Unary *u = dynamic_cast<Unary *>(source);
            // u = -(...)
            if (u->op == Token::SUB)
                return Simplify(SimplifyNegation(u));
            // u = f(num)
            else if (u->arg->type == NodeType::NumNode)
                return Simplify(EvaluateFunction(u));
            else
                return Simplify(new Unary(Simplify(u->arg), u->op));
        }
        else
        {
            assert(source->type == NodeType::Multi);
            Multi *m = dynamic_cast<Multi *>(source);
            // if (m->op == Token::ADD)
            //     return SimplifyAddition(m);

            std::map<std::string, int> varNames;
            int numCount = 0;
            bool doSimplify = false;

            for (Node *n : m->args)
            {
                if (n->type == NodeType::NumNode)
                    numCount++;
                else if (n->type == NodeType::VarNode)
                    varNames[n->SelfToString()]++;
                else if (n->type == NodeType::Multi)
                {
                    doSimplify = true;
                    break;
                }
            }
            doSimplify = doSimplify || (numCount > 1);
            for (auto const &[var, occurances] : varNames)
            {
                doSimplify = doSimplify || (occurances > 1);
            }
            std::cout << doSimplify << std::endl;
            if ((m->op == Token::MUL) && doSimplify)
            {
                return Simplify(SimplifyMultiplication(m));
            }
            return source;
        }
    }

    Node *SimplifyNegation(Unary *source)
    {
        if (source->arg->type != NodeType::UnaryNode || !(source->op == Token::SUB || source->op == Token::ADD))
            return source;

        Unary *uArg = dynamic_cast<Unary *>(source->arg);

        if (source->op == Token::SUB && uArg->op == Token::SUB)
            return uArg->arg;
    }

    Node *EvaluateFunction(Unary *source)
    {
        Num *nArg = dynamic_cast<Num *>(source->arg);
        switch (source->op)
        {
        case Token::LN:
            return new Num(log(nArg->val));
            break;
        case Token::EXP:
            return new Num(exp(nArg->val));
            break;
        case Token::SIN:
            return new Num(sin(nArg->val));
            break;
        case Token::COS:
            return new Num(cos(nArg->val));
            break;
        case Token::TAN:
            return new Num(tan(nArg->val));
            break;
        default:
            return source;
            break;
        }
    }

    Node *MulCanonicalForm(Multi *source)
    {
        if (source->op != Token::MUL)
            return source;
        Multi *canon = new Multi(Token::DIV);
        Multi *numerator = new Multi(Token::MUL);
        Multi *denominator = new Multi(Token::MUL);
        for (Node *n : source->args)
        {
            if (n->type != NodeType::Multi)
                numerator->AddArg(n);
            else
            {
                Multi *multiArg = dynamic_cast<Multi *>(n);
                if (multiArg->op != Token::DIV)
                    numerator->AddArg(multiArg);
                else
                    denominator->AddArg(multiArg->args[1]);
            }
        }
        if (numerator->args.size() == 0)
        {
            canon->AddArg(new Num(1));
            canon->AddArg(denominator);
        }
        else if (denominator->args.size() == 0)
            return numerator;
        else if (numerator->args.size() == 1)
        {
            canon->AddArg(numerator->args[0]);
            canon->AddArg(denominator);
        }
        else if (denominator->args.size() == 1)
        {
            canon->AddArg(numerator);
            canon->AddArg(denominator->args[0]);
        }
        else
        {
            canon->AddArg(numerator);
            canon->AddArg(denominator);
        }
        return canon;
    }

    Node *AddCanonicalForm(Multi *source)
    {
        if (source->op != Token::ADD)
            return source;
        Multi *canon = new Multi(Token::SUB);
        Multi *additive = new Multi(Token::ADD);
        Multi *subtractive = new Multi(Token::ADD);
        for (Node *n : source->args)
        {
            if (n->type == NodeType::NumNode)
            {
                Num *asNum = dynamic_cast<Num *>(n);
                if (asNum->val < 0)
                {
                    asNum->val *= -1;
                    subtractive->AddArg(asNum);
                }
                else
                    additive->AddArg(asNum);
            }
            else if (n->type == NodeType::UnaryNode)
            {
                Unary *asUnary = dynamic_cast<Unary *>(n);
                if (asUnary->op == Token::SUB)
                    subtractive->AddArg(asUnary->arg);
                else
                    additive->AddArg(asUnary);
            }
            else if (n->type == NodeType::VarNode)
                additive->AddArg(n);
            else
            {
                Multi *multiArg = dynamic_cast<Multi *>(n);
                if (multiArg->op != Token::MUL)
                    additive->AddArg(multiArg);
                else
                {
                    std::vector<Node*> nonNeg;
                    std::vector<Node*> neg;
                    for(Node* m : )
                }
            }
        }
        if (additive->args.size() == 0)
        {
            canon->AddArg(new Num(1));
            canon->AddArg(subtractive);
        }
        else if (subtractive->args.size() == 0)
            return additive;
        else if (additive->args.size() == 1)
        {
            canon->AddArg(additive->args[0]);
            canon->AddArg(subtractive);
        }
        else if (subtractive->args.size() == 1)
        {
            canon->AddArg(additive);
            canon->AddArg(subtractive->args[0]);
        }
        else
        {
            canon->AddArg(additive);
            canon->AddArg(subtractive);
        }
        return canon;
    }

    Node *SimplifyAddition(Multi *soruce) { return new Num(0.); }

    Node *SimplifyMultiplication(Multi *source)
    {
        if (source->op != Token::MUL)
            return source;

        // source = ... * 0 * ...
        for (Node *n : source->args)
        {
            if (n->type == NodeType::NumNode)
            {
                Num *num = dynamic_cast<Num *>(n);
                if (num->val == 0)
                    return new Num(0.);
            }
        }

        double numbers = 1;
        std::vector<Node *> notNums;
        // source can be rearranged to be equal to n1 * n2 * ... * nk * ...
        // which is simplified to n1n2...nk * ...
        for (Node *m : source->args)
        {
            if (m->type != NodeType::NumNode)
            {
                notNums.emplace_back(m);
            }
            else
            {
                Num *num = dynamic_cast<Num *>(m);
                numbers *= num->val;
            }
        }

        if (numbers != 1)
            notNums.emplace_back(new Num(numbers));

        return new Multi(notNums, Token::MUL);
    }

} // namespace Simplify