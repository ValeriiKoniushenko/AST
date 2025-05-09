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

#include "../BaseLog.h"
#include "../Readers/ContentStream.h"
#include "../Tree.h"
#include "Core/Enum.h"
#include "Utils/CopyableAndMoveableBehaviour.h"

#include <set>
#include <unordered_set>

namespace Ast
{
    extern const char __BaseLogHeader_DiskUnit[];

    class DiskUnit : public BaseLog<__BaseLogHeader_DiskUnit>, public Utils::NotCopyableButMoveable, public boost::intrusive_ref_counter<DiskUnit>
    {
    public:
        AST_CLASS(DiskUnit)

        struct NameValidator
        {
            inline static const char* regex = "^[0-9A-Za-z_\\-\\. ]+$";
            static bool IsValid(const String& name);
            static String GetHint();
        };

        struct Hash
        {
            [[nodiscard]] bool operator()(const DiskUnit::Ptr& unit) const { return unit->_name.makeHash(); }
        };

        enum class Type : char
        {
            None,
            File,
            Directory,
            Symlink
        };

    public:
        explicit DiskUnit(Type type, const String& name = nullptr)
            : _type{ type }
        {
            if (!name.isEmpty())
            {
                (void)setName(name);
            }
        };
        ~DiskUnit() override = default;

        static Ptr CreateFromPath(const std::filesystem::path& path);

        [[nodiscard]] Type getType() const noexcept { return _type; }

        [[nodiscard]] bool isExistOnDisk() const { return std::filesystem::exists(getPath()); }

        [[nodiscard]] std::filesystem::path getPath() const;

        [[nodiscard]] uint64_t getLastWriteTime() const noexcept { return _lastWriteTime; }

        [[nodiscard]] std::filesystem::perms getPermissions() const noexcept { return _permissions; }
        [[nodiscard]] bool isWriteable() const noexcept;

        [[nodiscard]] CPtr getParent() const { return _parent; }
        [[nodiscard]] Ptr getParent() { return _parent; }
        [[nodiscard]] bool hasParent() const noexcept { return _parent != nullptr; }

        [[nodiscard]] bool isValid() const;

        [[nodiscard]] bool setName(const String& name);
        void setNameUnsafe(const String& name) { _name = name; }
        [[nodiscard]] String getName() const { return _name; }

        virtual void clear();

        void setType(Type type) noexcept { _type = type; }
        void setLastWriteTime(uint64_t time) noexcept { _lastWriteTime = time; }
        void trySetParent(const Ptr& parent);
        void forceSetParent(const Ptr& parent);

        [[nodiscard]] bool hasAbsolutePath() const { return std::filesystem::path(_name.toStdString()).is_absolute(); }
        [[nodiscard]] bool hasRelativePath() const { return std::filesystem::path(_name.toStdString()).is_relative(); }

        [[nodiscard]] bool isSubPath(const DiskUnit* unit);
        [[nodiscard]] std::filesystem::path getRelativePathFrom(const DiskUnit* unit) const;

        [[nodiscard]] int distanceToRoot() const;

    protected:
        static void FillBaseInfo(DiskUnit* unit, const std::filesystem::path& path);

    protected:
        uint64_t _lastWriteTime = 0;
        String _name;
        std::filesystem::perms _permissions = std::filesystem::perms::none;
        Type _type = Type::None;

        Ptr _parent = nullptr;
    };

    class DirectoryUnit final : public DiskUnit
    {
    public:
        AST_CLASS(DirectoryUnit)

        explicit DirectoryUnit(const String& name = nullptr)
            : DiskUnit(Type::Directory, name)
        {
            _type = Type::Directory;
        }

        ~DirectoryUnit() override = default;

        static Ptr Create() { return { new DirectoryUnit() }; }
        static Ptr Create(const String& name) { return { new DirectoryUnit(name) }; }
        static Ptr CreateFromPath(const std::filesystem::path& path);

        void clear() override;

        bool addChild(const DiskUnit::Ptr& child, bool isIgnoreDiskCheck = true);

        template<class T>
        [[nodiscard]] boost::intrusive_ptr<T> addChildAndGetBack(const boost::intrusive_ptr<T>& child, bool isIgnoreDiskCheck = true)
        {
            if (addChild(child))
            {
                return child;
            }

            return nullptr;
        }
        [[nodiscard]] bool existChild(const DiskUnit::Ptr& child) const;
        void removeChild(const DiskUnit::Ptr& child);

        [[nodiscard]] std::unordered_set<DiskUnit::Ptr>& getChilds() { return _childs; }
        [[nodiscard]] const std::unordered_set<DiskUnit::Ptr>& getChilds() const { return _childs; }

        /**
         * @brief Can take a functions of next types:
         * bool(const std::filesystem::directory_entry&) - this function will work until it gets 'false' in return
         * void(const std::filesystem::directory_entry&) - will iterate without stopping through all a tree
         */
        template<class FuncT>
        void iterateOverPhysicalContent(FuncT&& callback) const
        {
            const auto thisPath = getPath();
            for (const auto& dirEntry : std::filesystem::directory_iterator(thisPath))
            {
                if constexpr (std::is_void_v<decltype(callback(dirEntry))>)
                {
                    std::invoke(std::forward<decltype(callback)>(callback), dirEntry);
                }
                else
                {
                    if (!std::invoke(std::forward<decltype(callback)>(callback), dirEntry))
                    {
                        return;
                    }
                }
            }
        }

    protected:
        std::unordered_set<DiskUnit::Ptr> _childs;

    protected:
    };

    class FileUnit final : public DiskUnit
    {
    public:
        struct DataContainer : public boost::intrusive_ref_counter<DataContainer>, public Utils::CopyableAndMoveable
        {
            AST_CLASS(DataContainer)
        };

    public:
        AST_CLASS(FileUnit)

        explicit FileUnit(const String& name = nullptr)
            : DiskUnit(Type::File, name)
        {
        }

        ~FileUnit() override = default;

        static Ptr Create() { return { new FileUnit() }; }
        static Ptr Create(const String& name) { return { new FileUnit(name) }; }
        static Ptr CreateFromPath(const std::filesystem::path& path);

        void clear() override;

        [[nodiscard]] String getContent() const;
        void putContent(const String& content);

        [[nodiscard]] DataContainer::Ptr& getData() { return _data; }
        [[nodiscard]] const DataContainer::Ptr& getData() const { return _data; }

    protected:
        DataContainer::Ptr _data{};
    };

} // namespace Ast
