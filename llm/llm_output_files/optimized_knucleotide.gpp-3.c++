#include <iostream>
#include <iomanip>
#include <string>
#include <cstring>
#include <map>
#include <unordered_map>
#include <thread>
#include <future>
#include <vector>
#include <algorithm>
#include <unistd.h>

unsigned char tonum[256];
char tochar[4];
unsigned power4[32];

void init()
{
    tonum['A'] = 0;
    tonum['C'] = 1;
    tonum['T'] = 2;
    tonum['G'] = 3;
    tochar[0] = 'A';
    tochar[1] = 'C';
    tochar[2] = 'T';
    tochar[3] = 'G';
    for (int i = 0; i < 32; ++i)
        power4[i] = 1 << (2 * i);
}

struct T {
    uint64_t data = 0;
    unsigned char size = 0;

    T(const std::string &s = std::string(), unsigned beg = 0, unsigned length = 0) {
        size = length;
        reset(s, beg, beg + length);
    }

    void reset(const std::string &s, unsigned beg, unsigned end) {
        size = end - beg;
        data = 0;
        for (unsigned i = beg; i != end; ++i) {
            data <<= 2;
            data |= tonum[static_cast<unsigned char>(s[i])];
        }
    }

    void roll_forward(char out, char in) {
        data = ((data << 2) | tonum[static_cast<unsigned char>(in)]) & (~((~0ULL) << (2 * size)));
    }

    operator std::string() const {
        std::string tmp(size, ' ');
        uint64_t temp_data = data;
        for (unsigned i = 0; i != size; ++i) {
            tmp[size - i - 1] = tochar[temp_data & 3];
            temp_data >>= 2;
        }
        return tmp;
    }

    bool operator<(const T &in) const {
        return data < in.data;
    }

    bool operator==(const T &in) const {
        return data == in.data;
    }
};

struct hash_T {
    std::size_t operator()(const T &t) const {
        return t.data;
    }
};

using FrequencyMap = std::unordered_map<T, unsigned, hash_T>;

FrequencyMap calculate(const std::string &input, unsigned size, unsigned start = 0, unsigned step = 1) {
    FrequencyMap frequencies;
    T tmp(input, start, size);
    frequencies[tmp]++;
    for (unsigned i = start + size; i < input.size(); i += step) {
        tmp.roll_forward(input[i - size], input[i]);
        frequencies[tmp]++;
    }
    return frequencies;
}

FrequencyMap tcalculate(const std::string &input, unsigned size) {
    unsigned N = std::thread::hardware_concurrency();
    std::vector<std::future<FrequencyMap>> futures;

    for (unsigned i = 0; i < N; ++i) {
        futures.emplace_back(std::async(std::launch::async, calculate, std::ref(input), size, i, N));
    }

    FrequencyMap frequencies = futures[0].get();

    for (unsigned i = 1; i < N; ++i) {
        FrequencyMap local_frequencies = futures[i].get();
        for (const auto &entry : local_frequencies)
            frequencies[entry.first] += entry.second;
    }

    return frequencies;
}

void write_frequencies(const std::string &input, unsigned size) {
    unsigned sum = input.size() + 1 - size;
    FrequencyMap frequencies = tcalculate(input, size);
    std::map<unsigned, std::string, std::greater<unsigned>> freq;

    for (const auto &i : frequencies) {
        freq.emplace(i.second, static_cast<std::string>(i.first));
    }

    for (const auto &i : freq)
        std::cout << i.second << ' ' << (sum ? static_cast<double>(100 * i.first) / sum : 0.0) << '\n';

    std::cout << '\n';
}

void write_count(const std::string &input, const std::string &string) {
    unsigned size = string.size();
    FrequencyMap frequencies = tcalculate(input, size);

    std::cout << frequencies[T(string, 0, size)] << '\t' << string << '\n';
}

int main() {
    init();
    std::string input;
    char buffer[256];
    while (fgets(buffer, 100, stdin) && memcmp(">THREE", buffer, 6) != 0);
    while (fgets(buffer, 100, stdin) && buffer[0] != '>') {
        if (buffer[0] != ';') {
            input.append(buffer, strlen(buffer) - 1);
        }
    }
    std::transform(input.begin(), input.end(), input.begin(), ::toupper);

    std::cout << std::setprecision(3) << std::setiosflags(std::ios::fixed);
    write_frequencies(input, 1);
    write_frequencies(input, 2);
    write_count(input, "GGT");
    write_count(input, "GGTA");
    write_count(input, "GGTATT");
    write_count(input, "GGTATTTTAATT");
    write_count(input, "GGTATTTTAATTTATAGT");
}