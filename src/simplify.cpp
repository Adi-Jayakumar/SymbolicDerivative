#include "simplify.h"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <sstream>

namespace Simplify
{

    Node *Simplify(Node *source)
    {
        std::cout << "Simplify " << source->ToString() << std::endl;

        // no simplifications can be done to a number/variable
        if (source->type == NodeType::NumNode || source->type == NodeType::VarNode)
            return source;

        else
        {
            assert(source->type == NodeType::MultiNode);
            Multi *m = dynamic_cast<Multi *>(source);

            // checking if simplification is required
            bool doSimplify = NeedSimplify(m);

            // Carrying out the required simplification if it is required
            if ((m->op == Token::MUL) && doSimplify)
                return Simplify(SimplifyMultiplication(m));
            return source;
        }
    }

    bool NeedSimplify(Multi *source)
    {
        std::map<std::string, int> varNames;
        int numCount = 0;
        bool doSimplify = false;

        // counting the number of Num and Var nodes,
        // and checking if a child is a Multi
        for (Node *n : source->args)
        {
            if (n->type == NodeType::NumNode)
                numCount++;
            else if (n->type == NodeType::VarNode)
                varNames[n->SelfToString()]++;
            else if (n->type == NodeType::MultiNode)
            {
                doSimplify = true;
                break;
            }
        }

        // combining the conditions laid out before
        doSimplify = doSimplify || (numCount > 1);
        for (auto const &[var, occurances] : varNames)
        {
            doSimplify = doSimplify || (occurances > 1);
        }
        return doSimplify;
    }

    Multi *Unroll(Multi *source)
    {
        // std::cout << "Unroll: " << source->ToString() << " With op: " << static_cast<int>(source->op) << std::endl;
        if (source->op == Token::POW)
            return source;
        Multi *unrolled = new Multi(source->op);
        bool isUnrolled = true;
        for (Node *n : source->args)
        {
            if (n->type != NodeType::MultiNode)
                unrolled->AddArg(n);
            else
            {
                Multi *mArg = dynamic_cast<Multi *>(n);
                if (mArg->op != source->op)
                    unrolled->AddArg(Unroll(mArg));
                else
                {
                    isUnrolled = false;
                    for (Node *m : mArg->args)
                        unrolled->AddArg(m);
                }
            }
        }
        if (isUnrolled)
            return unrolled;
        return Unroll(unrolled);
    }

    Node *PowCanonicalForm(Multi *source)
    {
        if (source->op == Token::POW)
        {
            assert(source->args.size() == 2);
            if (source->args[0]->type == NodeType::MultiNode)
            {
                Multi *mChildArg = dynamic_cast<Multi *>(source->args[0]);
                if (mChildArg->op == Token::MUL)
                {
                    Multi *expanded = new Multi(Token::MUL);
                    for (Node *n : mChildArg->args)
                        expanded->AddArg(new Multi(n, source->args[1], Token::POW));
                    return expanded;
                }
            }
        }
        return source;
    }

