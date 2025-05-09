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

#include "Ast/FileSystem/FSTree.h"

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
    ASSERT_TRUE(dir);

    dir->setType(DirectoryUnit::Type::Directory);
    ASSERT_TRUE(dir->setName("someNewFolder"));
    ASSERT_TRUE(dir->isValid());

    EXPECT_EQ("someNewFolder", dir->getPath().string());
    EXPECT_EQ("someNewFolder", dir->getName());
    EXPECT_EQ(DiskUnit::Type::Directory, dir->getType());
    EXPECT_FALSE(dir->isExistOnDisk());
}

TEST(ProjectTreeTest, NameValidator)
{
    EXPECT_TRUE(DiskUnit::NameValidator::IsValid("hello"));
    EXPECT_TRUE(DiskUnit::NameValidator::IsValid("hello world"));
    EXPECT_TRUE(DiskUnit::NameValidator::IsValid("hello world."));
    EXPECT_TRUE(DiskUnit::NameValidator::IsValid(".config"));
    EXPECT_TRUE(DiskUnit::NameValidator::IsValid(".512config14123"));
    EXPECT_TRUE(DiskUnit::NameValidator::IsValid("123.512config14123"));
    EXPECT_TRUE(DiskUnit::NameValidator::IsValid("...--_--_--.512config14123"));
    EXPECT_FALSE(DiskUnit::NameValidator::IsValid("how are you?"));
    EXPECT_FALSE(DiskUnit::NameValidator::IsValid("how are you123###"));
}

TEST(ProjectTreeTest, WorkingWithChildsInFolder)
{
    auto root = DirectoryUnit::CreateFromPath(projectDir);

    ASSERT_TRUE(root);
    ASSERT_TRUE(root->isValid());
    EXPECT_TRUE(std::filesystem::path(root->getPath().c_str()).is_absolute());

    auto dir = DirectoryUnit::Create();
    ASSERT_TRUE(dir);
    dir->setType(DirectoryUnit::Type::Directory);
    ASSERT_TRUE(dir->setName("hello"));

    root->addChild(dir, true);

    auto dir2 = DirectoryUnit::Create();
    ASSERT_TRUE(dir2);
    dir2->setType(DirectoryUnit::Type::Directory);
    ASSERT_TRUE(dir2->setName("world"));

    dir->addChild(dir2, true);

    EXPECT_EQ("world", dir2->getName());
    EXPECT_EQ(projectDir / "hello" / "world", dir2->getPath());
}

TEST(ProjectTreeTest, InvalidPWD)
{
    auto dir1 = DirectoryUnit::CreateFromPath(projectDir);
    auto dir2 = DirectoryUnit::CreateFromPath(projectDir / "sources");

    dir1->addChild(dir2);

    auto path = dir2->getPath();

    EXPECT_EQ(projectDir / "sources", dir2->getPath());
    EXPECT_EQ("sources", dir2->getName());
}

TEST(ProjectTreeTest, WorkingWithFiles)
{
    auto file = FileUnit::CreateFromPath(projectDir / "sources" / "CMakeLists.txt");
    ASSERT_TRUE(file);

    ASSERT_TRUE(file->isValid());
    ASSERT_TRUE(file->hasAbsolutePath());
    EXPECT_NE(0, file->getLastWriteTime());
    EXPECT_EQ(projectDir / "sources" / "CMakeLists.txt", file->getPath());

    auto fileContent = file->getContent();
    EXPECT_FALSE(fileContent.isEmpty());
    EXPECT_GT(fileContent.size(), 100);
}

TEST(ProjectTreeTest, IterateOverDirectory)
{
    auto dir1 = DirectoryUnit::CreateFromPath(projectDir);

    std::set<std::filesystem::path> set;

    dir1->iterateOverPhysicalContent(
        [&set](const std::filesystem::directory_entry& entry)
        {
            if (entry.exists())
            {
                set.insert(entry.path());
            }
        });

    ASSERT_EQ(3, set.size());
    EXPECT_TRUE(set.contains(projectDir / "excludedDir"));
    EXPECT_TRUE(set.contains(projectDir / "sources"));
    EXPECT_TRUE(set.contains(projectDir / ".gitignore"));
}

TEST(ProjectTreeTest, FSTreeBasedOnSmallProject)
{
    auto tree = FSTree::CreateTree(projectDir);

    auto root = boost::dynamic_pointer_cast<DirectoryUnit>(tree->getRoot());
    ASSERT_TRUE(root);

    auto generatedDir = root->addChildAndGetBack(DirectoryUnit::Create("generated"));
    generatedDir->addChild(FileUnit::Create("Reflect.h"));

    std::cout << "\n\nPretty print: " << std::endl;
    tree->prettyPrint(
        [](const DiskUnit* unit) -> FSTree::PrettyInfo
        {
            if (unit->isExistOnDisk())
            {
                return { true };
            }

            return { false, "N" };
        });
}
