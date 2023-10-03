#include "query/algorithm/rescore.hpp"
#include "query/queries.hpp"

#include <boost/algorithm/string.hpp>
#include <spdlog/spdlog.h>
#include <boost/lexical_cast.hpp>

namespace pisa {

    std::function<void(const std::string)> resolve_rescore_parser(
        std::vector<Rescore_Query>& rescore_query)
    {
        return [&rescore_query](std::string const& rescore_line) {
            rescore_query.push_back(parse_rescore_ids(rescore_line));
        };
    }

    auto parse_rescore_ids(std::string const& query_string) -> Rescore_Query
    {
        auto [id, raw_query] = split_query_at_colon(query_string);

        std::vector<std::string> string_scores;
        std::vector<uint64_t> parsed_query;
        boost::split(string_scores, raw_query, boost::is_any_of("\t, ,\v,\f,\r,\n"));

        auto is_empty = [](const std::string& val) { return val.empty(); };
        // remove_if move matching elements to the end, preparing them for erase.
        string_scores.erase(std::remove_if(string_scores.begin(), string_scores.end(), is_empty), string_scores.end());

        try {
            auto to_uint64 = [](const std::string& val) { return boost::lexical_cast<uint64_t>(val); };
            std::transform(string_scores.begin(), string_scores.end(), std::back_inserter(parsed_query), to_uint64);
        } catch (std::invalid_argument& err) {
            spdlog::error("Could not parse document identifiers of query `{}`", raw_query);
            exit(1);
        }
        return {std::move(id), std::move(parsed_query)};
    }

}