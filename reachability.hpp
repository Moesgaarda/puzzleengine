//
// Created by Daniel Moesgaard Andersen on 4/16/20.
//

#ifndef PUZZLEENGINE_REACHABILITY_HPP
#define PUZZLEENGINE_REACHABILITY_HPP

#include <list>
#include <functional>
#include <iostream>
#include <memory>
#include <iterator>
#include <fstream>


// Search order enum for requirement #4
enum search_order_t {
    breadth_first, depth_first
};

// Pass the transition generator function
template<class StateT, template<class...> class ContainerT>
std::function<ContainerT<std::function<void(StateT &)>>(StateT &)>
successors(ContainerT<std::function<void(StateT &)>> (*transitions)(const StateT &)) {
    return transitions;
}

// Struct to save the current trace.
template<class StateT>
struct trace_state {
    trace_state *parent = nullptr;
    StateT self = nullptr;
};

// Log function, logs to file so it doesn't flood the console
void log(const std::string& input) {
    std::ofstream out;
    out.open("/home/tekrus/output.txt", std::ofstream::app);
    out << input;
    out.close();
}


// The state space class
template<class StateT, template<class...> class ContainerT, class CostT = std::nullptr_t>
class state_space_t {
private:
    StateT _initialState; // Initial state
    CostT _initialCost;
    std::function<ContainerT<std::function<void(StateT &)>>(StateT &)> _transitionFunction;
    std::function<bool(const StateT &)> _invariantFunction;
    bool _useCost = false;
    std::function<CostT(const StateT &state, const CostT &cost)> _costFunction;
    template<class validation_f>
    ContainerT<ContainerT<StateT>> solver(validation_f isGoalState, search_order_t searchOrder);

    template<class validation_f>
    ContainerT<ContainerT<StateT>> costSolver(validation_f isGoalState);


public:
    // Default constructor with no cost
    state_space_t(
            const StateT initialState,
            std::function<ContainerT<std::function<void(StateT &)>>(StateT &)> transitionFunction,
            // Default value is a function that takes a const state and returns true.
            bool (*invariantFunction)(const StateT&) = [](const StateT &state) { return true; }
    ) {
        _initialState = initialState;
        _transitionFunction = transitionFunction;
        _invariantFunction = invariantFunction;

        // Fail if arguments are of wrong types (Requirement 9)
        static_assert(std::is_class<StateT>::value, "StateT must be struct or class.");

    };

    // Constructor with cost enabled
    template<typename lambda>
    state_space_t(
            const StateT initialState,
            const CostT initialCost,
            std::function<ContainerT<std::function<void(StateT &)>>(StateT &)> transitionFunction,
            bool (*invariantFunction)(const StateT &) = [](const StateT &s) { return true; },
            lambda costFunction = [](const StateT &s, const CostT &c) { return CostT{0, 0}; }
    ) {

        // Fail if arguments are of wrong types (Requirement 9)
        // It is enough to check if StateT and CostT are classes, as it also captures structs.
        static_assert(std::is_class<StateT>::value, "StateT must be a class or struct.");
        static_assert(std::is_class<CostT>::value, "CostT must be a class or struct.");
        static_assert(std::is_convertible<lambda, std::function<CostT (const StateT&, const CostT&)>>::value,
                "Cost function must be a function, that can be converted to function<CostT (const StateT&, const CostT&)>>");

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
    ContainerT<ContainerT<StateT>> check(
            validation_f isGoalState,
            search_order_t order = search_order_t::breadth_first) {

        if(_useCost){
            return costSolver(isGoalState);
        }
        return solver(isGoalState, order);
    }
};

// The default solver when cost is not involved
template<class StateT, template<class...> class ContainerT, class CostT>
template<class validation_f>
ContainerT<ContainerT<StateT>>
state_space_t<StateT, ContainerT, CostT>::solver(validation_f isGoalState, search_order_t order) {
    StateT currentState;
    trace_state<StateT> *traceState{};
    std::list<StateT> passed;
    std::list<trace_state<StateT> *> waiting;
    std::list<StateT> traces;
    ContainerT<StateT> containedSolution;
    ContainerT<ContainerT<StateT>> result;

    // Adding the states to waiting
    waiting.push_back(new trace_state<StateT>{nullptr, _initialState});

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

            // Convert to a generic type
            for(auto &trace: traces){
                containedSolution.push_back(trace);
            }

            result.push_back(containedSolution);
            // Return early if a goal state was found
           // return result;
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
    return result;
}

// The solver for when cost is involved.
template<class StateT, template<class...> class ContainerT, class CostT>
template<class validation_f>
ContainerT<ContainerT<StateT>>
state_space_t<StateT, ContainerT, CostT>::costSolver(validation_f isGoalState) {
    StateT currentState;
    CostT currentCost, newCost;
    currentCost = _initialCost;
    trace_state<StateT> *traceState;
    std::list<StateT> passed, solution;
    std::list<std::pair<CostT, trace_state<StateT> *>> waiting;
    ContainerT<StateT> containedSolution;
    ContainerT<ContainerT<StateT>> result;

    // Generate a set of cost and trace state to find the lowest cost aka where to go next
    waiting.push_back(std::make_pair(currentCost, new trace_state<StateT>{nullptr, _initialState}));

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


            // Convert to a container
            for (StateT &st: solution) {
                containedSolution.push_back(st);
            }

            result.push_back(containedSolution);
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
                waiting.push_back(std::make_pair(newCost, new trace_state<StateT>{traceState, successor}));
            }
            if (!transitions.empty()) {
                // Sort the list to make sure that the lowest cost is first.
                waiting.sort([](const std::pair<CostT, trace_state<StateT> *> &a,
                                std::pair<CostT, trace_state<StateT> *> &b) { return a.first < b.first; });
            }

        }
    }

    return result;
}

#endif //PUZZLEENGINE_REACHABILITY_HPP
