#ifndef SHUFFLER_H
#define SHUFFLER_H

#include <array>
#include <random>
#include <cstdint>

namespace shuffle {

class shuffler {
private:
    std::random_device rd;
    std::mt19937 g;
    std::array<int, 64> perm;

    void generate_permutation();
    uint64_t permute_bitboard(uint64_t board) const;

public:
    shuffler();
    std::array<uint64_t, 2> shuffle_bitboards(uint64_t black, uint64_t white);
};

}
#endif 