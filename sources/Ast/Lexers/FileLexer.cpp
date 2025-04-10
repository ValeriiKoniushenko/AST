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

#include "FileLexer.h"

#include "Ast/Tree.h"

namespace Ast
{

    bool FileLexer::GenerateTextSource(TextSourceT& source)
    {
        if (!ITextSourceReader::GenerateTextSource(source))
        {
            return false;
        }
        if (_hasPragmaOnce)
        {
            source.source += "#pragma once" + Code::Endl() + Code::Endl();
        }

        source.carets["write-point"_atom] = source.source.size();
        return true;
    }

    FileLexer::FileLexer(const ContentStream::Ptr& fileReader)
        : BaseLexer(fileReader, typeName)
    {
    }

    void FileLexer::OnPutAdditionalInfoToXml(Xml& xml, XmlNode* output) const
    {
        if (!Verify(output))
        {
            return;
        }

        auto* pragmaOnceNode = xml.allocate_node(rapidxml::node_element, "pragma_once");
        pragmaOnceNode->append_attribute(xml.allocate_attribute("exists", _hasPragmaOnce ? "1" : "0"));
        output->append_node(pragmaOnceNode);
    }

    bool FileLexer::DoParse()
    {
        if (const auto reader = boost::dynamic_pointer_cast<const FileContentStream>(_reader))
        {
            _lexerName = reader->GetFilePath();
        }

        std::cerr << "HERE BIG PROBLEM - FIX IT ASAP" << std::endl;
        if (_reader->Data().regexFind("#pragma +once"))
        {
            _hasPragmaOnce = true;
        }

        return true;
    }

} // namespace Ast
