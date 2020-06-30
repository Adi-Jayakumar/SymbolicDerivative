#include "dot.h"
#include "node.h"
#include "parser.h"
#include "simplify.h"
#include "tokeniser.h"
#include <iostream>

int main()
{

    Parser p = Parser("test.txt");
    p.tk.Tokenise();
    // for (Unit u : p.tk.tokens)
    // {
    //     std::cout << "Token: " << static_cast<int>(u.t) << " "
    //               << "Value: " << u.val << " "
    //               << "Var: " << u.var << " "
    //               << "Func: " << u.func << std::endl;
    // }

    int i = 0;

    std::cout << std::endl;

    Node *n = p.Parse(i);
    Multi *m = dynamic_cast<Multi *>(n);
    Node *mUnrolled = Simplify::Unroll(m);
    Node *foldV = Simplify::FoldVariables(mUnrolled);
    std::cout << "Folded Variables: " << foldV->ToString() << std::endl;
    return 0;
}