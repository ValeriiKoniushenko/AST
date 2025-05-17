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

#include "GeneratorFileComposer.h"

#include <fstream>

namespace Ast
{

    BaseGenerator* Ast::GeneratorFileComposer::getGenerator(const String& name)
    {
        auto i = _generators.find(name);
        if (i != _generators.end()) [[likely]]
        {
            return i->second.get();
        }

        errorLog("Generator for this lexer '{}' not found."_f << name);
        return nullptr;
    }

    BaseGenerator* GeneratorFileComposer::getGenerator(const BaseLexer* lexer)
    {
        return getGenerator(lexer->GetLexerType());
    }

    void GeneratorFileComposer::generate(const std::vector<BaseLexer*>& lexers, const std::filesystem::path& path)
    {
        std::set<String> globalHead;
        std::set<String> globalTail;

        String body(1024);

        auto getBlanks = [](int count)
        {
            String out;
            for (int i = 0; i < count; ++i)
            {
                out += ITextSourceReader::Code::Endl();
            }
            return out;
        };
        for (auto* lexer : lexers)
        {
            auto* generator = getGenerator(lexer);
            if (!generator)
            {
                continue;
            }

            generator->setLexer(lexer);

            globalHead.merge(generator->getGlobalHead());
            globalTail.merge(generator->getGlobalTail());

            if (!body.isEmpty())
            {
                body += getBlanks(_styleRules.blanksBetweenLexers);
            }
            body += generator->generateLocalHead();
            body += getBlanks(_styleRules.blanksAfterLocalHead);
            body += generator->generateBody();
            body += getBlanks(_styleRules.blanksBeforeLocalTail);
            body += generator->generateLocalTail();
        }

        String fullHeader(1024);
        fullHeader += _fileHeader;
        fullHeader.push_back(ITextSourceReader::Code::Endl());
        for (auto& str : globalHead)
        {
            fullHeader.push_back(str);
            fullHeader.push_back(ITextSourceReader::Code::Endl());
        }
        fullHeader += getBlanks(_styleRules.blanksAfterGlobalHead);

        String fullTail;
        fullTail += getBlanks(_styleRules.blanksBeforeGlobalTail);
        for (auto& str : globalTail)
        {
            fullTail.push_back(str);
            fullTail.push_back(ITextSourceReader::Code::Endl());
        }

        std::ofstream out(path);
        if (!out.is_open())
        {
            criticalLog("Can't put generated content to file. Impossible to open or create the next file for write: {}"_f << path.generic_string());
            return;
        }
        out.write(fullHeader.c_str(), fullHeader.byteSize());
        out.write(body.c_str(), body.byteSize());
        out.write(fullTail.c_str(), fullTail.byteSize());
    }

} // namespace Ast
