#include "format.h"

using format::FormatString;
using format::FormatDict;
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
        std::string toString(const P2d& p, const std::string& parms)
        {
            return FormatString("P2d(%+020>*/2,4:{x}; %011X,2:{y})") % FormatDict()
                ("x", p.x)
                ("y", p.y);
        }
    };
}

#include <iostream>
#include <vector>
#include <list>

int main()
{
    std::vector<int> v;
    std::list<int> L;
    for (int i=0; i<10; i++)
    {
        v.push_back(i*i);
        L.push_back(i*i);
    }

    FormatString fs("p = %{p}\nv = [%*/, {v}]\nL = %(*)/->{L}");
    FormatDict fd;
    fd  ("p", P2d(1234, 0xbadf00d))
        ("v", sequence(v.begin(), v.end()))
        ("L", sequence(L.begin(), L.end()));
    std::cout << fs % fd << std::endl;
    return 0;
}
