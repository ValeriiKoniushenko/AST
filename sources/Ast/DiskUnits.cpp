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

#include "DiskUnits.h"

namespace Ast
{
    const char __BaseLogHeader_DiskUnit[] = "FileSystem";

    void DiskUnit::_trySetParent(const Ptr& parent)
    {
        if (_parent)
        {
            Assert();
            return;
        }

        _parent = parent;
    }

    void DiskUnit::_forceSetParent(const Ptr& parent)
    {
        if (_parent)
        {
            Assert();
            return;
        }

        _parent = parent;
    }

    void DiskUnit::FillBaseInfo(DiskUnit* unit, const std::filesystem::path& path)
    {
        const auto status = std::filesystem::status(path);

        if (std::filesystem::is_directory(status))
        {
            unit->_type = DiskUnit::Type::Directory;
        }
        else if (std::filesystem::is_regular_file(status))
        {
            unit->_type = DiskUnit::Type::File;
        }
        else if (std::filesystem::is_symlink(status))
        {
            unit->_type = DiskUnit::Type::Symlink;
        }
        else
        {
            logger->warn("Impossible to identify unit type(folder, file, etc) for this path: {}", path.generic_string());
        }

        unit->_path = path;
        unit->_lastWriteTime = std::filesystem::last_write_time(path).time_since_epoch().count();
        unit->_permissions = status.permissions();
    }

    DiskUnit::Ptr DiskUnit::CreateFromPath(const std::filesystem::path& path)
    {
        if (path.empty() || !std::filesystem::exists(path))
        {
            logger->error("Impossible to find a disk unit by the next path: ", path.generic_string());
            Assert();
            return nullptr;
        }

        auto unit = Ptr(new DiskUnit());

        FillBaseInfo(unit.get(), path);

        if (!unit->isValid())
        {
            logger->error("Unit's component is invalid: {}", path.generic_string());
            Assert();
            return nullptr;
        }

        return unit;
    }

    bool DiskUnit::isWriteable() const noexcept
    {
        using T = std::filesystem::perms;
        return (T::owner_read & _permissions) == T::owner_read && (T::owner_write & _permissions) == T::owner_write;
    }

    bool DiskUnit::isValid() const
    {
        return !_path.empty() && _type != DiskUnit::Type::None && _lastWriteTime != 0;
    }

    void DiskUnit::Clear()
    {
        _parent = nullptr;
        _path.clear();
        _lastWriteTime = 0;
        _type = DiskUnit::Type::None;
    }

    DirectoryUnit::Ptr DirectoryUnit::CreateFromPath(const std::filesystem::path& path)
    {
        if (path.empty() || !std::filesystem::exists(path))
        {
            logger->error("Impossible to find a disk unit by the next path: {}", path.generic_string());
            Assert();
            return nullptr;
        }

        auto unit = Ptr(new DirectoryUnit());

        FillBaseInfo(unit.get(), path);
        if (unit->_type != DiskUnit::Type::Directory)
        {
            logger->error("Attempt to read a disk unit as a directory is failed: {}", path.generic_string());
            Assert();
            return nullptr;
        }

        if (!unit->isValid())
        {
            logger->error("Unit's component is invalid: {}", path.generic_string());
            Assert();
            return nullptr;
        }

        return unit;
    }

    void DirectoryUnit::Clear()
    {
        DiskUnit::Clear();

        _childs.clear();
    }

    void DirectoryUnit::addChild(const DiskUnit::Ptr& child)
    {
        _childs.insert(child);
    }

    bool DirectoryUnit::existChild(const DiskUnit::Ptr& child) const
    {
        return _childs.contains(child);
    }

    void DirectoryUnit::removeChild(const DiskUnit::Ptr& child)
    {
        _childs.erase(child);
    }

} // namespace Ast
