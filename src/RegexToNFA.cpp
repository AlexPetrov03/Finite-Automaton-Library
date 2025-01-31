#include "RegexToNFA.hpp"


bool RegexToNFA::isOperator(char c)
{
    return c == '+' || c == '*' || c == '.' || c == '&';
}

int RegexToNFA::priority(char oper)
{

    switch (oper)
    {
    case '*':
        return 3;
    case '.':
        return 2;
    case '+':
        return 1;
    case '&':
        return 1;
    default:
        return 0;
    }
}


std::string RegexToNFA::toPostfix(const std::string& regex)
{
    std::string postfix;
    std::stack<char> operatorStack;

    for (char c : regex)
    {
        if (c == '(')
        {
            operatorStack.push(c);
        }
        else if (c == ')')
        {
            while (!operatorStack.empty() && operatorStack.top() != '(')
            {
                postfix += operatorStack.top();
                operatorStack.pop();
            }
            operatorStack.pop();
        }
        else if (isOperator(c))
        {
            while (!operatorStack.empty() && priority(operatorStack.top()) >= priority(c))
            {
                postfix += operatorStack.top();
                operatorStack.pop();
            }
            operatorStack.push(c);
        }
        else
        {
            postfix += c;
        }
    }

    while (!operatorStack.empty())
    {
        postfix += operatorStack.top();
        operatorStack.pop();
    }

    return postfix;
}

NFA* RegexToNFA::handleChar(char c)
{
    NFA* nfa = new NFA();
    State* start = nfa->addState("start");
    State* end = nfa->addState("end", true);
    nfa->setStartState(start);

    //Добавя преход с произволен символ
    if (c == '?')
    {
        for (char symbol = 32; symbol < 127; ++symbol)
        {
            nfa->addTransition(start, symbol, end);
        }
    }
    else {
        nfa->addTransition(start, c, end);
    }

    return nfa;
}

NFA* RegexToNFA::handleOperator(char operation, std::stack<NFA*>& stack)
{
    switch (operation)
    {
    //Звезда на Клини
    case '*':
    {
        NFA* top = stack.top();
        stack.pop();
        return top->kleeneStar();
    }
    //Обединение
    case '+':
    {
        NFA* right = stack.top();
        stack.pop();
        NFA* left = stack.top();
        stack.pop();
        return left->unionWith(*right);
    }
    //Сечение
    case '&':
    {
        NFA* right = stack.top();
        stack.pop();
        NFA* left = stack.top();
        stack.pop();
        return left->intersectWith(*right);
    }
    //Конкатенация
    case '.':
    {
        NFA* right = stack.top();
        stack.pop();
        NFA* left = stack.top();
        stack.pop();
        return left->concatWith(*right);
    }
    default:
        return nullptr;
    }
}

NFA* RegexToNFA::fromRegex(const std::string& regex)
{
    std::string postfix = toPostfix(regex);
    std::cout << "Postfix expression: " << postfix << std::endl;
    std::stack<NFA*> stack;

    for (char c : postfix)
    {
        if (isOperator(c))
        {
            stack.push(handleOperator(c, stack));
        }
        else
        {
            stack.push(handleChar(c));
        }
    }

    return stack.top();
}