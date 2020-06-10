#include "unit.h"

Unit::Unit(Token _t, std::string _var, double _val, std::string _func)
{
    t = _t;
    var = _var;
    val = _val;
    func = _func;
}