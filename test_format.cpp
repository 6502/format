#include "format.hpp"

using format::fmt;
using format::Dict;
using format::sequence;
using format::str;

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

// Silly example, just to test inheritance
struct P3d : P2d
{
    double z;
    P3d(double x, double y, double z)
        : P2d(x, y), z(z)
    {
    }
};

#include <iostream>
#include <vector>
#include <list>
#include <set>

int main()
{
    std::vector<int> v;
    std::list<int> L;
    std::set<int> S;
    std::map<std::string, int> m;
    m["I"] = 1;
    m["II"] = 2;
    m["III"] = 3;
    m["IV"] = 4;
    m["V"] = 5;

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

    std::cout << fmt("{1}\n{2}\n{3}\n{4}\n") % Dict()
        ("1", v)
        ("2", L)
        ("3", S)
        ("4", m);

    std::string lines[] = {"This", "is a test", "for the string formatting options"};

    std::cout << fmt("{L:\n:{*:=60}}") % Dict()("L", sequence(&lines[0], &lines[3])) << std::endl;

    std::cout << fmt("{n:@(###)-########}") % Dict()("n", std::string("555123456789012")) << std::endl;

    std::cout << fmt("{m:\n:{*::{*1:=8l} => {*2:08/2}}}") % Dict()("m", sequence(m)) << std::endl;

    std::cout << fmt("{s:>30=.}") % Dict()("s", "This is a C string") << std::endl;

    std::cout << fmt("{v}") % Dict()("v", &v) << std::endl;

    P3d p3(11, 21, 31);
    try
    {
        std::cout << fmt("{p}") % Dict()("p", p3) << std::endl;
        printf("WE GOT A PROBLEM\n");
    }
    catch(const std::runtime_error& re)
    {
        std::cout << fmt("Exception thrown as expected --> {s}\n") % Dict()("s", re.what());
    }

    std::cout << fmt("{p}") % Dict()("p", static_cast<P2d *>(&p3)) << std::endl; // Formatted as P2d

    std::cout << str(3.141592654) << std::endl;
    std::cout << str(std::string("This is a test"), "{*:=40==}") << std::endl;
    std::cout << str(175676, "09X,4~:") << std::endl;

    return 0;
}
