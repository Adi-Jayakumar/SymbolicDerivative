#pragma once
#include <iostream>
#include <string>
#include "token.h"

enum class NodeType
{
    NumNode,
    VarNode,
    UnaryNode,
    Binary
};

struct Node
{
    NodeType type;
    virtual std::string Print() = 0;
    virtual Node *Differentiate() = 0;
    virtual Node *Simplify() = 0;
};

struct Num : Node
{
    double val;
    Num(double _val);
    std::string Print();
    Node *Differentiate();
    Node *Simplify();
};

struct Var : Node
{
    std::string id;
    Var(std::string _id);
    std::string Print();
    Node *Differentiate();
    Node *Simplify();
};

struct Unary : Node
{
    Node *arg;
    Token op;
    Unary(Node *_arg, Token _op);
    std::string Print();
    Node *Differentiate();
    Node * BaseArg();
    int Parity(int &p);
    Node * SimplifyNegations();
    Node *Simplify();
};

struct BinaryNode : Node
{
    Node *lhs;
    Node *rhs;
    Token op;
    BinaryNode(Node *_lhs, Node *_rhs, Token _op);
    std::string Print();
    Node *Differentiate();
    Node *Simplify();
};