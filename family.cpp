/**
 * Model for Japanese family river crossing puzzle:
 * https://www.funzug.com/index.php/flash-games/japanese-river-crossing-puzzle-game.html
 * Author: Marius Mikucionis <marius@cs.aau.dk>
 * Compile using:
 * g++ -std=c++17 -pedantic -Wall -DNDEBUG -O3 -o family family.cpp && ./family
 * Inspect the solution (only the traveling part):
 * ./family | grep trv | grep '~~~'
 */

#include "reachability.hpp" // your header-only library solution

#include <iostream>
#include <deque>
#include <array>
#include <functional> // std::function

/** Model of the river crossing: persons and a boat */
struct person_t
{
    enum { shore1, onboard, shore2 } pos = shore1;
    enum { mother, father, daughter1, daughter2, son1, son2, policeman, prisoner };
};

/** Model of a boat */
struct boat_t
{
    enum { shore1, travel, shore2 } pos = shore1;
    uint16_t capacity{2};
    uint16_t passengers{0};
};

/** Model of an entire system */
struct state_t
{
    boat_t boat;
    std::array<person_t,8> persons;
};

bool operator==(const person_t& p1, const person_t& p2) {
    return (p1.pos == p2.pos);
}

bool operator==(const boat_t& b1, const boat_t& b2) {
    return (b1.pos == b2.pos) &&
           (b1.capacity == b2.capacity) &&
           (b1.passengers == b2.passengers);
}

bool operator==(const state_t& s1, const state_t& s2) {
    return (s1.boat == s2.boat) && (s1.persons == s2.persons);
}

std::ostream& operator<<(std::ostream& os, const person_t& p) {
    os << '{';
    switch (p.pos) {
        case person_t::shore1: os << "sh1"; break;
        case person_t::onboard: os << "~~~"; break;
        case person_t::shore2: os << "SH2"; break;
        default: os << "ERROR" ; break;
    }
    return os << '}';
}

std::ostream& operator<<(std::ostream& os, const boat_t& b) {
    os << '{';
    switch (b.pos) {
        case boat_t::shore1: os << "sh1"; break;
        case boat_t::travel: os << "trv"; break;
        case boat_t::shore2: os << "SH2"; break;
        default: os << "ERROR" ; break;
    }
    return os << ',' << b.passengers << ',' << b.capacity << '}';
}

std::ostream& operator<<(std::ostream& os, const state_t& s){
    return os << s.boat << ','
              << s.persons[person_t::mother] << ','
              << s.persons[person_t::father] << ','
              << s.persons[person_t::daughter1] << ','
              << s.persons[person_t::daughter2] << ','
              << s.persons[person_t::son1] << ','
              << s.persons[person_t::son2] << ','
              << s.persons[person_t::policeman] << ','
              << s.persons[person_t::prisoner];
}


/** Returns a list of transitions applicable on a given state.
 * Transition is a function modifying a state */
auto transitions(const state_t& s)
{
    auto res = std::deque<std::function<void(state_t&)>>{};
    switch (s.boat.pos) {
        case boat_t::shore1:
        case boat_t::shore2:
            if (s.boat.passengers>0) // start traveling
                res.push_back([](state_t& state){ state.boat.pos = boat_t::travel; });
            break;
        case boat_t::travel:
            res.push_back([](state_t& state){ // arrive to shore1
                state.boat.pos = boat_t::shore1;
                state.boat.passengers = 0;
                for (auto& p: state.persons)
                    if (p.pos == person_t::onboard)
                        p.pos = person_t::shore1;
            });
            res.push_back([](state_t& state){	// arrive to shore2
                state.boat.pos = boat_t::shore2;
                state.boat.passengers = 0;
                for (auto& p: state.persons)
                    if (p.pos == person_t::onboard)
                        p.pos = person_t::shore2;
            });
            break;
    }
    for (auto i=0u; i<s.persons.size(); ++i) {
        switch (s.persons[i].pos) {
            case person_t::shore1:  // board the boat on shore1:
                if (s.boat.pos == boat_t::shore1)
                    res.push_back([i](state_t& state){
                        state.persons[i].pos = person_t::onboard;
                        ++state.boat.passengers;
                    });
                break;
            case person_t::shore2: // board the boat on shore2:
                if (s.boat.pos == boat_t::shore2)
                    res.push_back([i](state_t& state){
                        state.persons[i].pos = person_t::onboard;
                        ++state.boat.passengers;
                    });
                break;
            case person_t::onboard:
                if (s.boat.pos == boat_t::shore1) // leave the boat to shore1
                    res.push_back([i](state_t& state){
                        state.persons[i].pos = person_t::shore1;
                        --state.boat.passengers;
                    });
                else if (s.boat.pos == boat_t::shore2) // leave the boat to shore2
                    res.push_back([i](state_t& state){
                        state.persons[i].pos = person_t::shore2;
                        --state.boat.passengers;
                    });
                break;
        }
    }
    return res;
}

