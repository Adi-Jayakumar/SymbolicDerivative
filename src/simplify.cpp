#include "simplify.h"

namespace Simplify
{

    Node *Reduce(Node *source)
    {

        Node *res = nullptr;

        while (true)
        {
            res = Simplify(source);
            if (*res == *source)
                break;
            else
                source = res;
        }

        return res;
    }

    Node *Simplify(Node *source)
    {

        // no simplifications can be done to a number/variable
        if (source->type == NodeType::NumNode || source->type == NodeType::VarNode)
            return source;

        else
        {
            assert(source->type == NodeType::MultiNode);
            Multi *mSource = static_cast<Multi *>(source);

            Multi *unrolled = Unroll(mSource);
            Node *noConstants = FoldConstants(unrolled);
            Node *preProc = PreProcessed(noConstants);

            if (preProc->type != NodeType::MultiNode)
                return preProc;
            else
            {
                Multi *mPreProc = static_cast<Multi *>(preProc);
                if (mPreProc->op == Operator::ADD)
                    return SimplifyNodeByNode(SimplifyAddition(mPreProc));
                else if (mPreProc->op == Operator::SUB)
                    return SimplifyNodeByNode(SimplifySubtraction(mPreProc));
                else if (mPreProc->op == Operator::MUL)
                    return SimplifyNodeByNode(SimplifyMultiplication(mPreProc));
                else if (mPreProc->op == Operator::DIV)
                    return SimplifyNodeByNode(SimplifyDivision(mPreProc));
                else
                    return SimplifyNodeByNode(preProc);
            }
        }
    }

    Node *SimplifyNodeByNode(Node *source)
    {
        if (source->type != NodeType::MultiNode)
            return source;
        else
        {
            Multi *mSource = static_cast<Multi *>(source);
            Multi *res = new Multi(mSource->op);
            for (Node *n : mSource->args)
            {
                res->AddArg(Simplify(n));
            }
            return res;
        }
    }