    Node *MulCanonicalForm(Multi *source)
    {
        if (source->op != Token::MUL)
            return source;

        Multi *numerator = new Multi(Token::MUL);
        Multi *denominator = new Multi(Token::MUL);

        for (Node *n : source->args)
        {
            if (n->type != NodeType::MultiNode)
                numerator->AddArg(n);
            else
            {
                Multi *mArg = dynamic_cast<Multi *>(n);
                Node *canonical = CanonicalForm(mArg);
                if (canonical->type != NodeType::MultiNode)
                {
                    numerator->AddArg(canonical);
                    continue;
                }
                mArg = dynamic_cast<Multi *>(canonical);
                if (mArg->op != Token::DIV)
                    numerator->AddArg(mArg);
                else
                {
                    // division node cannot have more than 2 arguments
                    assert(mArg->args.size() == 2);
                    denominator->AddArg(mArg->args[1]);
                }
            }
        }

        Multi *canonical = new Multi(Token::DIV);

        if (numerator->args.size() == 0 && denominator->args.size() == 1)
        {
            canonical->AddArg(new Num(1));
            canonical->AddArg(denominator->args[0]);
        }
        else if (numerator->args.size() == 0 && denominator->args.size() > 1)
        {
            canonical->AddArg(new Num(1));
            canonical->AddArg(denominator);
        }
        else if (numerator->args.size() == 1 && denominator->args.size() == 0)
            return numerator->args[0];
        else if (numerator->args.size() == 1 && denominator->args.size() == 1)
        {
            canonical->AddArg(numerator->args[0]);
            canonical->AddArg(denominator->args[0]);
        }
        else if (numerator->args.size() == 1 && denominator->args.size() > 1)
        {
            canonical->AddArg(numerator->args[0]);
            canonical->AddArg(denominator);
        }
        else if (numerator->args.size() > 1 && denominator->args.size() == 0)
            return numerator;
        else if (numerator->args.size() > 1 && denominator->args.size() == 1)
        {
            canonical->AddArg(numerator);
            canonical->AddArg(denominator->args[0]);
        }
        else
        {
            canonical->AddArg(numerator);
            canonical->AddArg(denominator);
        }
        return canonical;
    }

    Node *AddCanonicalForm(Multi *source)
    {
        if (source->op != Token::ADD)
            return source;

        Multi *additive = new Multi(Token::ADD);
        Multi *subtractive = new Multi(Token::ADD);

        for (Node *n : source->args)
        {
            if (n->type == NodeType::NumNode)
                additive->AddArg(n);
            else if (n->type == NodeType::VarNode)
                additive->AddArg(n);
            else
            {
                Multi *mArg = dynamic_cast<Multi *>(n);
                if (mArg->op != Token::MUL)
                    additive->AddArg(mArg);
                else
                {
                    bool isNeg = false;
                    Multi *nonNeg = new Multi(Token::MUL);
                    for (Node *m : mArg->args)
                    {
                        if (m->type != NodeType::NumNode)
                            nonNeg->AddArg(m);
                        else
                        {
                            Num *num = dynamic_cast<Num *>(m);
                            if (num->val == -1)
                                isNeg = !isNeg;
                            else
                                nonNeg->AddArg(num);
                        }
                    }
                    if (nonNeg->args.size() == 1)
                    {
                        if (isNeg)
                            subtractive->AddArg(nonNeg->args[0]);
                        else
                            additive->AddArg(nonNeg->args[0]);
                    }
                    else
                    {
                        if (isNeg)
                            subtractive->AddArg(nonNeg);
                        else
                            additive->AddArg(nonNeg);
                    }
                }
            }
        }

        Multi *canonical = new Multi(Token::SUB);

        if (additive->args.size() == 0 && subtractive->args.size() == 1)
        {
            canonical->AddArg(new Num(0.));
            canonical->AddArg(subtractive->args[0]);
        }
        else if (additive->args.size() == 0 && subtractive->args.size() > 1)
        {
            canonical->AddArg(new Num(0.));
            canonical->AddArg(subtractive);
        }
        else if (additive->args.size() == 1 && subtractive->args.size() == 0)
            return additive->args[0];
        else if (additive->args.size() == 1 && subtractive->args.size() == 1)
        {
            canonical->AddArg(additive->args[0]);
            canonical->AddArg(subtractive->args[0]);
        }
        else if (additive->args.size() == 1 && subtractive->args.size() > 1)
        {
            canonical->AddArg(additive->args[0]);
            canonical->AddArg(subtractive);
        }
        else if (additive->args.size() > 1 && subtractive->args.size() == 0)
            return additive;
        else if (additive->args.size() > 1 && subtractive->args.size() == 1)
        {
            canonical->AddArg(additive);
            canonical->AddArg(subtractive->args[0]);
        }
        else
        {
            canonical->AddArg(additive);
            canonical->AddArg(subtractive);
        }
        return canonical;
    }

