#pragma once
#include "dot.h"
#include "node.h"

namespace Simplify
{

   Node *Simplify(Node *n);

   /*
      Returns true if further simplification is necessary.
      It is deemed that simplification is necessary if the number of
      Num nodes in a Multi node (in canonical form) is greater than 1 OR
      the number of occurances of the variable (currently only one variable
      is supported) is greater than 1 OR one of the children of the Multi 
      node is a Multi node itself.
    */
   bool NeedSimplify(Multi *source);

   /*---------------- Multi node simplifications ----------------*/


   /*
      Takes a tree of compositions of operators such as those produced
      when parsing and unrolls them, except if source->op == Token::POW
      since it makes little sense to unroll that operator, though it does
      mean that the POW operator is left associative.
    */
   Multi *Unroll(Multi *source);
   
   Node *PowCanonicalForm(Multi* source);

   /* 
      Simplfifies an UNROLLED Multi node of multiplications and divisions into
      a Multi node with one divide at the top and 2 children which are the
      numerators and denominators which are Multi nodes whose operation is multiplication.
    */
   Node *MulCanonicalForm(Multi *source);

   /* 
      Simplfifies an UNROLLED Multi node of additions and subtractions into a Multi
      node with one subtraction at the top and 2 children which are the additive
      and subtractive parts which are Multi nodes operation is addition
     */
   Node *AddCanonicalForm(Multi *source);

   /*
      Wrapper for MulCanonicalForm and AddCanonicalform and PowCanonicalForm (returns 
      the input if not a Multi node)
   */
   Node *CanonicalForm(Node *source);

   /*
      Folds the constants in a Node (acts like an evaluator but preserves variables).
      Requires the Node to be unrolled and in canonical form.
   */
   Node *FoldConstants(Node *source);

   /*
      Simplifies expressions like 3*x + 2*x into 5*x
   */
   Node *FoldAddVariables(Node *source);

   /*
      Simplifies expressions like x^2 * x^3 into x^5
   */
   Node *FoldMulVariables(Node *source);


   /*
      Wrapper for FoldAddVariables and FoldMulVariables
   */
   Node* FoldVariables(Node* source);

   Node *SimplifyAddition(Multi *source);
   Node *SimplifyMultiplication(Multi *source);
} // namespace Simplify