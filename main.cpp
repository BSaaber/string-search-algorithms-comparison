#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <unordered_map>

inline bool compareSymbols(const char& i, const char& j) {
    return i == j || i == '?' || j == '?';
}

std::vector<int> prefixFunction(const std::string& s) {
    std::vector<int> pi(s.length());
    pi[0] = 0;
    int j;
    for (int i = 1; i < static_cast<int>(s.length()); ++i) {
        j = pi[i - 1];

        while (j > 0 && !compareSymbols(s[i], s[j])) {
            j = pi[j - 1];
        }

        if (compareSymbols(s[i], s[j])) {
            j++;
        }

        pi[i] = j;
    }
    return pi;
}

std::vector<int> prefixFunctionSpecial(const std::string& s) {
    std::vector<int> pi = prefixFunction(s);
    for (int i = 0; i < static_cast<int>(pi.size()) - 1; ++i) {
        if (pi[i + 1] == pi[i] + 1) {
            pi[i] = 0;
        }
    }
    return pi;
}

std::vector<int> kmp(const std::string& text, const std::string& pattern, bool special = false) {
    std::string data = pattern + "#" + text;
    std::vector<int> pi = (special) ? prefixFunctionSpecial(data) : prefixFunction(data);
    std::vector<int> answers;
    int pattern_size = static_cast<int>(pattern.length());
    for (int i = 0; i < static_cast<int>(text.length()); ++i) {
        if (pi[i + pattern_size + 1] == static_cast<int>(pattern.length())) {
            answers.push_back(i - pattern_size + 1);
        }
    }
    return answers;
}

std::vector<int> naiveSearch(const std::string& text, const std::string& pattern) {
    std::vector<int> answers;
    bool flag;
    for (int i = 0; i < text.length(); ++i) {
        flag = true;
        for (int j = 0; j < pattern.length(); ++j) {
            if (!compareSymbols(pattern[j], text[i + j])) {
                flag = false;
            }
            if (flag  && j == pattern.length() - 1) {
                answers.push_back(i);
            }
        }
    }
    return answers;
}

std::string createRandomString(int size, int alphabet_size) {

    std::string res;
    for (int i = 0; i < size; ++i) {
        auto t = rand() % alphabet_size;
        res += 'A' + t;
    }
    return res;
}

struct Point {
    int pattern_length;
    long long time; // in milliseconds
};

void insertToFile(const std::vector<std::vector<Point>>& vv, const std::string& file_name) {
    std::ofstream file;
    file.open(file_name);
    std::string output;
    for (const auto& v : vv) {
        for (const auto& p : v) {
            output += std::to_string(p.pattern_length) + ',' + std::to_string(p.time) + ';';
        }
        output = output.substr(0, output.length() - 1);
        output += '|';
    }
    output = output.substr(0, output.length() - 1);
    file << output;
    file.close();
}

long long process(std::function<void()> const& function) {
    long long aggr = 0;
    long long start, end;
    for (int i = 0; i < 5; ++i) {
        auto time_start = std::chrono::system_clock::now();
        auto start_since_epoch = time_start.time_since_epoch();
        start = std::chrono::duration_cast<std::chrono::milliseconds>(start_since_epoch).count();
        function();
        auto time_end = std::chrono::system_clock::now();
        auto end_since_epoch = time_end.time_since_epoch();
        end = std::chrono::duration_cast<std::chrono::milliseconds>(end_since_epoch).count();
        aggr += (end - start);
    }
    return aggr / 5;
}

void doTest(int text_size, int alphabet_size, int magic_symbols_amount = 0, bool with_naive = true, int pattern_step = 100, int pattern_max_size = 3000, const std::string& suffix = "") {
    std::vector<Point> v_naive, v_kmp, v_kmp_special;
    for (int pattern_size = pattern_step; pattern_size <= pattern_max_size; pattern_size += pattern_step) {
        std::string text = createRandomString(text_size, alphabet_size);
        std::string pattern = createRandomString(pattern_size, alphabet_size);
        if (magic_symbols_amount != 0) {
            std::unordered_map<int, int> m;
            std::vector<int> magic_indexes;
            int max_size = pattern_size;
            int randomized;
            for (int i = 0; i < magic_symbols_amount; ++i) {
                randomized = rand() % (max_size - i);
                if (m.find(randomized) == m.end()) {
                    magic_indexes.push_back(randomized);
                    m[randomized] = max_size - 1;
                } else {
                    magic_indexes.push_back(m[randomized]);
                    m[randomized] = max_size - 1;
                }
            }
            for (const auto& i : magic_indexes) {
                pattern[i] = '?';
            }
        }

        if (with_naive) {
            auto naive_exec = [&text, &pattern]() { naiveSearch(text, pattern); };
            v_naive.push_back({pattern_size, process(naive_exec)});
        }

        auto kmp_exec = [&text, &pattern]() { kmp(text, pattern);};
        v_kmp.push_back({pattern_size, process(kmp_exec)});

        auto kmp_exec_special = [&text, &pattern]() { kmp(text, pattern, true);};
        v_kmp_special.push_back({pattern_size, process(kmp_exec_special)});
    }
    std::vector<std::vector<Point>> vv = {v_kmp, v_kmp_special};
    if (with_naive) {
        vv.push_back(v_naive);
    }
    insertToFile(vv, "text_size_" + std::to_string(text_size) +
                                                  "_alphabet_size_" + std::to_string(alphabet_size) +
                                                  "_magic_symbols_" +
                                                  std::to_string(magic_symbols_amount) + "__" + suffix + ".txt");
}

int main() {
    srand(time(NULL));

    // 2 * 2 * 5 = 20 in total
    // input data as it was requested
    int count = 0;
    int now;
    int start = time(NULL);
    for (const auto& text_size : {10000, 100000}) {
        for (const auto& alphabet_size : {2, 4}) {
            for (int i = 0; i < 5; ++i) {
                doTest(text_size, alphabet_size, i);
                count++;
                now = time(NULL);
                std::cout << (count * 5) << "%  | " << ((now - start) / 60) << " min from start\n";
            }
        }
    }
    std::cout << "requested data processed\n";

    count = 0;
    start = time(NULL);
    // data is too small for kmp algorithms - need special tests
    for (const auto& text_size : {1000000}) {
        for (const auto& alphabet_size : {2, 4}) {
            for (int i = 0; i < 5; ++i) {
                doTest(text_size, alphabet_size, i, false, text_size / 50, text_size / 2, "kmp");
                count++;
                now = time(NULL);
                std::cout << (count * 10) << "%  | " << ((now - start) / 60) << " min from start\n";
            }
        }
    }
}