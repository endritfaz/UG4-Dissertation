#include <array>
#include <algorithm>
#include <random>
#include <cstdint>

namespace shuffle {

class shuffler {
private:
    std::random_device rd;
    std::mt19937 g;
    std::array<int, 64> perm;

    void generate_permutation() {
        for (int i = 0; i < 64; ++i) {
            perm[i] = i;
        }
        std::shuffle(perm.begin(), perm.end(), g);
    }

    uint64_t permute_bitboard(uint64_t board) const {
        uint64_t result = 0ULL;

        for (int i = 0; i < 64; ++i) {
            if (board & (1ULL << i)) {
                result |= (1ULL << perm[i]);
            }
        }

        return result;
    }

public:
    shuffler();

    std::array<uint64_t, 2> shuffle_bitboards(uint64_t black, uint64_t white);
};

shuffler::shuffler()
    : g(rd()) {
    generate_permutation();
}

std::array<uint64_t, 2> shuffler::shuffle_bitboards(uint64_t black, uint64_t white) {
    uint64_t shuffled_black = permute_bitboard(black);
    uint64_t shuffled_white = permute_bitboard(white);

    return {shuffled_black, shuffled_white};
}

}