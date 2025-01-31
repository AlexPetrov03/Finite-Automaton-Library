#pragma once

#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <stack>
#include <string>
#include "Automaton.hpp"
#include "PairHash.hpp"

class NFA : public Automaton
{
public:
    //Празен конструктор
    NFA() : Automaton() {}

    //Добавя преход в автомата
    void addTransition(State* source, char symbol, State* destination);

    //Връща true, ако автоматът разпознава думата, false, ако не
    bool accepts(const std::string& input) const override;
    
    //Връща указател към автомат, който разпознава обединението на езиците на this и other
    NFA* unionWith(const NFA& other) const;

    //Връща указател към автомат, който разпознава конкатенацията на езиците на this и other
    NFA* concatWith(const NFA& other) const;

    //Връща указател към автомат, който разпознава сечението на езиците на this и other
    NFA* intersectWith(const NFA& other) const;

    ////Връща указател към автомат, който се получава след прилагането на звездата на Клини върху този автомат
    NFA* kleeneStar() const;

private:
    //Връща множество от указатели към достижимите с празни преходи състояния от подадено множество от указатели към състояния в автомата
    std::unordered_set<State*> epsilonClosure(const std::unordered_set<State*>& states) const;

    //Връща множество от указатели към достижимите с подаден символ състояния от подадено множество от указатели към състояния, включително и състоянията, достижими чрез празни преходи
    std::unordered_set<State*> getNextStatesWithEpsilon(const std::unordered_set<State*>& currentStates, char symbol) const;
};