#include "test_example_functions.h"
#include <cmath>

void AssertImpl(bool value, const std::string& expr_str, const std::string& file, const std::string& func, unsigned line,
    const std::string& hint) {
    if (!value) {
        std::cout << file << "(" << line << "): " << func << ": ";
        std::cout << "ASSERT(" << expr_str << ") failed.";
        if (!hint.empty()) {
            std::cout << " Hint: " << hint;
        }
        std::cout << std::endl;
        abort();
    }
}

void TestExcludeStopWordsFromAddedDocumentContent() {
    const int doc_id = 42;
    const std::string content = "cat in the city";
    const std::vector<int> ratings = { 1, 2, 3 };
    {
        SearchServer server("dffgffd"s);
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        const auto found_docs = server.FindTopDocuments("in");
        ASSERT_EQUAL(found_docs.size(), 1u);
        const Document& doc0 = found_docs[0];
        ASSERT_EQUAL(doc0.id, doc_id);
    }

    {
        SearchServer server("in the"s);
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        ASSERT_HINT(server.FindTopDocuments("in"s).empty(), "Stop words must be excluded from documents");
    }
}

void TestMinusWords()
{
    const std::string content1 = "cat in the city";
    const std::string content2 = "Claude Monet Poppy Field";
    const std::string content3 = "Pieter Paul Rubens Infanta Isabella";
    const std::vector<int> ratings = { 1, 2, 3 };
    SearchServer server("sdgs"s);
    server.AddDocument(0, content1, DocumentStatus::ACTUAL, ratings);
    server.AddDocument(1, content2, DocumentStatus::ACTUAL, ratings);
    server.AddDocument(2, content3, DocumentStatus::ACTUAL, ratings);
    ASSERT(server.FindTopDocuments("cat in the -city").empty());
    ASSERT(server.FindTopDocuments("Claude -Monet Poppy Field").empty());
    ASSERT(server.FindTopDocuments("Pieter -Paul Rubens Infanta Isabella").empty());
}

void TestMatchingDocs()
{
    //test1
    SearchServer search_server("gdgfdf"s);
    search_server.AddDocument(0, "белый кот и модный ошейник", DocumentStatus::ACTUAL, { 8, -3 });
    search_server.AddDocument(1, "пушистый кот пушистый хвост", DocumentStatus::ACTUAL, { 7, 2, 7 });
    const int document_count = search_server.GetDocumentCount();
    const std::vector<std::string> words;
    for (int document_id = 0; document_id < document_count; ++document_id) {
        const auto [words, status] = search_server.MatchDocument("пушистый кот", document_id);
        ASSERT_EQUAL(words[0], "кот");
    }

    //test2
    SearchServer search_server1("gdfg"s);
    search_server1.AddDocument(0, "Picasso Pablo girl on the ball blue period", DocumentStatus::ACTUAL, { 8, -3 });
    search_server1.AddDocument(1, "Absinthe Drinker Pablo Picasso blue period", DocumentStatus::ACTUAL, { 7, 2, 7 });
    const int document_count1 = search_server1.GetDocumentCount();
    const std::vector<std::string> words1;
    for (int document_id = 0; document_id < document_count1; ++document_id) {
        const auto [words1, status1] = search_server1.MatchDocument("Picasso blue", document_id);
        ASSERT_EQUAL(words1[0], "Picasso");
        ASSERT_EQUAL(words1[1], "blue");
    }
}

void TestSortByRelevance()
{
    SearchServer search_server("kek"s);
    search_server.AddDocument(0, "белый кот и модный ошейник", DocumentStatus::ACTUAL, { 8, -3 });
    search_server.AddDocument(1, "пушистый кот пушистый хвост", DocumentStatus::ACTUAL, { 7, 2, 7 });
    search_server.AddDocument(2, "ухоженный пёс выразительные глаза", DocumentStatus::ACTUAL, { 5, -12, 2, 1 });
    search_server.AddDocument(3, "пёс порода овчарка ухоженный", DocumentStatus::ACTUAL, { 3, 4, 2 });
    search_server.AddDocument(4, "кот сфинкс пушистый ", DocumentStatus::ACTUAL, { 1, 2, 3 });

    const auto docs = search_server.FindTopDocuments("пушистый кот ухоженный");

    for (size_t i = 0; i < docs.size(); i++)
        if (i - 1 > 0)
        {
            ASSERT(docs[i - 1].relevance >= docs[i].relevance);
        }
}

