#pragma once

#include "Automaton.hpp"
#include "PairHash.hpp"
#include <queue>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <fstream>
#include <iostream>

class DFA : public Automaton
{
private:
    //Връща състоянието след преход със символ c
    State* getNextState(State* state, char c) const;

public:
    //Добавя преход в автомата
    void addTransition(State* source, char c, State* destination);

    //Връща true, ако автоматът разпознава думата, false, ако не
    bool accepts(const std::string& input) const override;

    //Връща указател към автомат, който разпознава сечението на езиците на this и other. 
    DFA* intersectWith(const DFA& other) const;

    //Връща указател към автомат, който разпознава допълнението на езика на this
    DFA* complement() const;

    //Връща указател към автомат с минимален брой състояния, който разпознава езикът на this
    DFA* minimize()const;

    //Преобразува автомата в регулярен израз
    std::string toRegex()const;
};