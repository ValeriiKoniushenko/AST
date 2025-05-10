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

#include "ProjectTree.h"

namespace Ast
{
    const char __BaseLogHeader_ProjectTree[] = "ProjectTree";

    bool ProjectTree::addIgnorePath(const String& path)
    {
        if (!DiskUnit::PathValidator::IsValid(path))
        {
            logger->error(("Impossible to ignore this path: " + path + " " + DiskUnit::PathValidator::GetHint()).toStdStringView());
            return false;
        }

        _ignoredPaths.emplace_back(path);

        return true;
    }

    void ProjectTree::scanProject()
    {
        if (!canBeScanned())
        {
            return;
        }

        scanFilesystem();
    }

    void ProjectTree::scanFilesystem()
    {
        const auto start = std::chrono::system_clock::now();
        _fstree = FSTree::CreateTree(_projectPath,
                                     [this](const std::filesystem::path& path)
                                     {
                                         return !isIgnoredPath(path);
            },
            _ignoreSymlinks);

        const auto end = std::chrono::system_clock::now();
        uint64_t milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        logger->info(("Was passed {}ms for filesystem scan"_f << milliseconds).toStdStringView());

#ifdef AST_DEBUG
        logger->info(("Was read {} units"_f << _fstree->calculateUnitsCount()).toStdStringView());
#endif
    }

    bool ProjectTree::canBeScanned() const
    {
        return !_projectPath.empty();
    }

    bool ProjectTree::isIgnoredPath(const std::filesystem::path& path) const
    {
        if (_ignoredPaths.empty())
        {
            return false;
        }

        auto main = String((_projectPath / path).lexically_normal().generic_string());
        main.replaceAll("\\", "/");

        for (auto i : _ignoredPaths)
        {
            i = ((_projectPath / std::filesystem::path(i.toStdStringView())).lexically_normal()).generic_string();

            // Escaping of regex chars to avoid mixing of them
            // BUT we can pass * for match all as '.*'
            // So, from this string: /hello/.world/.how+are/you*.json
            // We must get this: /hello/\.world/\.how\+are/you.*\.json
            //               replace '*' to '.*' to match all ^^
            i.replaceAll("\\", "/");
            i.replaceAll("+", "\\+");
            i.replaceAll("!", "\\!");
            i.replaceAll("-", "\\-");
            i.replaceAll("?", "\\?");
            i.replaceAll("[", "\\[");
            i.replaceAll("]", "\\]");
            i.replaceAll(".", "\\.");

            i.replaceAll("*", ".*");

            i.push_front('^');
            i.push_back("/?$");

            if (main.regexMatch(i))
            {
                logger->info("Due to ignore config, the next path was ignored: " + path.generic_string());
                return true;
            }
        }
        // escaping all chars

        return false;
    }

} // namespace Ast

#ifdef false

    #include "Utils/Functions.h"
    #include "deprProjectTree.h"

namespace
{
    const auto separator = []
    {
        Ast::String temp;
        temp += static_cast<Ast::String::CharT>(std::filesystem::path::preferred_separator);
        return temp;
    }();
} // namespace

namespace Ast::Deprecated
{

    String deprProjectTree::Unit::GetGeneratedDummyHeader() const
    {
        if (!IsFile())
        {
            return {};
        }

        String out;
        out += generatedFileHeader_Head;
        out += String::MakeFrom(std::filesystem::last_write_time(_path).time_since_epoch().count());
        out += Code::Endl();
        out += generatedFileHeader_Body;
        out += String::MakeFrom(_path);
        out += Code::Endl();
        out += Code::Endl();
        return out;
    }

    String deprProjectTree::Unit::GetTextSource()
    {
        return _contentStream->Data();
    }

    bool deprProjectTree::Unit::IsExistsOnDisk() const
    {
        std::error_code ec;
        return std::filesystem::exists(_path, ec);
    }

