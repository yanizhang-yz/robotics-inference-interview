// 01_stl_containers — reference solution.
//
// Compile & run standalone:
//   clang++ -std=c++20 -Wall -Wextra -Werror=return-type -o /tmp/stl solution.cpp && /tmp/stl
// Prints ALL TESTS PASSED when every assert holds.

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

// reverseWords("robots move fast") -> "fast move robots"
// Words are runs of non-whitespace; output is single-space separated.
// istringstream's >> tokenizes while skipping whitespace. Walk the collected
// words backwards with reverse iterators and build the mutable result with +=.
std::string reverseWords(const std::string& text) {
    std::istringstream stream(text);
    std::vector<std::string> words;
    std::string word;
    while (stream >> word) {
        words.push_back(word);
    }
    std::string result;
    for (auto it = words.rbegin(); it != words.rend(); ++it) {
        if (!result.empty()) {
            result += ' ';
        }
        result += *it;
    }
    return result;
}

// charFrequencies("abbccc") -> {'a':1, 'b':2, 'c':3}
// operator[] default-inserts 0 for a missing key, so counting is one line.
std::unordered_map<char, int> charFrequencies(const std::string& text) {
    std::unordered_map<char, int> freq;
    for (char c : text) {
        ++freq[c];
    }
    return freq;
}

// topKSmallest({5,1,4,2,3}, 3) -> {1,2,3}; k >= size returns everything sorted.
// The parameter is by value on purpose: sorting needs a private copy, so the
// parameter is that copy. The caller's vector is untouched.
std::vector<int> topKSmallest(std::vector<int> values, std::size_t k) {
    std::sort(values.begin(), values.end());
    if (k < values.size()) {
        values.resize(k);
    }
    return values;
}

// groupByLength({"go","rust","cpp"}) -> {2:{"go"}, 3:{"cpp"}, 4:{"rust"}}
// Words keep their input order within each group.
// std::map keeps keys sorted. operator[] default-inserts an empty vector, so
// grouping only needs a loop with const auto&.
std::map<int, std::vector<std::string>> groupByLength(const std::vector<std::string>& words) {
    std::map<int, std::vector<std::string>> groups;
    for (const auto& w : words) {
        groups[static_cast<int>(w.size())].push_back(w);
    }
    return groups;
}

// sumOfUnique({1,2,2,3,3}) -> 6: each DISTINCT value counted once.
// insert() returns pair<iterator, bool>; .second reports a new value. long
// long is the guaranteed 64-bit type (plain long may be 32 bits).
long long sumOfUnique(const std::vector<int>& values) {
    std::unordered_set<int> seen;
    long long total = 0;
    for (int v : values) {
        if (seen.insert(v).second) {
            total += v;
        }
    }
    return total;
}

// Append a doubled copy of each ORIGINAL element to the same vector:
// {1, 2, 3} becomes {1, 2, 3, 2, 4, 6}.
// The trap this drill exists for: a range-for over v while push_back-ing
// into v is undefined behavior (growth can relocate the block mid-loop).
// The safe pattern: freeze the original size first, loop by INDEX —
// an index names a position, not an address, so it survives relocation.
void appendDoubled(std::vector<int>& v) {
    const std::size_t original = v.size();
    for (std::size_t i = 0; i < original; ++i) {
        v.push_back(v[i] * 2);
    }
}

int main() {
    // reverseWords
    assert(reverseWords("robots move fast") == "fast move robots");
    assert(reverseWords("hello") == "hello");
    assert(reverseWords("  extra   spaces  ") == "spaces extra");
    assert(reverseWords("") == "");

    // charFrequencies
    {
        auto freq = charFrequencies("abbccc");
        assert(freq.size() == 3);
        assert(freq['a'] == 1 && freq['b'] == 2 && freq['c'] == 3);
        int total = 0;
        for (const auto& [ch, count] : freq) {  // structured bindings name each pair member
            (void)ch;
            total += count;
        }
        assert(total == 6);
        assert(charFrequencies("").empty());
    }

    // topKSmallest
    {
        std::vector<int> nums = {5, 1, 4, 2, 3};
        assert((topKSmallest(nums, 3) == std::vector<int>{1, 2, 3}));
        assert((nums == std::vector<int>{5, 1, 4, 2, 3}));  // caller's vector untouched
        assert((topKSmallest(nums, 10) == std::vector<int>{1, 2, 3, 4, 5}));
        assert(topKSmallest(nums, 0).empty());
        assert(topKSmallest({}, 3).empty());
    }

    // groupByLength
    {
        auto groups = groupByLength({"go", "rust", "cpp", "ja" "va", "c"});
        assert(groups.size() == 4);
        assert((groups[1] == std::vector<std::string>{"c"}));
        assert((groups[2] == std::vector<std::string>{"go"}));
        assert((groups[3] == std::vector<std::string>{"cpp"}));
        assert((groups[4] == std::vector<std::string>{"rust", "ja" "va"}));
        std::vector<int> keys;
        for (const auto& [len, group] : groups) {  // std::map iterates in key order
            (void)group;
            keys.push_back(len);
        }
        assert((keys == std::vector<int>{1, 2, 3, 4}));
        assert(groupByLength({}).empty());
    }

    // sumOfUnique
    assert(sumOfUnique({1, 2, 3}) == 6);
    assert(sumOfUnique({1, 2, 2, 3, 3, 3}) == 6);
    assert(sumOfUnique({}) == 0);
    // Two distinct 2-billion-ish values: overflows int, fits long long.
    assert(sumOfUnique({2000000000, 1500000000, 2000000000}) == 3500000000LL);

    // appendDoubled
    {
        std::vector<int> v = {1, 2, 3};
        appendDoubled(v);
        assert((v == std::vector<int>{1, 2, 3, 2, 4, 6}));
        std::vector<int> empty;
        appendDoubled(empty);
        assert(empty.empty());
        // Force relocation: many elements, no reserve — the safe index
        // pattern must survive the block moving mid-append.
        std::vector<int> big;
        for (int i = 0; i < 1000; ++i) big.push_back(i);
        appendDoubled(big);
        assert(big.size() == 2000);
        assert(big[1000] == 0 && big[1001] == 2 && big[1999] == 999 * 2);
    }

    std::cout << "ALL TESTS PASSED" << std::endl;
    return 0;
}
