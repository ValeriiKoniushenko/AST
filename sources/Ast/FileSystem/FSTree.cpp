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

#include "FSTree.h"

namespace Ast
{

    FSTree::Ptr FSTree::CreateTree(const std::filesystem::path& path, std::function<bool(const std::filesystem::path&)> pred)
    {
        auto tree = Ptr(new FSTree);

        tree->_root = DirectoryUnit::CreateFromPath(path);
        if (!tree->_root)
        {
            Assert();
            logger->error("Impossible to create a FSTree");
            return nullptr;
        }

        if (!tree->_root->isValid())
        {
            Assert();
            logger->error("Impossible to create a FSTree: invalid root folder");
            return nullptr;
        }

        std::stack<DirectoryUnit*> units;
        units.push(dynamic_cast<DirectoryUnit*>(tree->_root.get()));

        std::function<void(const std::filesystem::directory_entry&)> readDiskUnit =
            [&units, &readDiskUnit](const std::filesystem::directory_entry& entry)
        {
            auto* top = units.top();
            if (!top)
            {
                Assert();
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
                    logger->warn(
                        "Trying to read a physical file tree. Was met NON-file(and non-dir) entry. It will be considered as regular file. Not supporting other types. Path: " +
                        entry.path().string());
                }

                auto file = FileUnit::CreateFromPath(entry.path());
                if (file->isValid())
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

} // namespace Ast