#include "differentiate.h"
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
    
    Node* n = p.Parse(0);

    Node *simpParsed = Simplify::Reduce(n);
    std::cout << std::endl
              << "Simplified input: " << simpParsed->ToString() << std::endl
              << std::endl;

    Node *differentiated = Differentiate::Differentiate(simpParsed, "x");
    std::cout << "Unsimplified derivative: " << differentiated->ToString() << std::endl
              << std::endl;

    Node *simpDiff = Simplify::Reduce(differentiated);
    std::cout << "Simplified derivative: " << simpDiff->ToString() << std::endl
              << std::endl;

    return 0;
}