bool river_crossing_valid(const state_t& s)
{
    if (s.boat.passengers > s.boat.capacity) {
        log(" boat overload\n");
        return false;
    }
    if (s.boat.pos == boat_t::travel) {
        if (s.persons[person_t::daughter1].pos == person_t::onboard) {
            if (s.boat.passengers==1 ||
                (s.persons[person_t::daughter2].pos == person_t::onboard) ||
                (s.persons[person_t::son1].pos == person_t::onboard) ||
                (s.persons[person_t::son2].pos == person_t::onboard) ||
                (s.persons[person_t::prisoner].pos == person_t::onboard)) {
                log(" d1 travel alone\n");
                return false;
            }
        } else if (s.persons[person_t::daughter2].pos == person_t::onboard) {
            if (s.boat.passengers==1 ||
                (s.persons[person_t::daughter1].pos == person_t::onboard) ||
                (s.persons[person_t::son1].pos == person_t::onboard) ||
                (s.persons[person_t::son2].pos == person_t::onboard) ||
                (s.persons[person_t::prisoner].pos == person_t::onboard)) {
                log(" d2 travel alone\n");
                return false;
            }
        } else if (s.persons[person_t::son1].pos == person_t::onboard) {
            if (s.boat.passengers==1 ||
                (s.persons[person_t::daughter1].pos == person_t::onboard) ||
                (s.persons[person_t::daughter2].pos == person_t::onboard) ||
                (s.persons[person_t::son2].pos == person_t::onboard) ||
                (s.persons[person_t::prisoner].pos == person_t::onboard)) {
                log(" s1 travel alone\n");
                return false;
            }
        } else if (s.persons[person_t::son2].pos == person_t::onboard) {
            if (s.boat.passengers==1 ||
                (s.persons[person_t::daughter1].pos == person_t::onboard) ||
                (s.persons[person_t::daughter2].pos == person_t::onboard) ||
                (s.persons[person_t::son1].pos == person_t::onboard) ||
                (s.persons[person_t::prisoner].pos == person_t::onboard)) {
                log(" s2 travel alone\n");
                return false;
            }
        }
        if (s.persons[person_t::prisoner].pos != s.persons[person_t::policeman].pos) {
            auto prisoner_pos = s.persons[person_t::prisoner].pos;
            if ((s.persons[person_t::daughter1].pos == prisoner_pos) ||
                (s.persons[person_t::daughter2].pos == prisoner_pos) ||
                (s.persons[person_t::son1].pos == prisoner_pos) ||
                (s.persons[person_t::son2].pos == prisoner_pos) ||
                (s.persons[person_t::mother].pos == prisoner_pos) ||
                (s.persons[person_t::father].pos == prisoner_pos)) {
                log(" pr with family\n");
                return false;
            }
        }
        if (s.persons[person_t::prisoner].pos == person_t::onboard && s.boat.passengers<2) {
            log(" pr on boat\n");
            return false;
        }
    }
    if ((s.persons[person_t::daughter1].pos == s.persons[person_t::father].pos) &&
        (s.persons[person_t::daughter1].pos != s.persons[person_t::mother].pos)) {
        log(" d1 with f\n");
        return false;
    } else if ((s.persons[person_t::daughter2].pos == s.persons[person_t::father].pos) &&
               (s.persons[person_t::daughter2].pos != s.persons[person_t::mother].pos)) {
        log(" d2 with f\n");
        return false;
    } else if ((s.persons[person_t::son1].pos == s.persons[person_t::mother].pos) &&
               (s.persons[person_t::son1].pos != s.persons[person_t::father].pos)) {
        log(" s1 with m\n");
        return false;
    } else if ((s.persons[person_t::son2].pos == s.persons[person_t::mother].pos) &&
               (s.persons[person_t::son2].pos != s.persons[person_t::father].pos)) {
        log(" s2 with m\n");
        return false;
    }
    log(" OK\n");
    return true;
}

struct cost_t {
    size_t depth{0}; // counts the number of transitions
    size_t noise{0}; // kids get bored on shore1 and start making noise there
    bool operator<(const cost_t& other) const {
        if (depth < other.depth)
            return true;
        if (other.depth < depth)
            return false;
        return noise < other.noise;
    }
};

void successors(deque<function<void(state_t &)>> (*transitions)(const state_t &));

