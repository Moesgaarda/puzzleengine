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

using namespace std;

// Search order enum for requirement #4
enum search_order_t {
    breadth_first, depth_first
};

// Pass the transition generator function
template <class StateT>
function<vector<function<void(StateT &)>>(StateT &)>
successors(vector<function<void(StateT &)>> (*transitions)(const StateT &)) {
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

template <typename StateT>
ostream& operator<<(ostream& os, const vector<StateT>& v)
{
    for(auto &var : v){
        os << (int)var << " ";
    }
    return os;
}

// The state space class
template<class StateT>
class state_space_t {
private:
    StateT _initialState; // Initial state
    function<vector<function<void(StateT &)>>(StateT &)> _transitionFunctions;
    function<bool(const StateT &)> _invariantFunction;

    template<class ValidationFunction>
    std::vector<StateT> solver(ValidationFunction isGoalState, search_order_t searchOrder);
public:
    state_space_t(
            StateT initialState,
            function<vector<function<void(StateT &)>>(StateT &)> sucessorGenerator
    );


    // The function to call the solver, default search order is breadth_first, as a sensible choice as defined in
    // requirement 8.
    template<class ValidationFunction>
    std::vector<StateT> check(ValidationFunction isGoalState, search_order_t order = search_order_t::breadth_first);
};

template<class StateT>
template<class ValidationFunction>
vector<StateT> state_space_t<StateT>::check(ValidationFunction isGoalState, search_order_t order) {
    std::vector<StateT> solution;
    solution = solver(isGoalState, order);

    // Returns the list of states.
    return solution;
}

template<class StateT>
state_space_t<StateT>::state_space_t(StateT initialState,
                                            function<vector<function<void(StateT &)>>(StateT &)> sucessorGeneratorFunction) {
    _initialState = initialState;
    _transitionFunctions = sucessorGeneratorFunction;
}

template<class StateT>
template<class ValidationF>
std::vector<StateT>
state_space_t<StateT>::solver(ValidationF isGoalState, search_order_t order) {
    StateT currentState;
    trace_state<StateT> *traceState;
    std::list<StateT> passed;
    std::list<trace_state<StateT> *> waiting;
    std::list<StateT> solution;
    vector<StateT> vectorSolution;

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
        if (!(std::find(passed.begin(), passed.end(), currentState) != passed.end())) {
            passed.push_back(currentState);
            auto transitions = _transitionFunctions(currentState);

            for (auto transition: transitions) {
                auto successor{currentState};
                transition(successor);

                waiting.push_back(new trace_state<StateT>{traceState, successor});
            }
        }
    }

    for (StateT &state: solution){
        vectorSolution.push_back(state);
    }

    return vectorSolution;
}

#endif //PUZZLEENGINE_REACHABILITY_HPP
