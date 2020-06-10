#include <iostream>
#include "parser.h"
#include "node.h"
#include "tokeniser.h"

using namespace std;

int main()
{

    Parser p = Parser("test.txt");
    p.tk.Tokenise();
    for (Unit u : p.tk.tokens)
    {
        cout << "Token: " << static_cast<int>(u.t) << " "
             << "Value: " << u.val << " "
             << "Var: " << u.var << " "
             << "Func: " << u.func << endl;
    }

    int i = 0;
    Node *n = p.Parse(i);
    cout << n->Print() << endl;
    Node *m = n->Simplify();
    cout << m->Print() << endl;
    // for (int i = 0; i < 5; i++)
    // {
    //     m = m->Simplify();
    //     cout << m->Print() << endl;
    // }
    // int p = 0;
    // Unary *u = new Unary(new Unary(new Unary(new Num(1000), LN), SUB), SUB);
    // cout << u->Print() << std::endl;
    // cout << u->Simplify()->Print() << std::endl;

    return 0;
}