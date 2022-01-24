#pragma once

#include <memory>
#include <string>

#include "structure/Word.h"
#include "structure/Namespace.h"

namespace ieml::structure {
class WordRegister {
public:
    /**********************************
     * WordRegister: Word
     **********************************/
    bool word_is_defined(std::shared_ptr<structure::Word> word) {
        return defined_words_.count(word->getScript()) > 0;
    }

    std::shared_ptr<structure::Word> get_word(const std::string& s) const {
        auto r = defined_words_.find(s);
        if (r == defined_words_.end()) return nullptr;
        return r->second;
    }

    std::shared_ptr<structure::Name> getName(const std::shared_ptr<structure::AuxiliaryWord>& word) const {
        return namespace_auxiliary_.find(word)->second;
    };

    std::shared_ptr<structure::Name> getName(const std::shared_ptr<structure::InflectionWord>& word) const {
        return namespace_inflection_.find(word)->second;
    };

    std::shared_ptr<structure::Name> getName(const std::shared_ptr<structure::JunctionWord>& word) const {
        return namespace_junction_.find(word)->second;
    };

    std::shared_ptr<structure::Name> getName(const std::shared_ptr<structure::Word>& word) const {
        switch (word->getWordType())
        {
        case ieml::structure::WordType::AUXILIARY:
            return namespace_auxiliary_.find(std::dynamic_pointer_cast<structure::AuxiliaryWord>(word))->second;
        case ieml::structure::WordType::JUNCTION:
            return namespace_junction_.find(std::dynamic_pointer_cast<structure::JunctionWord>(word))->second;
        case ieml::structure::WordType::INFLECTION:
            return namespace_inflection_.find(std::dynamic_pointer_cast<structure::InflectionWord>(word))->second;
        default:
            return nullptr;
        }
    };


    /**********************************
     * WordRegister: Auxiliary Words
     **********************************/
    void define_auxiliary(std::shared_ptr<structure::Name> name, std::shared_ptr<structure::AuxiliaryWord> word) {
        if (defined_words_.count(word->getScript()) > 0)
            throw std::invalid_argument("Word already defined.");

        defined_words_.insert({word->getScript(), word});
        namespace_auxiliary_.define(name, word);
    }
    std::shared_ptr<structure::AuxiliaryWord> resolve_auxiliary(const structure::LanguageString& s) const {
        return namespace_auxiliary_.resolve(s);
    }
    

    /**********************************
     * WordRegister: Inflection Words
     **********************************/
    void define_inflection(std::shared_ptr<structure::Name> name, std::shared_ptr<structure::InflectionWord> word) {
        if (defined_words_.count(word->getScript()) > 0)
            throw std::invalid_argument("Word already defined.");

        defined_words_.insert({word->getScript(), word});
        namespace_inflection_.define(name, word);
    }
    std::shared_ptr<structure::InflectionWord> resolve_inflection(const structure::LanguageString& s) const {
        return namespace_inflection_.resolve(s);
    }
    
    /**********************************
     * WordRegister: Junction Words
     **********************************/
    void define_junction(std::shared_ptr<structure::Name> name, std::shared_ptr<structure::JunctionWord> word) {
        if (defined_words_.count(word->getScript()) > 0)
            throw std::invalid_argument("Word already defined.");

        defined_words_.insert({word->getScript(), word});
        namespace_junction_.define(name, word);
    }
    std::shared_ptr<structure::JunctionWord> resolve_junction(const structure::LanguageString& s) const {
        return namespace_junction_.resolve(s);
    }

    /**********************************
     * WordRegister: Category Words
     **********************************/
    void define_word(std::shared_ptr<structure::CategoryWord> word) {
        if (defined_words_.count(word->getScript()) > 0)
            throw std::invalid_argument("Word already defined.");

        defined_words_.insert({word->getScript(), word});
        caterory_words_.insert({word->getScript(), word});
    }
    std::shared_ptr<structure::CategoryWord> resolve_category_word(const std::string& s) const {
        auto res = caterory_words_.find(s);
        if (res == caterory_words_.end()) {
            return nullptr;
        }

        return res->second;
    }

    typedef std::unordered_map<std::shared_ptr<structure::AuxiliaryWord>, std::shared_ptr<Name>>::const_iterator  const_iterator_auxiliary;
    const_iterator_auxiliary auxiliaries_begin() const {return namespace_auxiliary_.begin();};
    const_iterator_auxiliary auxiliaries_end()   const {return namespace_auxiliary_.end();};

    typedef std::unordered_map<std::shared_ptr<structure::InflectionWord>, std::shared_ptr<Name>>::const_iterator  const_iterator_inflection;
    const_iterator_inflection inflections_begin() const {return namespace_inflection_.begin();};
    const_iterator_inflection inflections_end()   const {return namespace_inflection_.end();};

    typedef std::unordered_map<std::shared_ptr<structure::JunctionWord>, std::shared_ptr<Name>>::const_iterator  const_iterator_junction;
    const_iterator_junction junctions_begin() const {return namespace_junction_.begin();};
    const_iterator_junction junctions_end()   const {return namespace_junction_.end();};

    typedef std::unordered_map<std::string, std::shared_ptr<structure::CategoryWord>>::const_iterator  const_iterator_category_word;
    const_iterator_category_word category_word_begin() const {return caterory_words_.begin();};
    const_iterator_category_word category_word_end()   const {return caterory_words_.end();};


private:
    structure::Namespace<structure::AuxiliaryWord> namespace_auxiliary_;
    structure::Namespace<structure::InflectionWord> namespace_inflection_;
    structure::Namespace<structure::JunctionWord> namespace_junction_;

    std::unordered_map<std::string, std::shared_ptr<structure::Word>> defined_words_;
    
    std::unordered_map<std::string, std::shared_ptr<structure::CategoryWord>> caterory_words_;
};
}
