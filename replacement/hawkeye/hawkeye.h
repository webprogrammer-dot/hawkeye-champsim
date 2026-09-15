#ifndef HAWKEYE_H
#define HAWKEYE_H

#include <cstdint>
#include <unordered_map>
#include <vector>

#include "../../inc/cache.h"
#include "../../inc/modules.h"

#include "optgen.h"
#include "predictor.h"
#include "rrip.h"


struct hawkeye : public champsim::modules::replacement {

    OPTgen optgen;

    HawkeyePredictor predictor;

    std::vector<std::vector<int>> rrpv_table;

    /*
     * For each set:
     *
     * cache-line address -> PC of previous reference
     */
    std::vector<std::unordered_map<uint64_t, uint64_t>> addr_to_pc;


    hawkeye(CACHE* cache);


    long find_victim(
        uint32_t triggering_cpu,
        uint64_t instr_id,
        uint32_t set,
        const champsim::cache_block* current_set,
        uint64_t ip,
        uint64_t full_addr,
        uint32_t type);


    void replacement_cache_fill(
        uint32_t triggering_cpu,
        long set,
        long way,
        champsim::address full_addr,
        champsim::address ip,
        champsim::address victim_addr,
        access_type type);


    void update_replacement_state(
        uint32_t triggering_cpu,
        uint32_t set,
        uint32_t way,
        uint64_t full_addr,
        uint64_t ip,
        uint64_t victim_addr,
        uint32_t type,
        uint8_t hit);


    void replacement_final_stats();
};

#endif

