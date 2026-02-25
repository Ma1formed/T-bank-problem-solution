/*
 *
 *  T-bank, "Similar Words Grouping" problem.
 *  Algorithm: DSU + Masking for O(N*L) complexity in the average case.
 *
 *  I believe that this implementation is close to ideal for industrial code,
 *  But better efficiency can be achieved by using trie or certain linear
 *  Sorting algorithms that can be applied due to the
 *  Specific conditions of the task.
 *
*/


#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <unordered_map>
#include <cctype>
#include <numeric>

struct DSU {
    std::vector<int> parent;
    std::vector<int> size;

    explicit DSU(int n) {
        parent.resize(n);
        size.assign(n, 1);
        std::iota(parent.begin(), parent.end(), 0);
    }

    int find(int i) {
        if (parent[i] == i) {
            return i;
        }
        return parent[i] = find(parent[i]);
    }

    void unite(int i, int j) {
        int root_i = find(i);
        int root_j = find(j);
        if (root_i != root_j) {
            if (size[root_i] < size[root_j]) {
                std::swap(root_i, root_j);
            }
            parent[root_j] = root_i;
            size[root_i] += size[root_j];
        }
    }
};

std::string normalize(const std::string& s) {
    std::string out;
    out.reserve(s.size());
    for (const char& c : s) {
        if (std::isalpha(static_cast<unsigned char>(c))) {
            out.push_back(static_cast<char>(std::tolower(c)));
        }
        else if (c == '\'') {
            out.push_back(c);
        }
    }
    return out;
}

struct GroupResult {
    std::string rep;
    int count;
};

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    int K;
    std::cin >> K;

    std::vector<std::string> wordlist;
    std::unordered_map<std::string, int> word_to_id;
    std::vector<int> text_ids;
    std::string buf;

    while (std::cin >> buf) {
        std::string w = normalize(buf);
        if (w.empty()) {
            continue;
        }
        auto [it, inserted] = word_to_id.try_emplace(w, static_cast<int>(wordlist.size()));
        if (inserted) {
            wordlist.push_back(w);
        }
        text_ids.push_back(it->second);
    }

    if (wordlist.empty()) {
        return 0;
    }

    int n_wordlist = static_cast<int>(wordlist.size());
    DSU dsu(n_wordlist);

    std::unordered_map<std::string, int> mask_map;
    mask_map.reserve(n_wordlist * 2);

    for (int i = 0; i < n_wordlist; ++i) {
        const std::string& s = wordlist[i];
        size_t len = s.length();
        if (len <= 1) {
            continue;
        }
        std::string temp = s;
        for (size_t j = 0; j < len; ++j) {
            char original = temp[j];
            temp[j] = '*';
            auto [it, inserted] = mask_map.try_emplace(temp, i);
            if (!inserted) {
                dsu.unite(i, it->second);
            }
            temp[j] = original;
        }

        char last = s.back();
        if (last == 's' || last == 'e') {
            std::string base = s.substr(0, len - 1);
            if (base.length() > 1) {
                auto it = word_to_id.find(base);
                if (it != word_to_id.end()) {
                    dsu.unite(i, it->second);
                }
            }
        }
    }

    std::vector<std::vector<int>> positions(n_wordlist);
    for (int i = 0; i < static_cast<int>(text_ids.size()); ++i) {
        int root = dsu.find(text_ids[i]);
        positions[root].push_back(i);
    }

    std::unordered_map<int, int> group_freq;
    std::unordered_map<int, std::string> group_reps;

    for (int i = 0; i < n_wordlist; ++i) {
        if (positions[i].empty()) {
            continue;
        }
        const auto& pos = positions[i];
        int freq = 0;
        if (size_t sz = pos.size(); sz > 1) {
            for (size_t k = 0; k < sz; ++k) {
                bool ok = false;
                if (k > 0 && (pos[k] - pos[k - 1] <= K)) {
                    ok = true;
                }
                if (!ok && k < sz - 1 && (pos[k + 1] - pos[k] <= K)) {
                    ok = true;
                }
                if (ok) {
                    freq++;
                }
            }
        }
        if (freq > 0) {
            group_freq[i] = freq;
        }
    }

    for (int i = 0; i < n_wordlist; ++i) {
        if (int root = dsu.find(i); group_freq.count(root)) {
            if (group_reps.find(root) == group_reps.end() || wordlist[i] < group_reps[root]) {
                group_reps[root] = wordlist[i];
            }
        }
    }

    std::vector<GroupResult> results;
    results.reserve(group_freq.size());
    for (const auto& [root, count] : group_freq) {
        results.push_back({group_reps[root], count});
    }

    std::sort(results.begin(), results.end(), [](const GroupResult& a, const GroupResult& b) {
        if (a.count != b.count) {
            return a.count > b.count;
        }
        return a.rep < b.rep;
    });

    for (const auto& res : results) {
        std::cout << res.rep << ": " << res.count << "\n";
    }
}