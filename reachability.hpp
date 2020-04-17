//
// Created by Daniel Moesgaard Andersen on 4/16/20.
//

#ifndef PUZZLEENGINE_REACHABILITY_HPP
#define PUZZLEENGINE_REACHABILITY_HPP

#include <vector>
#include <list>
#include <functional>
#include <iostream>
#include <algorithm>
#include <typeinfo>
#include <memory>

using namespace std;

// Search order enum for requirement #4
enum search_order_t {
    breadth_first, depth_first
};

// Log function to print output, used in family.cpp instead of cout.
void log(string input) {
    cout << input << endl;
};


// The state space class
template<class StateT, class CostT = std::nullptr_t>
class state_space_t {
private:
    StateT _initialState; // Initial state
    CostT _initialCost; // Initial cost
    function<vector<function<void(StateT &)>>(StateT &)> _transitionFunctions;
    function<bool(const StateT &)> _invariantFunction;
public:
    // Simple constructor which works without cost
    state_space_t(
            StateT initialState,
            function<vector<function<void(StateT &)>>(StateT &)> sucessorGenerator,
            function<bool(const StateT &)> invariantFunc
            );

    state_space_t(
            StateT initialState,
            function<vector<function<void(StateT &)>>(StateT &)> sucessorGenerator
    );

    // Requirement 8: If search order is not set, it should use breadth to avoid state spaces being deeper than the
    // stack allows for the frog puzzle.
    list<list<const StateT>*> check(
            function<bool(const StateT &)> goalStateCheck,
            const search_order_t& searchOrder = search_order_t::breadth_first) {
        return nullptr; // TODO: Implement check function.
    }
};

template<class StateT, class CostT>
state_space_t<StateT, CostT>::state_space_t(StateT initialState,
                                            function<vector<function<void(StateT &)>>(StateT &)> sucessorGenerator) {
    // TODO: implement constructor with no invariant func

}

// Overload of cout<< to print states
template <class StateT>
ostream& operator<< (ostream& os, const vector<StateT> &v) {
    for(auto &i: v) {
        os << *i << endl;
    }
    return os;
}

// Constructor for state_space_t for the simple examples in frogs and crossing with no cost.
template<class StateT, class CostT>
state_space_t<StateT, CostT>::state_space_t(StateT initialState,
                                            function<vector<function<void(StateT &)>>(StateT &)> sucessorGenerator,
                                            function<bool(const StateT &)> invariantFunc) {
    // TODO: implement constructor with invariant func
}

template <class StateT>
function<vector<function<void(StateT &)>>(StateT &)>
successors(vector<function<void(StateT &)>> (*transitions)(const StateT &)) {
    return transitions;
}

#endif //PUZZLEENGINE_REACHABILITY_HPP
