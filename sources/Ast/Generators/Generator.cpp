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

#include "Generator.h"

#include <fstream>

namespace Ast
{

    void Generator::SetTargetProject(const ProjectTree::Ptr& project)
    {
        _projectTree = project;
    }

    void Generator::Generate()
    {
        ForEachOverRegenerateableUnits(
            [this](const ProjectTree::Unit* unit)
            {
                auto source = unit->GetGeneratedDummyHeader();

                const auto tree = unit->GetTree();
                tree->ForEachOverSameType(
                    [this](std::vector<const BaseLexer*>& lexers)
                    {
                        if (Verify(!lexers.empty() && lexers.front()))
                        {
                            const auto* pinnedLexer = lexers.front();

                            auto generators = GetGeneratorUnitFor(*pinnedLexer);
                            if (!Verify(!!generators))
                            {
                                spdlog::error(("Generator wasn't found for lexer: '{}' by the next path: {}"_f
                                                                              << pinnedLexer->GetLexerType() << pinnedLexer->GetFullPath().first ).ToStdStringView());
                            }

                            for (const auto& generator : *generators)
                            {
                                generator->AddLexers(lexers);
                                generator->GenerateSourceToFile();
                            }
                        }
                    },
                    [](const BaseLexer* l)
                    {
                        return l->IsMarked();
                    });

                std::ofstream out(unit->GetGeneratedSiblingFilePath());
                out << source.c_str();
            });
    }

    bool Generator::IsNeedRegenerate() const
    {
        if (!Verify(!!_projectTree, "No project. Use Ast::Generator::SetTargetProject to set a project."))
        {
            return false;
        }

        bool isNeed = false;
        _projectTree->ForEach(
            [&isNeed, this](const ProjectTree::Unit* unit)
            {
                if (!Verify(unit))
                {
                    return true;
                }

                if (_projectTree->IsNeedRegeneration(*unit))
                {
                    isNeed = true;
                    return false;
                }

                return true;
            });

        return isNeed;
    }

    void Generator::ForEachOverRegenerateableUnits(std::function<void(const ProjectTree::Unit*)> callback) const
    {
        if (!Verify(!!_projectTree, "No project. Use Ast::Generator::SetTargetProject to set a project."))
        {
            return;
        }

        _projectTree->ForEach(
            [&callback, this](const ProjectTree::Unit* unit)
            {
                if (unit)
                {
                    if (_projectTree->IsNeedRegeneration(*unit))
                    {
                        std::invoke(callback, unit);
                    }
                }
            });
    }

    void Generator::ForEachOverRegenerateableUnits(std::function<void(ProjectTree::Unit*)> callback)
    {
        if (!Verify(!!_projectTree, "No project. Use Ast::Generator::SetTargetProject to set a project."))
        {
            return;
        }

        _projectTree->ForEach(
            [&callback, this](ProjectTree::Unit* unit)
            {
                if (unit)
                {
                    if (_projectTree->IsNeedRegeneration(*unit))
                    {
                        std::invoke(callback, unit);
                    }
                }
            });
    }

    const Generator::GeneratorContainerT* Generator::GetGeneratorUnitFor(const String& type) const
    {
        const auto found = _generatorUnits.find(type);
        return found != _generatorUnits.end() ? &found->second : nullptr;
    }

    const Generator::GeneratorContainerT* Generator::GetGeneratorUnitFor(const BaseLexer& lexer) const
    {
        return GetGeneratorUnitFor(lexer.GetLexerType());
    }
    const Generator::GeneratorContainerT* Generator::GetGeneratorUnitFor(const BaseLexer::Ptr& lexer) const
    {
        return Verify(!!lexer) ? GetGeneratorUnitFor(lexer->GetLexerType()) : nullptr;
    }

} // namespace Ast