void TestRatingCalculations()
{
    const std::vector<int> ratings1 = { 8 , -3 };
    const std::vector<int> ratings2 = { 7, 2 , 7 };
    const std::vector<int> ratings3 = { 5, -12, 2, 1 };
    SearchServer search_server("lel"s);
    search_server.AddDocument(0, "белый кот и модный ошейник", DocumentStatus::ACTUAL, ratings1);
    search_server.AddDocument(1, "пушистый кот пушистый хвост", DocumentStatus::ACTUAL, ratings2);
    search_server.AddDocument(2, "ухоженный пёс выразительные глаза", DocumentStatus::ACTUAL, ratings3);
    const auto docs = search_server.FindTopDocuments("пушистый кот ухоженный");
    ASSERT_EQUAL(docs[0].rating, 5);
    ASSERT_EQUAL(docs[1].rating, -1);
    ASSERT_EQUAL(docs[2].rating, 2);

}
//Here Testing Predicate work + find by status func.
void TestPredicateLambdaFunc()
{
    const std::vector<int> ratings1 = { 8 , -3 };
    const std::vector<int> ratings2 = { 7, 2 , 7 };
    const std::vector<int> ratings3 = { 5, -12, 2, 1 };
    SearchServer search_server("d"s);
    search_server.AddDocument(0, "белый кот и модный ошейник", DocumentStatus::ACTUAL, ratings1);
    search_server.AddDocument(1, "пушистый кот пушистый хвост", DocumentStatus::ACTUAL, ratings2);
    search_server.AddDocument(2, "ухоженный пёс выразительные глаза", DocumentStatus::BANNED, ratings3);
    const auto docs1 = search_server.FindTopDocuments("пушистый кот ухоженный", DocumentStatus::BANNED);
    ASSERT(docs1.size() < 2);
    const auto docs2 = search_server.FindTopDocuments("пушистый кот ухоженный",
        [](int document_id, DocumentStatus status, int rating) {
            return document_id % 2 == 1;
        });
    ASSERT(docs2.size() < 2);
}

void TestRelevanceFind() {
    const double kSubtractionDiff = 1e-6;
    SearchServer search_server("cvb"s);
    std::string_view str1 = "белый кот и модный ошейник";
    std::string_view str2 = "пушистый кот пушистый хвост";
    std::string_view str3 = "ухоженный пёс выразительные глаза кот"s;
    search_server.AddDocument(0, str1, DocumentStatus::ACTUAL, { 8, -3 });
    search_server.AddDocument(1, str2, DocumentStatus::ACTUAL, { 7, 2, 7 });
    search_server.AddDocument(2, str3, DocumentStatus::ACTUAL, { 5, -12, 2, 1 });
    const auto docs = search_server.FindTopDocuments("кот пушистый");

    //calculating relevancy by yourself
    std::vector<std::string_view> words1 = SplitIntoWords(str1);
    std::vector<std::string_view> words2 = SplitIntoWords(str2);
    std::vector<std::string_view> words3 = SplitIntoWords(str3);
    std::map<std::string_view, std::map<int, double>> word_to_document_freqs;
    std::vector <std::vector<std::string_view>> VectorForWordsVectors = { words1,words2,words3 };
    double inv_word_count;
    int i = 0;
    for (auto words : VectorForWordsVectors) {
        inv_word_count = 1.0 / words.size();
        for (auto word : words) {
            word_to_document_freqs[word][i] += inv_word_count;
        }
        i++;
    }
    double IDF;
    std::map <int, double> document_to_relevance;
    for (std::string word : SplitIntoWords("кот пушистый"s)) {
        IDF = log(3.0 * 1.0 / word_to_document_freqs.at(word).size());
        for (const auto [document_id, term_freq] : word_to_document_freqs.at(word)) {
            document_to_relevance[document_id] += term_freq * IDF;
        }
    }

    //For some reason document_to_relevance[1] is docs[0] relevance.
    ASSERT(abs(document_to_relevance[1] - docs[0].relevance) < kSubtractionDiff);
    ASSERT(abs(document_to_relevance[0] - docs[1].relevance) < kSubtractionDiff);
}

void TestSearchServer() {
    RUN_TEST(TestExcludeStopWordsFromAddedDocumentContent);
    RUN_TEST(TestExcludeStopWordsFromAddedDocumentContent);
    RUN_TEST(TestMinusWords);
    RUN_TEST(TestMatchingDocs);
    RUN_TEST(TestSortByRelevance);
    RUN_TEST(TestRatingCalculations);
    RUN_TEST(TestPredicateLambdaFunc);
    RUN_TEST(TestRelevanceFind);
}