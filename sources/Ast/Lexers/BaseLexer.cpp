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

#include "BaseLexer.h"

#include "../Readers/ContentStream.h"
#include "Ast/Rule.h"
#include "Ast/Utils/Scopes.h"
#include "Core/Assert.h"
#include "spdlog/spdlog.h"

namespace Ast
{

    bool BaseLexer::operator==(const BaseLexer& other) const
    {
        return other._reader == _reader && other._token.beginData == _token.beginData && other._token.endData == _token.endData &&
               other._lexerName == _lexerName && other._parentLexer == _parentLexer;
    }

    void BaseLexer::SetToken(const TokenReader& token)
    {
        _token = token;
    }

    bool BaseLexer::Parse()
    {
        if (!DoParse())
        {
            return false;
        }
        if (!DoScopeParse())
        {
            return false;
        }
        if (!DoMarkingParse())
        {
            return false;
        }
        if (!DoPostParse())
        {
            return false;
        }

        debugLog("Successful parsing of the {}: '{}'"_f << _lexerType << _lexerName);

        return IsValid();
    }

    void BaseLexer::ValidateAfterParse()
    {
        OnParse();

        ValidateMark();
    }

    bool BaseLexer::IsValid() const
    {
        return !_lexerType.isEmpty() && !_lexerName.isEmpty() && _reader;
    }

    bool BaseLexer::IsCorrespondingToRule(const Rule& rule, const char* additionalMessage /* = nullptr*/) const
    {
        return rule.IsCorrespondingTheRules(this, additionalMessage);
    }

    void BaseLexer::GetAsXml(Xml& xml) const
    {
        xml.clear();
        OnGetAsXml(xml, nullptr);
    }

    std::pair<const String::CharT* const, const String::CharT* const> BaseLexer::GetReaderLimits() const
    {
        if (Verify(!!_reader))
        {
            return std::make_pair(_reader->Data().c_str() - 1, _reader->Data().c_str() + _reader->Data().size());
        }
        return std::make_pair(nullptr, nullptr);
    }

    bool BaseLexer::HasTheSameParentAs(BaseLexer::Ptr parent) const
    {
        if (Verify(!!parent))
        {
            if (_parentLexer)
            {
                return *parent == *_parentLexer;
            }
        }
        return false;
    }

    void BaseLexer::TryToSetParent(const Ptr& parent)
    {
        parent->TryToSetAsChild(this);
    }

    void BaseLexer::ForceSetParent(const Ptr& parent)
    {
        parent->ForceSetAsChild(this);
    }

    void BaseLexer::TryToSetAsChild(const Ptr& child)
    {
        if (Verify(!!child) && !child->HasParent())
        {
            if (_modifierParams.wasModified || IsContainLexer(child.get()))
            {
                auto it = std::find_if(_childLexers.cbegin(), _childLexers.cend(),
                                       [&child](const auto& lexer)
                                       {
                                           return child->GetLexerName() == lexer->GetLexerName();
                                       });

                if (it == _childLexers.cend())
                {
                    _childLexers.push_back(child);
                    child->_parentLexer = this;
                }
            }
        }
    }
    void BaseLexer::ForceSetAsChild(const Ptr& child)
    {
        if (Verify(!!child) && !child->HasParent())
        {
            auto it = std::find_if(_childLexers.cbegin(), _childLexers.cend(),
                                   [&child](const auto& lexer)
                                   {
                                       return child->GetLexerName() == lexer->GetLexerName();
                                   });

            if (it != _childLexers.cend())
            {
                _childLexers.erase(it);
            }

            _childLexers.push_back(child);
            child->_parentLexer = this;
        }
    }

    bool BaseLexer::IsContainLexer(const BaseLexer* other, bool isInItsScope /* = false*/) const
    {
        const bool isValidOther = Verify(other, "BaseLexer 'other' is nullptr");
        const bool hasOpenedScope = Verify(_closeScope.has_value(), "This Lexer doesn't have a close scope(it should be bound to the source code)");
        const bool hasClosedScope = Verify(_openScope.has_value(), "This Lexer doesn't have an open scope(it should be bound to the source code)");
        const bool hasOpenedScopeOther =
            Verify(other->_closeScope.has_value(), "An 'other' Lexer doesn't have a close scope(it should be bound to the source code)");
        const bool hasClosedScopeOther =
            Verify(other->_openScope.has_value(), "An 'other' Lexer doesn't have an open scope(it should be bound to the source code)");

        if (isValidOther && hasOpenedScope && hasClosedScope && hasOpenedScopeOther && hasClosedScopeOther)
        {
            if (_openScope->string < other->_openScope->string && _closeScope->string > other->_closeScope->string)
            {
                if (isInItsScope)
                {
                    return !Utils::HasUnclosedBracket(_openScope->string, other->_openScope->string, '}', '{');
                }

                return true;
            }
        }
        return false;
    }

    void BaseLexer::Clear()
    {
        _token.Clear();
        _openScope.reset();
        _closeScope.reset();
        _lexerName.clear();
        _parentLexer = nullptr;
        _childLexers.clear();
    }

    String BaseLexer::GetTextSource()
    {
        TextSourceT source;

        this->GenerateTextSource(source);

        for (const auto& lexer : _childLexers)
        {
            lexer->GenerateTextSource(source);
        }

        return source.source;
    }

    void BaseLexer::OnGetAsXml(Xml& xml, XmlNode* output) const
    {
        auto* mainNode = xml.allocate_node(rapidxml::node_type::node_element, "lexer");
        mainNode->append_attribute(xml.allocate_attribute("type", _lexerType.c_str()));
        mainNode->append_attribute(xml.allocate_attribute("name", _lexerName.c_str()));
        if (Verify(!!_reader))
        {
            mainNode->append_attribute(xml.allocate_attribute("path", _reader->GetFilePath().c_str()));
        }

        if (auto path = GetFullPath().first; Verify(!!path))
        {
            mainNode->append_attribute(xml.allocate_attribute("ast_path", path.c_str()));
        }

        if (_marking)
        {
            auto* markNode = xml.allocate_node(rapidxml::node_type::node_element, "marking");
            markNode->append_attribute(xml.allocate_attribute("rule", _marking->rule.c_str()));
            for (const auto& param : _marking->params)
            {
                markNode->append_node(xml.allocate_node(rapidxml::node_type::node_element, "param", param.c_str()));
            }
            mainNode->append_node(markNode);
        }

        auto* childsNode = xml.allocate_node(rapidxml::node_type::node_element, "childs");
        mainNode->append_node(childsNode);

        auto* infoNode = xml.allocate_node(rapidxml::node_type::node_element, "additional_info");
        mainNode->append_node(infoNode);
        OnPutAdditionalInfoToXml(xml, infoNode);

        for (const auto& child : _childLexers)
        {
            if (Verify(!!child))
            {
                child->OnGetAsXml(xml, childsNode);
            }
        }

        if (output)
        {
            output->append_node(mainNode);
        }
        else
        {
            xml.append_node(mainNode);
        }
    }

    BaseLexer::BaseLexer(const ContentStream::Ptr& reader, const String& type)
        : _reader{ reader },
          _lexerType{ type }
    {
        Assert(!!_reader);
        Assert(!_lexerType.isEmpty());
    }

} // namespace Ast