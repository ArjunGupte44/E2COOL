#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <thread>
#include <mutex>
#include <cstring>

std::mutex print_mutex;

struct ReverseLookup {
    ReverseLookup(const std::string& from, const std::string& to) {
        std::fill(byteLookup, byteLookup + 256, 0);
        for (size_t i = 0; i < from.size(); ++i) {
            byteLookup[static_cast<unsigned char>(from[i])] = to[i];
            byteLookup[static_cast<unsigned char>(std::tolower(from[i]))] = to[i];
        }
    }

    char operator[](const char& c) const {
        return byteLookup[static_cast<unsigned char>(c)];
    }

    unsigned char byteLookup[256];
};

ReverseLookup lookup("acbdghkmnsrutwvy", "TGVHCDMKNSYAAWBR");

void reverse_and_complement(char* begin, char* end) {
    std::reverse(begin, end);
    std::transform(begin, end, begin, [&](char c) { return lookup[c]; });
}

struct Chunk {
    std::string header;
    std::vector<char> data;
};

void print_chunk(const Chunk& chunk) {
    std::lock_guard<std::mutex> guard(print_mutex);
    std::cout << chunk.header << std::endl;
    for (size_t i = 0; i < chunk.data.size(); i += 60) {
        std::cout.write(chunk.data.data() + i, std::min(size_t(60), chunk.data.size() - i)) << '\n';
    }
}

void process_chunk(const std::string& input, size_t start, size_t end, std::vector<Chunk>& chunks) {
    size_t pos = start;
    while (pos < end) {
        size_t next_pos = input.find('\n', pos);
        if (next_pos == std::string::npos || next_pos > end) break;

        Chunk chunk;
        chunk.header = input.substr(pos, next_pos - pos);
        pos = next_pos + 1;

        next_pos = input.find('>', pos);
        if (next_pos == std::string::npos || next_pos > end) next_pos = end;

        chunk.data.reserve(next_pos - pos);
        for (size_t i = pos; i < next_pos; ++i) {
            if (input[i] != '\n') chunk.data.push_back(input[i]);
        }

        pos = next_pos + 1;
        reverse_and_complement(chunk.data.data(), chunk.data.data() + chunk.data.size());
        chunks.push_back(std::move(chunk));
    }
}

int main() {
    std::ios::sync_with_stdio(false);
    std::string input((std::istreambuf_iterator<char>(std::cin)), std::istreambuf_iterator<char>());
    
    std::vector<Chunk> chunks;
    chunks.reserve(100); // Assumes a reasonable number of chunks

    const size_t hardware_threads = std::thread::hardware_concurrency();
    const size_t chunk_length = input.size() / hardware_threads + 1;
    std::vector<std::thread> threads;

    for (size_t i = 0; i < hardware_threads; ++i) {
        size_t start = i * chunk_length;
        size_t end = std::min(input.size(), (i + 1) * chunk_length);
        threads.emplace_back(process_chunk, std::ref(input), start, end, std::ref(chunks));
    }

    for (auto& t : threads) {
        if (t.joinable()) t.join();
    }

    for (const auto& chunk : chunks) {
        print_chunk(chunk);
    }

    return 0;
}