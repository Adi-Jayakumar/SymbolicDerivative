#include "dot.h"
#include <iostream>

namespace DOT
{

    void Tree(Node *n, std::map<int, Point> &map, int &label, std::string &DOT)
    {
        map[++label] = Point{n->type, n->SelfToString()};
        if (n->type == NodeType::NumNode || n->type == NodeType::VarNode)
        {
            DOT += std::to_string(label) + '\n';
            return;
        }
        else if (n->type == NodeType::UnaryNode)
        {
            Unary *u = dynamic_cast<Unary *>(n);
            DOT += std::to_string(label) + "--";
            Tree(u->arg, map, label, DOT);
            return;
        }
        else
        {
            assert(n->type == NodeType::Multi);
            Multi *m = dynamic_cast<Multi *>(n);
            int locLabel = label;
            for (Node *n : m->args)
            {
                DOT += std::to_string(locLabel) + "--";
                Tree(n, map, label, DOT);
            }
            return;
        }
    }

    std::string ToDOT(Node *n)
    {
        std::string mapping = "";
        std::map<int, Point> map;
        int label = 0;

        Tree(n, map, label, mapping);
        std::string labels = "";
        for (auto const &[id, pt] : map)
        {
            std::string style;
            if (pt.nt == NodeType::Multi || pt.nt == NodeType::UnaryNode)
                style = ", style = filled, color = black, fillcolor = gray95";
            else if (pt.nt == NodeType::VarNode)
                style = ", style = filled, color = black, fillcolor = cadetblue1";
            else
                style = ", style = filled, color = black, fillcolor = white";
            labels += std::to_string(id) + "[label = \"" + pt.s + "\" " + style + "] \n";
        }

        return "graph {\n" + labels + mapping + "}\n";
    }
} // namespace DOT