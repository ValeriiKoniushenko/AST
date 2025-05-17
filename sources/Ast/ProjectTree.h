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

#include "Ast/GeneratorFileComposer.h"
#include "FileSystem/FSTree.h"
#include "Tree.h"

namespace Ast
{

    class ProjectTree : public BaseLog, public Utils::NotCopyableButMoveable, public boost::intrusive_ref_counter<ProjectTree>
    {
    public:
        AST_CLASS(ProjectTree)

    public:
        ProjectTree() = default;
        ~ProjectTree() override = default;

        [[nodiscard]] static Ptr Create() { return { new ProjectTree }; }

        bool addIgnorePath(const String& path);
        void resetIgnorePaths() { _ignoredPaths.clear(); }
        [[nodiscard]] std::vector<String>& getIgnorePaths() { return _ignoredPaths; }
        [[nodiscard]] const std::vector<String>& getIgnorePaths() const { return _ignoredPaths; }
        [[nodiscard]] bool isIgnoredPath(const std::filesystem::path& path) const;

        [[nodiscard]] FSTree::CPtr getFSTree() const { return _fstree; }
        [[nodiscard]] FSTree::Ptr getFSTree() { return _fstree; }

        void setPathToProject(std::filesystem::path path);
        [[nodiscard]] std::filesystem::path getProjectPath() const { return _projectPath; }

        [[nodiscard]] bool scanProject();
        [[nodiscard]] bool canBeScanned() const;

        [[nodiscard]] bool isIgnoreSymlinks() const noexcept { return _ignoreSymlinks; }
        void setIgnoreSymlinks(bool value) noexcept { _ignoreSymlinks = value; }

        [[nodiscard]] std::optional<std::unordered_set<std::string>>& getAcceptableFileExtensions() { return _acceptableFileExtensions; }
        [[nodiscard]] const std::optional<std::unordered_set<std::string>>& getAcceptableFileExtensions() const { return _acceptableFileExtensions; }

        // ======= FOR-EACHes ========
        void forEach(const std::function<void(DiskUnit*)>& callback);
        void forEachFiles(const std::function<void(FileUnit*)>& callback);
        void forEachFilesWithData(const std::function<void(FileUnit*)>& callback);

        void setGeneratorFileComposer(const GeneratorFileComposer::Ptr& composer) { _composer = composer; }

    protected:
        [[nodiscard]] bool scanFilesystem();
        virtual void onFinishScanFilesystem() {}

    protected:
        [[nodiscard]] spdlog::logger* getLogger() const final
        {
            static std::shared_ptr<spdlog::logger> logger = spdlog::stdout_color_mt("ProjectTree");
            return logger.get();
        }

    protected:
        FSTree::Ptr _fstree;
        std::filesystem::path _projectPath;
        std::vector<String> _ignoredPaths;
        std::optional<std::unordered_set<std::string>> _acceptableFileExtensions;
        GeneratorFileComposer::Ptr _composer;

        // scan configs
        bool _ignoreSymlinks = false;
    };

} // namespace Ast