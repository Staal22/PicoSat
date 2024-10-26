#pragma once
#include <unordered_map>

class clause_set_generator
{
public:
    // Literals are represented as positive numbers, and negated literals as the corresponding negative number
    static const std::vector<std::vector<int>>& generate_clause_set(int num_literals);

    static void preload(const int ceiling)
    {
        for (int i = 1; i <= ceiling; i++)
        {
            generate_clause_set(i);
        }
    }

private:
    static std::unordered_map<int, std::vector<std::vector<int>>> clause_set_cache_;

};
