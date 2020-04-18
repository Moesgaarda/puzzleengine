//
// Created by Daniel Moesgaard Andersen on 4/16/20.
//

#ifndef PUZZLEENGINE_REACHABILITY_HPP
#define PUZZLEENGINE_REACHABILITY_HPP

#include <vector>
#include <list>
#include <functional>
#include <iostream>
#include <memory>
#include <iterator>

using namespace std;

// Search order enum for requirement #4
enum search_order_t {
    breadth_first, depth_first
};

// Pass the transition generator function
template<class state_t, template<class...> class container_t>
function<container_t<function<void(state_t &)>>(state_t &)>
successors(container_t<function<void(state_t &)>> (*transitions)(const state_t &)) {
    return transitions;
}

// Keep the trace with a copy of self and a pointer to parent.
template<class state_t>
struct trace_state {
    trace_state *parent = nullptr;
    state_t self = nullptr;
};

// Log function to print output, used in family.cpp instead of cout.
void log(string input) {
    cout << input << endl;
}

// Overload of << operator to print list content
template<class state_t>
ostream &operator<<(ostream &os, const vector<state_t> &v) {
    for (auto var : v) {
        os << (int) var;
    }
    return os;
}

// Overload of << operator to print array content
template<typename state_t>
ostream &operator<<(ostream &os, const array<state_t, 3> &v) {
    for (auto arr : v) {
        os << (int) arr;
    }
    return os;
}


// The state space class
template<class state_t, template<class...> class container_t, class cost_t = std::nullptr_t>
class state_space_t {
private:
    state_t _initialState; // Initial state
    cost_t _initialCost;
    function<container_t<function<void(state_t &)>>(state_t &)> _transitionFunction;
    function<bool(const state_t &)> _invariantFunction;
    bool _useCost = false;
    function<cost_t(const state_t &state, const cost_t &cost)> _costFunction;
    template<class validation_f>
    container_t<state_t> solver(validation_f isGoalState, search_order_t searchOrder);

    template<class validation_f>
    container_t<state_t> costSolver(validation_f isGoalState);


public:
    // Default constructor with no cost
    state_space_t(
            const state_t initialState,
            function<container_t<function<void(state_t &)>>(state_t &)> transitionFunction,
            // Default value is a function that takes a const state and returns true.
            bool (*invariantFunction)(const state_t&) = [](const state_t &state) { return true; }
    ) {
        _initialState = initialState;
        _transitionFunction = transitionFunction;
        _invariantFunction = invariantFunction;
        _useCost = false;
    };

    // Constructor with cost enabled
    template<typename lambda>
    state_space_t(
            const state_t initialState,
            const cost_t initialCost,
            function<container_t<function<void(state_t &)>>(state_t &)> transitionFunction,
            bool (*invariantFunction)(const state_t &) = [](const state_t &s) { return true; },
            lambda costFunction = [](const state_t &s, const cost_t &c) { return cost_t{0, 0}; }
    ) {
        _initialState = initialState;
        _initialCost = initialCost;
        _transitionFunction = transitionFunction;
        _invariantFunction = invariantFunction;
        _costFunction = costFunction;
        _useCost = true;
    }

    // The function to call the solver, default search order is breadth_first, as a sensible choice as defined in
    // requirement 8.
    template<class validation_f>
    container_t<state_t> check(
            validation_f isGoalState,
            search_order_t order = search_order_t::breadth_first) {

        if(_useCost){
            return costSolver(isGoalState);
        }
        return solver(isGoalState, order);
    }
};

template<class state_t, template<class...> class container_t, class cost_t>
template<class validation_f>
container_t<state_t>
state_space_t<state_t, container_t, cost_t>::solver(validation_f isGoalState, search_order_t order) {
    state_t currentState;
    trace_state<state_t> *traceState{};
    list<state_t> passed;
    list<trace_state<state_t> *> waiting;
    list<state_t> traces;
    container_t<state_t> containedSolution;

    // Adding the states to waiting
    waiting.push_back(new trace_state<state_t>{nullptr, _initialState});

    // Keep iterating through the waiting list until it is empty
    while (!waiting.empty()) {
        if (order == breadth_first) {
            currentState = waiting.front()->self;
            traceState = waiting.front();
            waiting.pop_front();
        } else if (order == depth_first) {
            currentState = waiting.back()->self;
            traceState = waiting.back();
            waiting.pop_back();
        } else {
            log("Invalid search order supplied.");
        }
        if (isGoalState(currentState)) {
            while (traceState->parent != nullptr) {
                // Add stack trace to the solution list
                traces.push_front(traceState->self);
                traceState = traceState->parent;
            }

            // Add self trace to solution list
            traces.push_front(traceState->self);


            // Convert to a vector
            for (state_t &trace: traces) {
                containedSolution.push_back(trace);
            }

            // Return early if a goal state was found
            return containedSolution;
        }
        if (!(find(passed.begin(), passed.end(), currentState) != passed.end())) {
            passed.push_back(currentState);
            auto transitions = _transitionFunction(currentState);

            for (auto transition: transitions) {
                auto successor{currentState};
                transition(successor);

                if (_invariantFunction(successor)) {
                    waiting.push_back(new trace_state<state_t>{traceState, successor});
                }
            }
        }
    }
    return containedSolution;
}

template<class state_t, template<class...> class container_t, class cost_t>
template<class validation_f>
container_t<state_t>
state_space_t<state_t, container_t, cost_t>::costSolver(validation_f isGoalState) {
    state_t currentState;
    cost_t currentCost, newCost;
    currentCost = _initialCost;
    trace_state<state_t> *traceState;
    list<state_t> passed, solution;
    list<pair<cost_t, trace_state<state_t> *>> waiting;
    container_t<state_t> containedSolution;

    // Generate a set of cost and trace state to find the lowest cost aka where to go next
    waiting.push_back(make_pair(currentCost, new trace_state<state_t>{nullptr, _initialState}));

    while(!waiting.empty()){
        // Prepare to go to the next state, which is next in the queue
        currentState = waiting.front().second->self;
        currentCost = waiting.front().first; // First element of pair is cost
        traceState = waiting.front().second; // Second element is trace state
        waiting.pop_front();

        if (isGoalState(currentState)) {
            while (traceState->parent != nullptr) {
                // Add stack trace to the solution list
                solution.push_front(traceState->self);
                traceState = traceState->parent;
            }

            // Add self trace to solution list
            solution.push_front(traceState->self);


            // Convert to a vector
            for (state_t &st: solution) {
                containedSolution.push_back(st);
            }

            // Return early if a goal state was found
            return containedSolution;
        }

        // Check if current state has already been passed otherwise push it
        if(!(find(passed.begin(), passed.end(), currentState) != passed.end())) {
            passed.push_back(currentState);
            auto transitions = _transitionFunction(currentState);

            for (auto transition: transitions) {
                auto successor{currentState};
                transition(successor);

                if (!_invariantFunction(successor)) {
                    continue;
                }
                newCost = _costFunction(successor, currentCost);
                waiting.push_back(make_pair(newCost, new trace_state<state_t>{traceState, successor}));
            }
            if (!transitions.empty()) {
                // Sort the list to make sure that the lowest cost is first.
                waiting.sort([](const pair<cost_t, trace_state<state_t> *> &a,
                             pair<cost_t, trace_state<state_t> *> &b) { return a.first < b.first; });
            }

        }
    }

    return containedSolution;
}

#endif //PUZZLEENGINE_REACHABILITY_HPP
