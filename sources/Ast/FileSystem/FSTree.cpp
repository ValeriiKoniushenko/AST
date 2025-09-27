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

#include "FSTree.h"

namespace Ast
{

    FSTree::Ptr FSTree::CreateTree(const std::filesystem::path& path, std::function<bool(const std::filesystem::path&)> pred,
                                   bool isIgnoreSymlinks /* = false*/)
    {
        auto tree = Ptr(new FSTree);

        tree->_root = DirectoryUnit::CreateFromPath(path);
        if (!tree->_root)
        {
            ASSERT(false);
            globalLog.errorLog("Impossible to create a FSTree");
            return nullptr;
        }

        if (!tree->_root->isValid())
        {
            ASSERT(false);
            globalLog.errorLog("Impossible to create a FSTree: invalid root folder");
            return nullptr;
        }

        std::stack<DirectoryUnit*> units;
        units.push(dynamic_cast<DirectoryUnit*>(tree->_root.get()));

        std::function<void(const std::filesystem::directory_entry&)> readDiskUnit =
            [&pred, &units, &readDiskUnit, isIgnoreSymlinks](const std::filesystem::directory_entry& entry)
        {
            auto* top = units.top();
            if (!top)
            {
                ASSERT(false);
                return;
            }

            if (isIgnoreSymlinks && entry.is_symlink())
            {
                return;
            }

            try
            {
                if (!entry.exists())
                {
                    return;
                }
            }
            catch (const std::filesystem::filesystem_error& er)
            {
                globalLog.warnLog("Can't detect file status. Details: {}"_f << er.what());
                return;
            }

            if (pred && !pred(entry.path()))
            {
                return;
            }

            if (entry.is_directory())
            {
                auto dir = DirectoryUnit::CreateFromPath(entry.path());
                if (dir->isValid())
                {
                    top->addChild(dir);
                    units.push(dir.get());
                    dir->iterateOverPhysicalContent(readDiskUnit);
                    units.pop();
                }
            }
            else
            {
                if (!entry.is_regular_file())
                {
                    globalLog.warnLog(
                        "Trying to read a physical file tree. Was met NON-file(and non-dir) entry. It will be considered as regular file. Not supporting other types. Path: " +
                        entry.path().string());
                }

                auto file = FileUnit::CreateFromPath(entry.path());
                if (file && file->isValid())
                {
                    top->addChild(file);
                }
            }
        };

        units.top()->iterateOverPhysicalContent(readDiskUnit);

        return tree;
    }

    void FSTree::clear()
    {
        _root = nullptr;
    }

    void FSTree::prettyPrint(std::function<PrettyInfo(const DiskUnit*)>&& pred /* = nullptr*/)
    {
        int prevDistance = -1;

        forEach(
            [&](const DiskUnit* unit)
            {
                PrettyInfo info;
                if (pred)
                {
                    info = pred(unit);
                }

                if (info.ignore)
                {
                    return;
                }

                static const char* defaultSplitter = "─── ";
                static const char* defaultSpace = "    ";
                const auto distance = unit->distanceToRoot();

                for (char i : info.prefix)
                {
                    std::cout << (i < 32 ? ' ' : i);
                }

                std::cout << " ";

                if (prevDistance != -1)
                {
                    for (int i = 0; i < distance - 1; ++i)
                    {
                        std::cout << "│" << defaultSpace;
                    }

                    std::cout << "├" << defaultSplitter;
                }

                std::cout << unit->getName();

                std::cout << "  ";
                for (char i : info.suffix)
                {
                    std::cout << (i < 32 ? ' ' : i);
                }

                prevDistance = distance;

                std::cout << std::endl;
            });
    }

    uint64_t FSTree::calculateUnitsCount() const
    {
        uint64_t count = 0;
        forEach(
            [&count](const auto*)
            {
                ++count;
            });

        return count;
    }

} // namespace Ast