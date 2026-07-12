// 01_stl_containers — YOUR attempt. Fill in the TODO bodies, then run:
//   PRACTICE=1 uv run pytest ramp_up/cpp/01_stl_containers -v
// Or compile and run directly:
//   clang++ -std=c++17 -Wall -o /tmp/stl starter.cpp && /tmp/stl
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
// JAVA: String.join(" ", reverse(Arrays.asList(text.trim().split("\\s+"))))
// C++:  std::istringstream + `while (stream >> word)` tokenizes for free (>>
//       skips whitespace). Collect into a vector, walk it backwards with
//       reverse iterators (rbegin()/rend()), build the result with +=.
std::string reverseWords(const std::string& text) {
    // TODO: implement
    return "";
}

// charFrequencies("abbccc") -> {'a':1, 'b':2, 'c':3}
// JAVA: HashMap<Character,Integer> + merge(c, 1, Integer::sum)
// C++:  range-based for over the string (`for (char c : text)`), then
//       ++freq[c] — operator[] inserts a 0 for missing keys, so no
//       getOrDefault needed.
std::unordered_map<char, int> charFrequencies(const std::string& text) {
    // TODO: implement
    return {};
}

// topKSmallest({5,1,4,2,3}, 3) -> {1,2,3}; k >= size returns everything sorted.
// JAVA: copy = new ArrayList<>(list); Collections.sort(copy); copy.subList(0, k)
// C++:  the parameter is BY VALUE on purpose — it IS your private copy.
//       std::sort(values.begin(), values.end()), then resize(k) if k < size().
std::vector<int> topKSmallest(std::vector<int> values, std::size_t k) {
    // TODO: implement
    return {};
}

// groupByLength({"go","rust","cpp"}) -> {2:{"go"}, 3:{"cpp"}, 4:{"rust"}}
// Words keep their input order within each group.
// JAVA: TreeMap + computeIfAbsent(len, x -> new ArrayList<>()).add(w)
// C++:  std::map keeps keys sorted like TreeMap. Loop with `const auto& w`,
//       and groups[w.size()].push_back(w) — operator[] creates the empty
//       vector on first touch.
std::map<int, std::vector<std::string>> groupByLength(const std::vector<std::string>& words) {
    // TODO: implement
    return {};
}

// sumOfUnique({1,2,2,3,3}) -> 6: each DISTINCT value counted once.
// JAVA: HashSet.add(x) returns false on duplicates; accumulate in a long.
// C++:  seen.insert(v) returns pair<iterator,bool>; use .second as the
//       "was it new?" boolean. Accumulate in long long (the real 64-bit type).
long long sumOfUnique(const std::vector<int>& values) {
    // TODO: implement
    return 0;
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
        for (const auto& [ch, count] : freq) {  // structured bindings: no Map.Entry
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
        auto groups = groupByLength({"go", "rust", "cpp", "java", "c"});
        assert(groups.size() == 4);
        assert((groups[1] == std::vector<std::string>{"c"}));
        assert((groups[2] == std::vector<std::string>{"go"}));
        assert((groups[3] == std::vector<std::string>{"cpp"}));
        assert((groups[4] == std::vector<std::string>{"rust", "java"}));
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

    std::cout << "ALL TESTS PASSED" << std::endl;
    return 0;
}
