#include <iostream>
#include <iomanip>
#include <cstdio>
#include <unordered_map>
#include <future>
#include <string>
#include <algorithm>
#include <vector>
#include <sys/sysinfo.h>
#include <cstring>

using namespace std;

unsigned char tonum[256], tochar[4];
static void init() {
    tonum['A'] = 0;
    tonum['C'] = 1;
    tonum['T'] = 2;
    tonum['G'] = 3;
    tochar[0] = 'A';
    tochar[1] = 'C';
    tochar[2] = 'T';
    tochar[3] = 'G';
}

struct T {
    uint64_t data;
    unsigned char size;

    T(const string& s = string()) : data(0), size(s.size()) {
        reset(s, 0, s.size());
    }

    void reset(const string& s, unsigned beg, unsigned end) {
        size = end - beg;
        data = 0;
        for(unsigned i = beg; i != end; ++i) {
            data <<= 2;
            data |= tonum[unsigned(s[i])];
        }
    }

    bool operator<(const T& in) const {
        return data < in.data;
    }

    bool operator==(const T& other) const {
        return data == other.data && size == other.size;
    }

    operator string() const {
        string tmp(size, ' ');
        uint64_t tmp1 = data;
        for(unsigned i = 0; i != size; ++i) {
            tmp[i] = tochar[tmp1 & 3];
            tmp1 >>= 2;
        }
        reverse(tmp.begin(), tmp.end());
        return tmp;
    }
};

namespace std {
    template<>
    struct hash<T> {
        std::size_t operator()(const T& t) const {
            return hash<uint64_t>()(t.data) ^ hash<unsigned char>()(t.size);
        }
    };
}

template<typename MapType>
MapType calculate(const string& input, unsigned size, unsigned beg = 0, unsigned incr = 1) {
    MapType frequencies;
    T tmp;
    for (unsigned i = beg, i_end = input.size() + 1 - size; i < i_end; i+= incr) {
        tmp.reset(input, i, i + size);
        ++frequencies[tmp];
    }
    return frequencies;
}

void write_frequencies(const string& input, unsigned size) {
    unsigned sum = input.size() + 1 - size;
    auto frequencies = calculate<unordered_map<T, unsigned>>(input, size);
    vector<pair<unsigned, string>> freq;
    for (const auto& p : frequencies) {
        freq.emplace_back(p.second, p.first);
    }
    sort(freq.rbegin(), freq.rend());
    for (const auto& p : freq) {
        cout << p.second << ' ' << (sum ? double(100 * p.first) / sum : 0.0) << '\n';
    }
    cout << '\n';
}

void write_count(const string& input, const string& str) {
    unsigned size = str.size();
    auto frequencies = calculate<unordered_map<T, unsigned>>(input, size);
    cout << frequencies[T(str)] << '\t' << str << '\n';
}

int main() {
    init();
    string input;
    char buffer[256];
    while (fgets(buffer, sizeof(buffer), stdin) && memcmp(">THREE", buffer, 6) != 0);
    while (fgets(buffer, sizeof(buffer), stdin) && buffer[0] != '>') {
        if (buffer[0] != ';') {
            input.append(buffer, strcspn(buffer, "\n"));
        }
    }
    transform(input.begin(), input.end(), input.begin(), ::toupper);

    cout << setprecision(3) << fixed;
    write_frequencies(input, 1);
    write_frequencies(input, 2);
    write_count(input, "GGT");
    write_count(input, "GGTA");
    write_count(input, "GGTATT");
    write_count(input, "GGTATTTTAATT");
    write_count(input, "GGTATTTTAATTTATAGT");
    return 0;
}