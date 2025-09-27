//  MIT License
//
//  Copyright (c) 2018-2025 Valerii Koniushenko
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

#include "Ast/FileSystem/DiskUnits.h"

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

        // errorLog("Generator for this lexer '{}' not found."_f << name);
        return nullptr;
    }

    BaseGenerator* GeneratorFileComposer::getGenerator(const BaseLexer* lexer)
    {
        return getGenerator(lexer->GetLexerType());
    }

    bool GeneratorFileComposer::generate(const std::vector<BaseLexer*>& lexers, const std::filesystem::path& targetPath, const DiskUnit* originFile,
                                         const std::filesystem::path& projectPath, DirectoryUnit* targetDir)
    {
        if (!targetDir) [[unlikely]]
        {
            ASSERT(false);
            return false;
        }

        infoLog("Generation of file: " + originFile->getPath().lexically_relative(projectPath).generic_string());

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

            auto tmpHead = generator->getGlobalHead();
            auto tmpTail = generator->getGlobalTail();
            globalHead.merge(tmpHead);
            globalTail.merge(tmpTail);

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

        if (body.isEmpty())
        {
            return false;
        }

        if (!targetDir->createOnDiskIfNotExists())
        {
            return false;
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

        const String headerFileDescription = getHeaderFileDescription(originFile, projectPath);
        std::ofstream out(targetPath);
        if (!out.is_open())
        {
            criticalLog("Can't put generated content to file. Impossible to open or create the next file for write: {}"_f
                        << targetPath.generic_string());
            return false;
        }
        out.write(headerFileDescription.c_str(), headerFileDescription.byteSize());
        out.write(fullHeader.c_str(), fullHeader.byteSize());
        out.write(body.c_str(), body.byteSize());
        out.write(fullTail.c_str(), fullTail.byteSize());

        return true;
    }

    String GeneratorFileComposer::getHeaderFileDescription(const DiskUnit* originFile, const std::filesystem::path& projectPath) const
    {
        String body;
        body += "// " + String::MakeFrom(originFile->getLastWriteTime());
        body += ITextSourceReader::Code::Endl();
        body += "// Line above - needed generated data. Don't touch it!";
        body += ITextSourceReader::Code::Endl();
        body += "//";
        body += ITextSourceReader::Code::Endl();
        body += "// This file was autogenerated. Don't modify it manually!";
        body += ITextSourceReader::Code::Endl();
        body += "// Your extra data will be overwritten on the next generation.";
        body += ITextSourceReader::Code::Endl();
        body += "//";
        body += ITextSourceReader::Code::Endl();
        body += "// Original file: " + originFile->getPath().lexically_relative(projectPath).generic_string();
        body += ITextSourceReader::Code::Endl();
        body += ITextSourceReader::Code::Endl();

        return body;
    }

    uint64_t GeneratorFileComposer::getModifTimeOfOriginalFile(const std::filesystem::path& path)
    {
        uint64_t time = 0;
        char line[64]{};

        {
            std::ifstream file(path);
            if (!file.is_open())
            {
                return 0;
            }
            file.getline(line, sizeof(line));
        }

        for (std::size_t i = 0; i < sizeof(line) && line[i]; ++i)
        {
            if (std::isdigit(line[i]))
            {
                char* end = nullptr;
                return strtoull(line + i, &end, 10);
            }
        }

        return 0;
    }

} // namespace Ast
