#include <iostream>
#include <fstream>
#include <filesystem>

#include <range/v3/view/join.hpp>
#include <range/v3/view/map.hpp>
#include <range/v3/view/filter.hpp>
#include <range/v3/range/conversion.hpp>
#include <range/v3/algorithm/equal.hpp>

#include <absl/strings/str_split.h>
#include <absl/strings/str_format.h>
#include <absl/strings/substitute.h>

#include <absl/log/log.h>
#include <absl/flags/flag.h>
#include <absl/flags/parse.h>

#include <cpputils/string.hh>
#include <cpputils/common.hh>

#include "common.hh"

ABSL_FLAG(std::string, out_dir, "", "output file dir");
ABSL_FLAG(std::string, grammar_file, "", "file containing the grammar description");

extern const char* AST_TEMPLATE;
extern const char* PARSER_TEMPLATE;
extern const char* PARSE_METHOD_TEMPLATE;
extern const char* MAIN_TEMPLATE;

std::string ReadFile(std::istream& in) {
  char buf[1024];
  std::string result;
  while (true) {
    in.read(buf, sizeof(buf));
    auto read = in.gcount();
    result.append(buf, read);
    if (read < sizeof(buf)) {
      break;
    }
  }
  return result;
}

int main(int argc, char** argv) {
  absl::ParseCommandLine(argc, argv);
  if (absl::GetFlag(FLAGS_grammar_file).empty() || absl::GetFlag(FLAGS_out_dir).empty()) {
    LOG(ERROR) << "Launch the program with grammar_file and out_dir flags. Aborting";
    exit(1);
  }
  std::fstream in{absl::GetFlag(FLAGS_grammar_file)};
  LOG(INFO) << "Reading grammar from file " << absl::GetFlag(FLAGS_grammar_file);
  auto grammarStr = ReadFile(in);
  LOG(INFO) << grammarStr;
  auto grammar = ParseGrammar(grammarStr);
  LOG(INFO) << "Grammar successfully parsed";
  grammar->CalculateFIRST();
  LOG(INFO) << "FIRST succsessfully calculated";
  grammar->CalculateFOLLOW();
  LOG(INFO) << "FOLLOW succsessfully calculated";

  if (!grammar->IsLL1()) {
    LOG(ERROR) << "The grammar is not LL1! Aborting";
    std::exit(1);
  }

  auto outDir = absl::GetFlag(FLAGS_out_dir);

  /****************************************************************************
  *                                AST header                                *
  ****************************************************************************/

  auto transSymbols = grammar->rules
    | ranges::views::values
    | ranges::views::join
    | ranges::views::join
    | ranges::views::filter(IS_TS)
    | ranges::views::transform([] (std::string_view str) -> std::string_view { return str.substr(1); });  // remove $

  auto visitorMethods = transSymbols
    | ranges::views::transform([] (std::string_view str) { return absl::StrFormat("virtual void visit_%s(TTree* ctx) = 0;", str); })
    | ranges::views::join(std::string{"\n  "})  // otherwise null-terminator gets added to output
    | ranges::to<std::string>();
  auto tokens = grammar->tokenToRegex
    | ranges::views::keys
    | ranges::views::join(std::string{",\n  "})  // otherwise null-terminator gets added to output
    | ranges::to<std::string>();

  std::string astHeader = utils::Replace(AST_TEMPLATE, {
    { "{{tokens}}", tokens },
    { "{{visitor_methods}}", visitorMethods },
  });
  {
    std::ofstream out{absl::StrCat(outDir, "/ast.hh")};
    out << astHeader;
  }

  /****************************************************************************
  *                          Parser & lexer header                           *
  ****************************************************************************/

  auto tokenToRegex = grammar->tokenToRegex
    | ranges::views::transform([](const auto& kv) { const auto& [k, v] = kv; return absl::StrFormat(R"({EToken::%s, std::regex{"%s"}})", k, v); })
    | ranges::views::join(std::string{",\n    "})
    | ranges::to<std::string>();

  std::string parsingMethods = "";
  for (const auto& [lhs, rhsGroup] : grammar->rules) {

    std::string ruleCases = "";
    for (const auto& rhs : rhsGroup) {
      auto first1 = CalculateRecurFIRST(*grammar, rhs);
      static_assert(!std::is_reference_v<decltype(first1)>, "shouldn't be a reference");
      if (first1.contains("EPS")) {
        for (const auto& tok : grammar->follow[lhs]) {
          first1.insert(tok);
        }
      }
      first1.erase("EPS");

      auto cases = first1
        | ranges::views::transform([] (std::string_view s) { return absl::StrFormat("      case EToken::%s:", s); })
        | ranges::views::join(std::string{"\n"})
        | ranges::to<std::string>();

      std::string caseBody;
      if (!ranges::equal(rhs, ranges::views::single("EPS"))) {
        caseBody = rhs
          | ranges::views::transform([] (std::string_view rhsItem) {
              if (IS_TS(rhsItem)) {
                std::string_view withoutDollar = rhsItem.substr(1);
                return absl::StrFormat("        visitor->visit_%s(r.get());", withoutDollar);
              } else if (IS_NTERM(rhsItem)) {
                return absl::StrFormat("        r->AddChild(Parse_%s(r.get()));", rhsItem);
              } else {
                EXPECT(IS_TOKEN(rhsItem), absl::StrFormat("Can only be token but got %s", rhsItem));
                return absl::StrFormat(
        R"(
        {
          auto [type, localTok] = lexer->Peek();
          assert(type == EToken::%s);
          lexer->NextToken();
          auto child = std::make_shared<TLeaf>();
          child->name = localTok;
          r->AddChild(child);
        })",
                    rhsItem);
              }
          })
          | ranges::views::join(std::string{"\n"})
          | ranges::to<std::string>();
      }
      ruleCases.append(absl::StrFormat("%s {\n%s\n        break;\n      }\n", cases, caseBody));
    }
    ruleCases.append(
      absl::StrFormat(
        R"(      default: throw std::runtime_error("Unexpected " + tok + " at Parse_%s");)",
        lhs
      )
    );
    std::string method = utils::Replace(PARSE_METHOD_TEMPLATE, {
        {"{{nterm}}", lhs},
        {"{{rule_cases}}", ruleCases},
    });
    parsingMethods.append("\n").append(method);
  }


  std::string parserHeader = utils::Replace(PARSER_TEMPLATE, {
      { "{{token_to_regex}}", tokenToRegex},
      { "{{parsing_methods}}", parsingMethods},
  });
  {
    std::ofstream out{absl::StrCat(outDir, "/parser.hh")};
    out << parserHeader;
  }

  if (auto outMain = absl::StrCat(outDir, "/main.cc"); !std::filesystem::exists(outMain)) {
    std::ofstream out{outMain};
    auto visitOverrides = transSymbols
      | ranges::views::transform([] (std::string_view str) { return absl::StrFormat("void visit_%s(TTree* ctx) override {}", str); })
      | ranges::views::join(std::string{"\n  "})
      | ranges::to<std::string>();
    out << utils::Replace(MAIN_TEMPLATE, {
        {"{{visit_overrides}}", visitOverrides},
    });
    LOG(INFO) << "No main in out_dir; generated " << outMain;
  }
}

