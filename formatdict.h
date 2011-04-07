#if !defined(FORMATDICT_H_INCLUDED)
#define FORMATDICT_H_INCLUDED

//              Copyright Andrea Griffini 2011.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#include <string>
#include <vector>
#include <map>
#include <stdexcept>

#include "formatstring.h"

namespace format
{
    template<typename T>
    struct Formatter
    {
        std::string toString(const T& x, const std::string& options)
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
        virtual std::string toString(const std::string& options) const = 0;
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
        virtual std::string toString(const std::string& options) const
        {
            return Formatter<T>().toString(x, options);
        }
    private:
        // Taboo
        ValueWrapper(const ValueWrapper&);
        ValueWrapper& operator=(const ValueWrapper&);
    };

    typedef std::map<std::string, ValueWrapperBase *> Env;

    class Dict
    {
    private:
        Env env;

    public:
        Dict() {}
        virtual ~Dict()
        {
            for (Env::iterator i=env.begin(), e=env.end(); i!=e; ++i)
                delete i->second;
        }

        template<typename T>
        Dict& operator()(const std::string& name, const T& value)
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
        Dict(const Dict&);
        Dict& operator=(const Dict&);
    };

    inline std::string format(const Format& fs, const Dict& fd)
    {
        std::string result;
        for (int i=0,n=fs.size(); i<n; i++)
        {
            const Field& f = fs[i];
            if (f.name.size())
            {
                // Read field
                result += fd[f.name].toString(f.options);
            }
            else
            {
                // Static field
                result += f.options;
            }
        }
        return result;
    }

    inline std::string operator % (const Format& fs, const Dict& fd)
    {
        return format(fs, fd);
    }

    ///////////////////////////////////////
    //                                   //
    //  Predefined formatting functions  //
    //                                   //
    ///////////////////////////////////////

    // Sequence utility class, can be created
    // from two iterators or from a container
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
    Sequence<T> sequence(const T& i0, const T& i1)
    {
        return Sequence<T>(i0, i1);
    }

    template<typename T>
    Sequence<typename T::const_iterator_type> sequence(const T& x)
    {
        return sequence(x.begin(), x.end());
    }

    //
    // Sequence formatting
    //
    // Options are interpreted as a single string to be
    // used as a separator between elements.
    // First subformat is instead used for elements and
    // the element name used is '*'.
    // If no subformats are present then "{*}" is used
    // for formatting elements.
    //
    template<typename T>
    struct Formatter< Sequence<T> >
    {
        std::string toString(const Sequence<T>& xx, const std::string& sep)
        {
            const Format& fs(subformats.size() ? subformats[0] : Format("{*}"));
            std::string result;
            Dict element;
            for (T i=xx.it0,e=xx.it1; i!=e;)
            {
                element("*", *i);
                result += fs % element;
                ++i;
                if (i != e) result += sep;
            }
            return result;
        }
    };

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
        std::string toString(const int& xx, const std::string& options)
        {
            const char *p = options.c_str();
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


#endif // FORMATDICT_H_INCLUDED
