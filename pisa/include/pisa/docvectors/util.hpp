#pragma once

#include <iostream>
#include <vector>
#include <iterator>
#include <fstream>
#include <istream>
#include <ios>
#include "docvectors/compress_qmx.hpp" // QMX
#include <limits>
#include <stdexcept>
#include <x86intrin.h>
#include <algorithm>
#include <numeric>
#include <sys/time.h>
#include <unordered_map>
#include <unordered_set>


    // Read lex file. Format = <string id, int id, f_t, c_t>
void read_lexicon_d (std::ifstream &is, 
                    std::unordered_map<std::string, uint32_t>& lexicon) {
    std::string term;
    size_t id, f_t, c_t;
    while (is >> term >> id >> f_t >> c_t) {
        lexicon.insert({term, id});
    }
    std::cerr << "Lexicon read: " << lexicon.size() << " terms" << std::endl;
}


void generate_stoplist(std::ifstream& is,
                    std::unordered_map<std::string, uint32_t>& lex,
                    std::unordered_set<uint32_t>& stoplist) {
    std::string term;
    while (is >> term) {
        auto lex_term = lex.find(term);
        // If not OOV
        if (lex_term != lex.end()) {
            stoplist.insert(lex_term->second);
        }
    }
    std::cerr << "Stoplist contains: " << stoplist.size() << " terms" << std::endl;
}
                                      
    

  // STOLEN FROM FASTPFORLIB: Please credit in final README/paper
template <class T> static void delta(T *data, const size_t size) {
  if (size == 0)
    throw std::runtime_error("delta coding impossible with no value!");
  for (size_t i = size - 1; i > 0; --i) {
    data[i] -= data[i - 1];
  }
}

template <class T> static void fastDelta(T *pData, const size_t TotalQty) {
  if (TotalQty < 5) {
    delta(pData, TotalQty); // no SIMD
    return;
  }

  const size_t Qty4 = TotalQty / 4;
  __m128i *pCurr = reinterpret_cast<__m128i *>(pData);
  const __m128i *pEnd = pCurr + Qty4;

  __m128i last = _mm_setzero_si128();
  while (pCurr < pEnd) {
    __m128i a0 = _mm_load_si128(pCurr);
    __m128i a1 = _mm_sub_epi32(a0, _mm_srli_si128(last, 12));
    a1 = _mm_sub_epi32(a1, _mm_slli_si128(a0, 4));
    last = a0;
    _mm_store_si128(pCurr++, a1);
  }

  if (Qty4 * 4 < TotalQty) {
    uint32_t lastVal = _mm_cvtsi128_si32(_mm_srli_si128(last, 12));
    for (size_t i = Qty4 * 4; i < TotalQty; ++i) {
      uint32_t newVal = pData[i];
      pData[i] -= lastVal; 
      lastVal = newVal;
    }
  }
}  