bool goal(const state_t& s){
    return std::all_of(std::begin(s.persons), std::end(s.persons),
                       [](const person_t& p) { return p.pos == person_t::shore2; });
}

template <typename CostFn>
void solve(CostFn&& cost) { // no type checking: OK hack here, but not good for library.
    // Overall there are 4*3*2*1/2 solutions to the puzzle
    // (children form 2 symmetric groups and thus result in 2 out of 4 permutations).
    // However the search algorithm may collapse symmetric solutions, thus only one is reported.
    // By changing the cost function we can express a preference and
    // then the algorithm should report different solutions
    auto states = state_space_t{
            state_t{}, // initial state
            cost_t{},   // initial cost
            successors<state_t>(transitions), // successor generator from your library
            &river_crossing_valid,            // invariant over states
            std::forward<CostFn>(cost)};      // cost over states
    auto solutions = states.check(&goal);
    if (solutions.empty()) {
        std::cout << "No solution\n";
    } else {
        for (auto&& trace: solutions) {
            std::cout << "Solution:\n";
            std::cout << "Boat,     Mothr,Fathr,Daug1,Daug2,Son1, Son2, Polic,Prisn\n";
            for (auto&& state: trace)
                std::cout << state << '\n';
        }
    }
}


int main() {
    std::cout << "-- Solve using depth as a cost: ---\n";
    solve([](const state_t& state, const cost_t& prev_cost){
        return cost_t{ prev_cost.depth+1, prev_cost.noise };
    }); // it is likely that daughters will get to shore2 first
    std::cout << "-- Solve using noise as a cost: ---\n";
    solve([](const state_t& state, const cost_t& prev_cost){
        auto noise = prev_cost.noise;
        if (state.persons[person_t::son1].pos == person_t::shore1)
            noise += 2; // older son is more noughty, prefer him first
        if (state.persons[person_t::son2].pos == person_t::shore1)
            noise += 1;
        return cost_t{ prev_cost.depth, noise };
    }); // son1 should get to shore2 first
    std::cout << "-- Solve using different noise as a cost: ---\n";
    solve([](const state_t& state, const cost_t& prev_cost){
        auto noise = prev_cost.noise;
        if (state.persons[person_t::son1].pos == person_t::shore1)
            noise += 1;
        if (state.persons[person_t::son2].pos == person_t::shore1)
            noise += 2; // younger son is more distressed, prefer him first
        return cost_t{ prev_cost.depth, noise };
    }); // son2 should get to the shore2 first
}
/** Example solutions (shows only the states with travel):
--- Solve using depth as a cost: ---
Boat,     Mothr,Fathr,Daug1,Daug2,Son1, Son2, Polic,Prisn
{trv,2,2},{sh1},{sh1},{sh1},{sh1},{sh1},{sh1},{~~~},{~~~}
{trv,1,2},{sh1},{sh1},{sh1},{sh1},{sh1},{sh1},{~~~},{SH2}{
{trv,2,2},{sh1},{sh1},{~~~},{sh1},{sh1},{sh1},{~~~},{SH2}
{trv,2,2},{sh1},{sh1},{SH2},{sh1},{sh1},{sh1},{~~~},{~~~}
{trv,2,2},{~~~},{sh1},{SH2},{~~~},{sh1},{sh1},{sh1},{sh1}
{trv,1,2},{~~~},{sh1},{SH2},{SH2},{sh1},{sh1},{sh1},{sh1}
{trv,2,2},{~~~},{~~~},{SH2},{SH2},{sh1},{sh1},{sh1},{sh1}
{trv,1,2},{SH2},{~~~},{SH2},{SH2},{sh1},{sh1},{sh1},{sh1}
{trv,2,2},{SH2},{sh1},{SH2},{SH2},{sh1},{sh1},{~~~},{~~~}
{trv,1,2},{~~~},{sh1},{SH2},{SH2},{sh1},{sh1},{SH2},{SH2}
{trv,2,2},{~~~},{~~~},{SH2},{SH2},{sh1},{sh1},{SH2},{SH2}
{trv,1,2},{SH2},{~~~},{SH2},{SH2},{sh1},{sh1},{SH2},{SH2}
{trv,2,2},{SH2},{~~~},{SH2},{SH2},{~~~},{sh1},{SH2},{SH2}
{trv,2,2},{SH2},{SH2},{SH2},{SH2},{SH2},{sh1},{~~~},{~~~}
{trv,2,2},{SH2},{SH2},{SH2},{SH2},{SH2},{~~~},{~~~},{sh1}
{trv,1,2},{SH2},{SH2},{SH2},{SH2},{SH2},{SH2},{~~~},{sh1}
{trv,2,2},{SH2},{SH2},{SH2},{SH2},{SH2},{SH2},{~~~},{~~~}
--- Solve using noise as a cost: ---
Boat,     Mothr,Fathr,Daug1,Daug2,Son1, Son2, Polic,Prisn
{trv,2,2},{sh1},{sh1},{sh1},{sh1},{sh1},{sh1},{~~~},{~~~}
{trv,1,2},{sh1},{sh1},{sh1},{sh1},{sh1},{sh1},{~~~},{SH2}
{trv,2,2},{sh1},{sh1},{sh1},{sh1},{~~~},{sh1},{~~~},{SH2}
{trv,2,2},{sh1},{sh1},{sh1},{sh1},{SH2},{sh1},{~~~},{~~~}
{trv,2,2},{sh1},{~~~},{sh1},{sh1},{SH2},{~~~},{sh1},{sh1}
{trv,1,2},{sh1},{~~~},{sh1},{sh1},{SH2},{SH2},{sh1},{sh1}
{trv,2,2},{~~~},{~~~},{sh1},{sh1},{SH2},{SH2},{sh1},{sh1}
{trv,1,2},{~~~},{SH2},{sh1},{sh1},{SH2},{SH2},{sh1},{sh1}
{trv,2,2},{sh1},{SH2},{sh1},{sh1},{SH2},{SH2},{~~~},{~~~}
{trv,1,2},{sh1},{~~~},{sh1},{sh1},{SH2},{SH2},{SH2},{SH2}
{trv,2,2},{~~~},{~~~},{sh1},{sh1},{SH2},{SH2},{SH2},{SH2}
{trv,1,2},{~~~},{SH2},{sh1},{sh1},{SH2},{SH2},{SH2},{SH2}
{trv,2,2},{~~~},{SH2},{~~~},{sh1},{SH2},{SH2},{SH2},{SH2}
{trv,2,2},{SH2},{SH2},{SH2},{sh1},{SH2},{SH2},{~~~},{~~~}
{trv,2,2},{SH2},{SH2},{SH2},{~~~},{SH2},{SH2},{~~~},{sh1}
{trv,1,2},{SH2},{SH2},{SH2},{SH2},{SH2},{SH2},{~~~},{sh1}
{trv,2,2},{SH2},{SH2},{SH2},{SH2},{SH2},{SH2},{~~~},{~~~}
-- Solve using different noise as a cost: ---
Boat,     Mothr,Fathr,Daug1,Daug2,Son1, Son2, Polic,Prisn
{trv,2,2},{sh1},{sh1},{sh1},{sh1},{sh1},{sh1},{~~~},{~~~}
{trv,1,2},{sh1},{sh1},{sh1},{sh1},{sh1},{sh1},{~~~},{SH2}
{trv,2,2},{sh1},{sh1},{sh1},{sh1},{sh1},{~~~},{~~~},{SH2}
{trv,2,2},{sh1},{sh1},{sh1},{sh1},{sh1},{SH2},{~~~},{~~~}
{trv,2,2},{sh1},{~~~},{sh1},{sh1},{~~~},{SH2},{sh1},{sh1}
{trv,1,2},{sh1},{~~~},{sh1},{sh1},{SH2},{SH2},{sh1},{sh1}
{trv,2,2},{~~~},{~~~},{sh1},{sh1},{SH2},{SH2},{sh1},{sh1}
{trv,1,2},{~~~},{SH2},{sh1},{sh1},{SH2},{SH2},{sh1},{sh1}
{trv,2,2},{sh1},{SH2},{sh1},{sh1},{SH2},{SH2},{~~~},{~~~}
{trv,1,2},{sh1},{~~~},{sh1},{sh1},{SH2},{SH2},{SH2},{SH2}
{trv,2,2},{~~~},{~~~},{sh1},{sh1},{SH2},{SH2},{SH2},{SH2}
{trv,1,2},{~~~},{SH2},{sh1},{sh1},{SH2},{SH2},{SH2},{SH2}
{trv,2,2},{~~~},{SH2},{~~~},{sh1},{SH2},{SH2},{SH2},{SH2}
{trv,2,2},{SH2},{SH2},{SH2},{sh1},{SH2},{SH2},{~~~},{~~~}
{trv,2,2},{SH2},{SH2},{SH2},{~~~},{SH2},{SH2},{~~~},{sh1}
{trv,1,2},{SH2},{SH2},{SH2},{SH2},{SH2},{SH2},{~~~},{sh1}
{trv,2,2},{SH2},{SH2},{SH2},{SH2},{SH2},{SH2},{~~~},{~~~}
 */
