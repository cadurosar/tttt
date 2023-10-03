#pragma once

#include "docvectors/util.hpp"
#include "docvectors/document_vector.hpp"
#include "query/algorithm.hpp"

namespace pisa {
    class document_index {
    
    private: 
        // m_doc_vectors[i] returns the document_vector for document i
        std::vector<document_vector> m_doc_vectors;
        uint32_t no_terms;
    public:
        uint32_t m_size;

        document_index() : m_size(0) {}

        // Build a document index from ds2i files
        document_index(std::string ds2i_basename, std::unordered_set<uint32_t>& stoplist) {
            // Temporary 'plain' index structures
            std::vector<std::vector<uint32_t>> plain_terms;
            std::vector<std::vector<uint32_t>> plain_freqs;
            std::cout << "Before reading" <<std::endl;

            // Read DS2i document file prefix
            std::ifstream docs (ds2i_basename + ".docs", std::ios::binary);
            std::cout << "Before freqs" <<std::endl;
            std::ifstream freqs (ds2i_basename + ".freqs", std::ios::binary);
            std::cout << "READ" <<std::endl;

            // Check files are OK

            // Read first sequence from docs
            uint32_t one;
            docs.read(reinterpret_cast<char *>(&one), sizeof(uint32_t));
            docs.read(reinterpret_cast<char *>(&m_size), sizeof(uint32_t));
            m_doc_vectors.reserve(m_size); // Note: not yet constructed
            plain_terms.resize(m_size); 
            plain_freqs.resize(m_size);

            uint32_t d_seq_len = 0;
            uint32_t f_seq_len = 0;
            uint32_t term_id = 0;
            // Sequences are now aligned. Walk them.        
            while(!docs.eof() && !freqs.eof()) {

                // Check if the term is stopped
                bool stopped = (stoplist.find(term_id) != stoplist.end());


                docs.read(reinterpret_cast<char *>(&d_seq_len), sizeof(uint32_t));
                freqs.read(reinterpret_cast<char *>(&f_seq_len), sizeof(uint32_t));
                if (d_seq_len != f_seq_len) {
                    std::cerr << "ERROR: Freq and Doc sequences are not aligned. Exiting."
                            << std::endl;
                    exit(EXIT_FAILURE);
                }
                uint32_t seq_count = 0;
                uint32_t docid = 0;
                uint32_t fdt = 0;
                while (seq_count < d_seq_len) {
                    docs.read(reinterpret_cast<char *>(&docid), sizeof(uint32_t));
                    freqs.read(reinterpret_cast<char *>(&fdt), sizeof(uint32_t));
                    // Only emplace unstopped terms
                    if (!stopped) {
                        plain_terms[docid].emplace_back(term_id);
                        plain_freqs[docid].emplace_back(fdt);
                    }
                    ++seq_count;
                }
                ++term_id;
            }
            no_terms = term_id;
            
            std::cerr << "Read " << m_size << " lists and " << term_id 
                    << " unique terms. Compressing.\n";

            // Now iterate the plain index, compress, and store
            for (size_t i = 0; i < m_size; ++i) {
                m_doc_vectors.emplace_back(i, plain_terms[i], plain_freqs[i]);
            }

        } 

        void serialize(std::ostream& out) {
            out.write(reinterpret_cast<const char *>(&no_terms), sizeof(no_terms));
            out.write(reinterpret_cast<const char *>(&m_size), sizeof(m_size));
            for (size_t i = 0; i < m_size; ++i) {
                m_doc_vectors[i].serialize(out);
            }
        }

        void load(std::string inf) {
            std::ifstream in(inf, std::ios::binary);
            load(in);
        }

        void load(std::istream& in) {
            in.read(reinterpret_cast<char *>(&no_terms), sizeof(no_terms));
            in.read(reinterpret_cast<char *>(&m_size), sizeof(m_size));
            std::cout << "Reading " << to_string(m_size) << "Vectors" << std::endl;
            m_doc_vectors.resize(m_size);
            for (size_t i = 0; i < m_size; ++i) {
                m_doc_vectors[i].load(in);
            }
        }

        std::vector<typename topk_queue::entry_type>
        forward_retrieval (Query query, uint64_t k, std::vector<uint64_t> doc_ids) {
            // 0. Result init
            topk_queue topk(k);
            auto terms = query.terms;
            auto query_term_freqs = query_freqs(terms);

            // Sort the combined vector based on the values in vec1
            std::sort(query_term_freqs.begin(), query_term_freqs.end(), [](const auto& a, const auto& b) {
                return a.first < b.first;
            });


            // 1. For each document to rescore
            for (const uint64_t& current_doc_id : doc_ids) {
                auto cur_doc = m_doc_vectors[current_doc_id];
                auto vec_it = document_vector::vector_iterator(cur_doc,0);
                uint64_t final_score = 0;
                bool break_the_loop = false;
                // for each term of the query
                for (auto cur_term : query_term_freqs) {

                    while (vec_it.termid() < cur_term.first)
                    {
                        if (vec_it.is_end())
                        {
                            break_the_loop = true;
                            break;
                        }
                        else
                        {
                            vec_it.next();
                        }
                    }
                    if (break_the_loop) {  break;}
                    else if (cur_term.first == vec_it.termid())
                    {
                        final_score += cur_term.second * vec_it.freq();
                    }
                }
                topk.insert(final_score, current_doc_id);
            }
            return topk.topk();
            
        }
    };
}