#pragma once
#include <chrono>
#include <vector>

struct result
{
    bool satisfiable = true;
    bool timed_out = false;

    size_t num_variables;

    std::vector<bool> interpretation;
    std::chrono::milliseconds calculation_time{0};

    std::vector<std::vector<int>> clause_set;
};

class solver
{
public:
    explicit solver(const std::vector<std::vector<int>>& input_clauses);

    result solve();
    void print_result(bool print_interpretation = false, bool print_clauses = false) const;

private:
    void atomic_cut(int variable, const std::vector<std::vector<int>>& clause_set);
    bool unit_propagation(std::vector<std::vector<int>>& clauses);
    int find_most_frequent_variable(const std::vector<std::vector<int>>& clause_set) const;

    result result_;
    std::vector<std::vector<int>> input_clauses_;
    std::chrono::time_point<std::chrono::steady_clock> start_time_;

};
