#include "parser.h"

Parser::Parser(std::string _file)
{
    tk = Tokeniser(_file);
}

Node *Parser::Parse(int &i)
{
    Node *lhs = ParseMul(i);
    while (true)
    {
        if (tk.tokens[i].t == Token::EoF || tk.tokens[i].t == Token::CP)
            return lhs;

        Token op = Token::EoF;

        if (tk.tokens[i].t == Token::ADD)
            op = Token::ADD;
        else if (tk.tokens[i].t == Token::SUB)
            op = Token::SUB;

        if (tk.tokens[i].t == Token::EoF || op == Token::EoF)
            return lhs;

        i++;
        Node *rhs = ParseMul(i);
        if (op == Token::SUB)
            rhs = new Multi(new Num(-1), rhs, Utility::TokenToOperator(Token::MUL));

        lhs = new Multi(lhs, rhs, Operator::ADD);
    }
}

Node *Parser::ParseMul(int &i)
{

    Node *lhs = ParseInd(i);
    while (true)
    {
        Token op = Token::EoF;

        if (tk.tokens[i].t == Token::MUL)
            op = Token::MUL;
        else if (tk.tokens[i].t == Token::DIV)
            op = Token::DIV;

        if (tk.tokens[i].t == Token::EoF || op == Token::EoF)
            return lhs;

        i++;
        Node *rhs = ParseInd(i);

        if (op == Token::DIV)
            rhs = new Multi(new Num(1), rhs, Operator::DIV);

        lhs = new Multi(lhs, rhs, Operator::MUL);
    }
}

Node *Parser::ParseInd(int &i)
{
    Node *lhs = ParseUnit(i);
    while (true)
    {
        i++;
        Token op = Token::EoF;

        if (tk.tokens[i].t == Token::POW)
            op = Token::POW;

        if (tk.tokens[i].t == Token::EoF || op == Token::EoF)
            return lhs;

        i++;
        Node* rhs = ParseUnit(i);
        lhs = new Multi(lhs, rhs, Operator::POW);
    }
}

Node *Parser::ParseUnit(int &i)
{
    if (tk.tokens[i].t == Token::NUM)
        return new Num(tk.tokens[i].val);
    else if (tk.tokens[i].t == Token::ADD)
    {
        i++;
        return ParseUnit(i);
    }
    else if (tk.tokens[i].t == Token::SUB)
    {
        i++;
        return new Multi(new Num(-1), ParseUnit(i), Operator::MUL);
    }
    else if (tk.tokens[i].t == Token::VAR)
        return new Var(tk.tokens[i].var);
    else if (tk.tokens[i].t == Token::OP)
    {
        i++;
        Node *brackets = Parse(i);
        if (tk.tokens[i].t != Token::CP)
            throw std::invalid_argument("Missing close parenthesis");
        return brackets;
    }
    else if (tk.tokens[i].t == Token::LN || tk.tokens[i].t == Token::EXP || tk.tokens[i].t == Token::SIN || tk.tokens[i].t == Token::COS || tk.tokens[i].t == Token::TAN)
    {
        Operator f = Utility::TokenToOperator(tk.tokens[i].t);
        i++;
        return new Multi(ParseUnit(i), f);
    }
    return new Num(0.);
}