#include <iostream>
#include <map>
#include <set>
#include <functional>
#include "parser.h"
#include "node.h"
#include "tokeniser.h"

using namespace std;

std::string DOTLabels(std::map<Node *, int> map)
{
    std::string res = "";
    auto order = [](std::pair<Node*, int> x, std::pair<Node*, int> y){return x.second < y.second;};
    std::set<std::pair<Node*, int>, decltype(order)> s(map.begin(), map.end(), order);
    for (auto x : s)
    {
        res += std::to_string(map[x.first]) + " [label = \"" + x.first->GetDOTLabel() + "\" ]" + '\n';
    }
    return res;
}

int main()
{

    Parser p = Parser("test.txt");
    p.tk.Tokenise();
    for (Unit u : p.tk.tokens)
    {
        std::cout << "Token: " << static_cast<int>(u.t) << " "
             << "Value: " << u.val << " "
             << "Var: " << u.var << " "
             << "Func: " << u.func << endl;
    }

    int i = 0;

    std::cout << endl;

    Node *n = p.Parse(i);
    std::cout << n->ToString() << endl;

    std::cout << endl;

    std::map<Node *, int> map;
    int label = 0;

    n->BuildDOTNodeAlias(label, map);

    std::cout << endl;

    for (auto x : map)
    {
        std::cout << x.first->GetDOTLabel() << " " << x.second << endl;
    }

    std::cout << endl;

    std::string DOT = "";

    std::cout << endl;

    std::cout << DOTLabels(map) << endl;
    n->BuildDOTTree(DOT, map);
    std::cout << DOT << endl;

    return 0;
}