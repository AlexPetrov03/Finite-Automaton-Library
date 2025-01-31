#pragma once
#include <vector>
#include <iostream>
#include <unordered_set>
#include <unordered_map>
#include <functional>
#include <fstream>
#include "State.hpp"

class Automaton {
public:
    //Празен конструктор
    Automaton() : startState(nullptr) {}


    Automaton(const Automaton& other);

    virtual ~Automaton() {
        for (State* state : states) {
            delete state;
        }
    }

    //Добавя състояние в автомата
    State* addState(const std::string& name, bool isFinal = false);

    //Задава кое е началното състояние
    void setStartState(State* state) {
        startState = state;
    }

    //Добавя преход в автомата
    virtual void addTransition(State* source, char symbol, State* destination) = 0;

    //Проверява дали дума се разпознава от автомата
    virtual bool accepts(const std::string& input) const = 0;

    friend bool operator>>(Automaton& fa, const std::string& input)
    {
        return fa.accepts(input);
    }

    //Описва автомата в стандартния изход
    void print() const;

    //Връща обект от тип vector, съдържащ състоянията на автомата
    std::vector<State*> getStates() const {
        return states;
    }

    //Връша указател към началното състояние на автомата
    State* getStartState() const { return startState; }

    //Връща състоянията, достижими от дадено състояние след преход с даден симбол
    std::vector<State*> getNextStates(State* state, char symbol) const;

    //Връша азбуката на автомата
    const std::unordered_set<char>& getAlphabet() const;

    //Връща броя на състоянията на автомата
    size_t getStateCount()const;

    //Чисти масивът, съдържащ състоянията на автомата. Освобождава и динамичната памет
    void clearStates();

    //Чисти множеството, което съдържа азбуката на автомата
    void clearAlphabet();

    //Добавя символ към азбуката на автомата
    void addSymbolToAlphabet(char c);

    //Копира състоянията в нов автомат като пълни map с ключ оригиналното състояние, по който да могат да се добавят преходите.
    //Има включена функция като параметър, която се изпълнява върху всяко състояние при копирането
    void copyStates(const Automaton& source, Automaton& target,
        std::unordered_map<State*, State*>& stateMap,
        std::function<void(State*, State*)> func) const;

    //Копира преходите от source в target, използвайки предварително попълнен map на състоянията
    virtual void copyTransitions(const Automaton& source, Automaton& target, std::unordered_map<State*, State*>& stateMap)const;

    //Запазва автомата във файл в двоичен вид
    void saveToFile(const std::string& fileName)const;

    //Зарежда автомата от файл
    void loadFromFile(const std::string& fileName);

    //Създава dot и png файл на автомата, използвайки Graphviz, след това го отваря
    void vizualize()const;

    //Създава dot и png файл на автомата, използвайки Graphviz 
    void saveAsPng(const std::string& fileName)const;

private:
    //Масив от указатели към състоянията на автомата
    std::vector<State*> states;
    
    //Указател към началното състояние на автомата
    State* startState;

    //Множество, представляващо азбуката на автомата
    std::unordered_set<char> alphabet;

    //Генерира Graphviz файл, описващ насочен граф, представляващ автомата
    void generateGraphvizFile(const std::string& fileName)const;

};