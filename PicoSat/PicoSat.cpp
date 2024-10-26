#include <cassert>
#include "solver.h"
#include <iostream>
#include <vector>
#include "clause_set_generator.h"

std::vector<std::vector<std::vector<int>>> satisfiable_cases = {
    // Simple single clause
    {
        {1}
        // p = true
    },

    // Two clauses, easy
    {
        {1}, {2},
        // p = true, q = true
    },

    // Simple OR relationship
    {
        {1, 2},
        // p or q = true, other arbitrary
    },

    // More complex satisfiable case
    {
        {1, 2}, {-1, 2}, {1, -2},
        // p = true, q = true
    },

    // Simplified graph coloring
    {
        {1, 2}, {-1, 3}, {-2, -3}, {1, -3}, {-1, 2, 3},
        // p = true, q = false, r = true
    }
};

std::vector<std::vector<std::vector<int>>> unsatisfiable_cases = {
    // Simple contradiction
    {
        {1}, {-1}
    },

    // Classic unsatisfiable case
    {
        {1}, {-1, 2}, {-2}
    },

    // More complex unsatisfiable case
    {
        {1, 2}, {-1, 2}, {1, -2}, {-1, -2}
    },

    // Pigeon-hole principle: trying to put 3 pigeons in 2 holes
    {
            {1, 2},      // Pigeon 1 must be in a hole
            {3, 4},      // Pigeon 2 must be in a hole
            {5, 6},      // Pigeon 3 must be in a hole
            {-1, -3},    // No two pigeons in hole 1
            {-1, -5},
            {-3, -5},
            {-2, -4},    // No two pigeons in hole 2
            {-2, -6},
            {-4, -6}
    }
};

std::pair<std::vector<std::vector<std::vector<int>>>, std::vector<std::vector<std::vector<int>>>>
get_all_test_cases()
{
    return {satisfiable_cases, unsatisfiable_cases};
}

int main(int argc, char* argv[])
{
    // The max nr of propositional variables that will be used to create an unsatisfiable clause set
    // Reduce this if you are impatient or your computer is exploding
    int power_ceiling = 20;
    // Generate and cache clause sets up to power_ceiling literals beforehand
    clause_set_generator::preload(power_ceiling);

    // --- Satisfiable test cases ---
    std::cout << "=== Satisfiable test cases BEGIN ===\n";
    // Test case with a satisfiable clause set, interpreting every literal to true is a model
    // {{p, q, r}, ...}
    {
        auto const& clause_set = clause_set_generator::generate_clause_set(3);
        std::vector modified_set(clause_set.begin(), clause_set.end() - 1); // \ {!p, !q, !r}
        solver s(modified_set);
        auto result = s.solve();
        s.print_result(true);
    }
    // Test case with another satisfiable clause set, interpreting every literal to false is a model
    // {..., {!p, !q, !r}}
    {
        auto const& clause_set = clause_set_generator::generate_clause_set(3);
        std::vector modified_set(clause_set.begin() + 1, clause_set.end()); // \ {p, q, r}
        solver s(modified_set);
        auto result = s.solve();
        s.print_result(true);
    }
    std::cout << "=== Satisfiable test cases END ===\n\n";

    // Non-logging but more comprehensive test battery, fails loudly
    auto [sat_cases, unsat_cases] = get_all_test_cases();
    for (const auto& instance : sat_cases)
    {
        auto s = solver(instance);
        auto result = s.solve();
        assert(result.satisfiable);
    }
    for (const auto& instance : unsat_cases)
    {
        auto s = solver(instance);
        auto result = s.solve();
        assert(!result.satisfiable);
    }

    // --- Unsatisfiable increasing tests ---
    int literals = 3;
    while (true)
    {
        auto const& clause_set = clause_set_generator::generate_clause_set(literals);
        solver s(clause_set);
        auto result = s.solve();
        s.print_result();
        if (result.timed_out || literals == power_ceiling)
        {
            if (literals == power_ceiling && !result.timed_out)
            {
                std::cout << "Reached end of pre-generated clause sets. Terminating...\n";
            }
            return 0;
        }
        literals++;
    }
}