    Node *CanonicalForm(Node *source)
    {
        if (source->type != NodeType::MultiNode)
            return source;
        else
        {
            Multi *mSource = dynamic_cast<Multi *>(source);
            if (mSource->op == Token::MUL)
                return MulCanonicalForm(mSource);
            else if (mSource->op == Token::ADD)
                return AddCanonicalForm(mSource);
            else if (mSource->op == Token::POW)
                return PowCanonicalForm(mSource);
            return mSource;
        }
    }

    Node *FoldConstants(Node *source)
    {
        // std::cout << "Fold constants of: " << source->ToString() << std::endl;
        if (source->type != NodeType::MultiNode)
            return source;
        else
        {
            Multi *mSource = dynamic_cast<Multi *>(source);
            if (mSource->op == Token::ADD || mSource->op == Token::MUL)
            {
                double id = mSource->op == Token::ADD ? 0 : 1;
                double val = id;
                std::vector<Node *> notNum;
                for (Node *n : mSource->args)
                {
                    Node *eval = FoldConstants(n);
                    if (eval->type == NodeType::NumNode)
                    {
                        Num *nEval = dynamic_cast<Num *>(eval);
                        if (mSource->op == Token::ADD)
                            val += nEval->val;
                        else
                            val *= nEval->val;
                    }
                    else
                        notNum.emplace_back(eval);
                }
                if (val != id)
                    notNum.emplace_back(new Num(val));
                if (notNum.size() == 1)
                    return notNum[0];
                return new Multi(notNum, mSource->op);
            }
            else if (mSource->op == Token::SUB || mSource->op == Token::DIV)
            {
                assert(mSource->args.size() == 2);
                Node *operand = FoldConstants(mSource->args[0]);
                Node *invOperand = FoldConstants(mSource->args[1]);
                if (operand->type == NodeType::NumNode && invOperand->type == NodeType::NumNode)
                {
                    Num *nNum = dynamic_cast<Num *>(operand);
                    Num *nDenom = dynamic_cast<Num *>(invOperand);
                    if (mSource->op == Token::SUB)
                        return new Num(nNum->val - nDenom->val);
                    else
                        return new Num(nNum->val / nDenom->val);
                }
                else
                    return new Multi(operand, invOperand, mSource->op);
            }
            else if (mSource->op == Token::LN || mSource->op == Token::EXP || mSource->op == Token::SIN || mSource->op == Token::COS || mSource->op == Token::TAN)
            {
                assert(mSource->args.size() == 1);
                Node *simpArg = FoldConstants(mSource->args[0]);
                if (simpArg->type != NodeType::NumNode)
                    return new Multi(simpArg, mSource->op);
                else
                {
                    Num *nArg = dynamic_cast<Num *>(simpArg);
                    switch (mSource->op)
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
            }
            else if (mSource->op == Token::POW)
            {
                assert(mSource->args.size() == 2);
                Node *base = FoldConstants(mSource->args[0]);
                Node *power = FoldConstants(mSource->args[1]);
                // std::cout << "base " << static_cast<int>(base->type) << " " << base->ToString() << std::endl;
                // std::cout << "power " << static_cast<int>(power->type) << " " << power->ToString() << std::endl;
                if (base->type == NodeType::NumNode && power->type == NodeType::NumNode)
                {
                    Num *nBase = dynamic_cast<Num *>(base);
                    Num *nPower = dynamic_cast<Num *>(power);
                    return new Num(pow(nBase->val, nPower->val));
                }
                else
                    return new Multi(base, power, Token::POW);
            }
        }
        return source;
    }

    Node *FoldAddVariables(Node *source)
    {
        if (source->type != NodeType::MultiNode)
            return source;
        else
        {
            Multi *mSource = dynamic_cast<Multi *>(source);
            if (mSource->op != Token::ADD)
                return mSource;

            // add to this when updating a variable's coefficient
            std::unordered_map<std::string, Node *> coeff;
            std::map<std::string, int> vOccurances;

            // add to this when not a variable
            std::vector<Node *> notVars;

            //------------------------------------- ITERATING THROUGH THE ARGUMENTS OF THE ROOT NODE -------------------------------------
            for (Node *n : mSource->args)
            {
                if (n->type == NodeType::NumNode)
                    notVars.emplace_back(n);
                else if (n->type == NodeType::VarNode)
                {
                    Var *vArg = dynamic_cast<Var *>(n);
                    if (coeff.find(vArg->id) != coeff.end())
                        coeff[vArg->id] = new Multi(coeff[vArg->id], new Num(1), Token::ADD);
                    else
                        coeff[vArg->id] = new Num(1);
                }
                else
                {
                    Multi *mArg = dynamic_cast<Multi *>(n);

                    if (mSource->op == Token::ADD)
                    {
                        if (mArg->op != Token::MUL)
                        {
                            notVars.emplace_back(mArg);
                            continue;
                        }
                    }

                    std::vector<std::string> seenVars;
                    bool containsVar = false;

                    //------------------------------------- ITERATING THROUGH ARGUMENTS OF A CHILD NODE TO COUNT THE VARIABLE OCCURANCES -------------------------------------
                    for (Node *m : mArg->args)
                    {
                        if (m->type == NodeType::VarNode)
                        {
                            containsVar = true;
                            Var *vChildArg = dynamic_cast<Var *>(m);
                            if (std::find(seenVars.begin(), seenVars.end(), vChildArg->id) == seenVars.end())
                            {
                                seenVars.emplace_back(vChildArg->id);
                                vOccurances[vChildArg->id]++;
                            }
                        }
                    }
                    if (!containsVar)
                        notVars.emplace_back(n);
                }
            }

            std::cout << vOccurances.size() << std::endl;

            if (vOccurances.size() != 0)
            {
                std::string mostFreqVar = std::max_element(std::begin(vOccurances), std::end(vOccurances), [](const std::pair<std::string, int> &p1, const std::pair<std::string, int> &p2) { return p1.second < p2.second; })->first;
                std::cout << "mostFreqVar: " << mostFreqVar << std::endl;
                if (coeff.find(mostFreqVar) == coeff.end())
                {
                    std::cout << "RUNNING " << mostFreqVar << std::endl;
                    coeff[mostFreqVar] = new Num(0.);
                }

                //------------------------------------- ITERATING THROUGH ARGUMENTS OF A CHILD NODE TO COLLECT THE MOST FREQUENTLY OCCURING VARIABLE -------------------------------------

                for (Node *p : mSource->args)
                {
                    if (p->type != NodeType::MultiNode)
                        continue;

                    Multi *mArg = dynamic_cast<Multi *>(p);

                    if (mArg->op != Token::MUL)
                        continue;

                    std::vector<Node *> childCoeffs;
                    std::vector<std::string> seenVars;
                    Var *seenVar = nullptr;
                    bool isSeenBefore = false;

                    for (Node *q : mArg->args)
                    {

                        if (q->type != NodeType::VarNode)
                            childCoeffs.emplace_back(q);
                        else
                        {
                            Var *vChildArg = dynamic_cast<Var *>(q);

                            // variable we have seen is not the most frequent variable
                            if (vChildArg->id != mostFreqVar)
                                childCoeffs.emplace_back(vChildArg);

                            // variable we see is the most frequent variable
                            else
                            {
                                if (isSeenBefore)
                                    childCoeffs.emplace_back(vChildArg);
                                else
                                {
                                    seenVar = vChildArg;
                                    isSeenBefore = true;
                                }
                            }
                        }
                    }

                    std::cout << "Data in childCoeffs: " << std::endl;
                    for (Node *r : childCoeffs)
                        std::cout << r->ToString() << std::endl;

                    // std::cout<< "seenVar: " << seenVar-<< std::endl;
                    if (seenVar == nullptr)
                        notVars.emplace_back(new Multi(childCoeffs, Token::MUL));
                    else
                    {
                        Multi *child = new Multi(childCoeffs, Token::MUL);
                        coeff[seenVar->id] = new Multi(child, coeff[seenVar->id], Token::ADD);
                    }
                }
            }

            // std::cout << "Data in vOccurances: " << std::endl;
            // for (auto &[var, num] : vOccurances)
            //     std::cout << var << " " << num << std::endl;

            // std::cout << "Data in notVar: " << std::endl;
            // for (Node *o : notVars)
            //     std::cout << o->ToString() << std::endl;

            for (auto &[var, coefficient] : coeff)
            {
                notVars.emplace_back(new Multi(coefficient, new Var(var), Token::MUL));
            }
            if (notVars.size() == 1)
                return FoldConstants(notVars[0]);
            else
            {
                return FoldConstants(new Multi(notVars, mSource->op));
            }
        }
        return source;
    }

    Node *FoldMulVariables(Node *source)
    {
        if (source->type != NodeType::MultiNode)
            return source;
        else
        {
            Multi *mSource = dynamic_cast<Multi *>(source);
            if (mSource->op != Token::MUL)
                return mSource;

            std::unordered_map<std::string, Node *> coeff;
            std::vector<Node *> notVars;

            for (Node *n : mSource->args)
            {
                if (n->type != NodeType::MultiNode)
                {
                    if (coeff.find(n->ToString()) != coeff.end())
                        coeff[n->ToString()] = new Multi(coeff[n->ToString()], new Num(1), Token::ADD);
                    else
                        coeff[n->ToString()] = new Num(1);
                }
                else
                {
                    Multi *mArg = dynamic_cast<Multi *>(n);
                    if (mArg->op != Token::POW)
                        notVars.emplace_back(mArg);

                    assert(mArg->args.size() == 2);
                    if (mArg->args[0]->type == NodeType::MultiNode)
                    {
                        notVars.emplace_back(mArg);
                        continue;
                    }
                    else
                    {
                        if (coeff.find(mArg->args[0]->ToString()) != coeff.end())
                            coeff[mArg->args[0]->ToString()] = new Multi(coeff[mArg->args[0]->ToString()], mArg->args[1], Token::ADD);
                        else
                            coeff[mArg->args[0]->ToString()] = mArg->args[1];
                    }
                }
            }

            for (auto &[base, pow] : coeff)
            {
                std::istringstream num(base);
                double candidate = 0;
                num >> candidate;
                if (!num.fail() && num.eof())
                    notVars.emplace_back(new Multi(new Num(candidate), pow, Token::POW));
                else
                    notVars.emplace_back(new Multi(new Var(base), pow, Token::POW));
            }

            if (notVars.size() == 1)
                return FoldConstants(notVars[0]);
            else
                return FoldConstants(new Multi(notVars, Token::MUL));
        }
    }

    Node *FoldVariables(Node *source)
    {
        // std::cout << "FoldVariables: " << source->ToString() << std::endl;
        if (source->type != NodeType::MultiNode)
            return source;
        else
        {
            Multi *mSource = dynamic_cast<Multi *>(source);
            if (mSource->op == Token::ADD)
            {
                Node *folded = FoldAddVariables(mSource);
                if (folded->type != NodeType::MultiNode)
                    return folded;

                Multi *mFolded = dynamic_cast<Multi *>(folded);
                Multi *res = new Multi(mFolded->op);

                for (Node *n : mFolded->args)
                    res->AddArg(FoldVariables(n));
                return res;
            }
            else if (mSource->op == Token::MUL)
            {
                Node *folded = FoldAddVariables(mSource);
                if (folded->type != NodeType::MultiNode)
                    return folded;

                Multi *mFolded = dynamic_cast<Multi *>(folded);
                Multi *res = new Multi(mFolded->op);

                for (Node *n : mFolded->args)
                    res->AddArg(FoldVariables(n));
                return res;
            }
        }
        return source;
    }

    Node *SimplifyMultiplication(Multi *source)
    {
        return source;
    }

    Node *SimplifyAddition(Multi *source)
    {
        return source;
    }

} // namespace Simplify