    Multi *Unroll(Multi *source)
    {
        if (source->op == Operator::POW)
            return source;
        else if (IsFunc(source))
        {
            assert(source->args.size() == 1);
            Node *arg = source->args[0];
            if (arg->type != NodeType::MultiNode)
                return source;
            else
            {
                Multi *mArg = static_cast<Multi *>(source->args[0]);
                return new Multi(Unroll(mArg), source->op);
            }
        }

        if (source->op == Operator::DIV || source->op == Operator::SUB)
        {
            assert(source->args.size() == 2);
            Node *lhs = source->args[0];
            Node *rhs = source->args[1];
            Multi *simpLHS;
            Multi *simpRHS;
            if (source->op == Operator::DIV)
            {
                simpLHS = new Multi(Operator::MUL);
                simpRHS = new Multi(Operator::MUL);
            }
            else
            {
                simpLHS = new Multi(Operator::ADD);
                simpRHS = new Multi(Operator::ADD);
            }
            if (lhs->type != NodeType::MultiNode)
                simpLHS->AddArg(lhs);
            else
            {
                Multi *mLHS = static_cast<Multi *>(lhs);
                mLHS = Unroll(mLHS);
                if (mLHS->op != source->op)
                    simpLHS->AddArg(mLHS);
                // (a - b) - c = a - (b + c)
                else
                {
                    assert(mLHS->args.size() == 2);
                    simpLHS->AddArg(mLHS->args[0]);
                    simpRHS->AddArg(mLHS->args[1]);
                }
            }
            if (rhs->type != NodeType::MultiNode)
                simpRHS->AddArg(rhs);
            else
            {
                Multi *mRHS = static_cast<Multi *>(rhs);
                if (mRHS->op != source->op)
                    simpRHS->AddArg(mRHS);
                // a - (b - c) = (a + c) - b
                else
                {
                    assert(mRHS->args.size() == 2);
                    simpLHS->AddArg(mRHS->args[1]);
                    simpRHS->AddArg(mRHS->args[0]);
                }
            }
            simpLHS = Unroll(simpLHS);
            simpRHS = Unroll(simpRHS);

            if (simpLHS->args.size() == 1 && simpRHS->args.size() == 1)
                return new Multi(simpLHS->args[0], simpRHS->args[0], source->op);
            else if (simpLHS->args.size() == 1 && simpRHS->args.size() > 1)
                return new Multi(simpLHS->args[0], simpRHS, source->op);
            else if (simpLHS->args.size() > 1 && simpRHS->args.size() == 1)
                return new Multi(simpLHS, simpRHS->args[0], source->op);
            else
                return new Multi(simpLHS, simpRHS, source->op);
        }

        Multi *unrolled = new Multi(source->op);
        bool isUnrolled = true;
        for (Node *n : source->args)
        {
            if (n->type != NodeType::MultiNode)
                unrolled->AddArg(n);
            else
            {
                Multi *mArg = static_cast<Multi *>(n);
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

    Node *PowPreProcessed(Multi *source)
    {
        if (source->op == Operator::POW)
        {
            assert(source->args.size() == 2);
            if (source->args[0]->type == NodeType::MultiNode)
            {
                Multi *mChildArg = static_cast<Multi *>(source->args[0]);

                if (mChildArg->op == Operator::MUL)
                {
                    Multi *expanded = new Multi(Operator::MUL);
                    for (Node *n : mChildArg->args)
                        expanded->AddArg(new Multi(n, source->args[1], Operator::POW));
                    return expanded;
                }
            }
        }
        return source;
    }

    Node *MulPreProcessed(Multi *source)
    {
        if (source->op != Operator::MUL)
            return source;

        Multi *numerator = new Multi(Operator::MUL);
        Multi *denominator = new Multi(Operator::MUL);

        for (Node *n : source->args)
        {
            if (n->type != NodeType::MultiNode)
                numerator->AddArg(n);
            else
            {
                Multi *mArg = static_cast<Multi *>(n);
                Node *preProc = PreProcessed(mArg);
                if (preProc->type != NodeType::MultiNode)
                {
                    numerator->AddArg(preProc);
                    continue;
                }
                mArg = static_cast<Multi *>(preProc);
                if (mArg->op != Operator::DIV)
                    numerator->AddArg(mArg);
                else
                {
                    // division node cannot have more than 2 arguments
                    assert(mArg->args.size() == 2);
                    if (mArg->args[0]->type != NodeType::NumNode)
                        numerator->AddArg(mArg->args[0]);
                    else
                    {
                        Num *n = static_cast<Num *>(mArg->args[0]);
                        if (n->val != 1)
                            numerator->AddArg(n);
                    }
                    denominator->AddArg(mArg->args[1]);
                }
            }
        }

        Multi *preProc = new Multi(Operator::DIV);

        if (numerator->args.size() == 0 && denominator->args.size() == 1)
        {
            preProc->AddArg(new Num(1));
            preProc->AddArg(denominator->args[0]);
        }
        else if (numerator->args.size() == 0 && denominator->args.size() > 1)
        {
            preProc->AddArg(new Num(1));
            preProc->AddArg(denominator);
        }
        else if (numerator->args.size() == 1 && denominator->args.size() == 0)
            return numerator->args[0];
        else if (numerator->args.size() == 1 && denominator->args.size() == 1)
        {
            preProc->AddArg(numerator->args[0]);
            preProc->AddArg(denominator->args[0]);
        }
        else if (numerator->args.size() == 1 && denominator->args.size() > 1)
        {
            preProc->AddArg(numerator->args[0]);
            preProc->AddArg(denominator);
        }
        else if (numerator->args.size() > 1 && denominator->args.size() == 0)
            return numerator;
        else if (numerator->args.size() > 1 && denominator->args.size() == 1)
        {
            preProc->AddArg(numerator);
            preProc->AddArg(denominator->args[0]);
        }
        else
        {
            preProc->AddArg(numerator);
            preProc->AddArg(denominator);
        }
        return preProc;
    }

    Node *AddPreProcessed(Multi *source)
    {
        if (source->op != Operator::ADD)
            return source;

        Multi *additive = new Multi(Operator::ADD);
        Multi *subtractive = new Multi(Operator::ADD);

        for (Node *n : source->args)
        {
            if (n->type == NodeType::NumNode)
                additive->AddArg(n);
            else if (n->type == NodeType::VarNode)
                additive->AddArg(n);
            else
            {
                Multi *mArg = static_cast<Multi *>(n);
                if (mArg->op != Operator::MUL)
                    additive->AddArg(mArg);
                else
                {
                    bool isNeg = false;
                    Multi *nonNeg = new Multi(Operator::MUL);
                    for (Node *m : mArg->args)
                    {
                        if (m->type != NodeType::NumNode)
                            nonNeg->AddArg(m);
                        else
                        {
                            Num *num = static_cast<Num *>(m);
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

        Multi *preProc = new Multi(Operator::SUB);

        if (additive->args.size() == 0 && subtractive->args.size() == 1)
        {
            preProc->AddArg(new Num(0.));
            preProc->AddArg(subtractive->args[0]);
        }
        else if (additive->args.size() == 0 && subtractive->args.size() > 1)
        {
            preProc->AddArg(new Num(0.));
            preProc->AddArg(subtractive);
        }
        else if (additive->args.size() == 1 && subtractive->args.size() == 0)
            return additive->args[0];
        else if (additive->args.size() == 1 && subtractive->args.size() == 1)
        {
            preProc->AddArg(additive->args[0]);
            preProc->AddArg(subtractive->args[0]);
        }
        else if (additive->args.size() == 1 && subtractive->args.size() > 1)
        {
            preProc->AddArg(additive->args[0]);
            preProc->AddArg(subtractive);
        }
        else if (additive->args.size() > 1 && subtractive->args.size() == 0)
            return additive;
        else if (additive->args.size() > 1 && subtractive->args.size() == 1)
        {
            preProc->AddArg(additive);
            preProc->AddArg(subtractive->args[0]);
        }
        else
        {
            preProc->AddArg(additive);
            preProc->AddArg(subtractive);
        }
        return preProc;
    }

    Node *PreProcessed(Node *source)
    {
        if (source->type != NodeType::MultiNode)
            return source;
        else
        {
            Multi *mSource = static_cast<Multi *>(source);
            if (mSource->op == Operator::MUL)
                return MulPreProcessed(mSource);
            else if (mSource->op == Operator::ADD)
                return AddPreProcessed(mSource);
            else if (mSource->op == Operator::POW)
                return PowPreProcessed(mSource);
            else
            {
                Multi *res = new Multi(mSource->op);
                for (Node *n : mSource->args)
                    res->AddArg(PreProcessed(n));
                return res;
            }
        }
    }

    Node *FoldConstants(Node *source)
    {
        if (source->type != NodeType::MultiNode)
            return source;
        else
        {
            Multi *mSource = static_cast<Multi *>(source);
            if (mSource->op == Operator::ADD || mSource->op == Operator::MUL)
            {
                double id = mSource->op == Operator::ADD ? 0 : 1;
                double val = id;
                std::vector<Node *> notNum;
                for (Node *n : mSource->args)
                {
                    Node *eval = FoldConstants(n);
                    if (eval->type == NodeType::NumNode)
                    {
                        Num *nEval = static_cast<Num *>(eval);
                        if (mSource->op == Operator::ADD)
                            val += nEval->val;
                        else
                        {
                            if (nEval->val == 0)
                                return new Num(0.);
                            val *= nEval->val;
                        }
                    }
                    else
                        notNum.emplace_back(eval);
                }
                if (val != id)
                    notNum.emplace_back(new Num(val));
                else if (val == id)
                {
                    if (notNum.size() == 0)
                        return new Num(id);
                }
                if (notNum.size() == 1)
                    return notNum[0];
                return new Multi(notNum, mSource->op);
            }
            else if (mSource->op == Operator::SUB || mSource->op == Operator::DIV)
            {
                assert(mSource->args.size() == 2);
                Node *operand = FoldConstants(mSource->args[0]);
                Node *invOperand = FoldConstants(mSource->args[1]);
                if (operand->type == NodeType::NumNode && invOperand->type == NodeType::NumNode)
                {
                    Num *nNum = static_cast<Num *>(operand);
                    Num *nDenom = static_cast<Num *>(invOperand);
                    if (mSource->op == Operator::SUB)
                        return new Num(nNum->val - nDenom->val);
                    else
                        return new Num(nNum->val / nDenom->val);
                }
                else
                {
                    if (mSource->op == Operator::DIV)
                    {
                        if (invOperand->type == NodeType::NumNode)
                        {
                            Num *nDenom = static_cast<Num *>(invOperand);
                            if (nDenom->val == 1)
                                return operand;
                        }
                    }
                    return new Multi(operand, invOperand, mSource->op);
                }
            }
            else if (mSource->op == Operator::LN || mSource->op == Operator::EXP || mSource->op == Operator::SIN || mSource->op == Operator::COS || mSource->op == Operator::TAN)
            {
                assert(mSource->args.size() == 1);
                Node *simpArg = FoldConstants(mSource->args[0]);
                if (simpArg->type != NodeType::NumNode)
                    return new Multi(simpArg, mSource->op);
                else
                {
                    Num *nArg = static_cast<Num *>(simpArg);
                    switch (mSource->op)
                    {
                    case Operator::LN:
                        return new Num(log(nArg->val));
                        break;
                    case Operator::EXP:
                        return new Num(exp(nArg->val));
                        break;
                    case Operator::SIN:
                        return new Num(sin(nArg->val));
                        break;
                    case Operator::COS:
                        return new Num(cos(nArg->val));
                        break;
                    case Operator::TAN:
                        return new Num(tan(nArg->val));
                        break;
                    default:
                        return source;
                        break;
                    }
                }
            }
            else if (mSource->op == Operator::POW)
            {
                assert(mSource->args.size() == 2);
                Node *base = FoldConstants(mSource->args[0]);
                Node *power = FoldConstants(mSource->args[1]);
                if (base->type == NodeType::NumNode && power->type == NodeType::NumNode)
                {
                    Num *nBase = static_cast<Num *>(base);
                    Num *nPower = static_cast<Num *>(power);
                    return new Num(pow(nBase->val, nPower->val));
                }
                else
                {
                    if (power->type == NodeType::NumNode)
                    {
                        Num *nPow = static_cast<Num *>(power);
                        if (nPow->val == 1)
                            return base;
                        else if (nPow->val == 0)
                            return new Num(1);
                    }
                    return new Multi(base, power, Operator::POW);
                }
            }
        }
        return source;
    }

    Node *SimplifyAddition(Node *source)
    {
        if (source->type != NodeType::MultiNode)
            return source;
        else
        {
            Multi *mSource = static_cast<Multi *>(source);
            if (mSource->op != Operator::ADD)
                return source;

            Multi *res = new Multi(Operator::ADD);
            std::unordered_map<Node *, Node *> firstPassCoeffs;
            std::vector<Multi *> notVisited;

            //------------------------------------------ COLLECTING ANY NODES THAT ARE EQUAL (1 DEEP CHECKING) ------------------------------------------
            for (Node *n : mSource->args)
            {
                if (n->type == NodeType::MultiNode)
                {
                    Multi *mArg = static_cast<Multi *>(n);
                    if (mArg->op == Operator::MUL)
                    {
                        notVisited.emplace_back(mArg);
                        continue;
                    }
                }
                else if (n->type == NodeType::NumNode)
                {
                    res->AddArg(n);
                    continue;
                }
                Node *seen = nullptr;

                for (std::pair<Node *, Node *> element : firstPassCoeffs)
                {
                    if (*element.first == *n)
                        seen = element.first;
                }
                // we have seen this argument before
                if (seen != nullptr)
                    firstPassCoeffs[seen] = new Multi(firstPassCoeffs[seen], new Num(1), Operator::ADD);
                // this is the first time we have seen it
                else
                    firstPassCoeffs[n] = new Num(1);
            }

            std::map<Node *, int> nOccurances;
            //------------------------------------------ COUNTING OCCURANCES OF NON-NUMBER NODES ------------------------------------------
            for (Multi *mArg : notVisited)
            {
                for (Node *m : mArg->args)
                {
                    bool isSeenBef = false;
                    for (std::pair<Node *, Node *> element : firstPassCoeffs)
                    {
                        if (*element.first == *m)
                        {
                            nOccurances[element.first]++;
                            isSeenBef = true;
                        }
                    }
                    if (!isSeenBef && !(m->type == NodeType::NumNode))
                    {
                        Node *seen = nullptr;
                        for (std::pair<Node *, int> element : nOccurances)
                        {
                            if (*element.first == *m)
                                seen = element.first;
                        }
                        if (seen != nullptr)
                            nOccurances[seen]++;
                        else if (m->type != NodeType::NumNode)
                            nOccurances[m]++;
                    }
                }
            }

            Node *mostFreq = std::max_element(std::begin(nOccurances), std::end(nOccurances), [](const std::pair<Node *, int> &p1, const std::pair<Node *, int> &p2) { return p1.second < p2.second; })->first;
            if (firstPassCoeffs.find(mostFreq) == firstPassCoeffs.end())
                firstPassCoeffs[mostFreq] = new Num(0.);

            //------------------------------------------ COLLECTING THE MOST FREQUENTLY OCCURING NODE ------------------------------------------

            std::vector<Node *> noFreq;

            for (Multi *mArg : notVisited)
            {
                // most frequent node IS NOT in this node
                if (std::find_if(mArg->args.begin(), mArg->args.end(), [mostFreq](const Node *n) { return *mostFreq == *n; }) == mArg->args.end())
                    noFreq.emplace_back(mArg);
                // most frequent node IS in this node
                else
                {
                    std::vector<Node *> childCoeff;
                    for (Node *p : mArg->args)
                    {
                        if (*p == *mostFreq)
                            continue;
                        childCoeff.emplace_back(p);
                    }
                    if (childCoeff.size() == 1)
                        firstPassCoeffs[mostFreq] = new Multi(firstPassCoeffs[mostFreq], childCoeff[0], Operator::ADD);
                    else
                        firstPassCoeffs[mostFreq] = new Multi(firstPassCoeffs[mostFreq], new Multi(childCoeff, Operator::MUL), Operator::ADD);
                }
            }

            for (auto &[val, coeff] : firstPassCoeffs)
                res->AddArg(new Multi(coeff, val, Operator::MUL));
            for (Node *m : noFreq)
                res->AddArg(m);

            if (res->args.size() == 1)
                return FoldConstants(res->args[0]);
            else
                return FoldConstants(res);
        }
    }

    Node *SimplifyMultiplication(Node *source)
    {
        if (source->type != NodeType::MultiNode)
            return source;
        else
        {
            Multi *mSource = static_cast<Multi *>(source);
            if (mSource->op != Operator::MUL)
                return source;

            Multi *res = new Multi(Operator::MUL);
            std::unordered_map<Node *, Node *> firstPassCoeffs;
            std::vector<Multi *> notVisited;
            //------------------------------------------ COLLECTING ANY NODES THAT ARE EQUAL (1 DEEP CHECKING) ------------------------------------------
            for (Node *n : mSource->args)
            {
                if (n->type == NodeType::MultiNode)
                {
                    Multi *mArg = static_cast<Multi *>(n);
                    if (mArg->op == Operator::POW)
                    {
                        notVisited.emplace_back(mArg);
                        continue;
                    }
                }
                else if (n->type == NodeType::NumNode)
                {
                    res->AddArg(n);
                    continue;
                }

                Node *seen = nullptr;
                for (std::pair<Node *, Node *> element : firstPassCoeffs)
                {
                    if (*element.first == *n)
                        seen = element.first;
                }
                if (seen != nullptr)
                    firstPassCoeffs[seen] = new Multi(firstPassCoeffs[seen], new Num(1), Operator::ADD);
                else
                    firstPassCoeffs[n] = new Num(1);
            }

            for (Multi *pow : notVisited)
            {
                assert(pow->args.size() == 2 && pow->op == Operator::POW);
                Node *seen = nullptr;
                for (auto &[base, p] : firstPassCoeffs)
                {
                    if (*base == *pow->args[0])
                        seen = base;
                }
                // we have seen this power node's base before so we can combine it
                if (seen != nullptr)
                    firstPassCoeffs[seen] = new Multi(firstPassCoeffs[seen], pow->args[1], Operator::ADD);
                // we havent seen this power node's base before
                else
                    firstPassCoeffs[pow->args[0]] = pow->args[1];
            }

            for (auto &[val, coeff] : firstPassCoeffs)
            {
                res->AddArg(new Multi(val, coeff, Operator::POW));
            }

            if (res->args.size() == 1)
                return FoldConstants(res->args[0]);
            else
                return FoldConstants(res);
        }
    }

    Node *SimplifySubtraction(Multi *source)
    {
        if (source->op != Operator::SUB)
            return source;

        assert(source->args.size() == 2);

        Node *additive = source->args[0];
        Node *subtractive = source->args[1];
        Multi *simpAdd = new Multi(Operator::ADD);
        Multi *simpSub = new Multi(Operator::ADD);

        // if they are equal, can straight away return 0, no need to do the more indepth checks
        if (*additive == *subtractive)
            return new Num(0.);

        // one of them is a Multi and the other is not
        else if ((additive->type == NodeType::MultiNode && subtractive->type != NodeType::MultiNode) || (additive->type != NodeType::MultiNode && subtractive->type == NodeType::MultiNode))
        {
            Multi *m;
            Node *notM;

            if (additive->type == NodeType::MultiNode && subtractive->type != NodeType::MultiNode)
            {
                m = static_cast<Multi *>(additive);
                notM = subtractive;
            }
            else
            {
                m = static_cast<Multi *>(subtractive);
                notM = additive;
            }

            if (m->op != Operator::ADD)
                return source;

            bool isConsumed = false;
            for (Node *a : m->args)
            {
                if (*a == *notM && !isConsumed)
                {
                    isConsumed = true;
                    continue;
                }
                else
                {
                    if (additive->type == NodeType::MultiNode && subtractive->type != NodeType::MultiNode)
                        simpAdd->AddArg(a);
                    else
                        simpSub->AddArg(a);
                }
            }
            if (!isConsumed)
            {
                if (additive->type == NodeType::MultiNode && subtractive->type != NodeType::MultiNode)
                    simpAdd->AddArg(notM);
                else
                    simpSub->AddArg(notM);
            }
        }

        // both are multi nodes
        else
        {
            Multi *add = static_cast<Multi *>(additive);
            Multi *sub = static_cast<Multi *>(subtractive);

            if (add->op != Operator::ADD || sub->op != Operator::ADD)
                return source;

            // one of the 2 is a function node no need to check the case that both
            // are function nodes since that is handled up top
            if (IsFunc(add) ^ IsFunc(sub))
            {
                Multi *func;
                Multi *notFunc;
                if (IsFunc(add))
                {
                    func = add;
                    notFunc = sub;
                }
                else
                {
                    func = sub;
                    notFunc = add;
                }
                bool isConsumed = false;
                for (Node *a : notFunc->args)
                {
                    if ((*a == *func) && !isConsumed)
                    {
                        isConsumed = true;
                        continue;
                    }
                    else
                    {
                        if (func == add)
                            simpSub->AddArg(a);
                        else
                            simpAdd->AddArg(a);
                    }
                }
                if (!isConsumed)
                {
                    if (func == add)
                        simpSub->AddArg(func);
                    else
                        simpAdd->AddArg(func);
                }
            }
            else
            {

                // neither of the 2 are function nodes themselves
                std::unordered_map<Node *, bool> isAddConsumed;
                std::unordered_map<Node *, bool> isSubConsumed;

                if (add->op != Operator::ADD || sub->op != Operator::ADD)
                    return source;

                for (Node *a : add->args)
                {
                    for (Node *s : sub->args)
                    {

                        if (*a == *s && !isSubConsumed[s])
                        {
                            isAddConsumed[a] = true;
                            isSubConsumed[s] = true;
                            break;
                        }
                    }
                }

                for (Node *p : add->args)
                {
                    if (!isAddConsumed[p])
                        simpAdd->AddArg(p);
                }
                for (Node *q : sub->args)
                {
                    if (!isSubConsumed[q])
                        simpSub->AddArg(q);
                }
            }
        }

        if (simpAdd->args.size() == 0 && simpSub->args.size() == 0)
            return new Num(0.);
        else if (simpAdd->args.size() == 0 && simpSub->args.size() == 1)
            return FoldConstants(new Multi(new Num(-1), simpSub->args[0], Operator::MUL));
        else if (simpAdd->args.size() == 0 && simpSub->args.size() > 1)
            return FoldConstants(new Multi(new Num(-1), simpSub, Operator::MUL));
        else if (simpAdd->args.size() == 1 && simpSub->args.size() == 0)
            return FoldConstants(simpAdd->args[0]);
        else if (simpAdd->args.size() == 1 && simpSub->args.size() == 1)
            return FoldConstants(new Multi(simpAdd->args[0], simpSub->args[0], Operator::SUB));
        else if (simpAdd->args.size() == 1 && simpSub->args.size() > 1)
            return FoldConstants(new Multi(simpAdd->args[0], simpSub, Operator::SUB));
        else if (simpAdd->args.size() > 1 && simpSub->args.size() == 0)
            return FoldConstants(simpAdd);
        else if (simpAdd->args.size() > 1 && simpSub->args.size() == 1)
            return FoldConstants(new Multi(simpAdd, simpSub->args[0], Operator::SUB));
        else
            return FoldConstants(new Multi(simpAdd, simpSub, Operator::SUB));
    }

    Node *SimplifyDivision(Multi *source)
    {
        if (source->op != Operator::DIV)
            return source;

        assert(source->args.size() == 2);

        Node *numerator = source->args[0];
        Node *denominator = source->args[1];
        Multi *simpNum = new Multi(Operator::MUL);
        Multi *simpDenom = new Multi(Operator::MUL);

        if (*numerator == *denominator)
            return new Num(1);

        // exactly one of the numerator or the denominator is a MultiNode
        else if ((numerator->type == NodeType::MultiNode && denominator->type != NodeType::MultiNode) || (denominator->type == NodeType::MultiNode && numerator->type != NodeType::MultiNode))
        {
            Multi *mNode;
            Node *notM;
            std::vector<Multi *> seenPowers;
            if (numerator->type == NodeType::MultiNode && denominator->type != NodeType::MultiNode)
            {
                mNode = static_cast<Multi *>(numerator);
                notM = static_cast<Multi *>(denominator);
            }
            else
            {
                mNode = static_cast<Multi *>(denominator);
                notM = static_cast<Multi *>(numerator);
            }

            bool isConsumed = false;
            if (mNode->op != Operator::POW)
            {
                if (mNode->op != Operator::MUL)
                    return new Multi(mNode, notM, Operator::DIV);
                for (Node *n : mNode->args)
                {
                    if (n->type == NodeType::MultiNode)
                    {
                        Multi *mArg = static_cast<Multi *>(n);
                        if (mArg->op == Operator::POW)
                        {
                            assert(mArg->args.size() == 2);
                            seenPowers.emplace_back(mArg);
                            continue;
                        }
                    }
                    if (*n == *notM && !isConsumed)
                        isConsumed = true;
                    else
                    {
                        if (numerator->type == NodeType::MultiNode)
                            simpNum->AddArg(n);
                        else
                            simpDenom->AddArg(n);
                    }
                }
            }
            else
                seenPowers.emplace_back(mNode);

            for (Multi *p : seenPowers)
            {
                if (*p->args[0] == *notM && !isConsumed)
                {
                    isConsumed = true;
                    if (numerator->type == NodeType::MultiNode)
                        simpNum->AddArg(new Multi(p->args[0], new Multi(p->args[1], new Num(1), Operator::SUB), Operator::POW));
                    else
                        simpDenom->AddArg(new Multi(p->args[0], new Multi(p->args[1], new Num(1), Operator::SUB), Operator::POW));
                }
                else
                {
                    if (numerator->type == NodeType::MultiNode)
                        simpNum->AddArg(p);
                    else
                        simpDenom->AddArg(p);
                }
            }

            if (!isConsumed)
            {

                if (numerator->type == NodeType::MultiNode)
                    simpDenom->AddArg(notM);
                else
                    simpNum->AddArg(notM);
            }
        }

        // both the numerator and the denominator are MultiNodes
        else
        {
            Multi *num = static_cast<Multi *>(numerator);
            Multi *denom = static_cast<Multi *>(denominator);

            // both are functions
            if (IsFunc(num) && IsFunc(denom))
            {
                if (*num == *denom)
                    return new Num(1);
                else
                    return source;
            }

            // exactly one is a function
            else if (IsFunc(num) || IsFunc(denom))
            {
                Multi *func;
                Multi *notFunc;
                if (IsFunc(num))
                {
                    func = num;
                    notFunc = denom;
                }
                else
                {
                    func = denom;
                    notFunc = num;
                }

                if (notFunc->op != Operator::POW && notFunc->op != Operator::MUL)
                    return source;

                if (notFunc->op == Operator::POW)
                {
                    if (*notFunc->args[0] == *func)
                    {
                        if (func == num)
                            simpDenom->AddArg(new Multi(func, new Multi(notFunc->args[1], new Num(1), Operator::SUB), Operator::POW));
                        else
                            simpNum->AddArg(new Multi(func, new Multi(notFunc->args[1], new Num(1), Operator::SUB), Operator::POW));
                    }
                }
                else
                {
                    bool isConsumed = false;
                    std::vector<Multi *> seenPowers;
                    for (Node *n : notFunc->args)
                    {
                        if (n->type == NodeType::MultiNode)
                        {
                            Multi *mArg = static_cast<Multi *>(n);
                            if (mArg->op == Operator::POW)
                            {
                                assert(mArg->args.size() == 2);
                                seenPowers.emplace_back(mArg);
                                continue;
                            }
                        }
                        if (*n == *func && !isConsumed)
                        {
                            isConsumed = true;
                            continue;
                        }
                        else
                        {
                            if (func == num)
                                simpDenom->AddArg(n);
                            else
                                simpNum->AddArg(n);
                        }
                    }

                    for (Multi *p : seenPowers)
                    {
                        if (*p->args[0] == *func && !isConsumed)
                        {
                            if (func == num)
                                simpDenom->AddArg(new Multi(func, new Multi(notFunc->args[1], new Num(1), Operator::SUB), Operator::POW));
                            else
                                simpNum->AddArg(new Multi(func, new Multi(notFunc->args[1], new Num(1), Operator::SUB), Operator::POW));
                            isConsumed = true;
                        }
                    }

                    if (!isConsumed)
                    {
                        if (func == num)
                            simpNum->AddArg(func);
                        else
                            simpDenom->AddArg(func);
                    }
                }
            }

            // both of them are not functions
            else
            {
                std::unordered_map<Node *, bool> isNumConsumed;
                std::unordered_map<Node *, bool> isDenomConsumed;
                bool isNumNotPower = true;
                bool isDenomNotPower = true;

                std::vector<Multi *> seenNumPowers;
                std::vector<Multi *> seenDenomPowers;

                // both the numerator and denominator are powers
                if (num->op == Operator::POW && denom->op == Operator::POW)
                {
                    assert(num->args.size() == 2 && denom->args.size() == 2);
                    if (*num->args[0] == *denom->args[0])
                        return new Multi(num->args[0], new Multi(num->args[1], denom->args[1], Operator::SUB), Operator::POW);
                }

                //exactly one is a power since we must have rejected the condition above
                else if (num->op == Operator::POW || denom->op == Operator::POW)
                {
                    if (num->op == Operator::POW)
                    {
                        if (denom->op != Operator::MUL)
                            return source;
                        isNumNotPower = false;
                        seenNumPowers.emplace_back(num);
                    }
                    else
                    {
                        if (num->op != Operator::MUL)
                            return source;
                        isDenomNotPower = false;
                        seenDenomPowers.emplace_back(denom);
                    }
                }

                // neither of them are powers
                for (Node *n : num->args)
                {
                    if (n->type == NodeType::MultiNode)
                    {
                        Multi *mNumArg = static_cast<Multi *>(n);
                        if (mNumArg->op == Operator::POW)
                        {
                            seenNumPowers.emplace_back(mNumArg);
                            continue;
                        }
                    }
                    for (Node *d : denom->args)
                    {
                        if (*n == *d && !isDenomConsumed[d])
                        {
                            isNumConsumed[n] = true;
                            isDenomConsumed[d] = true;
                            break;
                        }
                    }
                }

                for (Node *m : denom->args)
                {
                    if (m->type == NodeType::MultiNode)
                    {
                        Multi *mDenomArg = static_cast<Multi *>(m);
                        if (mDenomArg->op == Operator::POW)
                        {
                            seenDenomPowers.emplace_back(mDenomArg);
                        }
                    }
                }

                for (Multi *numPow : seenNumPowers)
                {
                    for (Multi *denomPow : seenDenomPowers)
                    {
                        if (*numPow->args[0] == *denomPow->args[0])
                        {
                            simpNum->AddArg(new Multi(numPow->args[0], new Multi(numPow->args[1], denomPow->args[1], Operator::SUB), Operator::POW));
                            isNumConsumed[numPow] = true;
                            isDenomConsumed[denomPow] = true;
                        }
                    }
                    for (Node *d : denom->args)
                    {
                        if (*numPow->args[0] == *d && !isDenomConsumed[d] && !isNumConsumed[numPow])
                        {
                            simpNum->AddArg(new Multi(d, new Multi(numPow->args[1], new Num(1), Operator::SUB), Operator::POW));
                            isNumConsumed[numPow] = true;
                            isDenomConsumed[d] = true;
                        }
                    }
                }

                for (Multi *denomPow : seenDenomPowers)
                {
                    for (Node *n : num->args)
                    {
                        if (*denomPow->args[0] == *n && !isNumConsumed[n] && !isDenomConsumed[denomPow])
                        {
                            simpDenom->AddArg(new Multi(n, new Multi(denomPow->args[1], new Num(1), Operator::SUB), Operator::POW));
                            isNumConsumed[n] = true;
                            isDenomConsumed[denomPow] = true;
                        }
                    }
                }

                if (isNumNotPower)
                {
                    for (Node *p : num->args)
                    {
                        if (!isNumConsumed[p])
                            simpNum->AddArg(p);
                    }
                }
                if (isDenomNotPower)
                {
                    for (Node *q : denom->args)
                    {
                        if (!isDenomConsumed[q])
                            simpDenom->AddArg(q);
                    }
                }
            }
        }

        if (simpNum->args.size() == 0 && simpDenom->args.size() == 0)
            return FoldConstants(new Num(1));
        else if (simpNum->args.size() == 0 && simpDenom->args.size() == 1)
            return FoldConstants(new Multi(new Num(1), simpDenom->args[0], Operator::DIV));
        else if (simpNum->args.size() == 0 && simpDenom->args.size() > 1)
            return FoldConstants(new Multi(new Num(1), simpDenom, Operator::DIV));
        else if (simpNum->args.size() == 1 && simpDenom->args.size() == 0)
            return FoldConstants(simpNum->args[0]);
        else if (simpNum->args.size() == 1 && simpDenom->args.size() == 1)
            return FoldConstants(new Multi(simpNum->args[0], simpDenom->args[0], Operator::DIV));
        else if (simpNum->args.size() == 1 && simpDenom->args.size() > 1)
            return FoldConstants(new Multi(simpNum->args[0], simpDenom, Operator::DIV));
        else if (simpNum->args.size() > 1 && simpDenom->args.size() == 0)
            return FoldConstants(simpNum);
        else if (simpNum->args.size() > 1 && simpDenom->args.size() == 1)
            return FoldConstants(new Multi(simpNum, simpDenom->args[0], Operator::DIV));
        else
            return FoldConstants(new Multi(simpNum, simpDenom, Operator::DIV));
    }

    bool IsFunc(Multi *source)
    {
        return (source->op != Operator::ADD && source->op != Operator::SUB && source->op != Operator::MUL && source->op != Operator::DIV && source->op != Operator::POW);
    }

} // namespace Simplify