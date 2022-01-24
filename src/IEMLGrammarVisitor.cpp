#include "IEMLGrammarVisitor.h"

#include "ast/Declaration.h"
#include "ast/Constants.h"
#include "ast/Identifier.h"
#include "ast/Program.h"
#include "ast/Phrase.h"
#include "ast/Reference.h"
#include "ast/Word.h"
#include "ast/CategoryParadigm.h"
#include "ast/InflectedCategory.h"
#include "ast/ReferenceStringValue.h"
#include "ast/InflectionList.h"
#include "ast/InflectionListParadigm.h"
#include "ast/Auxiliary.h"
#include "ast/AuxiliaryParadigm.h"
#include "ast/Path.h"
#include "ast/Junction.h"

#define RETURN_VISITOR_RESULT_NO_ARGS(ReturnType, DerivedType) \
  return antlrcpp::Any(VisitorResult<ReturnType>(std::make_unique<DerivedType>(charRangeFromContext(ctx))));

#define RETURN_VISITOR_RESULT(ReturnType, DerivedType, ...) \
  return antlrcpp::Any(VisitorResult<ReturnType>(std::make_unique<DerivedType>(charRangeFromContext(ctx), __VA_ARGS__)));

#define RETURN_VISITOR_RESULT_MOVE(ReturnType, UNIQUE_PTR) \
  return antlrcpp::Any(VisitorResult<ReturnType>(std::move(UNIQUE_PTR)));

#define RETURN_VISITOR_RESULT_ERROR(ReturnType) \
  return antlrcpp::Any(VisitorResult<ReturnType>());


