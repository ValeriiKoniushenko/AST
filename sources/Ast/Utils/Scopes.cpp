//  MIT License
//
//  Copyright (c) 2019-2025 Valerii Koniushenko
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in all
//  copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//  SOFTWARE.

#include "Scopes.h"

namespace Ast::Utils
{

    const String::CharT* FindFirstBracket(const String::CharT* source, String::CharT bracket)
    {
        if (!Verify(source, "Impossible to find the first bracket because was passed the NULL string"))
        {
            return nullptr;
        }

        while (*source != 0)
        {
            source = TryToSkipAnyQuotes(source);

            if (*source == bracket)
            {
                break;
            }

            ++source;
        }

        return *source != 0 ? source : nullptr;
    }

    const String::CharT* FindClosedBracket(const String::CharT* source, String::CharT closedBracket, String::CharT openedBracket)
    {
        if (!Verify(source, "Impossible to find the first bracket because was passed the NULL string"))
        {
            return nullptr;
        }

        // skipping of the opened bracket
        if (*source == openedBracket)
        {
            ++source;
        }

        std::size_t bracketCounter = 1;

        for (; *source != 0 && bracketCounter != 0; ++source)
        {
            auto* old = source;
            source = TryToSkipAnyQuotes(source);

            auto diff = old - source;
            if (diff != 0)
            {
                int i = 1;
            }
            if (*source == openedBracket)
            {
                ++bracketCounter;
            }
            else if (*source == closedBracket)
            {
                --bracketCounter;
            }
        }

        Assert(bracketCounter == 0);

        return bracketCounter == 0 && *source != 0 ? --source : nullptr;
    }

    const String::CharT* FindClosedBracketR(const String::CharT* source, String::CharT closedBracket, String::CharT openedBracket,
                                            const String::CharT* const stopPointBegin)
    {
        if (!Verify(source, "Impossible to find the first bracket because was passed the NULL string"))
        {
            return nullptr;
        }

        // skipping of the opened bracket
        if (source > stopPointBegin && String::Toolset::IsSpace(*source))
        {
            --source;
        }

        std::size_t bracketCounter = 0;
        if (*source == closedBracket)
        {
            ++bracketCounter;
            --source;
        }
        for (; source != stopPointBegin && *source != 0 && bracketCounter != 0; --source)
        {
            if (*source == closedBracket)
            {
                ++bracketCounter;
            }
            else if (*source == openedBracket)
            {
                --bracketCounter;
            }
        }
        ++source;

        return *source != 0 ? source : nullptr;
    }

    bool HasUnclosedBracket(const String::CharT* from, const String::CharT* to, String::CharT closedBracket, String::CharT openedBracket)
    {
        if (!Verify(from && to, "Impossible to find the unclosed bracket because was passed the NULL string[s]"))
        {
            return false;
        }

        if (!Verify(from < to, "Was passed 'from' like an end of string & 'to' like a start of the string"))
        {
            return false;
        }

        if (*from == openedBracket)
        {
            ++from;
        }

        if (*to == openedBracket)
        {
            --to;
        }

        std::size_t bracketCounter = 0;
        for (; from != to; ++from)
        {
            if (*from == openedBracket)
            {
                ++bracketCounter;
            }
            else if (*from == closedBracket)
            {
                --bracketCounter;
            }
        }

        return bracketCounter != 0;
    }

    const String::CharT* TryToSkipAnyQuotes(const String::CharT* src)
    {
        if (!src)
        {
            return src;
        }

        if (*src == '\'')
        {
            return SkipSingleQuotes(src);
        }
        else if (*src == 'R' && src[1] && src[1] == '"' && src[2] && src[2] == '(')
        {
            return SkipRawQuotes(src);
        }
        else if (*src == '"')
        {
            return SkipDoubleQuotes(src);
        }

        return src;
    }

    const String::CharT* SkipDoubleQuotes(const String::CharT* src)
    {
        if (!src || *src != '"')
        {
            return src;
        }

        ++src;

        String::CharT prev = 0;
        int counter = 1;

        while (*src && counter != 0)
        {
            if (prev != '\\' && *src == '"')
            {
                --counter;
            }

            prev = *src;
            ++src;
        }

        Assert(counter == 0);

        return src;
    }

    const String::CharT* SkipSingleQuotes(const String::CharT* src)
    {
        if (!src || *src != '\'')
        {
            return src;
        }

        // Maybe it's not char literal, let's check
        int len = 0;

        ++src;

        String::CharT prev = 0;
        int counter = 1;

        while (*src && counter != 0 && len < 2)
        {
            if (prev != '\\' && *src == '\'')
            {
                --counter;
            }

            prev = *src;
            ++src;
            ++len;
        }

        if (counter != 0)
        {
            return src - len;
        }

        return src;
    }

    const String::CharT* SkipRawQuotes(const String::CharT* src)
    {
        if (!src || !(*src == 'R' && src[1] && src[1] == '"' && src[2] && src[2] == '('))
        {
            return src;
        }

        String::CharT prev = 0;
        int counter = 1;

        while (*src && counter != 0)
        {
            if (prev == ')' && *src == '"')
            {
                --counter;
            }

            prev = *src;
            ++src;
        }

        Assert(counter == 0);

        return src;
    }

} // namespace Ast::Utils