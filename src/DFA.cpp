#include "DFA.hpp"
#include <iostream>
#include <string>

State* DFA::getNextState(State* state, char c) const
{
    std::vector<State*> nextTrans = getNextStates(state, c);
    if (nextTrans.empty())
        return nullptr;
    return nextTrans.at(0);
}

void DFA::addTransition(State* source, char c, State* destination)
{
    if (c == '@') //Преходите с празния символ не са позволени в детерминираните автомати
    {
        std::cerr << "Epsilon transitions are not allowed." << std::endl;
        return;
    }

    addSymbolToAlphabet(c);

    if (source->hasTransition(c)) //Ако вече съществува преход с този символ от това състояние, не го добавяме
    {
        std::cerr << "Cannot add transition: " << source->name << "--" << c << "-->" << destination->name << std::endl;
        return;
    }
    source->addTransition(c, destination);
}

bool DFA::accepts(const std::string& input) const
{
    State* current = getStartState();
    for (char c : input)
    {
        current = getNextState(current, c);
        if (!current)
            return false;
    }
    if (!current->isFinal)
    {
        return false;
    }
    return true;
}

DFA* DFA::intersectWith(const DFA& other) const
{
    //Намираме обща азбука на двата автомата
    std::unordered_set<char> commonAlphabet;
    for (char c : getAlphabet())
    {
        if (other.getAlphabet().find(c) != other.getAlphabet().end())
        {
            commonAlphabet.insert(c);
        }
    }

    DFA* result = new DFA();

    //Персонализирана хешираща функция
    std::unordered_map<std::pair<State*, State*>, State*, PairHash> stateMap;

    State* thisStart = this->getStartState();
    State* otherStart = other.getStartState();
    State* start = result->addState(thisStart->name + "_" + otherStart->name, thisStart->isFinal && otherStart->isFinal);
    result->setStartState(start);
    stateMap[{thisStart, otherStart}] = start;

    std::queue<std::pair<State*, State*>> queue;
    queue.push({ thisStart, otherStart });

    //Използваме BFS, за да построим резултатния автомат
    while (!queue.empty())
    {
        std::pair<State*, State*> current = queue.front();
        queue.pop();

        State* stateA = current.first;
        State* stateB = current.second;
        State* currentState = stateMap[current];

        //Добавяме преходи само за символите от общата азбука
        for (char c : commonAlphabet)
        {
            State* nextA = this->getNextState(stateA, c);
            State* nextB = other.getNextState(stateB, c);

            //Ако дори и в един от двата автомата няма преход с този символ, прескачаме
            if (!nextA || !nextB)
            {
                continue;
            }

            std::pair<State*, State*> nextPair(nextA, nextB);

            //Ако двойката не е добавена в stateMap все още, създаваме ново състояние от нея и го правим
            if (stateMap.find(nextPair) == stateMap.end())
            {
                //Състояние в резултатния автомат е финално, само ако и двете състояния от двойката, която отговаря на него, са финални
                bool nextIsFinal = nextA->isFinal && nextB->isFinal;
                State* nextState = result->addState(nextA->name + "_" + nextB->name, nextIsFinal);
                stateMap[nextPair] = nextState;
                queue.push(nextPair);
            }

            result->addTransition(currentState, c, stateMap[nextPair]);
        }
    }

    return result;
}

DFA* DFA::complement() const
{
    //Правим финалните състояния нефинални, а нефиналните - финални
    DFA* result = new DFA();

    std::unordered_map<State*, State*> stateMap;

    copyStates(*this, *result, stateMap, [](State* state, State* copied)
        { copied->isFinal = !copied->isFinal; });

    copyTransitions(*this, *result, stateMap);

    result->setStartState(stateMap[this->getStartState()]);

    return result;
}

