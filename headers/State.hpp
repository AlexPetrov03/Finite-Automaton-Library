#pragma once
#include <string>
#include <unordered_map>
#include <vector>

struct State {
    std::string name;
    bool isFinal;

    /*Поддържа се възможността за множество преходи с един и същ символ специално за епсилон преходите на недетерминираните автомати
    Предприети са мерки да може да се добави преход най-много с един символ за детерминирани автомати*/
    std::unordered_map<char, std::vector<State*>> transitions;

    //Ако състоянието е финално, вторият параметър се слага true. За нефинални е false или може да се пропусне
    State(const std::string& name, bool isFinal = false) : name(name), isFinal(isFinal) {}

    void addTransition(char symbol, State* destination);

    const std::vector<State*> getTransitions(char symbol) const;

    bool hasTransition(char symbol) const;
};