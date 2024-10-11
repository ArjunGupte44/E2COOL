#include <iostream>
#include <vector>
#include <numeric>
#include <algorithm>
#include <thread>
#include <mutex>

struct IUB {
    float p;
    char c;
};

const std::string alu = "GGCCGGGCGCGGTGGCTCACGCCTGTAATCCC";
std::array<IUB, 15> iub = {{{0.27f, 'a'}, {0.12f, 'c'}, {0.12f, 'g'}, {0.27f, 't'},
                            {0.02f, 'B'}, {0.02f, 'D'}, {0.02f, 'H'}, {0.02f, 'K'},
                            {0.02f, 'M'}, {0.02f, 'N'}, {0.02f, 'R'}, {0.02f, 'S'},
                            {0.02f, 'V'}, {0.02f, 'W'}, {0.02f, 'Y'}}};
std::array<IUB, 4> homosapiens = {{{0.3029549426680f, 'a'}, {0.1979883004921f, 'c'},
                                   {0.1975473066391f, 'g'}, {0.3015094502008f, 't'}}};

const int IM = 139968;
const float IM_RECIPROCAL = 1.0f / IM;

uint32_t generateRandomNumber() {
    static thread_local int last = 42;
    const int IA = 3877, IC = 29573;
    last = (last * IA + IC) % IM;
    return last;
}

void makeCumulative(std::array<IUB, 15> &data) {
    std::partial_sum(data.begin(), data.end(), data.begin(),
                     [](IUB a, IUB b) -> IUB { b.p += a.p; return b; });
}

char convertRandom(uint32_t rand, const std::array<IUB, 15> &data) {
    const float p = rand * IM_RECIPROCAL;
    auto it = std::find_if(data.begin(), data.end(), [p](IUB x) { return p <= x.p; });
    return it->c;
}

char convertIUB(uint32_t rand) {
    return convertRandom(rand, iub);
}

void process(std::vector<char> &block, const std::array<IUB, 15> &data) {
    for (auto &c : block) {
        uint32_t rand = generateRandomNumber();
        c = convertIUB(rand);
    }
    std::for_each(block.begin(), block.end(), [](char &c) { std::cout << c; });
    std::cout << "\n";
}

int main() {
    const size_t blockSize = 1024;
    std::vector<char> block(blockSize);  
    makeCumulative(iub);
    
    std::thread worker(process, std::ref(block), std::cref(iub));
    worker.join();
    
    return 0;
}