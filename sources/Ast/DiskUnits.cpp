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

#include "DiskUnits.h"

namespace Ast
{
    const char __BaseLogHeader_DiskUnit[] = "FileSystem";

    void DiskUnit::trySetParent(const Ptr& parent)
    {
        if (_parent)
        {
            Assert();
            return;
        }

        _parent = parent;
    }

    void DiskUnit::forceSetParent(const Ptr& parent)
    {
        if (_parent)
        {
            Assert();
            return;
        }

        _parent = parent;
    }

    bool DiskUnit::isSubPath(const DiskUnit* unit)
    {
        const auto&& base = getPath();
        const auto&& path = unit->getPath();

        return std::mismatch(path.begin(), path.end(), base.begin(), base.end()).second == base.end();
    }

    std::filesystem::path DiskUnit::getRelativePathFrom(const DiskUnit* unit) const
    {
        return std::filesystem::relative(unit->getPath(), getPath());
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

        unit->_name = path.generic_string();
        unit->_lastWriteTime = std::filesystem::last_write_time(path).time_since_epoch().count();
        unit->_permissions = status.permissions();
    }

    bool DiskUnit::NameValidator::IsValid(const String& name)
    {
        return name.regexMatch(regex);
    }

    String DiskUnit::NameValidator::GetHint()
    {
        return "Available names for a disk unit should be matched with this regular expression: " + String(regex);
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

    std::filesystem::path DiskUnit::getPath() const
    {
        std::vector<std::string> units;
        auto* i = this;

        while (i)
        {
            if (i->_name.isEmpty())
            {
                logger->error("Can't return a path to a disk unit. Because name of the unit is not defined.");
                return {};
            }
            units.emplace_back(i->_name.toStdString());
            i = i->_parent.get();
        }

        std::reverse(units.begin(), units.end());
        std::filesystem::path ret;
        for (const auto& u : units)
        {
            ret /= u;
        }

        return ret;
    }

    bool DiskUnit::isWriteable() const noexcept
    {
        using T = std::filesystem::perms;
        return (T::owner_read & _permissions) == T::owner_read && (T::owner_write & _permissions) == T::owner_write;
    }

    bool DiskUnit::isValid() const
    {
        return !_name.isEmpty() && _type != DiskUnit::Type::None;
    }

    bool DiskUnit::setName(const String& name)
    {
        if (NameValidator::IsValid(name))
        {
            setNameUnsafe(name);
            return true;
        }

        logger->error((("Invalid name for file '{}'. "_f << name.c_str()) + NameValidator::GetHint()).toStdStringView());

        return false;
    }

    void DiskUnit::clear()
    {
        _parent = nullptr;
        _name.clear();
        _lastWriteTime = 0;
        _type = DiskUnit::Type::None;
        _permissions = std::filesystem::perms::none;
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

    void DirectoryUnit::clear()
    {
        DiskUnit::clear();

        _childs.clear();
    }

    void DirectoryUnit::addChild(const DiskUnit::Ptr& child, bool isIgnoreDiskCheck /* = false*/)
    {
        if (!isIgnoreDiskCheck && !child->isExistOnDisk())
        {
            Assert();
            logger->error("Impossible to add child: {} - which not exists on the disk.", child->getPath().string());
            return;
        }

        if (child->hasAbsolutePath())
        {
            if (isSubPath(child.get()))
            {
                const auto p = getRelativePathFrom(child.get());
                if (p.empty())
                {
                    Assert();
                    logger->error(("Invalid path of the child: " + child->getName()).toStdStringView());
                    return;
                }

                const auto newName = String(p.generic_string());
                if (!child->setName(newName))
                {
                    logger->error(("Can't set name for the child: " + newName).toStdStringView());
                    return;
                }
            }
        }

        _childs.insert(child);
        child->forceSetParent(this);
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
