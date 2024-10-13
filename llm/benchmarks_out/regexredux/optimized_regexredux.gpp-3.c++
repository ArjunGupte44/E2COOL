#include <boost/regex.hpp>
#include <cassert>
#include <fstream>
#include <iostream>
#include <cstdio>
#include <iterator>
#include <sstream>
#include <unordered_map>

using namespace std;

const boost::regex::flag_type re_flags = boost::regex::perl;

int main(void) {
    // Read the entire input into a string in one go
    istreambuf_iterator<char> beg(stdin), end;
    string input(beg, end);
    
    // Predefine patterns
    char const * pattern1[] = {
        "agggtaaa|tttaccct",
        "[cgt]gggtaaa|tttaccc[acg]",
        "a[act]ggtaaa|tttacc[agt]t",
        "ag[act]gtaaa|tttac[agt]ct",
        "agg[act]taaa|ttta[agt]cct",
        "aggg[acg]aaa|ttt[cgt]ccct",
        "agggt[cgt]aa|tt[acg]accct",
        "agggta[cgt]a|t[acg]taccct",
        "agggtaa[cgt]|[acg]ttaccct"
    };

    string const pattern2[] = {
        "tHa[Nt]", "<4>", "aND|caN|Ha[DS]|WaS", "<3>", "a[NSt]|BY", "<2>",
        "<[^>]*>", "|", "\\|[^|][^|]*\\|", "-"
    };

    int len1 = input.length();
    int len2;
    
    // Remove unwanted parts using regex
    boost::regex re1(">[^\n]+\n|[\n]", re_flags);
    string clean = boost::regex_replace(input, re1, "");
    len2 = clean.length();

    // Using unordered_map for frequency counting
    unordered_map<string, int> counts;
    
    // Capture frequencies
    for (const char* pat : pattern1) {
        boost::regex r(pat, re_flags);
        auto words_begin = boost::sregex_iterator(clean.begin(), clean.end(), r);
        auto words_end = boost::sregex_iterator();
        
        for (auto it = words_begin; it != words_end; ++it) {
            ++counts[pat];
        }
    }

    string result = clean;

    // Regex replacements
    for (size_t i = 0; i < pattern2.size(); i += 2) {
        boost::regex r(pattern2[i], re_flags);
        result = boost::regex_replace(result, r, pattern2[i + 1]);
    }

    // Output
    for (const char* pat : pattern1) {
        cout << pat << " " << counts[pat] << "\n";
    }

    cout << "\n" << len1 << "\n" << len2 << "\n" << result.length() << endl;
}