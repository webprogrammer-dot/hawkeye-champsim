#include "hawkeye.h"

namespace {

bool is_hawkeye_access(access_type type)
{
    // Let every LLC access participate in Hawkeye.
    (void)type;
    return true;
}

} // namespace


hawkeye::hawkeye(CACHE* cache)
    : champsim::modules::replacement(cache),
      optgen(cache->NUM_SET, cache->NUM_WAY),
      predictor(8192, 3),
      rrpv_table(
          cache->NUM_SET,
          std::vector<int>(cache->NUM_WAY, MAX_RRPV)),
      addr_to_pc(cache->NUM_SET)
{
}


long hawkeye::find_victim(
    uint32_t triggering_cpu,
    uint64_t instr_id,
    uint32_t set,
    const champsim::cache_block* current_set,
    uint64_t ip,
    uint64_t full_addr,
    uint32_t type)
{
    (void)triggering_cpu;
    (void)instr_id;
    (void)ip;
    (void)full_addr;
    (void)type;

    // Always prefer an invalid block.
    for (std::size_t way = 0;
         way < rrpv_table[set].size();
         ++way) {

        if (!current_set[way].valid) {
            return static_cast<long>(way);
        }
    }

    // Otherwise use Hawkeye RRIP victim selection.
    return static_cast<long>(
        ::find_victim(rrpv_table[set])
    );
}


void hawkeye::replacement_cache_fill(
    uint32_t triggering_cpu,
    long set,
    long way,
    champsim::address full_addr,
    champsim::address ip,
    champsim::address victim_addr,
    access_type type)
{
    (void)triggering_cpu;
    (void)full_addr;
    (void)victim_addr;
    (void)type;

    const std::size_t set_idx =
        static_cast<std::size_t>(set);

    const std::size_t way_idx =
        static_cast<std::size_t>(way);

    const uint64_t current_ip =
        ip.to<uint64_t>();

    /*
     * Hawkeye prediction determines the insertion position:
     *
     * Friendly -> RRPV 0
     * Averse   -> RRPV MAX_RRPV
     */
    const bool is_friendly =
        predictor.predict(current_ip);

    const Classification classification =
        is_friendly
            ? Classification::CACHE_FRIENDLY
            : Classification::CACHE_AVERSE;

    update_rrpv(
        rrpv_table[set_idx],
        way_idx,
        classification,
        false);
}


void hawkeye::update_replacement_state(
    uint32_t triggering_cpu,
    uint32_t set,
    uint32_t way,
    uint64_t full_addr,
    uint64_t ip,
    uint64_t victim_addr,
    uint32_t type,
    uint8_t hit)
{
    (void)triggering_cpu;
    (void)victim_addr;

    const access_type access =
        static_cast<access_type>(type);

    if (!is_hawkeye_access(access)) {
        return;
    }

    const std::size_t set_idx =
        static_cast<std::size_t>(set);

    const std::size_t way_idx =
        static_cast<std::size_t>(way);

    /*
     * ChampSim cache lines are 64 bytes.
     */
    const uint64_t line_addr =
        full_addr >> 6;

    /*
     * ---------------------------------------------------------
     * OPTgen training
     * ---------------------------------------------------------
     *
     * Keep track of the PC associated with the previous
     * reference to this cache line.
     */
    auto it =
        addr_to_pc[set_idx].find(line_addr);

    if (it != addr_to_pc[set_idx].end()) {

        const uint64_t previous_pc =
            it->second;

        const bool opt_hit =
            optgen.access(
                set_idx,
                line_addr);

        predictor.train(
            previous_pc,
            opt_hit);

    } else {

        /*
         * First reference to this cache line.
         *
         * OPTgen records the reference but does not train
         * the predictor because there is no previous PC
         * whose reuse behavior can be classified.
         */
        optgen.access(
            set_idx,
            line_addr);
    }

    /*
     * The current PC becomes the previous PC for the next
     * reference to this cache line.
     */
    addr_to_pc[set_idx][line_addr] = ip;


    /*
     * ---------------------------------------------------------
     * Replacement-state update
     * ---------------------------------------------------------
     *
     * On a miss, replacement_cache_fill() handles insertion.
     *
     * On a hit, update the existing block's RRPV here.
     */
    if (!hit) {
        return;
    }

    const bool is_friendly =
        predictor.predict(ip);

    const Classification classification =
        is_friendly
            ? Classification::CACHE_FRIENDLY
            : Classification::CACHE_AVERSE;

    update_rrpv(
        rrpv_table[set_idx],
        way_idx,
        classification,
        true);
}


void hawkeye::replacement_final_stats()
{
    /*
     * No debug output.
     *
     * ChampSim's normal cache statistics are sufficient for
     * the assignment experiments.
     */
}