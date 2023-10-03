#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>

#include "index_scorer.hpp"
namespace pisa {

template <typename Wand>
struct saturated_tf: public index_scorer<Wand> {
    using index_scorer<Wand>::index_scorer;

    saturated_tf(const Wand& wdata, const float k1)
        : index_scorer<Wand>(wdata), m_k1(k1)
    {}

    float doc_term_weight(uint64_t freq) const
    {
        auto f = static_cast<float>(freq);
        return f / (f + m_k1);
    }

    // IDF (inverse document frequency)
    float query_term_weight(uint64_t df, uint64_t num_docs) const
    {
        return (1.0F + m_k1);
    }

    term_scorer_t term_scorer(uint64_t term_id) const override
    {
        auto term_weight = (1.0F + m_k1);
        auto s = [&, term_weight](uint32_t doc, uint32_t freq) {
            return term_weight * doc_term_weight(freq);
        };
        return s;
    }

  private:
    float m_k1;
};
}  // namespace pisa
