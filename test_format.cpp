#include "format.h"

using format::fmt;
using format::Dict;
using format::sequence;

struct P2d
{
    int x, y;
    P2d(int x, int y)
        : x(x), y(y)
    {
    }
};

namespace format {
    template<>
    struct Formatter<P2d>
    {
        std::string toString(const P2d& p, const Field& field)
        {
            return fmt("P2d({x:+020>*/2,4}; {y:011X,2~:})") % Dict()
                ("x", p.x)
                ("y", p.y);
        }
    };
}

#include <iostream>
#include <vector>
#include <list>
#include <set>

int main()
{
    std::vector<int> v;
    std::list<int> L;
    std::set<int> S;
    for (int i=0; i<10; i++)
    {
        v.push_back(i*i);
        L.push_back(i*i);
        S.insert(i*i);
    }

    format::Format fs = fmt("p = {p}\nv = [{v:, }]\n"
                            "L = {L:->:({*})}\n"
                            "S = ~{{S: }~}");
    Dict fd;
    fd  ("p", P2d(1234, 0xbadf00d))
        ("v", sequence(v))
        ("L", sequence(L))
        ("S", sequence(S));
    std::cout << fs % fd << std::endl;
    return 0;
}
