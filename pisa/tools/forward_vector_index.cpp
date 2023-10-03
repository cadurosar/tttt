#include "docvectors/document_index.hpp"
#include "docvectors/util.hpp"

using namespace pisa;
// 
void printUsage(const std::string &programName) {
  std::cerr << "Usage: " << programName
            << " ds2i_prefix output_file"
            << std::endl;
}

int main(int argc, const char **argv) {

    std::string programName = argv[0];
    if (argc != 3 && argc != 4) {
    printUsage(programName);
    return 1;
    }

    std::string ds2i_prefix = argv[1];
    std::string output_filename = argv[2];
    std::string lexicon_file = ds2i_prefix + ".lexicon";
    std::string stop_file;
    std::unordered_map<std::string, uint32_t> lexicon;
    std::unordered_set<uint32_t> stoplist;

    document_index idx(ds2i_prefix, stoplist);
    std::ofstream ofs(output_filename, std::ios::binary);
    idx.serialize(ofs);

}
