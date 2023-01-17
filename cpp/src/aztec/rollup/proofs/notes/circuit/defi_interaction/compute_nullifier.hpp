#pragma once
#include <stdlib/types/types.hpp>
#include "../../constants.hpp"

namespace rollup {
namespace proofs {
namespace notes {
namespace circuit {
namespace defi_interaction {

using namespace plonk::stdlib::types;

/**
 * nonce - randomness provided by the user (sdk) to ensure uniqueness.
 */
inline field_ct compute_nullifier(field_ct const& defi_interaction_note_commitment, field_ct const& nonce)
{
    return pedersen_commitment::compress(std::vector<field_ct>{ defi_interaction_note_commitment, nonce },
                                         GeneratorIndex::DEFI_INTERACTION_NULLIFIER);
}

} // namespace defi_interaction
} // namespace circuit
} // namespace notes
} // namespace proofs
} // namespace rollup