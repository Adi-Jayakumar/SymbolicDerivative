#include "tokeniser.h"

Tokeniser::Tokeniser(std::string _file)
{
    std::ifstream myFile;
    myFile.open(_file);
    if (!myFile)
    {
        std::cout << "Could not open file" << std::endl;
        exit(1);
    }
    std::getline(myFile, expr);
    myFile.close();
}

void Tokeniser::Tokenise()
{
    std::unordered_map<std::string, Token> funcs =
        {
            {"ln", Token::LN},
            {"exp", Token::EXP},
            {"sin", Token::SIN},
            {"cos", Token::COS},
            {"tan", Token::TAN}
        };
    Token token = Token::EoF;
    std::string var = "";
    std::string func = "";
    double value = 0;
    for (long unsigned int i = 0; i < expr.size(); i++)
    {
        if (isspace(expr[i]))
            continue;
        if (isdigit(expr[i]))
        {
            token = Token::NUM;
            std::string num;
            while (true)
            {
                num += expr[i];
                i++;

                if (!isdigit(expr[i]) && expr[i] != '.')
                {
                    i--;
                    break;
                }
            }
            value = stod(num);
        }
        else if (isalpha(expr[i]))
        {
            token = Token::VAR;
            std::string varName;
            while (true)
            {
                varName += expr[i];
                i++;
                if (!isalpha(expr[i]))
                {
                    if (expr[i] == '(')
                    {
                        func = varName;
                        if(funcs.find(func) != funcs.end())
                            token = funcs[func];
                        else
                            throw std::invalid_argument("Function name not recognised");
                    }
                    i--;
                    break;
                }
            }
            var = varName;
        }
        else if (expr[i] == '+')
            token = Token::ADD;
        else if (expr[i] == '-')
            token = Token::SUB;
        else if (expr[i] == '*')
            token = Token::MUL;
        else if (expr[i] == '/')
            token = Token::DIV;
        else if (expr[i] == '^')
            token = Token::POW;
        else if (expr[i] == '(')
            token = Token::OP;
        else if (expr[i] == ')')
            token = Token::CP;

        // std::cout << token << " " << value << " " << var << endl;
        // if (token == FUNC)
        // {
        //     tokens.emplace_back(Unit(token, "", 0, func));
        //     continue;
        // }
        if (token == Token::VAR)
        {
            tokens.emplace_back(Unit(token, var, 0, ""));
            continue;
        }
        tokens.emplace_back(Unit(token, var, value, func));
    }
    tokens.emplace_back(Unit(Token::EoF, "", 0, ""));
}