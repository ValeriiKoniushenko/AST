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

#include "ProjectTree.h"

#include "Core/Timer.h"

namespace Ast
{

    bool ProjectTree::addIgnorePath(const String& path)
    {
        if (!DiskUnit::PathValidator::IsValid(path))
        {
            errorLog("Impossible to ignore this path: " + path + " " + DiskUnit::PathValidator::GetHint());
            return false;
        }

        _ignoredPaths.emplace_back(path);

        return true;
    }

    bool ProjectTree::scanProject()
    {
        if (!canBeScanned())
        {
            return false;
        }

        if (!scanFilesystem())
        {
            return false;
        }
        onFinishScanFilesystem();

        return true;
    }

    bool ProjectTree::scanFilesystem()
    {
        uint64_t count = 0;

        Core::Repeater repeater(0.25);
        repeater.setCallback(
            [&count, this](auto)
            {
                infoLog("Status: have scanned {} entries."_f << count);
            });

        _fstree = FSTree::CreateTree(
            _projectPath,
            [this, &count, &repeater](const std::filesystem::path& path)
            {
                ++count;
                repeater.startOrUpdate();

                bool isValidFileExt = true;
                if (_acceptableFileExtensions && !_acceptableFileExtensions->empty())
                {
                    if (std::filesystem::is_regular_file(path))
                    {
                        isValidFileExt = _acceptableFileExtensions->contains(path.extension());
                    }
                }
                return isValidFileExt && !isIgnoredPath(path);
            },
            _ignoreSymlinks);

        String metricsStr;
        const auto timeGap = repeater.getTimeGap();
        if (timeGap >= 0.001)
        {
            metricsStr = "~{} entries per second."_f << int(static_cast<double>(count) / timeGap);
        }

        infoLog("Was took {}s for filesystem scan of {} entries. {}"_f << timeGap << count << metricsStr);

        return _fstree && _fstree;
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
                infoLog("Due to ignore config, the next path was ignored: {}"_f << path.generic_string());
                return true;
            }
        }
        // escaping all chars

        return false;
    }

    void ProjectTree::setPathToProject(std::filesystem::path path)
    {
        if (!path.is_absolute())
        {
            warnLog("The passed project path is not absolute. You can meet some errors while working with a relative path. Passed path: " +
                    path.generic_string());
        }

        if (!std::filesystem::exists(path))
        {
            criticalLog("The passed project path doesn't exist: " + path.generic_string());
            return;
        }
        _projectPath = std::move(path);
    }

    void ProjectTree::forEach(const std::function<void(DiskUnit*)>& callback)
    {
        if (!callback)
        {
            return;
        }

        _fstree->forEach(
            [&callback](DiskUnit* unit)
            {
                if (unit)
                {
                    callback(unit);
                }
            });
    }

    void ProjectTree::forEachFiles(const std::function<void(FileUnit*)>& callback)
    {
        if (!callback)
        {
            return;
        }

        _fstree->forEach(
            [&callback](DiskUnit* unit)
            {
                if (unit)
                {
                    auto* file = dynamic_cast<FileUnit*>(unit);
                    if (file)
                    {
                        callback(file);
                    }
                }
            });
    }

    void ProjectTree::forEachFilesWithData(const std::function<void(FileUnit*)>& callback)
    {
        if (!callback)
        {
            return;
        }

        _fstree->forEach(
            [&callback](DiskUnit* unit)
            {
                if (unit)
                {
                    auto* file = dynamic_cast<FileUnit*>(unit);
                    if (file && file->getData())
                    {
                        callback(file);
                    }
                }
            });
    }

} // namespace Ast