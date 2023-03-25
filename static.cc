const char* AST_TEMPLATE = R"(
#pragma once

#include <memory>
#include <vector>
#include <string>
#include <any>

enum class EToken {
  MY_EOF,
  EPS,
  {{tokens}}
};

struct TNode;

using TPtr = std::shared_ptr<TNode>;

struct TNode {
  TNode* parent;
  std::string name;
  std::any value;

  virtual ~TNode() = 0;
};

struct TTree : TNode {
  std::vector<TPtr> children;

  inline void AddChild(TPtr child) {
    children.push_back(child);
    child->parent = this;
  }

  ~TTree() = default;
};

struct TLeaf : TNode {
  ~TLeaf() = default;
};

struct IVisitor {

  // virtual void visit_<translation symbol>(TNode* ctx) = 0;
  {{visitor_methods}}

  static TTree* GetAncestor(TNode* n, int i) {
    if (i <= 0) {
      throw std::runtime_error("Bad index for parent access");
    }
    while (i > 0) {
      auto parent = dynamic_cast<TTree*>(n->parent);
      if (parent == nullptr) {
        throw std::runtime_error("Can't access parent");
      }
      n = parent;
      i--;
    }
    return static_cast<TTree*>(n);
  }

  static TPtr GetBrother(TNode* n, int i) {
    auto parent = dynamic_cast<TTree*>(n->parent);
    if (parent == nullptr) {
      throw std::runtime_error("Can't access parent");
    }
    if (i < 0 || i >= parent->children.size()) {
      throw std::runtime_error("Can't access brother: index out of range");
    }
    return parent->children[i];
  }

  static std::shared_ptr<IVisitor> GetVisitor();  // user should define this, we provide only the declaration
};
)";

const char* PARSER_TEMPLATE = R"(
#pragma once

#include <regex>
#include <vector>
#include <utility>
#include <algorithm>
#include <iostream>

#include "ast.hh"


struct TLexer {
public:
  TLexer(std::shared_ptr<std::istream> input) : curTokenType{EToken::EPS}, is{input} {
    buf.reserve(CAPACITY);
    FillBuffer();
  }

  void NextToken() {
    if (curTokenType == EToken::MY_EOF) {
      throw std::runtime_error("Attempt to call NextToken past the end");
    }
    {
      // consume all whitespace (our language is whitespace-insensetive)
      auto [matched, s] = MatchPrefix(std::regex{"[ \t\n]*"});
      if (matched) {
        RemovePrefix(s.size());
      }
    }
    if (buf.empty()) {
      curToken.clear();
      curTokenType = EToken::MY_EOF;
      return;
    }
    std::pair<EToken, std::string> biggestMatch{EToken::EPS, ""};
    for (const auto& [tokType, regex] : TOKEN_TO_REGEX) {
      auto [matched, s] = MatchPrefix(regex);
      if (matched && s.size() > biggestMatch.second.size()) {
        biggestMatch = std::pair{tokType, s};
      }
    }

    if (biggestMatch.first == EToken::EPS) {
      curTokenType = EToken::MY_EOF;
      curToken.clear();
    } else {
      curTokenType = biggestMatch.first;
      curToken = biggestMatch.second;
      RemovePrefix(curToken.size());
    }
  }

  std::pair<EToken, std::string> Peek() {
    return {curTokenType, curToken};
  }

  void RemovePrefix(std::size_t n) {
    // NOTE: it's important that std::move moves backward
    std::move(buf.data() + n, buf.data() + buf.size(), buf.begin());
    buf.resize(buf.size() - n);
    FillBuffer();
  }

  void FillBuffer() {
    if (remains) {
      const auto oldSize = buf.size();
      buf.resize(CAPACITY);
      is->read(buf.data() + oldSize, CAPACITY - oldSize);
      const auto read = is->gcount();
      buf.resize(oldSize + read);  // shrink
      if (buf.size() < CAPACITY) {
        remains = false;
      }
    }
  }

  std::pair<bool, std::string> MatchPrefix(std::regex regex) {
      std::cmatch m;
      std::regex_search(buf.data(), buf.data() + buf.size(), m, regex);
      if (m.empty() || m.prefix().str() != "") {
        return {false, ""};
      }
      return {true, m[0].str()};
  }

private:
  const std::vector<std::pair<EToken, std::regex>> TOKEN_TO_REGEX = {
    // pairs of the form "{EToken::..., std::regex{...}}" joined by comma
    {{token_to_regex}}
  };

  bool remains{true};
  std::string curToken;
  EToken curTokenType;
  std::shared_ptr<std::istream> is;
  std::vector<char> buf;
  static constexpr int CAPACITY = 32;
  static constexpr int MAX_TOKEN_SIZE = 16;
};

struct TParser {

  TParser(std::shared_ptr<TLexer> l, std::shared_ptr<IVisitor> v = IVisitor::GetVisitor()) : lexer{l}, visitor{v} {}

  // signature: TPtr Parse_<nterm name>(TNode* parent);
  {{parsing_methods}}

  TPtr Parse() {
    return Parse_start(nullptr);
  }

private:
  std::shared_ptr<TLexer> lexer;
  std::shared_ptr<IVisitor> visitor;
};
)";

const char* PARSE_METHOD_TEMPLATE = R"(
  TPtr Parse_{{nterm}}(TNode* par) {
    auto r = std::make_shared<TTree>();
    r->name = "{{nterm}}";
    r->parent = par;

    auto [tokType, tok] = lexer->Peek();
    switch (tokType) {
      // inside case: if terminal -> AddChild and NextToken
      //              else if translation symbol -> execute visitor->visit_{{ts_name}}
      //              else if nterm -> AddChild(Parse_{{kid_nterm}}(r.get()))
{{rule_cases}}
    }

    return r;
  }
)";

