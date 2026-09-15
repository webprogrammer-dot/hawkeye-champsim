#include "predictor.h"

HawkeyePredictor::HawkeyePredictor(std::size_t num_entries, int counter_bits)
    : num_entries_(num_entries),
      counter_bits_(counter_bits),
      max_counter_val_((1 << counter_bits) - 1),
      table_(num_entries, 4) {}

std::size_t HawkeyePredictor::get_index(uint64_t pc) const {
    return (pc ^ (pc >> 2)) % num_entries_;
}

void HawkeyePredictor::train(uint64_t pc, bool opt_hit) {
    std::size_t idx = get_index(pc);

    if (opt_hit) {
        if (table_[idx] < max_counter_val_) {
            table_[idx]++;
        }
    } else {
        if (table_[idx] > 0) {
            table_[idx]--;
        }
    }
}

bool HawkeyePredictor::predict(uint64_t pc) const {
    std::size_t idx = get_index(pc);

    int msb_threshold = 1 << (counter_bits_ - 1);

    return table_[idx] >= msb_threshold;
}

int HawkeyePredictor::get_counter(uint64_t pc) const {
    std::size_t idx = get_index(pc);
    return table_[idx];
}