    bool deprProjectTree::Unit::HasChild(const Unit& unit) const
    {
        auto found = std::find_if(_childs.cbegin(), _childs.cend(),
                                  [&unit](const Ptr& a)
                                  {
                                      return *a.get() == unit;
                                  });

        return found != _childs.cend();
    }

    const deprProjectTree::Unit::Ptr deprProjectTree::Unit::FindChild(const Unit& unit) const
    {
        auto found = std::find_if(_childs.cbegin(), _childs.cend(),
                                  [&unit](const Ptr& a)
                                  {
                                      return *a.get() == unit;
                                  });

        if (found != _childs.cend())
        {
            return *found;
        }

        return nullptr;
    }

    deprProjectTree::Unit deprProjectTree::Unit::CreateFromPath(const std::filesystem::path& path, deprProjectTree* projectTree)
    {
        Unit unit(projectTree);
        unit._path = path;
        unit._permission = std::filesystem::status(path).permissions();
        unit._lastWriteTime = std::filesystem::last_write_time(path).time_since_epoch().count();

        if (!Verify(unit.IsExistsOnDisk()))
        {
            return deprProjectTree::Unit{ nullptr };
        }

        if (std::filesystem::is_directory(path))
        {
            unit._type = Type::Folder;
        }
        else if (std::filesystem::is_symlink(path))
        {
            unit._type = Type::Link;
        }
        else
        {
            unit._type = Type::File;
            unit._contentStream = FileContentStream::Ptr(new FileContentStream());
            if (Verify(!!unit._contentStream, "Impossible to allocate an object"))
            {
                Assert(unit._contentStream->ReadFromFile(path), "Can't read a file: "_dyn + String::MakeFrom(path));
            }

            if (!unit.IsGeneratedFile())
            {
                unit._tree = Tree<FileLexer>::Ptr(new Tree<FileLexer>(unit._contentStream));
                Assert(!!unit._tree, "Impossible to allocate an object");

                if (unit.CheckByPathIfWasGenerated())
                {
                    if (unit._lastWriteTime != unit.ExtrudeGenerationTime())
                    {
                        unit._isDirty = true;
                    }
                }
            }
        }

        return unit;
    }

    deprProjectTree::Unit::Ptr deprProjectTree::Unit::CreatePtrFromPath(const std::filesystem::path& path, deprProjectTree* projectTree)
    {
        return Ptr(new Unit(CreateFromPath(path, projectTree)));
    }

    deprProjectTree::Unit::Ptr deprProjectTree::Unit::GetUnitByPath(const std::filesystem::path& path)
    {
        auto relative = std::filesystem::relative(path, _path);
        if (relative.empty())
        {
            return nullptr;
        }

        const auto pathVector = String(relative.generic_string().c_str()).split(separator);
        if (pathVector.empty())
        {
            return nullptr;
        }

        Unit* temp = this;

        for (const auto& name : pathVector)
        {
            bool isFound = false;
            for (const auto& child : temp->_childs)
            {
                if (child->GetPath().string() == (temp->_path / name.toStdStringView()).string())
                {
                    temp = child.get();
                    isFound = true;
                    break;
                }
            }
            if (!isFound)
            {
                temp = nullptr;
                break;
            }
        }

        return temp;
    }

    bool deprProjectTree::Unit::IsExistUnitByPath(const std::filesystem::path& path)
    {
        return !!GetUnitByPath(path);
    }

    uint64_t deprProjectTree::Unit::GetLastModificationTime() const
    {
        if (_path.empty())
        {
            return 0;
        }

        if (IsGeneratedFile())
        {
            const auto time = ExtrudeGenerationTime();
            if (time != 0)
            {
                return time;
            }
        }

        return std::filesystem::last_write_time(_path).time_since_epoch().count();
    }

