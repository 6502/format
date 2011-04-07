#if !defined(FORMATSTRING_H_INCLUDED)
#define FORMATSTRING_H_INCLUDED

//              Copyright Andrea Griffini 2011.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

// Format strings are stored as a recursive structure with
// sequences of fields where each field can have subformats.
//
// This is done to allow specifying formatting options for
// containers or subsequences that will need to specify
// the formatting options for the element.
//
// The parser is a recursive descent one and the syntax is
//
//    Legend:  < x >            zero or one
//             x | y | z        alternative
//             [ x ]            zero or more
//             /.../            regexp
//
//    format     ::=  [ static | field ]
//    static     ::=  /([^{~]|~{|~~)+/
//    field      ::=  '{' name < ':' options [ ':' subformat ] > '}'
//    name       ::=  /([^:}~]|~:|~}|~~)*/
//    options    ::=  /([^:}~]|~:|~}|~~)*/
//    subformat  ::=  format
//
// examples:
//
//    "Hello {name}"         one static + one field with no
//                           options and no subformats
//
//    "Hello {name:30U}"     same with options == "30U"
//
//    "{vec::{x:+4}}"        no static, one format with no options
//                           but one subformat that has options == "+4"
//
//    "{mac:17x,2~:}"        one field with options "17x,2:"
//

#include <vector>
#include <string>
#include <stdexcept>

namespace format
{
    struct Field;

    typedef std::vector<Field> Format;

    struct Field
    {
        std::string name;
        std::string options;
        std::vector<Format> subformats;

        Field(const std::string& name,
              const std::string& options)
        : name(name), options(options)
        {
        }
    };

    Format parseFormat(const char *& p)
    {
        Format result;
        bool last_static = false;
        while (*p && *p != '}')
        {
            std::string sta;
            while (*p)
            {
                if (*p == '~' && p[1])
                    p++;
                else if (*p == '{' || *p == '}')
                    break;
                sta += *p++;
            }
            if (sta.size())
            {
                if (!last_static)
                {
                    result.push_back(Field("", ""));
                    last_static = true;
                }
                result.back().options += sta;
            }
            if (*p == '{')
            {
                p++;
                std::string name;
                while (*p)
                {
                    if (*p == '~' && p[1])
                        p++;
                    else if (*p == ':' || *p == '}')
                        break;
                    name += *p++;
                }
                std::string options;
                std::vector<Format> subformats;
                if (*p == ':')
                {
                    p++;
                    while (*p)
                    {
                        if (*p == '~' && p[1])
                            p++;
                        else if (*p == ':' || *p == '}')
                            break;
                        options += *p++;
                    }
                    while (*p == ':')
                    {
                        p++;
                        subformats.push_back(parseFormat(p));
                    }
                }
                if (*p != '}')
                    throw std::runtime_error("'}' expected");
                result.push_back(Field(name, options));
                result.back().subformats.swap(subformats);
                last_static = false;
                p++;
            }
        }
        return result;
    }
}

#endif // FORMATSTRING_H_INCLUDED
