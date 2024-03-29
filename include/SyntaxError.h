#pragma once

#include "ANTLRErrorListener.h"

#include "ast/interfaces/AST.h"


namespace ieml::parser {
using namespace antlr4;


class SyntaxError {
private:
    const ieml::AST::CharRange char_range_;
    const std::string msg_;

public:
    SyntaxError(const ieml::AST::CharRange& char_range, const std::string& msg): 
        char_range_(char_range), msg_(msg) {}

    const std::string to_string() const {
        return char_range_.to_string() + " " + msg_;
    }

    const std::string& getMessage() const {return msg_;};
    const ieml::AST::CharRange& getCharRange() const {return char_range_;};
};

class ParseError: public SyntaxError {
public:
    ParseError(const ieml::AST::CharRange& char_range, const std::string& msg): 
        SyntaxError(char_range, msg) {}
};

class VisitorError: public SyntaxError {
public:
    VisitorError(const ieml::AST::CharRange& char_range, const std::string& msg): 
        SyntaxError(char_range, msg) {}
};

class ErrorManager {
private:
    bool stdout_;
    std::vector<const SyntaxError*> errors_;
    std::vector<const SyntaxError*> warnings_;

public:
    ErrorManager() : stdout_(true) {};
    ErrorManager(bool print_stdout) : stdout_(print_stdout) {};

    void registerError(const SyntaxError* error) {
        if (stdout_)
            std::cout << "[ERROR] " << error->to_string() << std::endl;
        errors_.push_back(error);
    }

    void registerWarning(const SyntaxError* warning) {
        if (stdout_)
            std::cout << "[WARNING] " << warning->to_string() << std::endl;
        warnings_.push_back(warning);
    }

    void reset() {
        for (auto * entry : errors_)
            delete entry;
        errors_.clear();

        for (auto * entry : warnings_)
            delete entry;
        warnings_.clear();
    }

    ~ErrorManager() {
        reset();
    }

    const std::vector<const SyntaxError*> getSyntaxErrors() const { return errors_; }
    const std::vector<const SyntaxError*> getSyntaxWarnings() const { return warnings_; }
};

class IEMLParserErrorListener{
public:

    class ANTLR4IEMLParserErrorListener: public ANTLRErrorListener {
    public:
        ANTLR4IEMLParserErrorListener(const std::string& file_id, 
                                        ErrorManager* error_manager) : 
            file_id_(file_id), error_manager_(error_manager) {}

        ANTLR4IEMLParserErrorListener(const ANTLR4IEMLParserErrorListener&) = delete;

        void syntaxError(Recognizer *recognizer, Token *offendingSymbol, size_t line,
                        size_t charPositionInLine, const std::string &msg, std::exception_ptr e);

        void reportAmbiguity(Parser *recognizer, const dfa::DFA &dfa, size_t startIndex, size_t stopIndex, bool exact,
                            const antlrcpp::BitSet &ambigAlts, atn::ATNConfigSet *configs);

        void reportAttemptingFullContext(Parser *recognizer, const dfa::DFA &dfa, size_t startIndex, size_t stopIndex,
                                        const antlrcpp::BitSet &conflictingAlts, atn::ATNConfigSet *configs);

        void reportContextSensitivity(Parser *recognizer, const dfa::DFA &dfa, size_t startIndex, size_t stopIndex,
                                    size_t prediction, atn::ATNConfigSet *configs);
    private:
        std::string file_id_;
        ErrorManager* error_manager_;
    };

    IEMLParserErrorListener(bool print_stdout = false) : error_manager_(print_stdout) {}

    ANTLR4IEMLParserErrorListener* getANTLR4ErrorListener(const std::string& file_id) {
        listeners_.push_back(std::make_shared<ANTLR4IEMLParserErrorListener>(file_id, &error_manager_));
        return listeners_[listeners_.size() - 1].get();
    };
    
    void visitorError(const ieml::AST::CharRange& char_range, const std::string &msg);
    void parseError(const ieml::AST::CharRange& char_range, const std::string &msg);

    void visitorWarning(const ieml::AST::CharRange& char_range, const std::string &msg);
    void parseWarning(const ieml::AST::CharRange& char_range, const std::string &msg);

    const std::vector<const SyntaxError*> getSyntaxErrors() const { return error_manager_.getSyntaxErrors(); }
    const std::vector<const SyntaxError*> getSyntaxWarnings() const { return error_manager_.getSyntaxWarnings(); }

private:
    ErrorManager error_manager_;
    std::vector<std::shared_ptr<ANTLR4IEMLParserErrorListener>> listeners_;
};
}