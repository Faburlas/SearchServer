#pragma once

#include <string>
#include <iostream>

#include "search_server.h"

using namespace std::string_literals;


//Start point for macroses 
//ASSERT, ASSERT_EQUAL, ASSERT_EQUAL_HINT, ASSERT_HINT è RUN_TEST
template <typename Func>
void RunTestImpl(const Func& func, const std::string& fname) {
    func();
    std::cerr << fname << " " << "OK" << std::endl;

}
#define RUN_TEST(func) RunTestImpl(func,#func) 

template <typename T, typename U>
void AssertEqualImpl(const T& t, const U& u, const std::string& t_str, const std::string& u_str, const std::string& file,
    const std::string& func, unsigned line, const std::string& hint) {
    if (t != u) {
        std::cout << std::boolalpha;
        std::cout << file << "(" << line << "): " << func << ": ";
        std::cout << "ASSERT_EQUAL(" << t_str << ", " << u_str << ") failed: " << func;
        std::cout << t << " != " << u << ".";
        if (!hint.empty()) {
            std::cout << " Hint: " << hint;
        }
        std::cout << std::endl;
        abort();
    }
}

#define ASSERT_EQUAL(a, b) AssertEqualImpl((a), (b), #a, #b, __FILE__, __FUNCTION__, __LINE__, ""s)

#define ASSERT_EQUAL_HINT(a, b, hint) AssertEqualImpl((a), (b), #a, #b, __FILE__, __FUNCTION__, __LINE__, (hint))

void AssertImpl(bool value, const std::string& expr_str, const std::string& file, const std::string& func, unsigned line,
    const std::string& hint);
#define ASSERT(expr) AssertImpl(!!(expr), #expr, __FILE__, __FUNCTION__, __LINE__, ""s)

#define ASSERT_HINT(expr, hint) AssertImpl(!!(expr), #expr, __FILE__, __FUNCTION__, __LINE__, (hint))

void TestRelevanceFind();

void TestExcludeStopWordsFromAddedDocumentContent();

void TestMinusWords();

void TestMatchingDocs();

void TestSortByRelevance();

void TestRatingCalculations();

void TestPredicateLambdaFunc();

void TestSearchServer();
