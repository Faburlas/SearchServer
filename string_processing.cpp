#include "string_processing.h"
#include <string>
#include <iostream>
#include <algorithm>

std::vector<std::string> SplitIntoWords(const std::string& text) {
    std::vector<std::string> words;
    std::string word;
    for (const char c : text) {
        if (c == ' ') {
            if (!word.empty()) {
                words.push_back(word);
                word.clear();
            }
        }
        else {
            word += c;
        }
    }
    if (!word.empty()) {
        words.push_back(word);
    }
    return words;
}

std::vector<std::string_view> SplitIntoWords(std::string_view str)
{
    std::vector<std::string_view> words;
    int pos = 0;
    size_t pos_end = str.npos;
    while (true) {
        int space = str.find(' ', pos);
        words.push_back(space == pos_end ? str.substr(pos) : str.substr(pos, space - pos));
        if (space == pos_end) {
            break;
        }
        else {
            pos = space + 1;
        }
    }
    return words;
}