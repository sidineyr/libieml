#include "ScriptParser.h"
#include "ScriptGrammarVisitor.h"


using namespace ieml::parser;



ScriptParser::ScriptParser() {
    visitor_ = std::make_shared<ScriptGrammarVisitor>(nullptr); // no error listener yet for contextual error
    input_ = std::make_unique<antlr4::ANTLRInputStream>(std::string(""));


    lexer_ = std::make_unique<script_generated::ScriptLexerGrammar>(input_.get());
    lexer_->removeErrorListeners();
    lexer_->addErrorListener(&antlr_error_listener_);

    tokens_ = std::make_unique<antlr4::CommonTokenStream>(lexer_.get());

    parser_ = std::make_unique<script_generated::ScriptParserGrammar>(tokens_.get());
    parser_->removeErrorListeners();
    parser_->addErrorListener(&antlr_error_listener_);

    // use SSL prediction for parser
    // I wrote the grammar so it not need context information, speeding the parser by x10
    parser_->getInterpreter<atn::ParserATNSimulator>()->setPredictionMode(atn::PredictionMode::SLL);
}

const ieml::structure::Script* ScriptParser::get_or_parse(
        ieml::structure::ScriptRegister* reg,
        const std::string& input_str, 
        const std::string& file_id, 
        const size_t line_offset, 
        const size_t char_offset) {
    
    auto script = reg->get_defined_script_by_string(input_str);
    if (script)
        return script;

    std::string reversed(input_str);
    // parse the string reversed
    std::reverse(reversed.begin(), reversed.end());

    input_->load(reversed);
    antlr_error_listener_.clear();
    lexer_->reset();
    lexer_->setInputStream(input_.get());
    tokens_->setTokenSource(lexer_.get());
    parser_->reset();

    // TODO perform error fowarding to the root error_listener (with file position mapping)
    const auto parse_tree = parser_->script();
    if (antlr_error_listener_.has_error())
        return nullptr;
    
    auto script_ast_any = visitor_->visit_with_offset(reg, parse_tree, file_id, line_offset, char_offset);
    if (script_ast_any.isNull())
        return nullptr; // input is not matched ex: empty string

    auto script_ast = std::move(script_ast_any.as<ScriptGrammarVisitor::VisitorResult<const ieml::structure::Script*>>());
    if (script_ast.isError())
        return nullptr;
    
    return script_ast.value();
}

