/**
 * Model for goat, cabbage and wolf puzzle.
 * Author: Marius Mikucionis <marius@cs.aau.dk>
 * Compile and run:
 * g++ -std=c++17 -pedantic -Wall -DNDEBUG -O3 -o crossing crossing.cpp && ./crossing
 */
#include "reachability.hpp" // your header-only library solution

#include <functional> // std::function
#include <list>
#include <array>
#include <iostream>
#include <benchmark/benchmark.h>

enum actor { cabbage, goat, wolf }; // names of the actors
enum class pos_t { shore1, travel, shore2}; // names of the actor positions
using actors_t = std::array<pos_t,3>; // positions of the actors

// Overload to print position of actor
std::ostream& operator<<(std::ostream& os, const pos_t& pos) {
    switch(pos) {
        case pos_t::shore1: os << "1"; break;
        case pos_t::travel: os << "~"; break;
        case pos_t::shore2: os << "2"; break;
    }
    return os;
}

// Overload to print actor
std::ostream& operator<<(std::ostream& os, const actors_t& actors) {
    os << actors[actor::cabbage] << actors[actor::goat] << actors[actor::wolf];
    return os;
}

// Overload of << operator to print array content
template<class StateT, template<class...> class ContainerT>
ostream &operator<<(ostream &os, const ContainerT<array<StateT, 3>> &v) {
    auto step = 0;
    for(auto c : v){
        os << step++ << ": " << c << endl;
    }
    return os;
}

auto transitions(const actors_t& actors)
{
	auto res = std::list<std::function<void(actors_t&)>>{};
	for (auto i=0u; i<actors.size(); ++i)
		switch(actors[i]) {
		case pos_t::shore1:
			res.push_back([i](actors_t& actors){ actors[i] = pos_t::travel; });
			break;
		case pos_t::travel:
			res.push_back([i](actors_t& actors){ actors[i] = pos_t::shore1; });
			res.push_back([i](actors_t& actors){ actors[i] = pos_t::shore2; });
			break;
		case pos_t::shore2:
			res.push_back([i](actors_t& actors){ actors[i] = pos_t::travel; });
			break;
		}
	return res;
}

bool is_valid(const actors_t& actors) {
	// only one passenger:
	if (std::count(std::begin(actors), std::end(actors), pos_t::travel)>1)
		return false;
	// goat cannot be left alone with wolf, as wolf will eat the goat:
	if (actors[actor::goat]==actors[actor::wolf] && actors[actor::cabbage]==pos_t::travel)
		return false;
	// goat cannot be left alone with cabbage, as goat will eat the cabbage:
	if (actors[actor::goat]==actors[actor::cabbage] && actors[actor::wolf]==pos_t::travel)
		return false;
	return true;
}

void solve(){
	auto state_space = state_space_t{
		actors_t{},                // initial state
		successors<actors_t>(transitions), // successor generator from your library
		&is_valid};                        // invariant over all states
	auto solution = state_space.check(
		[](const actors_t& actors){ // all actors should be on the shore2:
			return std::count(std::begin(actors), std::end(actors), pos_t::shore2)==actors.size();
		});
	for (auto&& trace: solution)
		std::cout << "#  CGW\n" << trace;
}

/*int main(){
	solve();
}*/

// Enable for benchmarking
void BM_main(benchmark::State& state){
    for(auto _ : state) {
        solve();
    }
}

BENCHMARK(BM_main)->Iterations(1000);
BENCHMARK_MAIN();

/* Benchmark results:
 * g++ crossing.cpp --std=c++17 -lbenchmark -lpthread -O3 -o benchmarkcrossing && ./benchmarkcrossing
 * List: 110149 ns (47074 ns)
 *
 */

/** Sample output:
#  CGW
0: 111
1: 1~1
2: 121
3: ~21
4: 221
5: 2~1
6: 211
7: 21~
8: 212
9: 2~2
10: 222
*/
