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

        unit->_name = path.stem().generic_string();

        return unit;
    }
} // namespace Ast
