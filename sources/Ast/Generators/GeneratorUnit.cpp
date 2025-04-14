// Copyright (c) 2024 Valerii Koniushenko
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "GeneratorUnit.h"

#include "Ast/ProjectTree.h"
#include "spdlog/spdlog.h"

#include <bits/ranges_algo.h>
#include <fstream>

namespace Ast
{

    String GeneratorUnit::GenerateSource() const
    {
        String out;
        out += PreGenerate();
        out += OnGenerate();
        out += PostGenerate();
        return out;
    }

    void GeneratorUnit::GenerateSourceToFile(const ProjectTree::Unit* unit) const
    {
        RequireValidLexers();
        if (_lexers.empty() || unit)
        {
            return;
        }

        const auto path = GetGenerationPath(unit);
        if (path.empty())
        {
            Assert();
            spdlog::error(("Impossible to generate a code to the file. Invalid lexer's path was passed into GeneratorUnit of type '{}'"_f << _type).toStdStringView());
            return;
        }

        const auto sources = GenerateSource();
        std::ofstream file(path);
        if (!file.is_open())
        {
            Assert();
            spdlog::error((
                "Impossible to generate a code to the file, because the file can't be created by some reasons. Problem in: GeneratorUnit of type '{}'"_f
                << _type).toStdStringView());
            return;
        }
        file.write(sources.c_str(), sources.size() * sizeof(*sources.c_str()));
    }

    bool GeneratorUnit::AddLexer(const BaseLexer* lexer)
    {
        if (lexer == nullptr)
        {
            return false;
        }

        if (!lexer->IsValid())
        {
            Assert();
            return false;
        }

        if (lexer->GetLexerType() != _type)
        {
            Assert();
            return false;
        }

        _lexers.push_back(lexer);

        return true;
    }

    bool GeneratorUnit::AddLexers(std::vector<const BaseLexer*> lexers)
    {
        if (lexers.empty())
        {
            return false;
        }

        for (const auto& lexer : lexers)
        {
            AddLexer(lexer);
        }

        return true;
    }

    bool GeneratorUnit::RemoveLexer(const BaseLexer* lexer)
    {
        if (lexer == nullptr)
        {
            return false;
        }

        const auto found = std::ranges::find_if(_lexers,
                                                [&lexer](const BaseLexer::CPtr& inputLexer)
                                                {
                                                    return inputLexer ? *inputLexer == *lexer : false;
                                                });

        if (found != _lexers.end())
        {
            _lexers.erase(found);
        }

        return found != _lexers.end();
    }

    void GeneratorUnit::RequireValidLexers() const
    {
#ifdef AST_DEBUG
        if (_lexers.empty())
        {
            return;
        }

        std::filesystem::path validPath;

        auto checkLexer = [&](const BaseLexer::CPtr& inputLexer) -> bool
        {
            if (inputLexer == nullptr)
            {
                Assert();
                spdlog::error(("Nullptr lexer was passed into GeneratorUnit of type '{}'"_f << _type).toStdStringView());
                return false;
            }

            if (!inputLexer->IsValid())
            {
                Assert();
                spdlog::error(("Invalid lexer was passed into GeneratorUnit of type '{}'"_f << _type).toStdStringView());
                return false;
            }

            if (inputLexer->GetLexerType() != _type)
            {
                Assert();
                spdlog::error(("Invalid lexer's type was passed into GeneratorUnit of type '{}'. Lexer name is '{}'; and type is '{}'"_f
                              << _type << inputLexer->GetLexerName() << inputLexer->GetLexerType()).toStdStringView());
                return false;
            }

            if (!inputLexer->GetReader())
            {
                Assert();
                spdlog::error(("Nullptr lexer's Reader was passed into GeneratorUnit of type '{}'. Lexer name is '{}'; and type is '{}'"_f
                              << _type << inputLexer->GetLexerName() << inputLexer->GetLexerType()).toStdStringView());
                return false;
            }

            if (validPath.empty())
            {
                const auto strPath = inputLexer->GetReader()->GetFilePath();
                validPath = std::filesystem::path(strPath.c_str());
                if (strPath.isEmpty() || strPath == "none"_atom || validPath.empty() || !std::filesystem::exists(validPath))
                {
                    validPath.clear();
                    Assert();
                    spdlog::error((
                        "Nullptr or invalid lexer's Reader->filePath was passed into GeneratorUnit of type '{}'. Lexer name is '{}'; and type is '{}'"_f
                        << _type << inputLexer->GetLexerName() << inputLexer->GetLexerType()).toStdStringView());
                    return false;
                }
            }

            if (validPath.empty() || !std::filesystem::exists(validPath))
            {
                validPath.clear();
                Assert();
                spdlog::error(( "Nullptr or invalid lexer's Reader->filePath was passed into GeneratorUnit of type '{}'. Lexer name is '{}'; and type is '{}'"_f
                              << _type << inputLexer->GetLexerName() << inputLexer->GetLexerType()).toStdStringView());
                return false;
            }

            return true;
        };

        if (checkLexer(_lexers.front()))
        {
            return;
        }

        (void)std::ranges::all_of(_lexers, checkLexer);
#endif
    }

} // namespace Ast
