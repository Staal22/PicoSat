#include "solver.h"
#include <chrono>
#include <iostream>

static constexpr int timeout_secs = 30;

solver::solver(const std::vector<std::vector<int>>& input_clauses)
{
    // Safe estimation
    result_.num_variables = 0;
    for (const auto& clause : input_clauses)
    {
        for (int lit : clause)
        {
            result_.num_variables = std::max(static_cast<int>(result_.num_variables), std::abs(lit));
        }
    }

    input_clauses_ = input_clauses;

    // Initialize interpretation vector
    result_.interpretation.resize(result_.num_variables, false);

    result_.clause_set = input_clauses;

    start_time_ = std::chrono::steady_clock::now();
}

result solver::solve()
{
    // Initial unit propagation
    auto simplified_clauses = result_.clause_set;

    // Try unit propagation first
    bool propagation_result = unit_propagation(simplified_clauses);
    if (!propagation_result)
    {
        result_.satisfiable = false;
        return result_;
    }

    // If we have clauses left after unit propagation, do atomic cuts
    if (!simplified_clauses.empty())
    {
        // --- Atomic Cut ---
        /* Heuristic: the variable that appears the most times in the subset of clauses
         * that all have length equal to the length of the shortest clause
         */
        const int best_variable = find_most_frequent_variable(simplified_clauses);
        atomic_cut(best_variable, simplified_clauses);
    }

    // Update timing
    auto current_time = std::chrono::steady_clock::now();
    result_.calculation_time = std::chrono::duration_cast<std::chrono::milliseconds>(
        current_time - start_time_);

    // Check timeout
    if (result_.calculation_time > std::chrono::seconds(timeout_secs))
    {
        result_.satisfiable = false;
        result_.timed_out = true;
    }

    return result_;
}

void solver::atomic_cut(const int variable, const std::vector<std::vector<int>>& clause_set)
{
    // Check timeout
    if (result_.calculation_time > std::chrono::seconds(timeout_secs))
    {
        result_.satisfiable = false;
        result_.timed_out = true;
        return;
    }

    result_.interpretation[std::abs(variable) - 1] = variable > 0;

    // Try positive assignment first
    std::vector<std::vector<int>> pos_clauses;
    for (const auto& clause : clause_set)
    {
        // Skip clauses containing the positive literal (they're satisfied)
        if (std::find(clause.begin(), clause.end(), variable) != clause.end())
        {
            continue;
        }

        // Remove negative occurrences of the variable
        std::vector<int> new_clause;
        for (int lit : clause)
        {
            if (lit != -variable)
            {
                new_clause.push_back(lit);
            }
        }

        if (!new_clause.empty())
        {
            pos_clauses.push_back(new_clause);
        }
        else
        {
            // Empty clause - this branch failed
            pos_clauses.clear();
            break;
        }
    }

    if (!pos_clauses.empty())
    {
        // Try unit propagation on positive branch
        bool propagation_result = unit_propagation(pos_clauses);
        if (propagation_result)
        {
            if (pos_clauses.empty())
            {
                result_.satisfiable = true;
                return;
            }

            // Continue with atomic cuts
            int next_var = find_most_frequent_variable(pos_clauses);
            atomic_cut(next_var, pos_clauses);
            if (result_.satisfiable)
            {
                return;
            }
        }
    }

    // If positive assignment failed, try negative
    result_.interpretation[std::abs(variable) - 1] = false;

    std::vector<std::vector<int>> neg_clauses;
    for (const auto& clause : clause_set)
    {
        // Skip clauses containing the negative literal (they're satisfied)
        if (std::find(clause.begin(), clause.end(), -variable) != clause.end())
        {
            continue;
        }

        // Remove positive occurrences of the variable
        std::vector<int> new_clause;
        for (int lit : clause)
        {
            if (lit != variable)
            {
                new_clause.push_back(lit);
            }
        }

        if (!new_clause.empty())
        {
            neg_clauses.push_back(new_clause);
        }
        else
        {
            // Empty clause - this branch failed
            result_.satisfiable = false;
            return;
        }
    }

    if (!neg_clauses.empty())
    {
        // Try unit propagation on negative branch
        bool propagation_result = unit_propagation(neg_clauses);
        if (propagation_result)
        {
            if (neg_clauses.empty())
            {
                result_.satisfiable = true;
                return;
            }

            // Continue with atomic cuts
            int next_var = find_most_frequent_variable(neg_clauses);
            atomic_cut(next_var, neg_clauses);
        }
        else
        {
            result_.satisfiable = false;
        }
    }
    else
    {
        result_.satisfiable = true;
    }
}

