/**
 * Model for leaping frogs puzzle:
 * https://primefactorisation.com/frogpuzzle/
 * Author: Marius Mikucionis <marius@cs.aau.dk>
 * Compile and run:
 * g++ -std=c++17 -pedantic -Wall -DNDEBUG -O3 -o frogs frogs.cpp && ./frogs
 */

/** Benchmark results:
 * g++ frogs.cpp --std=c++17 -lbenchmark -lpthread -O3 -o benchmarkfrogs && ./benchmarkfrogs
 * List:                                  1920407 ns (543956 ns)
 * Deque:                                 2659367 ns (504552 ns)
 * With smart pointers (shared):          2189727 ns (524632 ns)
 */

#include "reachability.hpp" // your header-only library solution

#include <iostream>
#include <vector>
#include <list>
#include <functional> // std::function

// Enable or disable benchmarking.
// #define ENABLE_BENCHMARKING
#ifdef ENABLE_BENCHMARKING
#include <benchmark/benchmark.h>
#endif

enum class frog {
    empty, green, brown
};
using stones_t = std::vector<frog>;

// Overload to print frog positions
std::ostream &operator<<(std::ostream &os, const stones_t &stones) {
    for (auto &&stone: stones)
        switch (stone) {
            case frog::green:
                os << "G";
                break;
            case frog::brown:
                os << "B";
                break;
            case frog::empty:
                os << "_";
                break;
        }
    return os;
}

// Overload of << operator to print list content
template<class StateT, template<class...> class ContainerT, typename = std::enable_if_t<!std::is_same<StateT, char>::value>>
std::ostream &operator<<(std::ostream &os, const ContainerT<ContainerT<StateT>> &v) {
    for (auto stones : v) {
        os << "State of " << stones.size() << " stones: " << stones << '\n';
    }
    os << std::endl;
    return os;
}

auto transitions(const stones_t &stones) {
    auto res = std::vector<std::function<void(stones_t &)>>{};
    if (stones.size() < 2)
        return res;
    auto i = 0u;
    while (i < stones.size() && stones[i] != frog::empty) ++i; // find empty stone
    if (i == stones.size())
        return res;  // did not find empty stone
    // explore moves to fill the empty from left to right (only green can do that):
    if (i > 0 && stones[i - 1] == frog::green)
        res.push_back([i](stones_t &s) { // green jump to next
            s[i - 1] = frog::empty;
            s[i] = frog::green;
        });
    if (i > 1 && stones[i - 2] == frog::green)
        res.push_back([i](stones_t &s) { // green jump over 1
            s[i - 2] = frog::empty;
            s[i] = frog::green;
        });
    // explore moves to fill the empty from right to left (only brown can do that):
    if (i < stones.size() - 1 && stones[i + 1] == frog::brown) {
        res.push_back([i](stones_t &s) { // brown jump to next
            s[i + 1] = frog::empty;
            s[i] = frog::brown;
        });
    }
    if (i < stones.size() - 2 && stones[i + 2] == frog::brown) {
        res.push_back([i](stones_t &s) { // brown jump over 1
            s[i + 2] = frog::empty;
            s[i] = frog::brown;
        });
    }
    return res;
}

void show_successors(const stones_t &state, const size_t level = 0) {
    // Caution: this function uses recursion, which is not suitable for solving puzzles!!
    // 1) some state spaces can be deeper than stack allows.
    // 2) it can only perform depth-first search
    // 3) it cannot perform breadth-first search, cheapest-first, greatest-first etc.
    auto trans = transitions(state); // compute the transitions
    std::cout << std::string(level * 2, ' ')
              << "state " << state << " has " << trans.size() << " transitions";
    if (trans.empty())
        std::cout << '\n';
    else
        std::cout << ", leading to:\n";
    for (auto &t: trans) {
        auto succ = state; // copy the original state
        t(succ); // apply the transition on the state to compute successor
        show_successors(succ, level + 1);
    }
}

void explain() {
    const auto start = stones_t{{frog::green, frog::green, frog::empty,
                                        frog::brown, frog::brown}};
    std::cout << "Leaping frog puzzle start: " << start << '\n';
    show_successors(start);
    const auto finish = stones_t{{frog::brown, frog::brown, frog::empty,
                                         frog::green, frog::green}};
    std::cout << "Leaping frog puzzle start: " << start << ", finish: " << finish << '\n';
    auto space = state_space_t(start, successors<stones_t>(transitions));// define state space
    // explore the state space and find the solutions satisfying goal:
    std::cout << "--- Solve with default (breadth-first) search: ---\n";
    auto solutions = space.check([&finish](const stones_t &state) { return state == finish; });
    for (auto &&trace: solutions) { // iterate through solutions:
        std::cout << "Solution: a trace of " << trace.size() << " states\n";
        std::cout << trace; // print solution
    }
}

void solve(size_t frogs, search_order order = search_order::breadth_first) {
    const auto stones = frogs * 2 + 1; // frogs on either side and 1 empty in the middle
    auto start = stones_t(stones, frog::empty);  // initially all empty
    auto finish = stones_t(stones, frog::empty); // initially all empty
    while (frogs-- > 0) { // count down from frogs-1 to 0 and put frogs into positions:
        start[frogs] = frog::green;                  // green on left
        start[start.size() - frogs - 1] = frog::brown;   // brown on right
        finish[frogs] = frog::brown;                 // brown on left
        finish[finish.size() - frogs - 1] = frog::green; // green on right
    }
    std::cout << "Leaping frog puzzle start: " << start << ", finish: " << finish << '\n';
    auto space = state_space_t{
            std::move(start),                 // initial state
            successors<stones_t>(transitions) // successor-generating function from your library
    };
    auto solutions = space.check(
            [finish = std::move(finish)](const stones_t &state) { return state == finish; },
            order);
    for (u_int i = 0; i < solutions.size(); i++) {
        std::cout << "Solution: trace of " << solutions[i].size() << " states\n";
        std::cout << solutions[i] << std::endl;
    }
}

#ifndef ENABLE_BENCHMARKING
int main() {
    explain();
    std::cout << "--- Solve with depth-first search: ---\n";
    solve(2, search_order::depth_first);
    solve(4); // 20 frogs may take >5.8GB of memory
}
#endif

#ifdef ENABLE_BENCHMARKING
// Enable for benchmarking
void BM_main(benchmark::State& state){
    for(auto _ : state) {
        explain();
        std::cout << "--- Solve with depth-first search: ---\n";
        solve(2, search_order::depth_first);
        solve(4); // 20 frogs may take >5.8GB of memory
    }
}

BENCHMARK(BM_main)->Iterations(1000);
BENCHMARK_MAIN();
#endif
