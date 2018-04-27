#ifndef COST_SATURATION_COST_PARTITIONED_HEURISTIC_H
#define COST_SATURATION_COST_PARTITIONED_HEURISTIC_H

#include "types.h"

#include <vector>

namespace cost_saturation {
struct LookupTable {
    const int heuristic_index;
    const std::vector<int> h_values;

public:
    LookupTable(int heuristic_index, std::vector<int> &&h_values);
};


class CostPartitionedHeuristic {
    std::vector<LookupTable> lookup_tables;

public:
    void add_lookup_table_if_nonzero(
        int heuristic_id, std::vector<int> h_values, bool sparse);
    int compute_heuristic(const std::vector<int> &local_state_ids) const;
    const std::vector<LookupTable> &get_lookup_tables() const;
};
}

#endif
