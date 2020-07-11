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

    int i = 0;
    Node *n = p.Parse(i);
    std::cout << "parsed: " << n->ToString() << std::endl;
    
    Multi* mParsed = static_cast<Multi*>(n);
    mParsed = Simplify::Unroll(mParsed);

    std::cout << "mParsed: " << mParsed->ToString() << std::endl;

    Node* simpAdd = Simplify::SimplifyAddition(mParsed);
    std::cout << "addition simplified: " << simpAdd->ToString() << std::endl;































    // Node *simplify = Simplify::Simplify(n);
    // std::cout << "simplified: " << simplify->ToString() << std::endl;

    // Node *prime = Differentiate::Differentiate(simplify, "x");
    // std::cout << "derivative pre-simplification: " << prime->ToString() << std::endl;

    // Multi *mSimpPrime = static_cast<Multi *>(prime);

    // Multi *mSimpPrimeUnrolled = Simplify::Unroll(mSimpPrime);
    // std::cout << "unrolled derivative: " << mSimpPrimeUnrolled->ToString() << std::endl;

    // Node *derivFoldedC = Simplify::FoldConstants(mSimpPrimeUnrolled);
    // std::cout << "derivative with folded constants: " << derivFoldedC->ToString() << std::endl;

    // Node *derivFoldedV = Simplify::FoldVariables(derivFoldedC);
    // std::cout << "derivative with folded variables: " << derivFoldedV->ToString() << std::endl;

    // Node *derivCanon = Simplify::CanonicalForm(derivFoldedV);
    // std::cout << "derivative in canonical form: " << derivCanon->ToString() << std::endl;

    // Node *unrolledDCanon = Simplify::Unroll(static_cast<Multi *>(derivCanon));
    // std::cout << "unrolled canonical form: " << unrolledDCanon->ToString() << std::endl;

    // Node *simpPrime1 = Simplify::Simplify(prime);
    // std::cout << "derivative after 1 round of simplification: " << simpPrime1->ToString() << std::endl;

    // Node* simpPrime2 = Simplify::Simplify(simpPrime1);
    // std::cout << "derivative after 2 rounds of simplification: " << simpPrime2->ToString() << std::endl;

    return 0;
}