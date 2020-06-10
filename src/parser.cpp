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
        lhs = new BinaryNode(lhs, rhs, op);
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
        lhs = new BinaryNode(lhs, rhs, op);
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
        Node *rhs = ParseUnit(i);
        lhs = new BinaryNode(lhs, rhs, op);
    }
}

/*
    Parses a unary node (only Var and Num), leaves the index 
    on the node that was parsed
*/
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
        return new Unary(ParseUnit(i), Token::SUB);
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
        Token f = tk.tokens[i].t;
        i++;
        return new Unary(ParseUnit(i), f);
    }
        return new Num(0);
}