std::string DFA::toRegex()const
{
    std::unordered_map<std::string, std::unordered_map<std::string, std::string>> transitions;
    for (State* state : getStates())
    {
        for (const char& c : getAlphabet())
        {
            std::vector<State*> nextStates = getNextStates(state, c);
            for (State* next : nextStates)
            {
                // Добавяме прехода като регулярен израз
                transitions[state->name][next->name] += transitions[state->name][next->name].empty() ? std::string(1, c) : "+" + c;
            }
        }
    }

    // Запазваме състоянията, които не са финални и не са началното
    std::set<std::string> intermediateStates;
    for (State* state : getStates())
    {
        if (state != getStartState() && !state->isFinal)
        {
            intermediateStates.insert(state->name);
        }
    }

    // Премахваме междинните състояния
    for (const std::string& inter : intermediateStates)
    {
        for (State* pred : getStates())
        {
            if (transitions[pred->name].count(inter))
            {
                for (State* succ : getStates())
                {
                    if (transitions[inter].count(succ->name))
                    {
                        // Ако съществува цикъл го описваме
                        std::string loop = transitions[inter][inter].empty() ? "" : "(" + transitions[inter][inter] + ")*";
                        
                        // Конструираме нов преход от предходния до следващия
                        std::string newTransition = "(" + transitions[pred->name][inter] + ")" + loop + "(" + transitions[inter][succ->name] + ")";

                        // Добавяме новия преход към map-а и ако съществува минал преход ги комбинираме
                        transitions[pred->name][succ->name] += transitions[pred->name][succ->name].empty() ? newTransition : "+" + newTransition;
                    }
                }
            }
        }

        //Премахваме преработения преход от map-а
        transitions.erase(inter);
        for (auto& entry : transitions)
        {
            entry.second.erase(inter);
        }
    }

    // Ако има цикъл в началното състояние
    std::string startLoop = transitions[getStartState()->name][getStartState()->name];
    std::string regex;

    //Обхождаме финалните състояния
    for (State* finalState : getStates())
    {
        if (finalState->isFinal)
        {
            // Взимаме регулярния израз от началното състояние до крайното състояние
            std::string path = transitions[getStartState()->name][finalState->name];
            
            // Ако има цикъл, взимаме регулярния му израз
            std::string finalLoop = transitions[finalState->name][finalState->name];
            if (!path.empty())
            {
                // Добавяме ги във финалния регулярен израз
                regex += regex.empty() ? "" : "+";
                regex += "(" + path + ")" + (finalLoop.empty() ? "" : "(" + finalLoop + ")*");
            }
        }
    }
    return regex;
}

DFA* DFA::minimize()const { //Линк към алгоритъма, на който е базиран метода: https://www.geeksforgeeks.org/minimization-of-dfa
    //Разделяме състоянията на финални и нефинални
    std::unordered_set<State*> finalStates;
    std::unordered_set<State*> nonFinalStates;

    for (State* state : this->getStates()) {
        if (state->isFinal) {
            finalStates.insert(state);
        }
        else nonFinalStates.insert(state);
    }
    
    // Първо имаме само 2 множества - финални и нефинални
    std::vector<std::unordered_set<State*>> uniqueSetsOfStates = { finalStates, nonFinalStates };

    // Показва дали са спрели да възникват нови множества
    bool isStable = false;

    while (!isStable) {
        isStable = true;
        std::vector<std::unordered_set<State*>> newSets;

        // Обхождаме всички множества
        for (const std::unordered_set<State*>& set : uniqueSetsOfStates) {
            std::unordered_map<std::string, std::unordered_set<State*>> transitionGroups;

            // Групираме състоянията, в зависимост от преходите им и от кое множество са
            for (State* state : set) {
                std::string key;
                for (char c : getAlphabet()) {
                    State* nextState = getNextState(state, c);
                    if (nextState) {
                        for (size_t i = 0; i < uniqueSetsOfStates.size(); i++) {
                            if (uniqueSetsOfStates[i].count(nextState)) {
                                key += std::to_string(i) + c; // Правим уникален ключ
                                break;
                            }
                        }
                    }
                    // Ако няма преход със символа
                    else {
                        key += c;
                    }
                }
                // Групираме в зависимост от ключа
                transitionGroups[key].insert(state); 
            }

            // Ако множеството се е разделило, продължаваме алгоритъма
            if (transitionGroups.size() > 1) {
                isStable = false;
            }

            // Добавяме новите групи
            for (const auto& group : transitionGroups) {
                newSets.push_back(group.second);
            }
        }
        uniqueSetsOfStates = newSets;
    }

    DFA* result = new DFA();
    std::unordered_map<State*, State*> stateMap;

    // Създаваме състояние в резултатния автомат за всяка получила се група
    for (const std::unordered_set<State*>& set : uniqueSetsOfStates) {
        State* chosenOne = *set.begin();
        State* newState = result->addState(chosenOne->name, chosenOne->isFinal);
        for (State* state : set) {
            stateMap[state] = newState;
        }
    }

    for (State* state : getStates()) {
        State* mapped = stateMap[state];
        for (char c : getAlphabet()) {
            State* nextState = getNextState(state, c);
            if (nextState) {
                State* mappedNext = stateMap[nextState];
                if (!mapped->hasTransition(c))
                    result->addTransition(mapped, c, mappedNext);
            }
        }
    }

    result->setStartState(stateMap[getStartState()]);

    return result;
}