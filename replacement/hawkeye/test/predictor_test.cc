#include "../predictor.h"

#include <cassert>
#include <cstdint>
#include <iostream>

int main()
{
    HawkeyePredictor pred;

    const uint64_t pc = 0x1000;

    std::cout << "=== Predictor Test ===\n";

    // Initial value should be 4 for a 3-bit counter.
    assert(pred.get_counter(pc) == 4);
    assert(pred.predict(pc) == true);

    std::cout << "Initial: counter=4, prediction=FRIENDLY [PASS]\n";

    // OPT miss: 4 -> 3
    pred.train(pc, false);

    assert(pred.get_counter(pc) == 3);
    assert(pred.predict(pc) == false);

    std::cout << "After MISS: counter=3, prediction=AVERSE [PASS]\n";

    // OPT hit: 3 -> 4
    pred.train(pc, true);

    assert(pred.get_counter(pc) == 4);
    assert(pred.predict(pc) == true);

    std::cout << "After HIT: counter=4, prediction=FRIENDLY [PASS]\n";

    // Test upper saturation.
    for (int i = 0; i < 10; ++i)
        pred.train(pc, true);

    assert(pred.get_counter(pc) == 7);

    std::cout << "Upper saturation: counter=7 [PASS]\n";

    // Test lower saturation.
    for (int i = 0; i < 10; ++i)
        pred.train(pc, false);

    assert(pred.get_counter(pc) == 0);
    assert(pred.predict(pc) == false);

    std::cout << "Lower saturation: counter=0 [PASS]\n";

    std::cout << "PREDICTOR TEST PASSED\n";

    return 0;
}
