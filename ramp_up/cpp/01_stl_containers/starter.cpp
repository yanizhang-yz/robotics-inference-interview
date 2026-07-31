// 01_stl_containers — YOUR attempt. Fill in the TODO bodies, then run:
//   PRACTICE=1 uv run pytest ramp_up/cpp/01_stl_containers -v
// Or compile and run directly:
//   clang++ -std=c++20 -Wall -Wextra -Werror=return-type -o /tmp/stl starter.cpp && /tmp/stl
// The stubs compile as-is but fail main()'s asserts until you implement them.
// You're done when the last line printed is: ALL TESTS PASSED

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
// std::istringstream with `while (stream >> word)` skips whitespace while
// tokenizing. Collect words in a vector, walk it backwards with reverse
// iterators (rbegin()/rend()), and build the result with +=.
std::string reverseWords(const std::string& text) {
    // TODO: implement
    return "";
}

// charFrequencies("abbccc") -> {'a':1, 'b':2, 'c':3}
// Use a range-based loop over the string (`for (char c : text)`). `operator[]`
// inserts a 0 for a missing key, so `++freq[c]` counts in one expression.
std::unordered_map<char, int> charFrequencies(const std::string& text) {
    // TODO: implement
    return {};
}

// topKSmallest({5,1,4,2,3}, 3) -> {1,2,3}; k >= size returns everything sorted.
// The parameter is by value on purpose: it is the private copy to sort.
// Use std::sort(values.begin(), values.end()), then resize(k) if k < size().
std::vector<int> topKSmallest(std::vector<int> values, std::size_t k) {
    // TODO: implement
    return {};
}

// groupByLength({"go","rust","cpp"}) -> {2:{"go"}, 3:{"cpp"}, 4:{"rust"}}
// Words keep their input order within each group.
// std::map keeps keys sorted. Loop with `const auto& w`, and use
// groups[w.size()].push_back(w): operator[] creates an empty vector on first
// touch.
std::map<int, std::vector<std::string>> groupByLength(const std::vector<std::string>& words) {
    // TODO: implement
    return {};
}

// sumOfUnique({1,2,2,3,3}) -> 6: each DISTINCT value counted once.
// seen.insert(v) returns pair<iterator, bool>; use .second as the "was it new?"
// boolean. Accumulate in long long, the guaranteed 64-bit integer type.
long long sumOfUnique(const std::vector<int>& values) {
    // TODO: implement
    return 0;
}

// appendDoubled({1,2,3}) makes the SAME vector {1,2,3,2,4,6}.
// THE TRAP: `for (int x : v) v.push_back(x * 2);` is undefined behavior —
//           push_back may relocate the block while the range-for still holds
        //           an iterator into the old block; C++ makes no promises.
// Idiom: freeze the size first (`const std::size_t original = v.size();`),
//        then loop by index up to `original` — indexes survive relocation.
void appendDoubled(std::vector<int>& v) {
    // TODO: implement
    (void)v;
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
