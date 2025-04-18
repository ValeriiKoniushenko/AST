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

#pragma once

#include "Core/Enum.h"
#include "Readers/ContentStream.h"
#include "Tree.h"
#include "Utils/CopyableAndMoveableBehaviour.h"

#include <set>
#include <unordered_set>

namespace Ast
{
    class DiskUnit : public Utils::NotCopyableButMoveable, public boost::intrusive_ref_counter<DiskUnit>
    {
    public:
        AST_CLASS(DiskUnit)

        struct Hash
        {
            [[nodiscard]] bool operator()(const DiskUnit::Ptr& unit) const { return std::hash<std::filesystem::path>{}(unit->_name); }
        };

        enum class Type
        {
            None,
            File,
            Folder,
            Symlink
        };

    public:
        DiskUnit() = default;
        ~DiskUnit() override = default;

        static Ptr CreateFromPath(const std::filesystem::path& path);

        [[nodiscard]] Type getType() const noexcept { return _type; }
        void _setType(Type type) noexcept { _type = type; }

        [[nodiscard]] const std::filesystem::path& getName() const noexcept { return _name; }
        void _setName(const std::filesystem::path& name) { _name = name; }

        [[nodiscard]] uint64_t getLastWriteTime() const noexcept { return _lastWriteTime; }
        void _setType(uint64_t time) noexcept { _lastWriteTime = time; }

        void _trySetParent(const Ptr& parent);
        void _forceSetParent(const Ptr& parent);
        [[nodiscard]] CPtr getParent() const { return _parent; }
        [[nodiscard]] Ptr getParent() { return _parent; }
        [[nodiscard]] bool hasParent() const noexcept { return _parent != nullptr; }

    protected:
        uint64_t _lastWriteTime = 0;
        std::filesystem::path _name;

        Type _type = Type::None;
        Ptr _parent = nullptr;
        std::unordered_set<Ptr, Hash> _childs;
    };
} // namespace Ast
