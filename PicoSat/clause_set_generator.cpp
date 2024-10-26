#include "clause_set_generator.h"

std::unordered_map<int, std::vector<std::vector<int>>> clause_set_generator::clause_set_cache_;

const std::vector<std::vector<int>>& clause_set_generator::generate_clause_set(const int num_literals)
{
    // Check if we already have this in cache
    const auto it = clause_set_cache_.find(num_literals);
    if (it != clause_set_cache_.end())
    {
        return it->second;
    }

    std::vector<std::vector<int>> clauses;

    // Pre-allocate space
    clauses.reserve(1 << num_literals);

    const int num_possible_clauses = 1 << num_literals;
    std::vector<int> clause;

    // Pre-allocate space for each clause
    clause.reserve(num_literals);

    for (int i = 0; i < num_possible_clauses; i++)
    {
        clause.clear();

        for (int j = 0; j < num_literals; j++)
        {
            clause.push_back(i >> j & 1 ? -(j + 1) : j + 1);
        }

        clauses.push_back(clause);
    }

    // Store in cache and return reference
    return clause_set_cache_.emplace(num_literals, std::move(clauses)).first->second;
}
