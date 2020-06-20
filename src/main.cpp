#include "dot.h"
#include "node.h"
#include "parser.h"
#include "simplify.h"
#include "tokeniser.h"
#include <cassert>
#include <functional>
#include <iostream>
#include <map>
#include <set>

int main()
{

    Parser p = Parser("test.txt");
    p.tk.Tokenise();
    for (Unit u : p.tk.tokens)
    {
        std::cout << "Token: " << static_cast<int>(u.t) << " "
                  << "Value: " << u.val << " "
                  << "Var: " << u.var << " "
                  << "Func: " << u.func << std::endl;
    }

    int i = 0;

    std::cout << std::endl;

    Node *n = p.Parse(i);
    std::cout << n->ToString() << std::endl;
    Multi*m = dynamic_cast<Multi*>(n);
    n = Simplify::MulCanonicalForm(m);
    std::cout<<n->ToString()<< std::endl;

    // // std::cout << DOT::ToDOT(n) << std::endl;

    // n = Simplify::Simplify(n);
    // // std::cout << DOT::ToDOT(n) << std::endl;
    // std::cout << n->ToString() << std::endl;

    return 0;
}