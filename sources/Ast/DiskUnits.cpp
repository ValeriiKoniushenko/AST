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

    DiskUnit::Ptr DiskUnit::CreateFromPath(const std::filesystem::path& path)
    {
        if (path.empty() || !std::filesystem::exists(path))
        {
            Assert();
            return nullptr;
        }

        auto unit = Ptr(new DiskUnit());

        unit->_path = path;

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

        unit->_lastWriteTime = std::filesystem::last_write_time(path).time_since_epoch().count();
        unit->_permissions = status.permissions();

        return unit;
    }

    bool DiskUnit::isWriteable() const noexcept
    {
        using T = std::filesystem::perms;
        return (T::owner_read & _permissions) == T::owner_read && (T::owner_write & _permissions) == T::owner_write;
    }
} // namespace Ast
