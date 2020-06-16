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
    Node* n = p.Parse(i);
    cout << n->ToString() << endl;
    Node* m = n->Reduce();
    cout << m->ToString()<< endl;
    return 0;
}