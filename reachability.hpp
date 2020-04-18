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
template<class StateT, template<class...> class ContainerT>
function<ContainerT<function<void(StateT &)>>(StateT &)>
successors(ContainerT<function<void(StateT &)>> (*transitions)(const StateT &)) {
    return transitions;
}

// Keep the trace
template <class StateT>
struct trace_state {
    trace_state *parent = nullptr;
    StateT self = nullptr;
};

// Log function to print output, used in family.cpp instead of cout.
void log(string input) {
    cout << input << endl;
};

// Overload of << operator to print list content
template<class StateT>
ostream& operator<<(ostream& os, const vector<StateT>& v)
{
    int i = 0;
    for(auto var : v){
        os << (int)var;
        i++;
    }
    return os;
}

// Overload of << operator to print array content
template <typename StateT>
ostream & operator<<(ostream& os, const array<StateT, 3> &v)
{
    for(auto arr : v){
        os << (int)arr;
    }
    return os;
}


// The state space class
template<class StateT, template<class...> class ContainerT>
class state_space_t {
private:
    StateT _initialState; // Initial state
    function<ContainerT<function<void(StateT &)>>(StateT &)> _transitionFunction;
    function<bool(const StateT &)> _invariantFunction;

    template<class ValidationFunction>
    ContainerT<StateT> solver(ValidationFunction isGoalState, search_order_t searchOrder);


public:
    state_space_t(
            StateT initialState,
            function<ContainerT<function<void(StateT &)>>(StateT &)> transitionFunction,
            bool invariantFunc(const StateT &) = [](
                    const StateT &state) { return true; } // Default value is a function that takes a const state and returns true.
    ) {
        _initialState = initialState;
        _transitionFunction = transitionFunction;
        _invariantFunction = invariantFunc;
    }


    // The function to call the solver, default search order is breadth_first, as a sensible choice as defined in
    // requirement 8.
    template<class ValidationFunction>
    ContainerT<StateT> check(
            ValidationFunction isGoalState,
            search_order_t order = search_order_t::breadth_first) {
        return solver(isGoalState, order);
    }
};

template<class StateT, template<class...> class ContainerT>
template<class ValidationFunction>
ContainerT<StateT>
state_space_t<StateT, ContainerT>::solver(ValidationFunction isGoalState,  search_order_t order) {
    StateT currentState;
    trace_state<StateT> *traceState;
    list<StateT> passed;
    list<trace_state<StateT> *> waiting;
    list<StateT> solution;
    ContainerT<StateT> vectorSolution;

    // Adding the states to waiting
    waiting.push_back(new trace_state<StateT>{nullptr, _initialState});

    // Keep iterating through the waiting list until it is empty
    while (!waiting.empty()) {
        if(order == breadth_first) {
            currentState = waiting.front()->self;
            traceState = waiting.front();
            waiting.pop_front();
        }
        else if (order == depth_first) {
            currentState = waiting.back()->self;
            traceState = waiting.back();
            waiting.pop_back();
        }
        else {
            log("Invalid search order supplied.");
        }
        if (isGoalState(currentState)) {
            while (traceState->parent != nullptr) {
                solution.push_front(traceState->self);
                traceState = traceState->parent;
            }

            solution.push_front(traceState->self); // Adds the start state to the solution trace.

            for (StateT &state: solution){
                vectorSolution.push_back(state);
            }

            return vectorSolution;
        }
        if (!(find(passed.begin(), passed.end(), currentState) != passed.end())) {
            passed.push_back(currentState);
            auto transitions = _transitionFunction(currentState);

            for (auto transition: transitions) {
                auto successor{currentState};
                transition(successor);

                if (_invariantFunction(successor)) {
                    waiting.push_back(new trace_state<StateT>{traceState, successor});
                }
            }
        }
    }

    for (StateT &state: solution){
        vectorSolution.push_back(state);
    }

    return vectorSolution;
}


#endif //PUZZLEENGINE_REACHABILITY_HPP