    deprProjectTree::Unit* deprProjectTree::Unit::LinkSubFolder(const String& name)
    {
        if (!Verify(std::filesystem::exists(_path), "Invalid unit"))
        {
            return nullptr;
        }

        Assert(_projectTree);

        auto unit = Unit::Create(_projectTree);
        unit->_path = _path / name.toStdStringView();
        unit->_type = Type::Folder;
        unit->_parent = this;
        unit->_permission = _permission;
        if (unit->IsExistsOnDisk())
        {
            if (!Verify(!HasChild(*unit), "Such subfolder already exists"))
            {
                return nullptr;
            }

            return RawAddToChilds(std::move(unit));
        }

        return nullptr;
    }

    deprProjectTree::Unit* deprProjectTree::Unit::LinkSubFile(const String& name)
    {
        if (!Verify(std::filesystem::exists(_path), "Invalid unit"))
        {
            return nullptr;
        }

        auto unit = CreatePtrFromPath(_path / name.toStdStringView(), _projectTree);

        if (!Verify(!!unit))
        {
            return nullptr;
        }

        unit->_parent = this;
        if (!Verify(!HasChild(*unit), "Such file already exists"))
        {
            return nullptr;
        }

        return RawAddToChilds(std::move(unit));
    }

    bool deprProjectTree::Unit::HasGeneratedSiblingFile() const
    {
        if (!IsFile())
        {
            return false;
        }

        if (_path.empty())
        {
            return false;
        }

        const auto siblingFile = GetGeneratedSiblingFilePath();
        return siblingFile.empty() ? false : std::filesystem::exists(siblingFile);
    }

    std::filesystem::path deprProjectTree::Unit::GetGeneratedSiblingFilePath() const
    {
        if (_path.empty() || !_path.has_extension())
        {
            return {};
        }

        if (!_projectTree)
        {
            Assert();
            return {};
        }

        auto path = _path;
        const auto prefExt = _projectTree->GetPreferableExtensionForGeneration();
        std::string ext = !prefExt ? _path.extension().string() : prefExt.toStdString();
        path.replace_extension(generatedSuffixDecl + ext);
        return path;
    }

    deprProjectTree::Unit* deprProjectTree::Unit::RawAddToChilds(Ptr&& unit)
    {
        auto it = _childs.emplace(std::move(unit));
        Assert(it.second, "Undefined error. Impossible to add new subfolder to the childs");

        return it.second ? it.first->get() : nullptr;
    }

    bool deprProjectTree::Unit::CheckByPathIfWasGenerated() const
    {
        if (IsFile())
        {
            if (Verify(!_path.empty(), "Path is empty"))
            {
                auto tmp = _path;
                if (!tmp.has_extension())
                {
                    return false;
                }
                tmp.replace_extension("");
                if (!tmp.has_extension())
                {
                    return false;
                }
                return tmp.extension().string() == generatedSuffixDecl;
            }
        }
        return false;
    }

    uint64_t deprProjectTree::Unit::ExtrudeGenerationTime() const
    {
        if (!Verify(!!_contentStream))
        {
            return {};
        }

        if (!Verify(!_contentStream->Data().isEmpty()))
        {
            return {};
        }

        String timeString;

        _contentStream->Data().forEachByLine(
            [&timeString](String string)
            {
                const auto* found = string.find(generatedFileHeader_Head);
                if (!found)
                {
                    return true;
                }

                if (!(found = string.find(":")))
                {
                    return true;
                }

                String buff;
                string.subStr(found - string.c_str());
                string.regexIterate("[0-9]",
                                    [&buff, &string](const Core::BaseRegexMatch::MatchedData m)
                                    {
                                        buff += m.convertBasedOn(string);
                                        return true;
                                    });

                if (buff.isEmpty())
                {
                    return true;
                }

                timeString = std::move(buff);

                return false;
            });

        if (timeString.isEmpty())
        {
            Assert("Time in the generated file wasn't found. Maybe you cleared all comments before time was checked.");
            return std::filesystem::file_time_type().time_since_epoch().count();
        }

        const auto ret = timeString.convertTo<uint64_t>();
        Assert(ret != 0);
        return ret;
    }

