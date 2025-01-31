#include "State.hpp"

void State::addTransition(char symbol, State* destination) {
    transitions[symbol].push_back(destination);
}

const std::vector<State*> State::getTransitions(char symbol) const {
    auto it = transitions.find(symbol);
    return it != transitions.end() ? it->second : std::vector<State*>{};
}

bool State::hasTransition(char symbol) const {
    return transitions.find(symbol) != transitions.end();
}