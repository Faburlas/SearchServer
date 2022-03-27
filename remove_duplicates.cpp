#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <string>
#include <set>
#include <map>

#include "remove_duplicates.h"
#include "search_server.h"

void RemoveDuplicates(SearchServer& search_server) {
	std::map<std::string_view, double> map_with_words_and_frequencies;
	std::set<std::vector<std::string_view>> set_with_words;
	std::vector<int> ids_to_delete;
	for (const auto doc : search_server) {
		std::vector<std::string_view> vector_with_words;
		for (const auto& [key, val] : search_server.GetWordFrequencies(doc)) {
		    vector_with_words.push_back(key);
		}
		if (set_with_words.find(vector_with_words) == set_with_words.end()) {
			set_with_words.insert(vector_with_words);
		}
		else {
			ids_to_delete.push_back(doc);
		}
	}
	for (const auto id : ids_to_delete) {
		std::cout << "Found duplicate document id " << id << std::endl;
		search_server.RemoveDocument(id);
	}
}