    void deprProjectTree::Clear()
    {
        _fileExtensions.clear();
        _root = nullptr;
        _excluded.clear();
        _config = {};
    }
    bool deprProjectTree::IsValid() const
    {
        if (_root != nullptr)
        {
            bool foundAtLeastOneFile = false;
            ForEach(
                [&foundAtLeastOneFile](const auto*)
                {
                    foundAtLeastOneFile = true;
                    return false;
                });
            return foundAtLeastOneFile;
        }
        return false;
    }
    void deprProjectTree::SetFileExtensions(std::vector<String> extensions)
    {
        for (auto& extension : extensions)
        {
            extension.trim('*');
            extension.trim('.');
            if (Verify(!extension.isEmpty(), "Was passed invalid extension"))
            {
                extension.push_front('.');
                _fileExtensions.emplace(std::move(extension));
            }
        }
    }

    void deprProjectTree::ExcludeFromProject(std::filesystem::path path)
    {
        if (path.empty() || path.string() == ".")
        {
            spdlog::warn(
                ("Was passed a path to exclude it. But the path is empty or invalid. The passed path: {}"_f << path.string()).toStdStringView());
            return;
        }
        _excluded.emplace(std::move(path));
    }

    bool deprProjectTree::IsExcludedPath(std::filesystem::path path) const
    {
        if (_excluded.contains(path))
        {
            return true;
        }

        if (path.is_relative())
        {
            if (Verify(!!_root))
            {
                path = std::filesystem::canonical(path);
                if (path.empty())
                {
                    spdlog::error("Was trying to convert a path to absolute, but met some problem.");
                    return false;
                }
            }
        }

        for (auto p : _excluded)
        {
            if (p.is_relative())
            {
                if (Verify(!!_root))
                {
                    p = _root->GetPath() / p;
                    if (std::filesystem::exists(p) && !std::filesystem::is_symlink(p))
                    {
                        p = std::filesystem::canonical(p);
                    }

                    if (p.empty())
                    {
                        spdlog::error("Was trying to convert a path to absolute, but met some problem.");
                        return false;
                    }
                }
            }

            if (path.string().contains(p.string()))
            {
                return true;
            }
        }

        return false;
    }

    const std::unordered_set<std::filesystem::path>& deprProjectTree::GetExcludedPaths() const noexcept
    {
        return _excluded;
    }

    void deprProjectTree::SetTargetProject(const std::filesystem::path& path)
    {
        if (std::filesystem::exists(path))
        {
            _root = Unit::CreatePtrFromPath(path, this);
        }
        else
        {
            spdlog::error(("The project wasn't found: {}"_f << path.string()).toStdStringView());
        }
    }

    bool deprProjectTree::Process()
    {
        if (_fileExtensions.empty())
        {
            spdlog::error("File reader was nullptr");
            return false;
        }
        if (!_root)
        {
            spdlog::error("Target path is invalid");
            return false;
        }

        IterateOverDirectory(_root->GetPath());

        return true;
    }

    deprProjectTree::Unit::AdaptivePtr<> deprProjectTree::GetGeneratedFileOfUnit(const Unit::Ptr& unit)
    {
        return GetUnitByPath(unit->GetGeneratedSiblingFilePath());
    }

    deprProjectTree::Unit::AdaptivePtr<true> deprProjectTree::GetGeneratedFileOfUnit(const Unit::CPtr& unit) const
    {
        return GetUnitByPath(unit->GetGeneratedSiblingFilePath());
    }

    deprProjectTree::Unit::AdaptivePtr<> deprProjectTree::GetGeneratedFileOfUnit(const Unit& unit)
    {
        return GetUnitByPath(unit.GetGeneratedSiblingFilePath());
    }

    deprProjectTree::Unit::AdaptivePtr<true> deprProjectTree::GetGeneratedFileOfUnit(const Unit& unit) const
    {
        return GetUnitByPath(unit.GetGeneratedSiblingFilePath());
    }

