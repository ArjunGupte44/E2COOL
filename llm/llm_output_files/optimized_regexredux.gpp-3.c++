#include <boost/regex.hpp>
#include <iostream>
#include <cassert>
#include <cstdio>
#include <vector>
#include <omp.h>

using namespace std;

const std::size_t BUFSIZE = 1024;
const boost::regex::flag_type re_flags = boost::regex::perl;

int main(void)
{
    string str;
    int len1, len2;
    int read_size;
    
    // Define regex patterns
    boost::regex re_filters[9];
    re_filters[0] = boost::regex("agggtaaa|tttaccct", re_flags);
    re_filters[1] = boost::regex("[cgt]gggtaaa|tttaccc[acg]", re_flags);
    re_filters[2] = boost::regex("a[act]ggtaaa|tttacc[agt]t", re_flags);
    re_filters[3] = boost::regex("ag[act]gtaaa|tttac[agt]ct", re_flags);
    re_filters[4] = boost::regex("agg[act]taaa|ttta[agt]cct", re_flags);
    re_filters[5] = boost::regex("aggg[acg]aaa|ttt[cgt]ccct", re_flags);
    re_filters[6] = boost::regex("agggt[cgt]aa|tt[acg]accct", re_flags);
    re_filters[7] = boost::regex("agggta[cgt]a|t[acg]taccct", re_flags);
    re_filters[8] = boost::regex("agggtaa[cgt]|[acg]ttaccct", re_flags);

    fseek(stdin, 0, SEEK_END);
    read_size = ftell(stdin);
    assert(read_size > 0);

    str.resize(read_size);
    rewind(stdin);
    read_size = fread(&str[0], 1, read_size, stdin);
    assert(read_size);

    len1 = str.length();
    boost::regex re1(">[^\n]+\n|[\n]", re_flags);
    boost::regex_replace(str, re1, "").swap(str);
    len2 = str.length();

    // Prepare to count matches
    std::vector<int> counts(9, 0);

    // OpenMP parallel sections with compiled regex
    #pragma omp parallel for
    for (int i = 0; i < 9; i++) {
        boost::smatch m;
        std::string::const_iterator start = str.begin(), end = str.end();
        while (boost::regex_search(start, end, m, re_filters[i])) {
            ++counts[i];
            start += m.position() + m.length();
        }
    }

    // Alternative pattern replacements
    string out = str;
    string replacements[] = {"tHa[Nt]", "<4>", "aND|caN|Ha[DS]|WaS", "<3>", "a[NSt]|BY", "<2>", "<[^>]*>", "|", "\\|[^|][^|]*\\|", "-"};
    for (size_t i = 0; i < sizeof(replacements) / sizeof(replacements[0]); i += 2) {
        boost::regex re(replacements[i], re_flags);
        boost::regex_replace(out, re, replacements[i + 1]).swap(out);
    }

    // Print results
    for (int i = 0; i < 9; ++i)
        cout << re_filters[i].str() << " " << counts[i] << "\n";

    cout << "\n";
    cout << len1 << "\n";
    cout << len2 << "\n";
    cout << out.length() << endl;
}