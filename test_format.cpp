#include "format.hpp"

using format::fmt;
using format::Dict;
using format::sequence;

struct P2d
{
    double x, y;
    P2d(double x, double y)
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
            return fmt("P2d({x:+0.3f}, {y:+0.3f})") % Dict()
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
                            "S = ~{{S: :0x{*:04x}}~}");
    Dict fd;
    fd  ("p", P2d(1234, 0xbadf00d))
        ("v", sequence(v))
        ("L", sequence(L))
        ("S", sequence(S));
    std::cout << fs % fd << std::endl;

    std::string lines[] = {"This", "is a test", "for the string formatting options"};

    std::cout << fmt("{L:\n:{*:=60}}") % Dict()("L", sequence(&lines[0], &lines[3])) << std::endl;

    std::cout << fmt("{n:@(###)-########}") % Dict()("n", std::string("555123456789012")) << std::endl;

    std::map<std::string, int> m;
    m["I"] = 1;
    m["II"] = 2;
    m["III"] = 3;
    m["IV"] = 4;
    m["V"] = 5;

    std::cout << fmt("{m:\n:{*::{*1:=8l} => {*2:08/2}}}") % Dict()("m", sequence(m)) << std::endl;

    return 0;
}
