#if !defined(FORMAT_H_INCLUDED)
#define FORMAT_H_INCLUDED

//              Copyright Andrea Griffini 2011.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
//
// An attempt to try to get usable output formatting in C++
// Comments are of course welcome (agriff@tin.it).
//

#include <string>
#include <vector>
#include <map>
#include <stdexcept>

namespace format
{
    template<typename T>
    struct Formatter
    {
        std::string toString(const T& x, const std::string& parms)
        {
            throw std::runtime_error("Unsupported type");
            return "";
        }
    };

    class ValueWrapperBase
    {
    public:
        ValueWrapperBase() {}
        virtual ~ValueWrapperBase() {}
        virtual std::string toString(const std::string& parms) const = 0;
    private:
        // Taboo
        ValueWrapperBase(const ValueWrapperBase&);
        ValueWrapperBase& operator=(const ValueWrapperBase&);
    };

    template<typename T>
    class ValueWrapper : public ValueWrapperBase
    {
    public:
        T x;
        ValueWrapper(const T& x) : x(x) {}
        virtual std::string toString(const std::string& parms) const
        {
            return Formatter<T>().toString(x, parms);
        }
    private:
        // Taboo
        ValueWrapper(const ValueWrapper&);
        ValueWrapper& operator=(const ValueWrapper&);
    };

    typedef std::map<std::string, ValueWrapperBase *> Env;

    class FormatDict
    {
    private:
        Env env;

    public:
        FormatDict() {}
        virtual ~FormatDict()
        {
            for (Env::iterator i=env.begin(), e=env.end(); i!=e; ++i)
                delete i->second;
        }

        template<typename T>
        FormatDict& operator()(const std::string& name, const T& value)
        {
            Env::iterator p = env.find(name);
            if (p == env.end())
            {
                env[name] = new ValueWrapper<T>(value);
            }
            else
            {
                ValueWrapperBase *vw = new ValueWrapper<T>(value);
                delete p->second;
                p->second = vw;
            }
            return *this;
        }

        const ValueWrapperBase& operator[](const std::string& name) const
        {
            Env::const_iterator p = env.find(name);
            if (p == env.end())
                throw std::runtime_error("Field not present");
            return *(p->second);
        }

    private:
        // Taboo
        FormatDict(const FormatDict&);
        FormatDict& operator=(const FormatDict&);
    };

    struct Field
    {
        std::string name;
        std::string parms;

        Field(const std::string& name, const std::string& parms)
            : name(name), parms(parms)
        {
        }

        void write(std::string& result, const FormatDict& fd) const
        {
            if (name.size())
            {
                // A real field block
                result += fd[name].toString(parms);
            }
            else
            {
                // A static text block
                result += parms;
            }
        }
    };

    struct FormatString
    {
        std::vector<Field> fields;

        FormatString(const std::string& s)
        {
            const char *p = s.c_str();
            bool last_is_static = false;
            while (*p)
            {
                const char *static_start = p;
                while (*p && *p != '%')
                    p++;
                const char *static_end = p;
                if (static_end != static_start)
                {
                    if (!last_is_static)
                    {
                        last_is_static = true;
                        fields.push_back(Field("", ""));
                    }
                    fields.back().parms += std::string(static_start, static_end);
                }
                if (*p == '%')
                {
                    p++;
                    if (*p == '%')
                    {
                        // Escape
                        if (!last_is_static)
                        {
                            fields.push_back(Field("", ""));
                            last_is_static = true;
                        }
                        fields.back().parms += '%';
                        p++;
                    }
                    else
                    {
                        const char *parms_start = p;
                        while (*p && *p!='{')
                            p++;
                        if (*p != '{')
                            throw std::runtime_error("Invalid format string ('{' expected after '%')");
                        const char *parms_end = p++;
                        const char *name_start = p;
                        while (*p && *p!='}')
                            p++;
                        if (*p != '}')
                            throw std::runtime_error("Invalid format string ('}' expected after '{')");
                        const char *name_end = p++;
                        fields.push_back(Field(std::string(name_start, name_end),
                                               std::string(parms_start, parms_end)));
                        last_is_static = false;
                    }
                }
            }
        }

        std::string format(const FormatDict& fd) const
        {
            std::string result;
            for (int i=0,n=fields.size(); i<n; i++)
                fields[i].write(result, fd);
            return result;
        }
    };

    inline std::string operator % (const FormatString& fs, const FormatDict& fd)
    {
        return fs.format(fd);
    }

    ///////////////////////////////////////
    //                                   //
    //  Predefined formatting functions  //
    //                                   //
    ///////////////////////////////////////

    //
    // Sequence formatting options
    //
    // The parameter string is used as a format string for the
    // elements being formatted where '*' is used as name of
    // the element being iterated (this can be escaped by doubling).
    // If a character '/' is present then the part following
    // it is used as separator between elements and is not added
    // after last element.
    //
    // An empty string as parameter is assumed as being "*/, "
    //
    // A sequence object is defined by two iterators.
    //
    template<typename T>
    struct Sequence
    {
        T it0, it1;
        Sequence(const T& it0, const T& it1)
            : it0(it0), it1(it1)
        {
        }
    };

