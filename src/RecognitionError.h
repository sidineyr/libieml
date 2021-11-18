#pragma once

#include "ANTLRErrorListener.h"

#include "ast/interfaces/AST.h"

#include <nlohmann/json.hpp>


namespace ieml {
namespace parser {
    using namespace antlr4;


    class SyntaxError {
        private:
            std::unique_ptr<ieml::AST::CharRange> char_range_;
            const std::string msg_;

        public:
            SyntaxError(std::unique_ptr<ieml::AST::CharRange>&& char_range, const std::string& msg): 
                char_range_(std::move(char_range)), msg_(msg) {}

            const std::string to_string() const {
                return char_range_->to_string() + " " + msg_;
            }

            nlohmann::json toJson() const;
    };


    class ErrorManager {
    private:
        bool stdout_;
        std::vector<const SyntaxError*> errors_;

    public:
        ErrorManager() : stdout_(true) {};
        ErrorManager(bool print_stdout) : stdout_(print_stdout) {};

        void registerError(const SyntaxError* error) {
            if (stdout_)
                std::cout << error->to_string() << std::endl;
            errors_.push_back(error);
        }

        void reset() {
            for (auto * entry : errors_)
                delete entry;
            errors_.clear();
        }

        ~ErrorManager() {
            reset();
        }

        const std::vector<const SyntaxError*> getSyntaxErrors() const { return errors_; }
    };

    class IEMLParserErrorListener: public ANTLRErrorListener {
        public:
            IEMLParserErrorListener(bool print_stdout = false) : error_manager_(print_stdout) {}

            void syntaxError(Recognizer *recognizer, Token *offendingSymbol, size_t line,
                             size_t charPositionInLine, const std::string &msg, std::exception_ptr e);

            void reportAmbiguity(Parser *recognizer, const dfa::DFA &dfa, size_t startIndex, size_t stopIndex, bool exact,
                                 const antlrcpp::BitSet &ambigAlts, atn::ATNConfigSet *configs);

            void reportAttemptingFullContext(Parser *recognizer, const dfa::DFA &dfa, size_t startIndex, size_t stopIndex,
                                             const antlrcpp::BitSet &conflictingAlts, atn::ATNConfigSet *configs);

            void reportContextSensitivity(Parser *recognizer, const dfa::DFA &dfa, size_t startIndex, size_t stopIndex,
                                          size_t prediction, atn::ATNConfigSet *configs);

            
            void visitorError(std::unique_ptr<ieml::AST::CharRange>&& char_range, const std::string &msg);

            const std::vector<const SyntaxError*> getSyntaxErrors() const { return error_manager_.getSyntaxErrors(); }

            nlohmann::json toJson() const;

        private:
            ErrorManager error_manager_;
    };
}}
