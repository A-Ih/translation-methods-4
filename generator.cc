#include <iostream>
#include <fstream>

#include <range/v3/view/join.hpp>
#include <range/v3/view/map.hpp>
#include <range/v3/view/filter.hpp>
#include <range/v3/range/conversion.hpp>

#include <absl/strings/str_split.h>
#include <absl/strings/str_format.h>
#include <absl/strings/substitute.h>

#include <absl/log/log.h>
#include <absl/flags/flag.h>
#include <absl/flags/parse.h>

#include <cpputils/string.hh>

#include "common.hh"

ABSL_FLAG(std::string, out_dir, "", "output file dir");
ABSL_FLAG(std::string, grammar_file, "", "file containing the grammar description");

constexpr auto AST_TEMPLATE = R"(
#pragma once

#include <memory>
#include <vector>
#include <string>

enum class EToken {
  EOF,
  EPS,
  {{tokens}}
};

struct TNode;

using TPtr = std::shared_ptr<TNode>;

struct TNode {
  TNode* parent;
  std::string name;

  virtual ~TNode() = 0;
};

struct TTree : TNode {
  std::vector<TPtr> children;

  inline TPtr AddChild(TPtr child) {
    children.push_back(child);
    child->dad = this;
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
    return n;
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

constexpr auto PARSER_TEMPLATE = R"(
#pragma once

#include <regex>
#include <vector>
#include <utility>
#include <algorithm>

#include "ast.hh"


struct TLexer {
public:
  TLexer(std::shared_ptr<std::istream> input) : is{input}, curTokenType{EToken::EPS} {
    buf.reserve(CAPACITY);
    FillBuffer();
  }

  void NextToken() {
    if (curTokenType == EToken::EOF) {
      throw std::runtime_error("Attempt to call NextToken past the end");
    }
    {
      // consume all whitespace (our language is whitespace-insensetive)
      auto [matched, s] = MatchPrefix(std::regex{"[ \t\n]*"});
      if (matched) {
        int removed = s.size();
        // NOTE: it's important that std::move moves backward
        std::move(buf.begin() + removed, buf.end(), buf.begin());
        FillBuffer();
      }
    }
    if (buf.empty()) {
      curToken.clear();
      curTokenType = EToken::EOF;
      return;
    }
    std::pair<EToken, std::string> biggestMatch{EToken::EPS, ""};
    for (const auto& [tokType, regex] : TOKEN_TO_REGEX) {
      auto [matched, s] = MatchPrefix(regex);
      if (matched && s > biggestMatch.second.size()) {
        biggestMatch = std::pair{tokType, m[0].str};
      }
    }

    if (biggestMatch.first == EToken::EPS) {
      curTokenType = EToken::EOF;
      curToken.clear();
    } else {
      curTokenType = biggestMatch.first;
      curToken = biggestMatch.second;
      int removed = curToken.size();
      // NOTE: it's important that std::move moves backward
      std::move(buf.begin() + removed, buf.end(), buf.begin());
      FillBuffer();
    }
  }

  std::pair<EToken, std::string> Peek() {
    return {curTokenType, curToken};
  }

  void FillBuffer() {
    if (remains) {
      const auto oldSize = buf.size();
      buf.resize(CAPACITY);
      const auto read = is->read(buf.data() + oldSize, CAPACITY - oldSize);
      buf.resize(oldSize + read);  // shrink
      if (buf.size() < CAPACITY) {
        remains = false;
      }
    }
  }

  std::pair<bool, std::string> MatchPrefix(std::regex regex) {
      std::cmatch m;
      std::regex_search(buf.begin(), buf.end(), m, regex);
      if (m.empty() || m.prefix().str() != "") {
        return {false, ""};
      }
      return {true, m[0].str()};
  }

private:
  static const std::vector<std::pair<EToken, std::regex>> TOKEN_TO_REGEX = {
    // pairs of the form "{EToken::..., std::regex{...}}" joined by comma
    {{token_to_regex}}
  };

  bool remains{true};
  std::string curToken;
  EToken curTokenType;
  std::shared_ptr<std::istream> is;
  std::vector<char> buf;  // TODO: reserve
  static constexpr int CAPACITY = 32;
  static constexpr int MAX_TOKEN_SIZE = 16;
};

struct TParser {

  // signature: TPtr Parse_<nterm name>(TNode* parent);
  {{parsing_methods}}

  TPtr Parse() {
    return Parse_start(nullptr);
  }

  std::shared_ptr<TLexer> lexer;
  std::shared_ptr<IVisitor> visitor;
};
)";

constexpr auto PARSE_METHOD_TEMPLATE = R"(
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

int main(int argc, char** argv) {
  absl::ParseCommandLine(argc, argv);
  if (absl::GetFlag(FLAGS_grammar_file).empty() || absl::GetFlag(FLAGS_out_dir).empty()) {
    LOG(ERROR) << "Launch the program with grammar_file and out_dir flags. Aborting";
    exit(1);
  }
  std::fstream in{absl::GetFlag(FLAGS_grammar_file), std::ios_base::ate};
  std::stringstream ss;
  ss << in.rdbuf();
  // TODO: the file isn't properly read
  LOG(INFO) << ss.str() << std::endl;
  auto grammar = ParseGrammar(ss.str());
  LOG(INFO) << "Grammar successfully parsed";
  grammar->CalculateFIRST();
  LOG(INFO) << "FIRST succsessfully calculated";
  grammar->CalculateFOLLOW();
  LOG(INFO) << "FOLLOW succsessfully calculated";

  auto visitorMethods = grammar->rules
    | ranges::views::values
    | ranges::views::join
    | ranges::views::join
    | ranges::views::filter(IS_TS)
    | ranges::views::transform([] (std::string_view str) { return absl::StrFormat("virtual void visit_%s(TNode* ctx) = 0;", str); })
    | ranges::views::join("\n  ")
    | ranges::to<std::string>();
  auto tokens = grammar->tokenToRegex
    | ranges::views::keys
    | ranges::views::join(",\n  ")
    | ranges::to<std::string>();

  std::string astHeader = utils::Replace(AST_TEMPLATE, {
    { "{{tokens}}", tokens },
    { "{{visitor_methods}}", visitorMethods },
  });

  auto outDir = absl::GetFlag(FLAGS_out_dir);
  {
    std::ofstream out{absl::StrCat(outDir, "/ast.hh")};
    out << astHeader;
  }

}

