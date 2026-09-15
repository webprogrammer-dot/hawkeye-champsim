#include "../rrip.h"

#include <cassert>
#include <iostream>
#include <vector>

int main()
{
    std::cout << "=== RRIP Test ===\n";

    // ------------------------------------
    // Friendly insertion
    // ------------------------------------

    {
        std::vector<int> rrpv = {7, 7, 7, 7};

        update_rrpv(
            rrpv,
            0,
            Classification::CACHE_FRIENDLY,
            false);

        assert(rrpv[0] == 0);

        std::cout << "Friendly insertion -> RRPV 0 [PASS]\n";
    }

    // ------------------------------------
    // Averse insertion
    // ------------------------------------

    {
        std::vector<int> rrpv = {0, 0, 0, 0};

        update_rrpv(
            rrpv,
            1,
            Classification::CACHE_AVERSE,
            false);

        assert(rrpv[1] == 7);

        std::cout << "Averse insertion -> RRPV 7 [PASS]\n";
    }

    // ------------------------------------
    // Hit
    // ------------------------------------

    {
        std::vector<int> rrpv = {7, 5, 3, 1};

        update_rrpv(
            rrpv,
            2,
            Classification::CACHE_FRIENDLY,
            true);

        assert(rrpv[2] == 0);

        std::cout << "Hit -> RRPV 0 [PASS]\n";
    }

    // ------------------------------------
    // Victim already at 7
    // ------------------------------------

    {
        std::vector<int> rrpv = {0, 7, 3, 5};

        long victim = find_victim(rrpv);

        assert(victim == 1);

        std::cout << "Existing RRPV 7 -> victim way 1 [PASS]\n";
    }

    // ------------------------------------
    // Aging
    // ------------------------------------

    {
        std::vector<int> rrpv = {0, 1, 2, 3};

        long victim = find_victim(rrpv);

        assert(victim >= 0);
        assert(victim < 4);

        // At least one line must reach 7.
        bool has_max = false;

        for (int value : rrpv) {
            if (value == 7)
                has_max = true;
        }

        assert(has_max);

        std::cout << "Aging -> line reaches RRPV 7 [PASS]\n";
        std::cout << "Victim = way " << victim << " [PASS]\n";
    }

    std::cout << "RRIP TEST PASSED\n";

    return 0;
}