    template<typename T>
    struct Formatter< Sequence<T> >
    {
        std::string toString(const Sequence<T>& xx, const std::string& parms)
        {
            std::string result;
            std::string fmt;
            std::string sep;
            for (int i=0,n=parms.size(); i<n; i++)
            {
                if (parms[i] == '*' && i < n-1 && parms[i+1] == '*')
                {
                    fmt += '*';
                    i += 1;
                }
                else if (parms[i] == '*')
                {
                    fmt += "%{x}";
                }
                else
                {
                    fmt += parms[i];
                }
            }
            if (fmt == "")
            {
                fmt = "%{x}/,";
            }

            if (fmt.find('/') != std::string::npos)
            {
                sep = fmt.substr(fmt.find('/') + 1);
                fmt = fmt.substr(0, fmt.size() - sep.size() - 1);
            }

            FormatString fs(fmt);
            FormatDict element;
            for (T i=xx.it0,e=xx.it1; i!=e;)
            {
                element("x", *i);
                result += fs % element;
                ++i;
                if (i != e) result += sep;
            }
            return result;
        }
    };

    template<typename T>
    Sequence<T> sequence(const T& i0, const T& i1)
    {
        return Sequence<T>(i0, i1);
    }

    //
    // Int formatting options
    //
    // plus        ::=  '+'
    //
    // width       ::=  [ '0'-'9' ]
    //
    // overflow    ::=  char
    //
    // filler      ::=  char
    //
    // base        ::=  '1'-'9' [ '0'-'9' ]
    //
    // dsep        ::=  ',' [ '0'-'9' ]
    //
    // sepchar     ::=  char
    //
    // align       ::=  { '<' | '=' | '>' }
    //
    // upcase      ::=  'U'
    //
    // format      ::=  { align }
    //                  { plus }
    //                  { { '0' | '=' filler } }
    //                  width
    //                  { '>' overflowchar }
    //                  { { 'x' | 'X' | '/' base { upcase } }
    //                  { dsep { sepchar } }
    //
    template<>
    struct Formatter<int>
    {
        std::string toString(const int& xx, const std::string& parms)
        {
            const char *p = parms.c_str();
            int align = 1;
            int plus = 0;
            int filler = ' ';
            int width = 0;
            int overflowchar = 0;
            int base = 10;
            int dsep = 0;
            int dsepchar = ',';
            int upcase = 0;

            int x = xx;

            // Parse alignment if present
            if (*p == '<')          { align = -1; p++; }
            else if (*p == '=')     { align = 0; p++; }
            else if (*p == '>')     { align = 1; p++; }

            // Parse plus request if present
            if (*p == '+')          { plus = 1; p++; }

            // Parse filler if present
            if (*p == '=' && p[1])  { filler = p[1]; p += 2; }
            else if (*p == '0')     { filler = *p++; }

            // Parse width
            while (*p >= '0' && *p <= '9')
                width = width*10 + *p++ - '0';

            // Parse overflow char if present
            if (*p == '>' && p[1])  { overflowchar = p[1]; p += 2; }

            // Parse base if present
            if (*p == '/')
            {
                p++;
                base = 0;
                while (*p >= '0' && *p <= '9')
                    base = base * 10 + *p++ - '0';
                if (*p == 'U')
                {
                    upcase = 1;
                    p++;
                }
            }
            else if (*p == 'x')
            {
                base = 16;
                p++;
            }
            else if (*p == 'X')
            {
                base = 16;
                upcase = 1;
                p++;
            }

            // Parse digit separation if present
            if (*p == ',')
            {
                p++;
                while (*p >= '0' && *p <= '9')
                    dsep = dsep * 10 + *p++ - '0';
                if (dsep == 0)
                    dsep = 3;
                if (*p)
                    dsepchar = *p++;
            }

            if (base < 2 || base > 36 || *p)
                throw std::runtime_error("Invalid parameters for integer formatting");

            int neg = 0;
            if (x < 0)
            {
                neg = 1;
                x = -x;
            }

            // Generate digits (in reverse order)
            std::string result;
            int digits = 0;
            int count = 0;

            do {
                int d = x % base;
                if (d < 10)
                    result += '0' + d;
                else if (upcase)
                    result += 'A' + (d - 10);
                else
                    result += 'a' + (d - 10);
                x /= base;

                // Eventually add digit separator
                if (++digits == dsep && x != 0)
                {
                    result += dsepchar;
                    digits = 0;
                    ++count;
                }
                ++count;
            } while (x != 0);

            // Special case; if right-aligned and filler is '0' then fill
            // now before adding the sign. Also use digit separator during
            // the fill.
            if (align == 1 && filler == '0')
            {
                while (count < width - (neg | plus))
                {
                    result += '0';
                    count++;
                    if (++digits == dsep && count < width - (neg | plus) - 1)
                    {
                        result += dsepchar;
                        count++;
                        digits = 0;
                    }
                }
            }

            // Add sign
            if (neg)
            {
                result += '-'; count++;
            }
            else if (plus)
            {
                result += '+'; count++;
            }

            // Align and check for overflow
            if (width)
            {
                if (count > width)
                {
                    if (overflowchar)
                    {
                        result.resize(width);
                        for (int i=0; i<width; i++)
                            result[i] = overflowchar;
                        return result;
                    }
                }
                else if (count < width)
                {
                    // left/right/center padding
                    while (count < width)
                    {
                        if (align != 1)
                        {
                            result.insert(result.begin(), filler);
                            count += 1;
                        }
                        if (count < width && align != -1)
                        {
                            result += filler;
                            count += 1;
                        }
                    }
                }
            }

            // Finished; flip result
            for (int i=0,j=count-1; i<j; i++,j--)
            {
                int t = result[i];
                result[i] = result[j];
                result[j] = t;
            }

            return result;
        }
    };
}

#endif // FORMAT_H_INCLUDED
