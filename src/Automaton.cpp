#include "Automaton.hpp"


Automaton::Automaton(const Automaton& other)
{

}

State* Automaton::addState(const std::string& name, bool isFinal) {
    State* state = new State(name, isFinal);
    states.push_back(state);

    return state;
}

size_t Automaton::getStateCount() const
{
    return states.size();
}

void Automaton::clearStates() {
    for (State* state : getStates())
    {
        delete state;
    }

    states.clear();
}

void Automaton::clearAlphabet() {
    alphabet.clear();
}

void Automaton::addSymbolToAlphabet(char c) {
    alphabet.insert(c);
}

std::vector<State*> Automaton::getNextStates(State* state, char symbol) const {
    return state->getTransitions(symbol);
}

const std::unordered_set<char>& Automaton::getAlphabet() const
{
    return alphabet;
}

void Automaton::print() const {
    for (State* state : states) {
        bool hasTransitions = false;

        std::cout << "State: " << state->name;

        if (state == startState) {
            std::cout << " (Start)";
        }
        if (state->isFinal) {
            std::cout << " (Final)";
        }

        std::cout << std::endl << "Transitions: " << std::endl;

        int maxLength = 0;

        for (const char& c : alphabet) {
            std::vector<State*> nextStates = getNextStates(state, c);
            if (!nextStates.empty()) {
                hasTransitions = true;
                for (State* next : nextStates) {
                    std::cout << state->name << "--" << c << "-->" << next->name << std::endl;
                }
            }
        }

        std::vector<State*> epsilonTransitions = getNextStates(state, '@');
        if (!epsilonTransitions.empty()) {
            hasTransitions = true;
            for (State* next : epsilonTransitions) {
                std::cout << state->name << "--@-->" << next->name << std::endl;
            }
        }


        if (!hasTransitions) {
            std::cout << "None" << std::endl;
        }

        std::cout << std::endl;
    }
}

void Automaton::copyStates(const Automaton& source, Automaton& target, std::unordered_map<State*, State*>& stateMap, std::function<void(State*, State*)> func)const {
    for (State* state : source.getStates()) {
        State* copied = target.addState(state->name, state->isFinal);
        stateMap[state] = copied;

        func(state, copied); //Позполява преизползването на метода за различни алгоритми
    }
}

void Automaton::copyTransitions(const Automaton& source, Automaton& target, std::unordered_map<State*, State*>& stateMap) const
{
    for (State* state : source.getStates())
    {
        for (char c : source.getAlphabet())
        {
            for (State* next : state->getTransitions(c))
            {
                target.addTransition(stateMap[state], c, stateMap[next]);
            }
        }
        for (State* next : state->getTransitions('@'))
        {
            target.addTransition(stateMap[state], '@', stateMap[next]);
        }
    }
}
void Automaton::saveToFile(const std::string& fileName)const
{
    std::ofstream file(fileName, std::ios::out | std::ios::binary);

    if (!file.is_open())
    {
        throw std::invalid_argument("Could not open file for writing.");
    }

    size_t alphabetSize = getAlphabet().size();
    file.write(reinterpret_cast<const char*>(&alphabetSize), sizeof(alphabetSize));
    for (char c : getAlphabet())
    {
        file.write(reinterpret_cast<const char*>(&c), sizeof(c));
    }

    size_t stateCount = getStateCount();
    file.write(reinterpret_cast<const char*>(&stateCount), sizeof(stateCount));

    std::unordered_map<State*, size_t> stateIndexMap;
    size_t index = 0;
    for (State* state : getStates())
    {
        stateIndexMap[state] = index++;

        file.write(reinterpret_cast<const char*>(&state->name), sizeof(state->name));

        bool isFinal = state->isFinal;
        file.write(reinterpret_cast<const char*>(&isFinal), sizeof(isFinal));
    }

    State* start = getStartState();
    size_t startIndex = stateIndexMap[start];
    file.write(reinterpret_cast<const char*>(&startIndex), sizeof(startIndex));

    for (State* state : getStates())
    {
        size_t stateIdx = stateIndexMap[state];

        for (char c : alphabet)
        {
            std::vector<State*> nextStates = state->getTransitions(c);
            for (State* nextState : nextStates)
            {
                size_t nextStateIdx = stateIndexMap[nextState];
                file.write(reinterpret_cast<const char*>(&stateIdx), sizeof(stateIdx));
                file.write(reinterpret_cast<const char*>(&c), sizeof(c));
                file.write(reinterpret_cast<const char*>(&nextStateIdx), sizeof(nextStateIdx));
            }

        }

        char epsilon = '@';

        if(state->hasTransition(epsilon))
        for (State* nextState : state->getTransitions(epsilon)) {
            size_t nextStateIdx = stateIndexMap[nextState];
            file.write(reinterpret_cast<const char*>(&stateIdx), sizeof(stateIdx));
            file.write(reinterpret_cast<const char*>(&epsilon), sizeof(char));
            file.write(reinterpret_cast<const char*>(&nextStateIdx), sizeof(nextStateIdx));
        }

    }

    file.close();
}

