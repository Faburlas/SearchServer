#include <cmath>
#include <iostream>
#include <stdexcept>
#include <string>
#include <execution>
#include "search_server.h"


SearchServer::SearchServer(const std::string& stop_words_text)
    : SearchServer(SplitIntoWords(stop_words_text))
{
}

bool SearchServer::IsValidWord(const std::string_view& word) {
    return std::none_of(word.begin(), word.end(), [](char c) {
        return c >= '\0' && c < ' ';
        });
}

void SearchServer::AddDocument(int document_id, const std::string_view document, DocumentStatus status,
    const std::vector<int>& ratings) {
    if ((document_id < 0) || (documents_.count(document_id) > 0)) {
        throw std::invalid_argument("Invalid document id");
    }

    const auto words = SplitIntoWordsNoStop(document);

    const double inv_word_count = 1.0 / words.size();
    for (const std::string_view& word : words) {
        std::string temp{ word };
        word_to_document_freqs_[temp][document_id] += inv_word_count;
        document_to_word_freqs_[document_id][temp] += inv_word_count;
    }
    documents_.emplace(document_id, DocumentData{ ComputeAverageRating(ratings), status });
    document_ids_.push_back(document_id);
}

std::vector<Document> SearchServer::FindTopDocuments(const std::string_view& raw_query, DocumentStatus status) const {
    return FindTopDocuments(raw_query, [status](int document_id, DocumentStatus document_status, int rating) {
        return document_status == status;
        });
}

std::vector<Document> SearchServer::FindTopDocuments(const std::string_view& raw_query) const {
    return FindTopDocuments(raw_query, DocumentStatus::ACTUAL);
}

int SearchServer::GetDocumentCount() const {
    return documents_.size();
}

int SearchServer::GetDocumentId(int index) const {
    return document_ids_.at(index);
}

const std::vector<int>::iterator SearchServer::begin() {
    return document_ids_.begin();
}

const std::vector<int>::iterator SearchServer::end() {
    return document_ids_.end();
}

std::tuple<std::vector<std::string_view>, DocumentStatus> SearchServer::MatchDocument(std::string_view raw_query, int document_id) const {
    return MatchDocument(std::execution::seq, raw_query, document_id);
}

std::tuple<std::vector<std::string_view>, DocumentStatus> SearchServer::MatchDocument(const std::execution::sequenced_policy&, std::string_view raw_query, int document_id) const {
    Query query;
    query = ParseQuery(raw_query);
    std::vector<std::string_view> matched_words;
    for (const std::string_view word : query.plus_words) {
        std::string temp{word};
        if (word_to_document_freqs_.count(temp) == 0) {
            continue;
        }
        if (word_to_document_freqs_.at(temp).count(document_id)) {
            matched_words.push_back(word);
        }
    }
    for (const std::string_view word : query.minus_words) {
        std::string temp{word};
        if (word_to_document_freqs_.count(temp) == 0) {
            continue;
        }
        if (word_to_document_freqs_.at(temp).count(document_id)) {
            matched_words.clear();
            break;
        }
    }
    return { matched_words, documents_.at(document_id).status };
}

std::tuple<std::vector<std::string_view>, DocumentStatus> SearchServer::MatchDocument(const std::execution::parallel_policy&, std::string_view raw_query, int document_id) const {

    std::string from_raw_query{ raw_query.begin(), raw_query.end() };
    SearchServer::Query temp_words_ = ParseQuery(from_raw_query);
    std::vector<std::string_view> matched_words;
    std::for_each(std::execution::par , temp_words_.plus_words.begin(), temp_words_.plus_words.end(), [this, document_id, &matched_words](const std::string_view& word) {
        std::string temp{word};
        if (word_to_document_freqs_.count(temp) == 0) {
            return;
        }
        if (word_to_document_freqs_.at(temp).count(document_id)) {
            matched_words.push_back(word);
        }
        });
    std::for_each(std::execution::par, temp_words_.minus_words.begin(), temp_words_.minus_words.end(), [this, document_id, &matched_words](const std::string_view& word) {
        std::string temp{ word };
        if (word_to_document_freqs_.count(temp) == 0) {
            return;
        }
        if (word_to_document_freqs_.at(temp).count(document_id)) {
            matched_words.clear();
            return;
        }
        });
    return { matched_words, documents_.at(document_id).status };
}


