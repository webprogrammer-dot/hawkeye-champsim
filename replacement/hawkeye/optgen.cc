#include "optgen.h"

OPTgen::OPTgen(
    std::size_t num_sets,
    std::size_t associativity,
    std::size_t history_multiplier)
    : num_sets_(num_sets),
      associativity_(associativity),
      history_multiplier_(history_multiplier),
      vector_size_(history_multiplier * associativity),
      sets_(num_sets)
{
    for (auto& set : sets_) {
        set.occupancy_vector.resize(vector_size_, 0);
    }
}

bool OPTgen::access(
    std::size_t set_idx,
    uint64_t address)
{
    SetState& set = sets_[set_idx];

    /*
     * Current access occupies the newest position in the
     * circular history.
     */
    const uint64_t current_time = set.access_count++;

    const std::size_t current_idx =
        current_time % vector_size_;

    /*
     * The newest history entry corresponds to the current
     * access. It starts with zero occupancy.
     */
    set.occupancy_vector[current_idx] = 0;

    /*
     * Look for the previous access to this address.
     */
    auto it = set.last_access_time.find(address);

    /*
     * First reference:
     *
     * OPTgen cannot determine a reuse interval yet.
     * According to Section 3.1, first references do not
     * modify the occupancy vector.
     */
    if (it == set.last_access_time.end()) {
        set.last_access_time[address] = current_time;
        return false;
    }

    const uint64_t previous_time = it->second;

    /*
     * If the reuse is older than the tracked history,
     * we cannot reconstruct its complete interval.
     *
     * Treat it as a miss and start tracking from the
     * current reference.
     */
    const uint64_t distance =
        current_time - previous_time;

    if (distance > vector_size_) {
        set.last_access_time[address] = current_time;
        return false;
    }

    /*
     * Examine the reuse interval:
     *
     * [previous_time, current_time)
     *
     * Every occupancy entry must be below W for the
     * reuse to fit in the cache under OPT.
     */
    bool opt_hit = true;

    for (uint64_t t = previous_time;
         t < current_time;
         ++t) {

        const std::size_t idx =
            t % vector_size_;

        if (set.occupancy_vector[idx] >= associativity_) {
            opt_hit = false;
            break;
        }
    }

    /*
     * Only an OPT hit creates a new liveness interval.
     *
     * For an OPT miss, the vector is intentionally left
     * unchanged because the line would bypass the cache.
     */
    if (opt_hit) {
        for (uint64_t t = previous_time;
             t < current_time;
             ++t) {

            const std::size_t idx =
                t % vector_size_;

            ++set.occupancy_vector[idx];
        }
    }

    /*
     * This reference is now the most recent reference to
     * this address.
     */
    set.last_access_time[address] = current_time;

    return opt_hit;
}