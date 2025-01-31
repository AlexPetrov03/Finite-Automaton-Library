#include "NFA.hpp"
#include <iostream>
#include <queue>

void NFA::addTransition(State* source, char symbol, State* destination)
{
    if (symbol != '@')
    {
        addSymbolToAlphabet(symbol);
    }

    source->addTransition(symbol, destination);
}

std::unordered_set<State*> NFA::epsilonClosure(const std::unordered_set<State*>& states) const
{
    //Множество от състояния достижими с празни преходи
    std::unordered_set<State*> closure(states);
    std::stack<State*> stack;

    for (State* state : states)
    {
        stack.push(state);
    }

    //DFS
    while (!stack.empty())
    {
        State* state = stack.top();
        stack.pop();

        for (State* nextState : state->getTransitions('@'))
        {
            //Ако състоянието не е вече в множеството, го добавяме
            if (closure.find(nextState) == closure.end())
            {
                closure.insert(nextState);
                stack.push(nextState);
            }
        }
    }

    return closure;
}

std::unordered_set<State*> NFA::getNextStatesWithEpsilon(const std::unordered_set<State*>& currentStates, char symbol) const
{
    std::unordered_set<State*> nextStates;

    for (State* state : currentStates)
    {
        for (State* nextState : state->getTransitions(symbol))
        {
            nextStates.insert(nextState);
        }
    }

    return epsilonClosure(nextStates);
}

bool NFA::accepts(const std::string& input) const
{
    if (!getStartState())
    {
        return false;
    }

    std::unordered_set<State*> currentStates = { getStartState() };
    currentStates = epsilonClosure(currentStates);

    for (char symbol : input)
    {
        currentStates = getNextStatesWithEpsilon(currentStates, symbol);
        if (currentStates.empty())
        {
            return false;
        }
    }

    for (State* state : currentStates)
    {
        if (state->isFinal)
        {
            return true;
        }
    }

    return false;
}

NFA* NFA::unionWith(const NFA& other) const
{
    NFA* result = new NFA();

    //Създаваме ново "общо" начално състояние
    State* newStart = result->addState("newStart");
    result->setStartState(newStart);

    std::unordered_map<State*, State*> stateMap;

    copyStates(*this, *result, stateMap, [this, result, newStart](State* state, State* copied)
        {
            copied->name = copied->name + "_1";
            if (state == this->getStartState())
            {
                result->addTransition(newStart, '@', copied); //Добавяме празен преход от общото начално състояние към началното на първия
            } });

            copyTransitions(*this, *result, stateMap);

            copyStates(other, *result, stateMap, [&other, result, newStart](State* state, State* copied)
                {
                    copied->name = copied->name + "_2";
                    if (state == other.getStartState())
                    {
                        result->addTransition(newStart, '@', copied);
                    } });

                    copyTransitions(other, *result, stateMap);

                    return result;
}

NFA* NFA::concatWith(const NFA& other) const 
{
    NFA* result = new NFA();

    std::vector<State*> thisFinalStates;
    State* otherStartState = nullptr;

    std::unordered_map<State*, State*> stateMap;

    copyStates(*this, *result, stateMap, [&thisFinalStates, result, this](State* state, State* copied)
        {
            //Копираме състоянията на първия автомат в резултатния автомат, като използваме ламбда израз да променим имената, да копираме началното състояние
            //и да запазим финалните му състояния

            copied->name = copied->name + "_1";
            if (state->isFinal) {
                thisFinalStates.push_back(copied); 
                copied->isFinal = false; //финалните състояния на първия вече не са финални
            }

            if (state == getStartState())
            {
                result->setStartState(copied);
            } });

            copyTransitions(*this, *result, stateMap);

            copyStates(other, *result, stateMap, [&otherStartState, result, &other](State* state, State* copied)
            {
                    //Копираме състоянията на втория автомат в резултатния автомат, като използваме ламбда израз да променим имената им и да запазим началното състояние

                    copied->name = copied->name + "_2";
                    if (state == other.getStartState())
                    {
                        otherStartState = copied;
                    } });

            copyTransitions(other, *result, stateMap);

            for (State* state : thisFinalStates) //Добавяме празен преход от всички бивши финални състояния на първия към началното на втория автомат
            {
                result->addTransition(state, '@', otherStartState);
            }

            return result;
}

