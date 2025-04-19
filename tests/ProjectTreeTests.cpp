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

#include "Ast/DiskUnits.h"

#include <gtest/gtest.h>

using namespace Ast;

namespace
{

    const std::filesystem::path projectDir = PATH_TO_TEST_PROJECT_1;

} // namespace

TEST(ProjectTreeTest, SimpleActionsWithDiskUnit)
{
    auto unit = DiskUnit::CreateFromPath(projectDir);

    ASSERT_TRUE(unit);
    EXPECT_EQ(projectDir.string(), unit->getPath().string());
    EXPECT_EQ(DiskUnit::Type::Directory, unit->getType());
    EXPECT_TRUE(unit->isWriteable());
    EXPECT_TRUE(unit->isExistOnDisk());
}

TEST(ProjectTreeTest, SimpleActionsWithFolder)
{
    auto unit = DirectoryUnit::CreateFromPath(projectDir);

    ASSERT_TRUE(unit);
    ASSERT_TRUE(unit->isValid());
    EXPECT_EQ(projectDir.string(), unit->getPath().string());
    EXPECT_EQ(DiskUnit::Type::Directory, unit->getType());
    EXPECT_TRUE(unit->isWriteable());
    EXPECT_TRUE(unit->isExistOnDisk());

    auto dir = DirectoryUnit::Create();
    dir->_setType(DirectoryUnit::Type::Directory);
    dir->_setPath("hello/world");

    ASSERT_TRUE(dir);
    ASSERT_TRUE(dir->isValid());
    EXPECT_EQ("hello/world", dir->getPath().string());
    EXPECT_EQ(DiskUnit::Type::Directory, dir->getType());
    EXPECT_FALSE(dir->isExistOnDisk());
}