bool SearchServer::IsStopWord(const std::string_view& word) const {
    std::string temp{word};
    return stop_words_.count(temp) > 0;
}

const std::vector<std::string_view> SearchServer::SplitIntoWordsNoStop(const std::string_view& text) const {
    std::vector<std::string_view> words;
    for (const std::string_view& word : SplitIntoWords(text)) {
        if (!IsValidWord(word)) {
            throw std::invalid_argument("Word is invalid");
        }
        if (!IsStopWord(word)) {
            words.push_back(word);
        }
    }
    return words;
}

int SearchServer::ComputeAverageRating(const std::vector<int>& ratings) {
    if (ratings.empty()) {
        return 0;
    }
    int rating_sum = 0;
    for (const int rating : ratings) {
        rating_sum += rating;
    }
    return rating_sum / static_cast<int>(ratings.size());
}

SearchServer::QueryWord SearchServer::ParseQueryWord(std::string_view text) const {
    if (text.empty()) {
        throw std::invalid_argument("Word is invalid");
    }
    bool is_minus = false;
    if (text[0] == '-') {
        is_minus = true;
        text = text.substr(1);
    }
    if (text.empty() || text[0] == '-' || !IsValidWord(text)) {
        throw std::invalid_argument("Word is invalid");
    }
    return QueryWord{ text, is_minus, IsStopWord(text) };
}

SearchServer::Query SearchServer::ParseQuery(const std::string_view& text) const {
    Query result;
    for (const std::string_view& word : SplitIntoWords(text)) {
        QueryWord query_word = ParseQueryWord(word);       
        if (!query_word.is_stop) {
            if (query_word.is_minus) {
                result.minus_words.insert(query_word.data);
            }
            else {
                result.plus_words.insert(query_word.data);
            }
        }
    }
    return result;
}

double SearchServer::ComputeWordInverseDocumentFreq(const std::string_view& word) const {
    std::string temp{ word };
    return log(GetDocumentCount() * 1.0 / word_to_document_freqs_.at(temp).size());
}

const std::map<std::string, double>& SearchServer::GetWordFrequencies(int document_id) const {
    static const std::map<std::string, double> temporary;
    if (document_to_word_freqs_.count(document_id) == 0) {
        return temporary;
    }
    return document_to_word_freqs_.at(document_id);
}

void SearchServer::RemoveDocument(int document_id) {
    return RemoveDocument(std::execution::seq, document_id);
}

void SearchServer::RemoveDocument(const std::execution::sequenced_policy&, int document_id) {
    documents_.erase(document_id);
    document_ids_.erase(remove(document_ids_.begin(), document_ids_.end(), document_id), document_ids_.end());
    for (const auto& map_word : word_to_document_freqs_) {
        if (word_to_document_freqs_.at(map_word.first).count(document_id)) {
            word_to_document_freqs_.at(map_word.first).erase(document_id);
        }
    }
}

void SearchServer::RemoveDocument(const std::execution::parallel_policy&, int document_id) {
    if (document_to_word_freqs_.count(document_id) == 0) {
        return;
    }
    documents_.erase(document_id);
    document_ids_.erase(remove(document_ids_.begin(), document_ids_.end(), document_id), document_ids_.end());
    const auto& word_freqs = document_to_word_freqs_.at(document_id);
    std::vector<std::string> words(word_freqs.size());
    std::transform(
        std::execution::par,
        word_freqs.begin(), word_freqs.end(),
        words.begin(),
        [](const auto& item) { 
            return item.first; }
    );
    for_each(
        std::execution::par,
        words.begin(), words.end(),
        [this, document_id](std::string word) {
            word_to_document_freqs_.at(word).erase(document_id);
        });
    document_to_word_freqs_.erase(document_id);
}