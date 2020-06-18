#pragma once
#include <iostream>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>
#include "token.h"

enum class NodeType
{
    NumNode,
    VarNode,
    UnaryNode,
    Multi
};

struct Node
{
    NodeType type;
    virtual std::string ToString() = 0;
    // virtual Node *Differentiate() = 0;
    virtual Node *Simplify() = 0;
    Node *Reduce();
    virtual void BuildDOTNodeAlias(int &label, std::map<Node *, int> &map) = 0;
    virtual std::string GetDOTLabel() = 0;
    virtual void BuildDOTTree(std::string &DOT, std::map<Node*, int> &map) = 0;
};

struct Num : Node
{
    double val;
    Num(double _val);
    std::string ToString();
    // Node *Differentiate();
    Node *Simplify();
    void BuildDOTNodeAlias(int &label, std::map<Node *, int> &map);
    std::string GetDOTLabel();
    void BuildDOTTree(std::string &DOT, std::map<Node *, int> &map);
};

struct Var : Node
{
    std::string id;
    Var(std::string _id);
    std::string ToString();
    // Node *Differentiate();
    Node *Simplify();
    void BuildDOTNodeAlias(int &label, std::map<Node *, int> &map);
    std::string GetDOTLabel();
    void BuildDOTTree(std::string &DOT, std::map<Node *, int> &map);
};

struct Unary : Node
{
    Node *arg;
    Token op;
    Unary(Node *_arg, Token _op);
    std::string ToString();
    // Node *Differentiate();
    Node *BaseArg();
    int Parity(int &p);
    Node *SimplifyNegations();
    Node *Simplify();
    void BuildDOTNodeAlias(int &label, std::map<Node *, int> &map);
    std::string GetDOTLabel();
    void BuildDOTTree(std::string &DOT, std::map<Node *, int> &map);
};

struct Multi : Node
{
    std::vector<Node *> args;
    Token op;
    Multi(Token _op);
    Multi(Node *_lhs, Token _op);
    Multi(Node *_lhs, Node *_rhs, Token _op);
    void AddArg(Node *a);
    std::string ToString();
    // Node *Differentiate();
    std::vector<int> FindDivisons();
    Node *MulStdForm();
    Node *Simplify();
    void BuildDOTNodeAlias(int &label, std::map<Node *, int> &map);
    std::string GetDOTLabel();
    void BuildDOTTree(std::string &DOT, std::map<Node *, int> &map);
};