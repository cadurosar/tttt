#pragma once

#include <vector>

#include "query/queries.hpp"
#include "scorer/index_scorer.hpp"
#include "wand_data.hpp"

namespace pisa {

template <typename Cursor>
class ScoredCursor {
  public:
    using base_cursor_type = Cursor;

    ScoredCursor(Cursor cursor, TermScorer term_scorer, float query_weight, bool scorer=true)
        : m_base_cursor(std::move(cursor)),
          m_term_scorer(std::move(term_scorer)),
          m_query_weight(query_weight),
          m_scorer(scorer)
    {}
    ScoredCursor(Cursor cursor, float query_weight, bool scorer=false)
        : m_base_cursor(std::move(cursor)),
          m_query_weight(query_weight),
          m_scorer(scorer)
    {}
    ScoredCursor(ScoredCursor const&) = delete;
    ScoredCursor(ScoredCursor&&) = default;
    ScoredCursor& operator=(ScoredCursor const&) = delete;
    ScoredCursor& operator=(ScoredCursor&&) = default;
    ~ScoredCursor() = default;

    [[nodiscard]] PISA_ALWAYSINLINE auto query_weight() const noexcept -> float
    {
        return m_query_weight;
    }
    [[nodiscard]] PISA_ALWAYSINLINE auto docid() const -> std::uint32_t
    {
        return m_base_cursor.docid();
    }
    [[nodiscard]] PISA_ALWAYSINLINE auto freq() -> std::uint32_t { return m_base_cursor.freq(); }
    [[nodiscard]] PISA_ALWAYSINLINE auto score() -> float { 
        if (m_scorer)
        {
            return m_term_scorer(docid(), freq());
        }
        else
        {
            return freq()*m_query_weight;
        }
     }
    void PISA_ALWAYSINLINE next() { m_base_cursor.next(); }
    void PISA_ALWAYSINLINE next_geq(std::uint32_t docid) { m_base_cursor.next_geq(docid); }
    [[nodiscard]] PISA_ALWAYSINLINE auto size() -> std::size_t { return m_base_cursor.size(); }

  private:
    Cursor m_base_cursor;
    TermScorer m_term_scorer;
    float m_query_weight = 1.0;
    bool m_scorer = true;
};

template <typename Index, typename Scorer>
[[nodiscard]] auto
make_scored_cursors(Index const& index, Scorer const& scorer, Query query, bool weighted = false)
{
    auto terms = query.terms;
    auto query_term_freqs = query_freqs(terms);

    std::vector<ScoredCursor<typename Index::document_enumerator>> cursors;

    cursors.reserve(query_term_freqs.size());
    std::transform(
        query_term_freqs.begin(), query_term_freqs.end(), std::back_inserter(cursors), [&](auto&& term) {
            auto term_weight = 1.0f;
            auto term_id = term.first;

            if (weighted) {
                term_weight = term.second;
                auto indexed_term = index[term_id];
                auto new_scorer = scorer.term_scorer(term_id);
                return ScoredCursor<typename Index::document_enumerator>(
                    indexed_term,
                    [new_scorer, weight = term_weight](
                        uint32_t doc, uint32_t freq) { return weight * new_scorer(doc, freq); },
                    term_weight);
            }
            return ScoredCursor<typename Index::document_enumerator>(
                index[term_id], scorer.term_scorer(term_id), term_weight);
        });
    return cursors;
}

template <typename Index>
[[nodiscard]] auto
make_scored_cursors_2(Index const& index2, Query query, bool weighted = false)
{
    auto terms = query.terms;
    auto query_term_freqs = query_freqs(terms);

    std::vector<ScoredCursor<typename Index::document_enumerator>> cursors;

    cursors.reserve(query_term_freqs.size());
    std::transform(
        query_term_freqs.begin(), query_term_freqs.end(), std::back_inserter(cursors), [&](auto&& term) {
            auto term_weight = 1.0f;
            auto term_id = term.first;

            term_weight = term.second;
            auto indexed_term = index2[term_id];
            return ScoredCursor<typename Index::document_enumerator>(
                indexed_term,
                term_weight);
        }
        );
    return cursors;
}


}  // namespace pisa
