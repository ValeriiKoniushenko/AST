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

#include "DiskUnits.h"

#include <utility>

namespace Ast
{

    class FSTree : public boost::intrusive_ref_counter<FSTree>, public Utils::CopyableAndMoveable, public BaseLog
    {
    public:
        AST_CLASS(FSTree)

        struct PrettyInfo
        {
            bool ignore = false; // if 'true' - will skip print of specific line
            char prefix[4]{};
            char suffix[4]{};
        };

    public:
        FSTree() = default;
        ~FSTree() override = default;

        explicit FSTree(DiskUnit::Ptr root)
            : _root{ std::move(root) }
        {
        }

        /**
         * @brief Create a FSTree based on existing path in the OS file system.
         * @param path pass a path to root folder. This folder will be as a start point of all tree.
         * @param pred pass a predicate. If the pred will return true - this file\folder will be added to this class. Default pred - is acceptable
         * every unit.
         * @return generated tree.
         */
        static Ptr CreateTree(const std::filesystem::path& path, std::function<bool(const std::filesystem::path&)> pred = nullptr,
                              bool isIgnoreSymlinks = false);

        void clear();

        [[nodiscard]] bool isEmpty() const { return !_root; }

        /**
         * @brief Can take a functions of next types:
         * bool([const] DiskUnit*) - this function will work until it gets 'false' in return
         * void([const] DiskUnit*) - will iterate without stopping through all a tree
         */
        template<class FuncT>
        void forEach(FuncT&& callback)
        {
            ForEachImpl<false, FuncT>(_root.get(), std::forward<decltype(callback)>(callback));
        }

        /**
         * @brief Can take a functions of next types:
         * bool(const DiskUnit*) - this function will work until it gets 'false' in return
         * void(const DiskUnit*) - will iterate without stopping through all a tree
         */
        template<class FuncT>
        void forEach(FuncT&& callback) const
        {
            ForEachImpl<true, FuncT>(_root.get(), std::forward<decltype(callback)>(callback));
        }

        [[nodiscard]] DiskUnit::Ptr getRoot() { return _root; }

        template<class T>
        [[nodiscard]] T::Ptr getRootAs()
        {
            return boost::dynamic_pointer_cast<T>(_root);
        }
        template<class T>
        [[nodiscard]] T::CPtr getRootAs() const
        {
            return boost::dynamic_pointer_cast<const T>(_root);
        }

        [[nodiscard]] DiskUnit::CPtr getRoot() const { return _root; }

        void prettyPrint(std::function<PrettyInfo(const DiskUnit*)>&& pred = nullptr);
        [[nodiscard]] uint64_t calculateUnitsCount() const;

    protected:
        [[nodiscard]] spdlog::logger* getLogger() const final
        {
            static std::shared_ptr<spdlog::logger> logger = spdlog::stdout_color_mt("Parser");
            return logger.get();
        }

    private:
        DiskUnit::Ptr _root = nullptr;

    private:
        // ================== PIMPLs =======================
        template<bool IsConst, class FuncT>
        static bool ForEachImpl(DiskUnit::AdaptiveRawPtr<IsConst> base, FuncT&& callback)
        {
            if constexpr (std::is_void_v<decltype(callback(base))>)
            {
                std::invoke(callback, base);
            }
            else
            {
                if (!std::invoke(callback, base))
                {
                    return false;
                }
            }

            if (auto* dir = dynamic_cast<DirectoryUnit::AdaptiveRawPtr<IsConst>>(base))
            {
                for (auto& child : dir->getChilds())
                {
                    if (child)
                    {
                        if (!ForEachImpl<IsConst>(child.get(), callback))
                        {
                            return false;
                        }
                    }
                }
            }

            return true;
        }
    };

} // namespace Ast