NFA* NFA::kleeneStar() const
{
    NFA* result = new NFA();

    std::unordered_map<State*, State*> stateMap;
    std::vector<State*> finalStates;

    copyStates(*this, *result, stateMap, [&finalStates, this, &result](State* state, State* copied)
        {
            if (state->isFinal) {
                finalStates.push_back(copied); //Запазваме финалните състояния
            }
            if (state == getStartState()) {
                result->setStartState(copied); //Задаваме началното състояние
            } });

            copyTransitions(*this, *result, stateMap);

            //Добавяме финално състояние достижимо с празен преход от началното. Това позволява разпознаване на празната дума от автомата
            //Можем и просто да направим началното състояние финално, но този начин е по-верен към оригиналния алгоритъм
            State* finalStart = result->addState("final-start", true);
            result->addTransition(stateMap[getStartState()], '@', finalStart);


            //Добавяме празни преходи от финалните състояния към началното състояние.
            for (State* state : finalStates)
            {
                result->addTransition(state, '@', result->getStartState());
            }

            return result;
}

NFA* NFA::intersectWith(const NFA& other)const { //Аналогично на реализацията за детерминиран автомат, само дето с всеки преход отиваме в множество от състояния
    //Намираме обща азбука на двата автомата
    std::unordered_set<char> commonAlphabet;
    for (char symbol : getAlphabet())
    {
        if (other.getAlphabet().find(symbol) != other.getAlphabet().end())
        {
            commonAlphabet.insert(symbol);
        }
    }

    NFA* result = new NFA();

    //Персонализирана хешираща функция
    std::unordered_map<std::pair<State*, State*>, State*, PairHash> stateMap;
    std::unordered_set<State*> thisStartClosure = epsilonClosure({ getStartState() });
    std::unordered_set<State*> otherStartClosure = epsilonClosure({ other.getStartState() });

    State* start = result->addState("start", false);
    result->setStartState(start);

    //Използваме BFS, за да построим резултатния автомат
    std::queue<std::pair<std::unordered_set<State*>, std::unordered_set<State*>>> queue;
    queue.push({ thisStartClosure, otherStartClosure });
    stateMap[{*thisStartClosure.begin(), * otherStartClosure.begin()}] = start;

    while (!queue.empty())
    {
        auto current = queue.front();
        queue.pop();

        auto currentStatesA = current.first;
        auto currentStatesB = current.second;

        State* currentState = stateMap[{*currentStatesA.begin(), * currentStatesB.begin()}];

        for (State* stateA : currentStatesA)
        {
            for (State* stateB : currentStatesB)
            {
                //Състояние в резултатния автомат е финално, само ако и двете състояния от двойката, която отговаря на него, са финални
                if (stateA->isFinal && stateB->isFinal)
                {
                    currentState->isFinal = true;
                }
            }
        }

        //Добавяме преходи само за символите от общата азбука
        for (char symbol : commonAlphabet)
        {
            std::unordered_set<State*> nextStatesA = getNextStatesWithEpsilon(currentStatesA, symbol);
            std::unordered_set<State*> nextStatesB = other.getNextStatesWithEpsilon(currentStatesB, symbol);

            //Ако дори и в един от двата автомата няма преход с този символ, прескачаме
            if (nextStatesA.empty() || nextStatesB.empty())
            {
                continue;
            }

            //Създаваме двойка за новите множества от състояния
            std::pair<std::unordered_set<State*>, std::unordered_set<State*>> nextPair = std::make_pair(nextStatesA, nextStatesB);

            //Ако двойката не е добавена в stateMap все още, създаваме ново състояние от нея и го правим
            if (stateMap.find({ *nextStatesA.begin(), *nextStatesB.begin() }) == stateMap.end())
            {
                State* nextState = result->addState("state" + std::to_string(stateMap.size()), false);
                stateMap[{*nextStatesA.begin(), * nextStatesB.begin()}] = nextState;
                //Добавяме новата двойка в опашката
                queue.push(nextPair);
            }

            //Добавяме преход от сегашното състояние към следващото в резултатния автомат
            result->addTransition(currentState, symbol, stateMap[{*nextStatesA.begin(), * nextStatesB.begin()}]);
        }
    }

    return result;
}