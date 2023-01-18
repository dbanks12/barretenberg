#pragma once
#include <array>
#include <stdlib/primitives/uint/uint.hpp>
#include <stdlib/primitives/packed_byte_array/packed_byte_array.hpp>
#include <stdlib/primitives/byte_array/byte_array.hpp>

namespace waffle {
class UltraComposer;
} // namespace waffle

namespace plonk {
namespace stdlib {
template <typename Composer> class bit_array;

/**
 * @brief Keccaaaaaaak
 *
 * Creates constraints that evaluate the Keccak256 hash algorithm.
 *
 * UltraPlonk only due to heavy lookup table use.
 *
 * Current cost 15,989 constraints for a 1-block hash
 *
 * @tparam Composer
 */
template <typename Composer> class keccak {
  public:
    using witness_ct = stdlib::witness_t<Composer>;
    using field_ct = stdlib::field_t<Composer>;
    using byte_array_ct = stdlib::byte_array<Composer>;

    // base of extended representation we use for efficient logic operations
    static constexpr uint256_t BASE = 11;

    // number of bits of hash output
    static constexpr size_t BITS = 256;

    // word size of hash lane
    static constexpr size_t WORD_SIZE = 8;

    // block size. We only support keccak256 with a 1088-bit rate!
    static constexpr size_t BLOCK_SIZE = (1600 - BITS * 2) / WORD_SIZE;

    // how many limbs fit into a block (17)
    static constexpr size_t LIMBS_PER_BLOCK = BLOCK_SIZE / 8;

    // round constants. Used in IOTA round
    static constexpr std::array<uint64_t, 24> RC = {
        0x0000000000000001, 0x0000000000008082, 0x800000000000808a, 0x8000000080008000, 0x000000000000808b,
        0x0000000080000001, 0x8000000080008081, 0x8000000000008009, 0x000000000000008a, 0x0000000000000088,
        0x0000000080008009, 0x000000008000000a, 0x000000008000808b, 0x800000000000008b, 0x8000000000008089,
        0x8000000000008003, 0x8000000000008002, 0x8000000000000080, 0x000000000000800a, 0x800000008000000a,
        0x8000000080008081, 0x8000000000008080, 0x0000000080000001, 0x8000000080008008
    };

    // Rotation offsets, y vertically, x horizontally: r[y * 5 + x]
    static constexpr std::array<size_t, 25> ROTATIONS = { 0,  1,  62, 28, 27, 36, 44, 6,  55, 20, 3,  10, 43,
                                                          25, 39, 41, 45, 15, 21, 8,  18, 2,  61, 56, 14 };

    static constexpr uint256_t convert_to_sparse(uint256_t input)
    {
        uint256_t output = 0;
        uint256_t scale_factor = 1;
        while (input > 0) {
            uint256_t slice = input & 1;
            output += (slice * scale_factor);
            scale_factor *= 11;
            input = input >> 1;
        }
        return output;
    };

    static constexpr std::array<uint256_t, 24> get_sparse_round_constants()
    {
        std::array<uint256_t, 24> output;
        for (size_t i = 0; i < 24; ++i) {
            output[i] = convert_to_sparse(RC[i]);
        }
        return output;
    }

    static constexpr std::array<uint256_t, 24> SPARSE_RC = get_sparse_round_constants();

    /**
     * @brief Compute the constant offset added in the `Chi` round
     *
     * We want to compute, for each bit slice of the inputs A, B, C...
     *  1 + 2A - B + C
     *
     * i.e. we need to add the constant value \sum_{i=0}^63 11^i
     *
     * @return constexpr uint256_t
     */
    static constexpr uint256_t get_chi_offset()
    {
        uint256_t result = 0;
        for (size_t i = 0; i < 64; ++i) {
            result *= 11;
            result += 1;
        }
        return result;
    }

    struct keccak_state {
        std::array<field_ct, 25> state;
        std::array<field_ct, 25> state_msb;
        std::array<field_ct, 25> twisted_state;
        Composer* context;
    };

    static void compute_twisted_state(keccak_state& internal);
    static void theta(keccak_state& state);
    static void rho(keccak_state& state);
    static void pi(keccak_state& state);
    static void chi(keccak_state& state);
    static void iota(keccak_state& state, size_t round);
    static void sponge_absorb(keccak_state& internal,
                              const std::vector<field_ct>& input_buffer,
                              const std::vector<field_ct>& msb_buffer);
    static byte_array_ct sponge_squeeze(keccak_state& internal);
    static void keccakf1600(keccak_state& state);
    static byte_array_ct hash(byte_array_ct& input);
};

extern template class keccak<waffle::UltraComposer>;

} // namespace stdlib
} // namespace plonk