bool solver::unit_propagation(std::vector<std::vector<int>>& clauses)
{
    bool changes_made;
    do
    {
        changes_made = false;

        // Find unit clauses
        for (const auto& clause : clauses)
        {
            if (clause.size() == 1)
            {
                int unit = clause[0];
                result_.interpretation[std::abs(unit) - 1] = unit > 0;

                // Apply unit propagation
                std::vector<std::vector<int>> new_clauses;
                for (const auto& c : clauses)
                {
                    // Skip clauses that are satisfied by the unit literal
                    if (std::find(c.begin(), c.end(), unit) != c.end())
                    {
                        continue;
                    }

                    // Remove negative unit literal from remaining clauses
                    std::vector<int> new_clause;
                    for (int lit : c)
                    {
                        if (lit != -unit)
                        {
                            new_clause.push_back(lit);
                        }
                    }

                    if (new_clause.empty())
                    {
                        return false; // Contradiction found
                    }

                    if (!new_clause.empty())
                    {
                        new_clauses.push_back(new_clause);
                    }
                }

                clauses = new_clauses;
                changes_made = true;
                break;
            }
        }
    }
    while (changes_made);

    return true;
}

int solver::find_most_frequent_variable(const std::vector<std::vector<int>>& clause_set) const
{
    std::vector<std::vector<int>> shortest_clauses;
    size_t shortest_length = std::numeric_limits<int>::max();
    for (const auto& clause : clause_set)
    {
        shortest_length = std::min(shortest_length, clause.size());
    }
    for (const auto& clause : clause_set)
    {
        if (clause.size() == shortest_length)
        {
            shortest_clauses.push_back(clause);
        }
    }

    std::vector frequencies(result_.num_variables, 0);
    for (const auto& clause : shortest_clauses)
    {
        for (const int lit : clause)
        {
            frequencies[std::abs(lit) - 1]++;
        }
    }

    const auto it = std::max_element(frequencies.begin(), frequencies.end());
    const auto most_frequent_index = std::distance(frequencies.begin(), it);
    return static_cast<int>(most_frequent_index + 1);
}

void solver::print_result(const bool print_interpretation, const bool print_clauses) const
{
    if (result_.timed_out)
    {
        std::cout << "Solver timed out. (" << timeout_secs << "s)\n";
        return;
    }

    std::cout << "Number of literals: " << result_.num_variables << "\n";
    std::cout << "Satisfiable: " << (result_.satisfiable ? "yes" : "no") << "\n";
    std::cout << "Time to solve (ms): " << result_.calculation_time.count() << "\n";

    if (print_interpretation && result_.satisfiable)
    {
        std::cout << "Model interpretation:\n";
        for (size_t i = 0; i < result_.num_variables; i++)
        {
            // Translate 1 to p, 2 to q, 3 to r, ...
            const char symbol = static_cast<char>('p' + i);
            std::cout << symbol << " -> " << (result_.interpretation[i] ? "true" : "false") << "\n";
        }
    }

    if (print_clauses)
    {
        std::cout << "Clause set:\n";
        std::cout << "{";
        for (size_t i = 0; i < input_clauses_.size(); ++i)
        {
            std::cout << "{";
            for (size_t j = 0; j < result_.num_variables; ++j)
            {
                // Translate 1 to p, 2 to q, 3 to r, ...
                const char symbol = static_cast<char>('p' + j);
                if (j + 1 < result_.num_variables)
                {
                    std::cout << (input_clauses_[i][j] < 0 ? "!" : "") << symbol << ", ";
                }
                else
                {
                    std::cout << (input_clauses_[i][j] < 0 ? "!" : "") << symbol;
                }
            }
            if (i + 1 < input_clauses_.size())
            {
                std::cout << "}, ";
            }
            else
            {
                std::cout << "}}\n";
            }
        }
    }

    std::cout << "\n";
}
