#pragma once
#include <iostream>

#include <stack>
#include <string>
#include <unordered_map>
#include "NFA.hpp"

class RegexToNFA
{
public:
    //Връща УКАЗАТЕЛ към недетерминиран автомат, построен от регулярен израз
    static NFA* fromRegex(const std::string& regex);

private:
    //Връща дали символът е оператор
    static bool isOperator(char c);

    //Връща приоритета на оператора
    static int priority(char oper);

    //Преобразува регулярен израз в обратен полски запис
    static std::string toPostfix(const std::string& regex);

    //Обработва символ, който не е оператор
    static NFA* handleChar(char c);

    //Обработва символ, който е оператор
    static NFA* handleOperator(char op, std::stack<NFA*>& stack);
};