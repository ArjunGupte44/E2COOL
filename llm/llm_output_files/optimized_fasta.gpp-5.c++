```
#include <algorithm>
#include <array>
#include <vector>
#include <thread>
#include <iostream>
#include <numeric>
#include <atomic>

struct IUB
{
   float p;
   char c;
};

const std::string alu =
{
   "GGCCGGGCGCGGTGGCTCACGCCTGTAATCCCAGCACTTTGG"
   "GAGGCCGAGGCGGGCGGATCACCTGAGGTCAGGAGTTCGAGA"
   "CCAGCCTGGCCAACATGGTGAAACCCCGTCTCTACTAAAAAT"
   "ACAAAAATTAGCCGGGCGTGGTGGCGCGCGCCTGTAATCCCA"
   "GCTACTCGGGAGGCTGAGGCAGGAGAATCGCTTGAACCCGGG"
   "AGGCGGAGGTTGCAGTGAGCCGAGATCGCGCCACTGCACTCC"
   "AGCCTGGGCGACAGAGCGAGACTCCGTCTCAAAAA"
};

std::array<IUB,15> iub =
{{
   { 0.27f, 'a' },
   { 0.12f, 'c' },
   { 0.12f, 'g' },
   { 0.27f, 't' },
   { 0.02f, 'B' },
   { 0.02f, 'D' },
   { 0.02f, 'H' },
   { 0.02f, 'K' },
   { 0.02f, 'M' },
   { 0.02f, 'N' },
   { 0.02f, 'R' },
   { 0.02f, 'S' },
   { 0.02f, 'V' },
   { 0.02f, 'W' },
   { 0.02f, 'Y' }
}};

std::array<IUB, 4> homosapiens =
{{
   { 0.3029549426680f, 'a' },
   { 0.1979883004921f, 'c' },
   { 0.1975473066391f, 'g' },
   { 0.3015094502008f, 't' }
}};

const int IM = 139968;
const float IM_RECIPROCAL = 1.0f / IM;

uint32_t gen_random()
{
   static const int IA = 3877, IC = 29573;
   static thread_local int last = 42;  // Use thread-local for better concurrency
   last = (last * IA + IC) % IM;
   return last;
}

char convert_trivial(char c)
{
   return c;
}

char convert_random(uint32_t random, const std::array<IUB, 15>& distribution)
{
   float p = random * IM_RECIPROCAL;
   auto result = std::lower_bound(distribution.begin(), distribution.end(), p, [](const IUB& iub, float prob) { return iub.p < prob; });
   return result->c;
}

char convert_IUB(uint32_t random)
{
   return convert_random(random, iub);
}

char convert_homosapiens(uint32_t random)
{
   return convert_random(random, homosapiens);
}

void make_cumulative(std::array<IUB, 15>& distribution)
{
   std::partial_sum(distribution.begin(), distribution.end(), distribution.begin(), [](IUB& l, IUB& r) { r.p += l.p; return r; });
}

const size_t CHARS_PER_LINE = 60;
const size_t VALUES_PER_BLOCK = CHARS_PER_LINE * 1024;
const unsigned THREADS_TO_USE = std::max( 1U, std::min( 4U, std::thread::hardware_concurrency() ));

std::atomic<size_t> g_fillThreadIndex{ 0 };
std::atomic<size_t> g_totalValuesToGenerate{ 0 };

template<class generator_type>
size_t fillBlock(size_t currentThread, char* begin, generator_type& generator)
{
   while(true)
   {
      size_t currentIndex = g_fillThreadIndex.fetch_add(1, std::memory_order_relaxed) % THREADS_TO_USE;

      if(currentThread == currentIndex)
      {
         const size_t valuesToGenerate = std::min(g_totalValuesToGenerate.load(), VALUES_PER_BLOCK);
         g_totalValuesToGenerate.fetch_sub(valuesToGenerate, std::memory_order_relaxed);
         
         for(size_t i = 0; i < valuesToGenerate; ++i)
         {
            begin[i] = generator();
         }
         return valuesToGenerate;
      }
   }
}

template<class converter_type>
size_t convertBlock(const char* begin, char* outCharacter, size_t count, converter_type converter)
{
   const auto beginCharacter = outCharacter;
   size_t col = 0;
   for(size_t i = 0; i < count; ++i)
   {
      *outCharacter++ = converter(begin[i]);
      if(++col >= CHARS_PER_LINE)
      {
         col = 0;
         *outCharacter++ = '\n';
      }
   }
   if(col != 0)
   {
      *outCharacter++ = '\n';
   }
   return outCharacter - beginCharacter;
}

std::atomic<size_t> g_outThreadIndex{ 0 };

template<class iterator_type>
void writeCharacters(size_t currentThread, iterator_type begin, size_t count)
{
   while(true)
   {
      size_t currentIndex = g_outThreadIndex.load(std::memory_order_relaxed);
      if(currentThread == currentIndex)
      {
         g_outThreadIndex.store((currentThread + 1) % THREADS_TO_USE, std::memory_order_relaxed);
         fwrite( begin, count, 1, stdout );
         return;
      }
   }
}

template<class generator_type, class converter_type>
void work(size_t currentThread, generator_type& generator, converter_type& converter)
{
   std::array<char, VALUES_PER_BLOCK> block;
   std::array<char, (CHARS_PER_LINE + 1) * 1024> characters;
   
   while(true)
   {
      auto bytesGenerated = fillBlock(currentThread, block.data(), generator);
      if( bytesGenerated == 0 ) break;
      auto charactersGenerated = convertBlock(block.data(), characters.data(), bytesGenerated, converter);
      writeCharacters(currentThread, characters.data(), charactersGenerated);
   }
}

template <class generator_type, class converter_type >
void make(const char* desc, int n, generator_type generator, converter_type converter) {
   std::cout << '>' << desc << '\n';
   
   g_totalValuesToGenerate.store(n, std::memory_order_relaxed);
   g_outThreadIndex.store(0, std::memory_order_relaxed);
   g_fillThreadIndex.store(0, std::memory_order_relaxed);
   
   std::vector< std::thread > threads(THREADS_TO_USE - 1);
   for(size_t i = 0; i < threads.size(); ++i)
   {
      threads[ i ] = std::thread{ [&generator, &converter, i]() { work(i, generator, converter); } };
   }
   
   work(threads.size(), generator, converter);  // Main thread work
   
   for(auto& thread : threads)
   {
      thread.join();
   }
}

int main(int argc, char *argv[])
{
   int n = 1000;
   if (argc >= 2) {
      n = std::atoi(argv[1]);
   }
   if (n <= 0) {
      std::cerr << "usage: " << argv[0] << " length" << std::endl;
      return 1;
   }
   
   make_cumulative(iub);
   make_cumulative(homosapiens);
   
   make("ONE Homo sapiens alu"      , n * 2, alu.begin(), &convert_trivial );
   make("TWO IUB ambiguity codes"   , n * 3, []() { return gen_random(); }, &convert_IUB );
   make("THREE Homo sapiens frequency", n * 5, []() { return gen_random(); }, &convert_homosapiens );
   return 0;
}
```