    bool deprProjectTree::IsNeedRegeneration(const Unit::CPtr& unit) const
    {
        return Verify(!!unit) ? IsNeedRegeneration(*unit) : false;
    }

    bool deprProjectTree::IsNeedRegeneration(const Unit& unit) const
    {
        if (!unit.IsGeneratedFile() && unit.GetTree()->HasAtLeastOneMarkedLexer())
        {
            if (unit.HasGeneratedSiblingFile())
            {
                const auto generatedUnit = GetGeneratedFileOfUnit(unit);
                if (generatedUnit == nullptr)
                {
                    return true;
                }
                const auto a = generatedUnit->GetLastModificationTime();
                const auto b = unit.GetLastModificationTime();
                return a != b;
            }
            return true;
        }

        return false;
    }

    bool deprProjectTree::IsGeneratedFile(const std::filesystem::path& path)
    {
        auto file = path.filename();
        if (file.empty())
        {
            return false;
        }

        if (!file.has_extension())
        {
            return false;
        }

        file.replace_extension("");
        if (!file.has_extension())
        {
            return false;
        }

        return file.extension().string() == Unit::generatedSuffixDecl;
    }

    bool deprProjectTree::IsValidExtension(const std::filesystem::path& path) const
    {
        String mainExt;
        if (path.has_extension())
        {
            mainExt = path.extension().string();
        }
        else
        {
            return false;
        }

        /*if (IsGeneratedFile(path))
        {
            return false;
        }*/

        for (const auto& extension : _fileExtensions)
        {
            if (extension == mainExt)
            {
                return true;
            }
        }

        return false;
    }

    void deprProjectTree::ProcessFile(const std::filesystem::path& folders, const std::filesystem::path& fullPath)
    {
        if (!Verify(!!_root))
        {
            return;
        }

        Unit* i = nullptr;
        for (const auto& folder : String(folders.string()).split(separator))
        {
            if (i)
            {
                auto ptr = _root->GetUnitByPath(i->GetPath() / folder.toStdStringView());
                if (!ptr)
                {
                    auto* newUnit = i->LinkSubFolder(folder);
                    if (Verify(newUnit, "Impossible to create subfolder"))
                    {
                        i = newUnit;
                    }
                }
                else
                {
                    i = ptr.get();
                }
            }
            else
            {
                const auto finalPath = _root->GetPath() / folder.toStdStringView();
                if (auto found = _root->GetUnitByPath(finalPath))
                {
                    i = found.get();
                }
                else
                {
                    auto* newUnit = _root->TryToAddChild(Unit::CreateFromPath(finalPath, this));
                    if (Verify(newUnit, "Impossible to create new unit"))
                    {
                        i = newUnit;
                    }
                }
            }
        }

        if (i)
        {
            i->LinkSubFile(String(fullPath.filename().string()));
        }
        else
        {
            _root->LinkSubFile(String(fullPath.filename().string()));
        }
    }

    // TODO: change to stack-based recurse
    void deprProjectTree::IterateOverDirectory(const std::filesystem::path& path)
    {
        for (const auto& i : std::filesystem::directory_iterator(path))
        {
            auto tmp = String(i.path().string());
            const auto rootPath = _root->GetPath().string();
            if (!Verify(tmp.find(rootPath)))
            {
                spdlog::warn(("Can't process the next file: {} - it's not a part of the project"_f << tmp).toStdStringView());
                continue;
            }

            if (std::filesystem::is_directory(i))
            {
                if (!IsExcludedPath(i))
                {
                    IterateOverDirectory(i);
                }
                continue;
            }

            const auto targetPathSize = rootPath.size();
            tmp.subStr(targetPathSize).trimStart('\\');

            if (Verify(!tmp.isEmpty()))
            {
                auto newPath = std::filesystem::path(tmp.c_str());

                if (IsValidExtension(newPath))
                {
                    ProcessFile(newPath.parent_path(), i.path());
                }
            }
        }
    }

} // namespace Ast::Deprecated

#endif
