#ifndef OPTGEN_H
#define OPTGEN_H

#include <cstddef>
#include <cstdint>
#include <unordered_map>
#include <vector>

class OPTgen {
public:
    // num_sets: number of cache sets tracked independently
    // associativity: W, the cache associativity
    // history_multiplier: history length in units of cache capacity
    //                      (paper uses 8x)
    OPTgen(
        std::size_t num_sets,
        std::size_t associativity,
        std::size_t history_multiplier = 8);

    // Process one access to address in set_idx.
    // Returns true if OPT would hit, false otherwise.
    bool access(
        std::size_t set_idx,
        uint64_t address);

private:
    struct SetState {
        // Number of accesses processed for this set.
        uint64_t access_count = 0;

        // Occupancy vector containing history_multiplier * W entries.
        //
        // Each entry represents the number of overlapping
        // liveness intervals at that point in the history.
        std::vector<std::size_t> occupancy_vector;

        // Last access position of each address in this set.
        std::unordered_map<uint64_t, uint64_t> last_access_time;
    };

    std::size_t num_sets_;
    std::size_t associativity_;
    std::size_t history_multiplier_;
    std::size_t vector_size_;

    std::vector<SetState> sets_;
};

#endif