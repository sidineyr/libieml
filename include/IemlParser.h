#pragma once

#include <memory>

#include "antlr4-runtime.h"
#include "iemlLexer.h"
#include "iemlParser.h"

#include "RecognitionError.h"

#include "IEMLGrammarVisitor.h"

#include "ast/Program.h"
#include <nlohmann/json.hpp>


namespace ieml {
namespace parser {


class IEMLParser {
private:
    antlr4::ANTLRInputStream* input_ = nullptr;
    IEMLParserErrorListener* errorListener_ = nullptr;

    ieml_generated::iemlLexer* lexer_ = nullptr;
    ieml_generated::iemlParser* parser_ = nullptr;
    antlr4::CommonTokenStream* tokens_ = nullptr;

    antlr4::tree::ParseTree* parseTree_ = nullptr;
    
    std::unique_ptr<Program> ast_ = nullptr;
    std::unique_ptr<ParserContext> context = nullptr;

public:
    // explicit IEMLParser(const std::string& input_str);
    explicit IEMLParser(const std::string& input_str, bool error_stdout = false);

    ~IEMLParser();

    void parse();
    const antlr4::tree::ParseTree* getParseTree() const;
    std::string getParseString() const;

    const std::vector<const SyntaxError*> getSyntaxErrors() const { return errorListener_->getSyntaxErrors(); }

    const std::string getASTString() const;

    nlohmann::json toJson() const;
};

}}