void Automaton::loadFromFile(const std::string& fileName)
{
    std::ifstream file(fileName, std::ios::in | std::ios::binary);

    if (!file.is_open())
    {
        throw std::invalid_argument("Could not open file for reading.");
    }

    clearStates();
    clearAlphabet();
    setStartState(nullptr);

    size_t alphabetSize;
    file.read(reinterpret_cast<char*>(&alphabetSize), sizeof(alphabetSize));
    for (size_t i = 0; i < alphabetSize; ++i)
    {
        char c;
        file.read(reinterpret_cast<char*>(&c), sizeof(c));
        addSymbolToAlphabet(c);
    }

    size_t stateCount;
    file.read(reinterpret_cast<char*>(&stateCount), sizeof(stateCount));

    std::vector<State*> stateIndexMap(stateCount);

    for (size_t i = 0; i < stateCount; ++i)
    {
        std::string name;
        file.read(reinterpret_cast<char*>(&name), sizeof(name));

        bool isFinal;
        file.read(reinterpret_cast<char*>(&isFinal), sizeof(isFinal));

        State* state = addState(name, isFinal);
        stateIndexMap[i] = state;
    }

    size_t startIndex;
    file.read(reinterpret_cast<char*>(&startIndex), sizeof(startIndex));
    setStartState(stateIndexMap[startIndex]);

    while (!file.eof())
    {
        size_t sourceIndex, destinationIndex;
        char c;

        file.read(reinterpret_cast<char*>(&sourceIndex), sizeof(sourceIndex));
        file.read(reinterpret_cast<char*>(&c), sizeof(c));
        file.read(reinterpret_cast<char*>(&destinationIndex), sizeof(destinationIndex));

        State* source = stateIndexMap[sourceIndex];
        State* destination = stateIndexMap[destinationIndex];
        addTransition(source, c, destination);
    }

    file.close();
}

void Automaton::vizualize() const
{
    generateGraphvizFile("output.dot");
    system("dot -Tpng .\\GeneratedAutomatons\\output.dot -o .\\GeneratedAutomatons\\output.png");
    system("start .\\GeneratedAutomatons\\output.png");
}

void Automaton::saveAsPng(const std::string& fileName) const
{
    generateGraphvizFile(fileName + ".dot");
    system(("dot -Tpng .\\GeneratedAutomatons\\" + fileName + ".dot -o .\\GeneratedAutomatons\\" + fileName + ".png").c_str());
}

//Създава dot файл за автомата по синтаксиса на Graphviz
void Automaton::generateGraphvizFile(const std::string& fileName)const {
    std::ofstream file("GeneratedAutomatons\\" + fileName);

    if (!file.is_open()) {
        std::cerr << "Could not open file for writing." << std::endl;
        return;
    }

    file << "digraph {" << std::endl; //Създава насочен граф, който ще представя графично нашия автомат
    file << "node [shape = circle];" << std::endl;
    file << "rankdir=LR;" << std::endl; //Подрежда възлите от ляво надясно

    file << "n0 [label= \"\", shape=none,height=.0,width=.0]" << std::endl;

    for (State* state : states) { //Добавя състоянията като възли в графа
        if (state == startState) {
            file << "n0 -> " << state->name << std::endl; //Възелът отговарящ на началното състояние, обозначаваме с влизаща стрелка без източник
        }
        if (state->isFinal) { 
            file << state->name << " [shape = doublecircle]; " << std::endl; //Прави възлите, отговарящи на финалните състояния с двоен кръг
        }
        for (char c : alphabet) { //Добавя преходите като ребра в графа
            for (State* next : state->getTransitions(c)) { 
                file << state->name << " -> " << next->name << " [label = \"" << c << "\"];" << std::endl;
            }
        }
        for (State* next : state->getTransitions('@')) { 
            file << state->name << " -> " << next->name << " [label = \"" << '@' << "\"];" << std::endl;
        }
    }

    file << "}" << std::endl;

    file.close();
}