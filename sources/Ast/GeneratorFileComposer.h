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

#pragma once

#include "BaseGenerator.h"
#include "Lexers/BaseLexer.h"

#include <unordered_map>

namespace Ast
{
    class DiskUnit;

    class GeneratorFileComposer : public BaseLog, public Utils::NotCopyableButMoveable, public boost::intrusive_ref_counter<GeneratorFileComposer>
    {
    public:
        AST_CLASS(GeneratorFileComposer)

        struct StyleRules
        {
            uint8_t blanksBetweenLexers = 6;

            uint8_t blanksAfterGlobalHead = 2;
            uint8_t blanksBeforeGlobalTail = 1;

            uint8_t blanksAfterLocalHead = 2;
            uint8_t blanksBeforeLocalTail = 2;
        };

    public:
        [[nodiscard]] static Ptr Create() { return new GeneratorFileComposer(); }
        ~GeneratorFileComposer() override = default;

        template<IsLexer Lexer, IsGenerator Generator>
        void addGenerator()
        {
            _generators.emplace(Lexer::typeName, new Generator);
        }

        template<IsLexer Lexer>
        [[nodiscard]] BaseGenerator* getGenerator()
        {
            return getGenerator(Lexer::typeName);
        }

        [[nodiscard]] auto getGeneratorsCount() const noexcept { return _generators.size(); }

        [[nodiscard]] BaseGenerator* getGenerator(const String& name);
        [[nodiscard]] BaseGenerator* getGenerator(const BaseLexer* lexer);

        void generate(const std::vector<BaseLexer*>& lexers, const std::filesystem::path& path, const DiskUnit* originFile,
                      const std::filesystem::path& projectPath);

        void setFileHeader(const String& header) { _fileHeader = header; }
        void resetFileHeader() { _fileHeader.clear(); }
        [[nodiscard]] const String& resetFileHeader() const { return _fileHeader; }

        void setStyleRules(const StyleRules& rules) noexcept { _styleRules = rules; }
        [[nodiscard]] const StyleRules& getStyleRules() const noexcept { return _styleRules; }

        [[nodiscard]] uint64_t getModifTimeOfOriginalFile(const std::filesystem::path& path);

    protected:
        GeneratorFileComposer() = default;

        [[nodiscard]] String getHeaderFileDescription(const DiskUnit* originFile, const std::filesystem::path& projectPath) const;

        [[nodiscard]] spdlog::logger* getLogger() const final
        {
            static std::shared_ptr<spdlog::logger> logger = spdlog::stdout_color_mt("GeneratorFileComposer");
            return logger.get();
        }

        StyleRules _styleRules;

    private:
        std::unordered_map<String, BaseGenerator::Ptr> _generators;
        String _fileHeader;
    };

} // namespace Ast
