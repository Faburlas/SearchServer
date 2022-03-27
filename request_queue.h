#pragma once

#include <vector>
#include <deque>

#include "search_server.h"


class RequestQueue {
public:
    RequestQueue(const SearchServer& search_server);

    template <typename DocumentPredicate>
    std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentPredicate document_predicate);

    std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentStatus status);
    
    std::vector<Document> AddFindRequest(const std::string& raw_query);

    int GetNoResultRequests() const;
private:
    struct QueryResult {
        size_t documents_size;
        std::string query;
    };
private:
    static const int kMinInDay = 1440;
private:
    std::deque<QueryResult> requests_;
    int count_no_result_ = 0;
    const SearchServer& server;
};


template <typename DocumentPredicate>
std::vector<Document> RequestQueue::AddFindRequest(const std::string& raw_query, DocumentPredicate document_predicate)
{
    //full rework
    if (!requests_.empty())  {
        while (requests_.size() >= kMinInDay) {
            if (requests_.front().documents_size == 0) {
                --count_no_result_;
            }
            requests_.pop_front();
        }
    }
    std::vector<Document> result;
    result = server.FindTopDocuments(raw_query, document_predicate);
    if (result.empty()) {
        ++count_no_result_;
    }
    requests_.push_back({ result.size(),raw_query });
    return result;
}