#define CHECK_SYNTAX_ERROR(ErrorListener, Context, Attribute, Message, Required) \
antlrcpp::Any t_##Attribute; \
bool valid_##Attribute = true; \
if (Context->Attribute) {\
  t_##Attribute = visit(Context->Attribute); \
  if (t_##Attribute.isNull()) { \
    ErrorListener->parseError(*charRangeFromContext(Context), Message); \
    valid_##Attribute = false; \
  }\
} else if (Required) { \
  ErrorListener->parseError(*charRangeFromContext(Context), "Missing required " #Attribute " : " Message); \
  valid_##Attribute = false; \
}

#define CHECK_SYNTAX_ERROR_LIST(ErrorListener, Context, Type, Attribute, Message) \
std::vector<std::shared_ptr<Type>> Attribute;\
__attribute__ ((unused)) bool valid_##Attribute = true; \
for (auto child: Context->Attribute) { \
  auto t_tmp = visit(child); \
  if (t_tmp.isNull()) { \
    ErrorListener->parseError(*charRangeFromContext(Context), Message); \
    valid_##Attribute = false; \
  } else { \
    auto _tmp = std::move(t_tmp.as<VisitorResult<Type>>()); \
    if (_tmp.isError())\
      valid_##Attribute = false; \
    else \
      Attribute.emplace_back(std::move(_tmp.release())); \
  }\
}


#define CAST_OR_RETURN_IF_NULL(Context, Type, Attribute, ReturnType) \
if (!valid_##Attribute) \
  RETURN_VISITOR_RESULT_ERROR(ReturnType);\
std::shared_ptr<Type> Attribute;\
if(Context->Attribute) {\
  auto _tmp = std::move(t_##Attribute.as<VisitorResult<Type>>()); \
  if (_tmp.isError())\
    RETURN_VISITOR_RESULT_ERROR(ReturnType);\
  Attribute = std::move(_tmp.release()); \
}

#define CAST_OR_RETURN_IF_NULL_LIST(Attribute, ReturnType) \
if (!valid_##Attribute) \
  RETURN_VISITOR_RESULT_ERROR(ReturnType);

namespace ieml::parser {

  std::shared_ptr<CharRange> IEMLGrammarVisitor::charRangeFromToken(antlr4::Token* token) const {
    return std::make_unique<CharRange>(file_id_, token->getLine(), token->getLine(), token->getCharPositionInLine(), token->getCharPositionInLine() + token->getText().size());
  }

  std::shared_ptr<CharRange> IEMLGrammarVisitor::charRangeFromContext(antlr4::ParserRuleContext* ctx) const {
    antlr4::Token* start = ctx->getStart();
    antlr4::Token* end   = ctx->getStop();

    size_t line_s, line_e, char_s, char_e;

    // these tests are for the case if start is after end for a empty production rule (see documentation for Token.getStart())
    if (start->getLine() < end->getLine()) {
      line_s = start->getLine();
      line_e = end->getLine();
      char_s = start->getCharPositionInLine();
      char_e = end->getCharPositionInLine() + end->getText().size();
    } else if (start->getLine() > end->getLine()) {
      line_e = start->getLine();
      line_s = end->getLine();
      char_e = start->getCharPositionInLine() + start->getText().size();
      char_s = end->getCharPositionInLine();
    } else {
      line_s = line_e = start->getLine();
      if (start->getCharPositionInLine() <= end->getCharPositionInLine()) {
        char_s = start->getCharPositionInLine();
        char_e = end->getCharPositionInLine() + end->getText().size();
      } else {
        char_e = start->getCharPositionInLine() + start->getText().size();
        char_s = end->getCharPositionInLine();
      }
    } 
    
    return std::make_unique<CharRange>(file_id_, line_s, line_e, char_s, char_e);
  }


  /**
   * PROGRAM
   */

  antlrcpp::Any IEMLGrammarVisitor::visitProgram(iemlParser::ProgramContext *ctx) {
    CHECK_SYNTAX_ERROR_LIST(error_listener_, ctx, Declaration, declarations, "Invalid declaration.");
    // CAST_OR_RETURN_IF_NULL_LIST(declarations, Program);

    RETURN_VISITOR_RESULT(Program, Program, std::move(declarations));
  }


  /**
   * DECLARATION
   */

  antlrcpp::Any IEMLGrammarVisitor::visitComponentDeclaration(iemlParser::ComponentDeclarationContext *ctx) {
    CHECK_SYNTAX_ERROR(error_listener_, ctx, phrase_, "Invalid phrase definition in component declaration.", true);
    CHECK_SYNTAX_ERROR_LIST(error_listener_, ctx, LanguageString, language_strings, "Invalid language string.");

    CAST_OR_RETURN_IF_NULL(ctx, Phrase, phrase_, Declaration);
    CAST_OR_RETURN_IF_NULL_LIST(language_strings, Declaration);

    RETURN_VISITOR_RESULT(Declaration, ComponentDeclaration, std::move(language_strings), std::move(phrase_));
  }

  antlrcpp::Any IEMLGrammarVisitor::visitNodeDeclaration(iemlParser::NodeDeclarationContext *ctx) {
    CHECK_SYNTAX_ERROR(error_listener_, ctx, phrase_, "Invalid phrase definition in node declaration.", true);
    CHECK_SYNTAX_ERROR_LIST(error_listener_, ctx, LanguageString, language_strings, "Invalid language string.");

    CAST_OR_RETURN_IF_NULL(ctx, Phrase, phrase_, Declaration);
    CAST_OR_RETURN_IF_NULL_LIST(language_strings, Declaration);

    RETURN_VISITOR_RESULT(Declaration, NodeDeclaration, std::move(language_strings), std::move(phrase_));
  }

  antlrcpp::Any IEMLGrammarVisitor::visitParanodeDeclaration(iemlParser::ParanodeDeclarationContext *ctx) {
    CHECK_SYNTAX_ERROR(error_listener_, ctx, phrase_, "Invalid phrase definition in node declaration.", true);
    CHECK_SYNTAX_ERROR_LIST(error_listener_, ctx, LanguageString, language_strings, "Invalid language string.");

    CAST_OR_RETURN_IF_NULL(ctx, Phrase, phrase_, Declaration);
    CAST_OR_RETURN_IF_NULL_LIST(language_strings, Declaration);

    RETURN_VISITOR_RESULT(Declaration, ParanodeDeclaration, std::move(language_strings), std::move(phrase_));
  }

  antlrcpp::Any IEMLGrammarVisitor::visitWordDeclaration(iemlParser::WordDeclarationContext *ctx) {    
    CHECK_SYNTAX_ERROR(error_listener_, ctx, word_, "Invalid word for a word declaration.", true);

    CAST_OR_RETURN_IF_NULL(ctx, Word, word_, Declaration);

    RETURN_VISITOR_RESULT(Declaration, WordDeclaration, std::move(word_));
  }

  antlrcpp::Any IEMLGrammarVisitor::visitInflectionDeclaration(iemlParser::InflectionDeclarationContext *ctx) {
    CHECK_SYNTAX_ERROR_LIST(error_listener_, ctx, LanguageString, language_strings, "Invalid language string.");

    std::shared_ptr<Identifier> inflection_type;
    if(!ctx->inflection_type) 
      error_listener_->parseError(*charRangeFromContext(ctx), "Invalid inflection type for an inflection declaration. Expected 'NOUN' or 'VERB'.");
    else 
      inflection_type = std::make_unique<Identifier>(charRangeFromToken(ctx->inflection_type), ctx->inflection_type->getText());
    
    CHECK_SYNTAX_ERROR(error_listener_, ctx, word_, "Invalid word for an inflection declaration.", true);

    CAST_OR_RETURN_IF_NULL(ctx, Word, word_, Declaration);
    if (inflection_type == nullptr) RETURN_VISITOR_RESULT_ERROR(Declaration);
    CAST_OR_RETURN_IF_NULL_LIST(language_strings, Declaration);

    RETURN_VISITOR_RESULT(Declaration, 
                          InflectionDeclaration,
                          std::move(language_strings),
                          std::move(inflection_type),
                          std::move(word_));
  }

  antlrcpp::Any IEMLGrammarVisitor::visitAuxiliaryDeclaration(iemlParser::AuxiliaryDeclarationContext *ctx) {
    CHECK_SYNTAX_ERROR_LIST(error_listener_, ctx, LanguageString, language_strings, "Invalid language string.");

    int accepted_role_type = -1;
    if (!ctx->role_type)
      error_listener_->parseError(*charRangeFromContext(ctx), "Invalid role number for an auxiliary declaration.");
    else
      accepted_role_type = std::stoi(ctx->role_type->getText());

    if (accepted_role_type == -1)
      RETURN_VISITOR_RESULT_ERROR(Declaration);

    CHECK_SYNTAX_ERROR(error_listener_, ctx, word_, "Invalid word for an auxiliary declaration.", true);

    CAST_OR_RETURN_IF_NULL(ctx, Word, word_, Declaration);
    CAST_OR_RETURN_IF_NULL_LIST(language_strings, Declaration);

    RETURN_VISITOR_RESULT(Declaration, 
                          AuxiliaryDeclaration,
                          std::move(language_strings),
                          accepted_role_type,
                          std::move(word_));
  }

  antlrcpp::Any IEMLGrammarVisitor::visitJunctionDeclaration(iemlParser::JunctionDeclarationContext *ctx) {
    CHECK_SYNTAX_ERROR_LIST(error_listener_, ctx, LanguageString, language_strings, "Invalid language string.");

    CHECK_SYNTAX_ERROR(error_listener_, ctx, word_, "Invalid word for a junction declaration.", true);

    CAST_OR_RETURN_IF_NULL(ctx, Word, word_, Declaration);
    CAST_OR_RETURN_IF_NULL_LIST(language_strings, Declaration);
 
    RETURN_VISITOR_RESULT(Declaration, 
                          JunctionDeclaration,
                          std::move(language_strings),
                          std::move(word_));
  }

  antlrcpp::Any IEMLGrammarVisitor::visitLanguageDeclaration(iemlParser::LanguageDeclarationContext *ctx) {
    CHECK_SYNTAX_ERROR(error_listener_, ctx, language, "Invalid language type.", true);

    CAST_OR_RETURN_IF_NULL(ctx, Identifier, language, LanguageDeclaration);

    RETURN_VISITOR_RESULT(Declaration, 
                          LanguageDeclaration,
                          std::move(language));
  }

  /**
   * PHRASE
   */

  antlrcpp::Any IEMLGrammarVisitor::visitPhrase__lines(iemlParser::Phrase__linesContext *ctx) {
    CHECK_SYNTAX_ERROR_LIST(error_listener_, ctx, PhraseLine, phrase_lines, "Invalid phrase lines.");    
    CAST_OR_RETURN_IF_NULL_LIST(phrase_lines, Phrase);

    RETURN_VISITOR_RESULT(Phrase, SimplePhrase, std::move(phrase_lines));
  }

  antlrcpp::Any IEMLGrammarVisitor::visitPhrase__junction(iemlParser::Phrase__junctionContext *ctx) {
    CHECK_SYNTAX_ERROR_LIST(error_listener_, ctx, Phrase, phrases, "Invalid phrases in phrase junction.");
    CHECK_SYNTAX_ERROR(error_listener_, ctx, junction_, "Invalid junction identifier in phrase junction.", true);

    CAST_OR_RETURN_IF_NULL_LIST(phrases, Phrase);
    CAST_OR_RETURN_IF_NULL(ctx, IJunction, junction_, Phrase);

    RETURN_VISITOR_RESULT(Phrase, JunctionPhrase, std::move(phrases), std::move(junction_));
  }


  /**
   * PHRASELINE
   */

  antlrcpp::Any IEMLGrammarVisitor::visitPhrase_line__sub_phrase_line_auxiliary(iemlParser::Phrase_line__sub_phrase_line_auxiliaryContext *ctx) {
    int role_type = -1;
    if (!ctx->role_type)
      error_listener_->parseError(*charRangeFromContext(ctx), "Invalid role number.");
    else
      role_type = std::stoi(ctx->INTEGER()->getSymbol()->getText());
    
    CHECK_SYNTAX_ERROR(error_listener_, ctx, sub_phrase, "Invalid sub phrase line : invalid auxiliary, inflection, category or references.", true);

    CAST_OR_RETURN_IF_NULL(ctx, AuxiliarySubPhraseLine, sub_phrase, PhraseLine);
    
    if (role_type == -1)
      RETURN_VISITOR_RESULT_ERROR(PhraseLine);

    bool accentuation = ctx->accentuation;

    RETURN_VISITOR_RESULT(PhraseLine, SimplePhraseLine, role_type, accentuation, std::move(sub_phrase))
  }

  antlrcpp::Any IEMLGrammarVisitor::visitPhrase_line__jonction_auxiliary(iemlParser::Phrase_line__jonction_auxiliaryContext *ctx) {
    int role_type = -1;
    if (!ctx->role_type)
      error_listener_->parseError(*charRangeFromContext(ctx), "Invalid role number.");
    else
      role_type = std::stoi(ctx->role_type->getText());

    CHECK_SYNTAX_ERROR_LIST(error_listener_, ctx, AuxiliarySubPhraseLine, sub_phrases, "Invalid sub phrase in phrase line junction.");
    CHECK_SYNTAX_ERROR(error_listener_, ctx, junction_, "Invalid junction identifier in phrase line junction.", true);

    bool accentuation = ctx->accentuation;

    CAST_OR_RETURN_IF_NULL_LIST(sub_phrases, PhraseLine);
    CAST_OR_RETURN_IF_NULL(ctx, IJunction, junction_, PhraseLine);

    if (role_type == -1)
      RETURN_VISITOR_RESULT_ERROR(PhraseLine);
    
    RETURN_VISITOR_RESULT(PhraseLine, JunctionPhraseLine, std::move(sub_phrases), std::move(junction_), role_type, accentuation);
  }

  /**
   * INFLEXION LIST
   */
  antlrcpp::Any IEMLGrammarVisitor::visitInflection_list(iemlParser::Inflection_listContext *ctx) {
    CHECK_SYNTAX_ERROR_LIST(error_listener_, ctx, Identifier, inflections, "Invalid inflection identifiers in phrase line.");
    CAST_OR_RETURN_IF_NULL_LIST(inflections, InflectionList);

    RETURN_VISITOR_RESULT(InflectionList, InflectionList, std::move(inflections));
  }
  antlrcpp::Any IEMLGrammarVisitor::visitInflection_list_paradigm(iemlParser::Inflection_list_paradigmContext *ctx) {
    CHECK_SYNTAX_ERROR_LIST(error_listener_, ctx, InflectionList, inflection_lists, "Invalid inflection list in inflection paradigm.");
    CAST_OR_RETURN_IF_NULL_LIST(inflection_lists, IInflectionList);

    RETURN_VISITOR_RESULT(IInflectionList, InflectionListParadigm, std::move(inflection_lists));
  }

  /**
   * AUXILIARY
   */  
  antlrcpp::Any IEMLGrammarVisitor::visitAuxiliary__identifier(iemlParser::Auxiliary__identifierContext *ctx) {
    CHECK_SYNTAX_ERROR(error_listener_, ctx, identifier_, "Invalid auxiliary identifier in a phrase line or a path.", true);
    CAST_OR_RETURN_IF_NULL(ctx, Identifier, identifier_, Auxiliary);
    
    RETURN_VISITOR_RESULT(IAuxiliary, Auxiliary, std::move(identifier_));
  }

  antlrcpp::Any IEMLGrammarVisitor::visitAuxiliary__word(iemlParser::Auxiliary__wordContext *ctx) {
    CHECK_SYNTAX_ERROR(error_listener_, ctx, word_, "Invalid auxiliary word in a phrase line or a path.", true);
    CAST_OR_RETURN_IF_NULL(ctx, Word, word_, Auxiliary);
    
    RETURN_VISITOR_RESULT(IAuxiliary, WordAuxiliary, std::move(word_));
  }

  antlrcpp::Any IEMLGrammarVisitor::visitAuxiliary_paradigm(iemlParser::Auxiliary_paradigmContext *ctx) {
    CHECK_SYNTAX_ERROR_LIST(error_listener_, ctx, IAuxiliary, identifiers, "Invalid auxiliary in an auxiliary paradigm.");
    CAST_OR_RETURN_IF_NULL_LIST(identifiers, IAuxiliary);

    RETURN_VISITOR_RESULT(IAuxiliary, AuxiliaryParadigm, std::move(identifiers));
  }

  /**
   * AUXILIARY SIMPLE OR PARADIGM
   */  
  antlrcpp::Any IEMLGrammarVisitor::visitAuxiliary_simple_or_paradigm__simple(iemlParser::Auxiliary_simple_or_paradigm__simpleContext *ctx) {
    CHECK_SYNTAX_ERROR(error_listener_, ctx, auxiliary_, "Invalid auxiliary.", true);
    CAST_OR_RETURN_IF_NULL(ctx, IAuxiliary, auxiliary_, IAuxiliary);

    RETURN_VISITOR_RESULT_MOVE(IAuxiliary, auxiliary_);
  }

  antlrcpp::Any IEMLGrammarVisitor::visitAuxiliary_simple_or_paradigm__paradigm(iemlParser::Auxiliary_simple_or_paradigm__paradigmContext *ctx) {
    CHECK_SYNTAX_ERROR(error_listener_, ctx, auxiliary_, "Invalid auxiliary paradigm.", true);
    CAST_OR_RETURN_IF_NULL(ctx, IAuxiliary, auxiliary_, IAuxiliary);

    RETURN_VISITOR_RESULT_MOVE(IAuxiliary, auxiliary_);
  }

  /**
   * SUB PHRASELINE with AUXILIARY
   */

  antlrcpp::Any IEMLGrammarVisitor::visitSub_phrase_line_auxiliary__no_junction(iemlParser::Sub_phrase_line_auxiliary__no_junctionContext *ctx) {
    CHECK_SYNTAX_ERROR(error_listener_, ctx, auxiliary_, "Invalid auxiliary identifier.", false);
    CHECK_SYNTAX_ERROR(error_listener_, ctx, inflected_category_, "Missing a category : an identifier, a phrase or a word.", true);

    CAST_OR_RETURN_IF_NULL(ctx, InflectedCategory, inflected_category_, AuxiliarySubPhraseLine);
    CAST_OR_RETURN_IF_NULL(ctx, IAuxiliary, auxiliary_, AuxiliarySubPhraseLine);
    
    RETURN_VISITOR_RESULT(AuxiliarySubPhraseLine, SimpleAuxiliarySubPhraseLine, std::move(auxiliary_), std::move(inflected_category_));
  }

  antlrcpp::Any IEMLGrammarVisitor::visitSub_phrase_line_auxiliary__jonction(iemlParser::Sub_phrase_line_auxiliary__jonctionContext *ctx) {
    CHECK_SYNTAX_ERROR(error_listener_, ctx, auxiliary_, "Invalid auxiliary identifier.", true);
    CHECK_SYNTAX_ERROR_LIST(error_listener_, ctx, InflectedCategory, inflected_categories, "Invalid inflected categories in auxiliarized phrase line junction.");
    CHECK_SYNTAX_ERROR(error_listener_, ctx, junction_, "Invalid junction type identifier.", true);
    
    CAST_OR_RETURN_IF_NULL(ctx, IAuxiliary, auxiliary_, AuxiliarySubPhraseLine);
    CAST_OR_RETURN_IF_NULL(ctx, IJunction, junction_, AuxiliarySubPhraseLine);
    CAST_OR_RETURN_IF_NULL_LIST(inflected_categories, AuxiliarySubPhraseLine)

    RETURN_VISITOR_RESULT(AuxiliarySubPhraseLine, JunctionAuxiliarySubPhraseLine, std::move(auxiliary_), std::move(inflected_categories), std::move(junction_));
  }


  /**
   * INFLEXED CATEGORY (SUB PHRASELINE without AUXILIARY)
   */
  antlrcpp::Any IEMLGrammarVisitor::visitInflected_category__singular(iemlParser::Inflected_category__singularContext *ctx) {
    CHECK_SYNTAX_ERROR(error_listener_, ctx, inflection_list_, "Invalid inflection list in inflected category.", false);
    CHECK_SYNTAX_ERROR(error_listener_, ctx, category_, "Missing a category : an identifier, a phrase or a word.", true);
    CHECK_SYNTAX_ERROR_LIST(error_listener_, ctx, Reference, references, "Invalid reference.");

    CAST_OR_RETURN_IF_NULL(ctx, InflectionList, inflection_list_, InflectedCategory);
    CAST_OR_RETURN_IF_NULL(ctx, ICategory, category_, InflectedCategory);
    CAST_OR_RETURN_IF_NULL_LIST(references, InflectedCategory);

    RETURN_VISITOR_RESULT(InflectedCategory, InflectedCategory, std::move(inflection_list_), std::move(category_), std::move(references));
  }
  antlrcpp::Any IEMLGrammarVisitor::visitInflected_category__category_paradigm(iemlParser::Inflected_category__category_paradigmContext *ctx) {
    CHECK_SYNTAX_ERROR(error_listener_, ctx, inflection_list_, "Invalid inflection list in inflected category.", false);
    CHECK_SYNTAX_ERROR(error_listener_, ctx, category_, "Missing a substitution list of category.", true);
    
    CAST_OR_RETURN_IF_NULL(ctx, InflectionList, inflection_list_, InflectedCategory);
    CAST_OR_RETURN_IF_NULL(ctx, CategoryParadigm, category_, InflectedCategory);

    RETURN_VISITOR_RESULT(InflectedCategory, InflectedCategory, std::move(inflection_list_), std::move(category_), std::vector<std::shared_ptr<Reference>>{});
  }
  antlrcpp::Any IEMLGrammarVisitor::visitInflected_category__inflection_paradigm(iemlParser::Inflected_category__inflection_paradigmContext *ctx) {
    CHECK_SYNTAX_ERROR(error_listener_, ctx, inflection_list_, "Invalid inflection list in inflected category.", true);
    CHECK_SYNTAX_ERROR(error_listener_, ctx, category_, "Missing a substitution list of category.", true);
    
    CAST_OR_RETURN_IF_NULL(ctx, IInflectionList, inflection_list_, InflectedCategory);
    CAST_OR_RETURN_IF_NULL(ctx, ICategory, category_, InflectedCategory);

    RETURN_VISITOR_RESULT(InflectedCategory, InflectedCategory, std::move(inflection_list_), std::move(category_), std::vector<std::shared_ptr<Reference>>{});
  }

  antlrcpp::Any IEMLGrammarVisitor::visitInflected_category__inflection_and_category_paradigm(iemlParser::Inflected_category__inflection_and_category_paradigmContext *ctx) {
    CHECK_SYNTAX_ERROR(error_listener_, ctx, inflection_list_, "Invalid inflection list in inflected category.", true);
    CHECK_SYNTAX_ERROR(error_listener_, ctx, category_, "Missing a substitution list of category.", true);
    
    CAST_OR_RETURN_IF_NULL(ctx, IInflectionList, inflection_list_, InflectedCategory);
    CAST_OR_RETURN_IF_NULL(ctx, CategoryParadigm, category_, InflectedCategory);

    RETURN_VISITOR_RESULT(InflectedCategory, InflectedCategory, std::move(inflection_list_), std::move(category_), std::vector<std::shared_ptr<Reference>>{});
  }


  /**
   *  CATEGORY
   */

  antlrcpp::Any IEMLGrammarVisitor::visitCategory__identifier(iemlParser::Category__identifierContext *ctx) {
    CHECK_SYNTAX_ERROR(error_listener_, ctx, identifier_, "Invalid identifier for a category.", true);
    CAST_OR_RETURN_IF_NULL(ctx, Identifier, identifier_, ICategory);

    RETURN_VISITOR_RESULT_MOVE(ICategory, identifier_);
  }

  antlrcpp::Any IEMLGrammarVisitor::visitCategory__phrase(iemlParser::Category__phraseContext *ctx) {
    CHECK_SYNTAX_ERROR(error_listener_, ctx, phrase_, "Invalid phrase for a category.", true);
    CAST_OR_RETURN_IF_NULL(ctx, Phrase, phrase_, ICategory);

    RETURN_VISITOR_RESULT_MOVE(ICategory, phrase_);
  }

  antlrcpp::Any IEMLGrammarVisitor::visitCategory__word(iemlParser::Category__wordContext *ctx) {
    CHECK_SYNTAX_ERROR(error_listener_, ctx, word_, "Invalid word for a category.", true);
    CAST_OR_RETURN_IF_NULL(ctx, Word, word_, ICategory);
    RETURN_VISITOR_RESULT_MOVE(ICategory, word_);
  }

  antlrcpp::Any IEMLGrammarVisitor::visitCategory_paradigm(iemlParser::Category_paradigmContext *ctx) {
    CHECK_SYNTAX_ERROR_LIST(error_listener_, ctx, ICategory, categories, "Invalid category list for a paradigm declaration.");

    CAST_OR_RETURN_IF_NULL_LIST(categories, CategoryParadigm);

    RETURN_VISITOR_RESULT(CategoryParadigm, CategoryParadigm, std::move(categories));
  }

  /**
   *  WORD
   */

  antlrcpp::Any IEMLGrammarVisitor::visitWord(iemlParser::WordContext *ctx) {
    if (!ctx->word_) {
      error_listener_->parseError(*charRangeFromContext(ctx), "Invalid word for a category.");
      RETURN_VISITOR_RESULT_ERROR(Word);
    }

    std::string word_str_quoted = ctx->word_->getText();
    std::string word_str = word_str_quoted.substr(1, word_str_quoted.size() - 2);
    
    RETURN_VISITOR_RESULT(Word, Word, word_str);
  }

  /**
   *  LANGUAGE STRING
   */

  antlrcpp::Any IEMLGrammarVisitor::visitLanguage_string(iemlParser::Language_stringContext *ctx) {
    CHECK_SYNTAX_ERROR(error_listener_, ctx, value, "Invalid identifier value for language string.", true);
    if (!ctx->language) {
      error_listener_->parseError(*charRangeFromContext(ctx), "Invalid language identifier for language string.");
      RETURN_VISITOR_RESULT_ERROR(LanguageString);
    }

    std::shared_ptr<Identifier> language = std::make_shared<Identifier>(charRangeFromToken(ctx->language), 
                                                                        ctx->language->getText().substr(0, 2));
    CAST_OR_RETURN_IF_NULL(ctx, Identifier, value, LanguageString)

    RETURN_VISITOR_RESULT(LanguageString, LanguageString, std::move(language), std::move(value))
  }


  /**
   *  IDENTIFIER
   */

  antlrcpp::Any IEMLGrammarVisitor::visitIdentifier(iemlParser::IdentifierContext *ctx) {
    if (ctx->identifiers.empty()) {
      error_listener_->parseError(*charRangeFromContext(ctx), "Invalid identifier : empty identifier.");
      RETURN_VISITOR_RESULT_ERROR(Identifier);
    }

    std::ostringstream os;
    bool first = true;
    for (antlr4::Token* token : ctx->identifiers) {
      if (first) first = false;
      else os << " ";
      
      os << token->getText();
    }

    RETURN_VISITOR_RESULT(Identifier, Identifier, os.str());
  }
  /**
   * JUNCTION
   */
  antlrcpp::Any IEMLGrammarVisitor::visitJunction(iemlParser::JunctionContext *ctx) {
    CHECK_SYNTAX_ERROR(error_listener_, ctx, junction_type, "Invalid identifier for a junction.", true);
    CAST_OR_RETURN_IF_NULL(ctx, Identifier, junction_type, IJunction);
    RETURN_VISITOR_RESULT(IJunction, Junction, std::move(junction_type));
  }

  /**
   * PATH
   */
  antlrcpp::Any IEMLGrammarVisitor::visitPath(iemlParser::PathContext *ctx) {
    CHECK_SYNTAX_ERROR_LIST(error_listener_, ctx, PathNode, path_nodes, "Invalid path node list for a path definition.");
    CAST_OR_RETURN_IF_NULL_LIST(path_nodes, Path);
    RETURN_VISITOR_RESULT(Path, Path, std::move(path_nodes));
  }

  /**
   * PATH NODE
   */
  antlrcpp::Any IEMLGrammarVisitor::visitPath_node__root(iemlParser::Path_node__rootContext *ctx) {
    RETURN_VISITOR_RESULT_NO_ARGS(PathNode, RootPathNode);
  }

  antlrcpp::Any IEMLGrammarVisitor::visitPath_node__paradigm(iemlParser::Path_node__paradigmContext *ctx) {  
    if (!ctx->INTEGER()) {
      error_listener_->parseError(*charRangeFromContext(ctx), "Invalid paradigm index.");
      RETURN_VISITOR_RESULT_ERROR(PathNode);
    } 
    size_t index = std::stoi(ctx->INTEGER()->getText());
    RETURN_VISITOR_RESULT(PathNode, ParadigmPathNode, index);
  }

  antlrcpp::Any IEMLGrammarVisitor::visitPath_node__auxiliary(iemlParser::Path_node__auxiliaryContext *ctx) {
    CHECK_SYNTAX_ERROR(error_listener_, ctx, auxiliary_, "Invalid auxiliary for a path definition.", true);
    CAST_OR_RETURN_IF_NULL(ctx, IAuxiliary, auxiliary_, PathNode);
    RETURN_VISITOR_RESULT(PathNode, AuxiliaryPathNode, std::move(auxiliary_));
  }

  antlrcpp::Any IEMLGrammarVisitor::visitPath_node__inflection(iemlParser::Path_node__inflectionContext *ctx) {
    CHECK_SYNTAX_ERROR(error_listener_, ctx, inflection_list_, "Invalid inflection list for a path definition.", true);
    CAST_OR_RETURN_IF_NULL(ctx, InflectionList, inflection_list_, PathNode);
    RETURN_VISITOR_RESULT(PathNode, InflectionListPathNode, std::move(inflection_list_));
  }

  antlrcpp::Any IEMLGrammarVisitor::visitPath_node__word(iemlParser::Path_node__wordContext *ctx) {
    CHECK_SYNTAX_ERROR(error_listener_, ctx, word_, "Invalid word for a path definition.", true);
    CAST_OR_RETURN_IF_NULL(ctx, Word, word_, PathNode);
    RETURN_VISITOR_RESULT(PathNode, WordPathNode, std::move(word_));
  }

  antlrcpp::Any IEMLGrammarVisitor::visitPath_node__junction(iemlParser::Path_node__junctionContext *ctx) {
    CHECK_SYNTAX_ERROR(error_listener_, ctx, junction_, "Invalid junction identifier for a path definition.", true);
    
    if (!ctx->INTEGER()) {
      error_listener_->parseError(*charRangeFromContext(ctx), "Invalid junction index.");
      RETURN_VISITOR_RESULT_ERROR(PathNode);
    } 
    size_t index = std::stoi(ctx->INTEGER()->getText());

    CAST_OR_RETURN_IF_NULL(ctx, IJunction, junction_, PathNode);
    RETURN_VISITOR_RESULT(PathNode, JunctionPathNode, std::move(junction_), index);
  }

  antlrcpp::Any IEMLGrammarVisitor::visitPath_node__role_number(iemlParser::Path_node__role_numberContext *ctx) {
    if (!ctx->INTEGER()) {
      error_listener_->parseError(*charRangeFromContext(ctx), "Invalid role number");
      RETURN_VISITOR_RESULT_ERROR(PathNode);
    } 
    size_t role_number = std::stoi(ctx->INTEGER()->getText());

    RETURN_VISITOR_RESULT(PathNode, RoleNumberPathNode, role_number);
  }

  /**
   *  REFERENCE
   */

  antlrcpp::Any IEMLGrammarVisitor::visitReference(iemlParser::ReferenceContext *ctx) {
    std::shared_ptr<int> ref_id = nullptr;
    if (ctx->id) {
      int i = std::stoi(ctx->id->getText());
      ref_id = std::move(std::make_unique<int>(i));
    }

    CHECK_SYNTAX_ERROR(error_listener_, ctx, value, "Invalid value in reference.", true);

    if (!ctx->data_type) {
      error_listener_->parseError(*charRangeFromContext(ctx), "Invalid datatype identifier.");
      RETURN_VISITOR_RESULT_ERROR(Reference);
    }

    CAST_OR_RETURN_IF_NULL(ctx, IReferenceValue, value, Reference);
    auto data_type = std::move(std::make_unique<Identifier>(charRangeFromToken(ctx->data_type), ctx->data_type->getText()));

    RETURN_VISITOR_RESULT(Reference, Reference, std::move(ref_id), std::move(data_type), std::move(value));
  }


  /**
   *  REFERENCE VALUE
   */

  antlrcpp::Any IEMLGrammarVisitor::visitReference_value__identifier(iemlParser::Reference_value__identifierContext *ctx) {
    CHECK_SYNTAX_ERROR(error_listener_, ctx, identifier_, "Invalid identifier for reference value.", true);
    CAST_OR_RETURN_IF_NULL(ctx, Identifier, identifier_, IReferenceValue);

    RETURN_VISITOR_RESULT_MOVE(IReferenceValue, identifier_);
  }

  antlrcpp::Any IEMLGrammarVisitor::visitReference_value__STRING(iemlParser::Reference_value__STRINGContext *ctx) {
    if (!ctx->value) {
      error_listener_->parseError(*charRangeFromContext(ctx), "Invalid string for reference value.");
      RETURN_VISITOR_RESULT_ERROR(IReferenceValue);
    }

    RETURN_VISITOR_RESULT(IReferenceValue, ReferenceStringValue, ctx->value->getText());
  }

  antlrcpp::Any IEMLGrammarVisitor::visitReference_value__phrase(iemlParser::Reference_value__phraseContext *ctx) {
    CHECK_SYNTAX_ERROR(error_listener_, ctx, phrase_, "Invalid phrase for reference value.", true);
    CAST_OR_RETURN_IF_NULL(ctx, Phrase, phrase_, IReferenceValue);

    RETURN_VISITOR_RESULT_MOVE(IReferenceValue, phrase_);
  }


}