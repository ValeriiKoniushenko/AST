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

#pragma once

#include "BaseLog.h"
#include "CommonTypes.h"

#include <boost/smart_ptr/intrusive_ptr.hpp>
#include <boost/smart_ptr/intrusive_ref_counter.hpp>
#include <set>

namespace Ast
{
    class BaseLexer;

    /**
     * @brief abstract class which contains the main logic of generation ONLY one unit/lexer
     * @details this class like more advanced template for generation, where you must set:
     * 1. GlobalHead - all needed global dependencies (e.g. import[s], #include[s])
     * 2. LocalHead - all needed local dependencies (e.g. forward declaration, opening of a namespace)
     * 3. Body - your main template where you will construct some new code based on the lexer
     * 4. LocalTail - all needed local tail-moments (e.g. to close some namespace)
     * 5. GlobalTail - the same as above, but for global.
     *
     * @extends One important point: Global Head/Tail will contain only unique values. So, you will need
     * to add some lines to Global Head/Tail only to internal std::set<String> to store it as unique.
     * Also, this class will generate only one piece of code for your Ast::Tree.
     * If you want to change some logic per file generation, you should see into
     * Ast::GeneratorFileComposer
     */
    class BaseGenerator : public boost::intrusive_ref_counter<BaseGenerator>, public BaseLog, public Utils::NotCopyableButMoveable
    {
    public:
        AST_CLASS(BaseGenerator);

    public:
        BaseGenerator() = default;
        ~BaseGenerator() override = default;

        [[nodiscard]] const std::set<String>& getGlobalHead() const { return _globalHead; }
        [[nodiscard]] virtual String generateLocalHead() = 0;
        [[nodiscard]] virtual String generateBody() = 0;
        [[nodiscard]] virtual String generateLocalTail() = 0;
        [[nodiscard]] const std::set<String>& getGlobalTail() const { return _globalTail; }

        [[nodiscard]] virtual std::set<String>& getGlobalHead() { return _globalHead; }
        [[nodiscard]] virtual std::set<String>& getGlobalTail() { return _globalTail; }

        void setLexer(BaseLexer* lexer) { _lexer = lexer; }
        [[nodiscard]] bool hasLexer() const noexcept { return _lexer; }

    protected:
        /**
         * @brief All spaces before & after string will be removed
         */
        void addToGlobalHead(String value);

        /**
         * @brief All spaces before & after string will be removed
         */
        void addToGlobalTail(String value);

        [[nodiscard]] spdlog::logger* getLogger() const final
        {
            static std::shared_ptr<spdlog::logger> logger = spdlog::stdout_color_mt("Generator");
            return logger.get();
        }

    protected:
        BaseLexer* _lexer = nullptr;

    private:
        std::set<String> _globalHead;
        std::set<String> _globalTail;
    };

    template<class T>
    concept IsGenerator = std::derived_from<T, BaseGenerator>;

} // namespace Ast