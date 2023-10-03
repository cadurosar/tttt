#pragma once

#include "query/queries.hpp"
#include "topk_queue.hpp"
#include <string>
#include <vector>

namespace pisa {

struct Rescore_Query {
    std::optional<std::string> query_id;
    std::vector<std::uint64_t> document_ids;
};

[[nodiscard]] std::function<void(const std::string)> resolve_rescore_parser(
    std::vector<Rescore_Query>& rescore_query);

[[nodiscard]] auto parse_rescore_ids(std::string const& query_string) -> Rescore_Query;


struct rescore {
    explicit rescore(topk_queue& topk) : m_topk(topk) {}
    
    template <typename CursorRange>
    void operator()(CursorRange&& cursors, std::vector<uint64_t> rescore_query_vector)
    {
        if (cursors.empty()) {
            return;
        }

        for (const uint64_t& cur_doc : rescore_query_vector) {

            float score = 0;
            for (size_t i = 0; i < cursors.size(); ++i) {
                cursors[i].next_geq(cur_doc);
                if (cursors[i].docid() == cur_doc) {
                    score += cursors[i].score();
                }            
            }
            m_topk.insert(score, cur_doc);
        }
    }

    std::vector<typename topk_queue::entry_type> const& topk() const { return m_topk.topk(); }

  private:
    topk_queue& m_topk;
};

}  // namespace pisa
