#include "rrip.h"

void update_rrpv(std::vector<int>& rrpv,
                 std::size_t way,
                 Classification cls,
                 bool is_hit) {
    if (way >= rrpv.size())
        return;

    if (cls == Classification::CACHE_AVERSE) {
        // Cache-averse lines are always immediate eviction candidates.
        rrpv[way] = MAX_RRPV;
    } else {
        // Cache-friendly lines always get RRPV = 0.
        rrpv[way] = 0;

        // On a cache-friendly miss, age the other lines.
        if (!is_hit) {
            for (std::size_t i = 0; i < rrpv.size(); ++i) {
                if (i != way && rrpv[i] < 6) {
                    ++rrpv[i];
                }
            }
        }
    }
}

std::size_t find_victim(std::vector<int>& rrpv) {
    if (rrpv.empty())
        return 0;

    while (true) {
        // Prefer a cache-averse line.
        for (std::size_t way = 0; way < rrpv.size(); ++way) {
            if (rrpv[way] == MAX_RRPV) {
                return way;
            }
        }

        // No line has RRPV = 7, so age all lines.
        for (std::size_t way = 0; way < rrpv.size(); ++way) {
            if (rrpv[way] < MAX_RRPV) {
                ++rrpv[way];
            }
        }
    }
}