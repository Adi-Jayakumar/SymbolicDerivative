#pragma once
#include "token.h"
#include <iostream>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>

enum class NodeType
{
    NumNode,
    VarNode,
    MultiNode
};

struct Node
{
    NodeType type;
    virtual std::string ToString() = 0;
    virtual std::string SelfToString() = 0;
    virtual bool Equals(Node &rhs) = 0;
    friend bool operator==(const Node &lhs, const Node &rhs);
    virtual ~Node() = 0;
};

struct Num : Node
{
    double val;
    Num(double _val);
    Num(Num *n) : Num(n->val) {}
    std::string ToString();
    std::string SelfToString();
    bool Equals(Node &rhs);
};

struct Var : Node
{
    std::string id;
    Var(std::string _id);
    Var(Var *v) : Var(v->id) {}
    std::string ToString();
    std::string SelfToString();
    bool Equals(Node &rhs);
};

struct Multi : Node
{
    std::vector<Node *> args;
    Token op;
    Multi(Token _op);
    Multi(Node *_lhs, Token _op);
    Multi(Node *_lhs, Node *_rhs, Token _op);
    Multi(std::vector<Node *> _args, Token _op);
    Multi(Multi *m) : Multi(m->args, m->op) {}
    ~Multi();
    void AddArg(Node *a);
    std::string ToString();
    std::string SelfToString();
    bool Equals(Node &rhs);
};