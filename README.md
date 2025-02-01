# Finite-Automaton-Library
An open source library for creating and operating on Finite Automata (NFA and DFA).

## Описание
Библиотеката позволява работа с крайни детерминирани автомати (DFA) и недетерминирани автомати (NFA) с ℇ-преходи над произволна азбука. Поддържат се следните операции:

- **Създаване на автомат:**
  - Краен детерминиран автомат (DFA)
  - Краен недетерминиран автомат (NFA)
- **Операции върху автоматите:**
  - Проверка за принадлежност на дума към езика на автомата
  - Потокови операции за обработка на низове
  - Обединение, сечение, конкатенация на два автомата
  - Звезда на Клини
  - Допълнение на автомат
- **Файлови операции:**
  - Запис и прочитане на автомат от файл
- **Преобразувания:**
  - Регулярен израз → Автомат
  - Автомат → Регулярен израз
  - Минимизация на автомат
- **Визуализация:**
  - Чрез **Graphviz**

---

## Използване

### Пример: Създаване на детерминиран краен автомат (DFA)
```c
// Пример за добавяне на състояния и преходи
DFA dfa;
State* q0 = dfa.addState("q0");
State* q1 = dfa.addState("q1", true);  // Крайно състояние

dfa.setStartState(q0); // Задаваме началното състояние

dfa.addTransition(q0, 'a', q1);
```

Създаването на недетерминиран краен автомат (NFA) е идентично, но позволява преходи с епсилон (ε) (празния символ). В библиотеката епсилон се представя с '@'.
```c
// Пример за добавяне на състояния и преходи
NFA nfa;
State* q0 = nfa.addState("q0");
State* q1 = nfa.addState("q1", true);
State* q2 = nfa.addState("q2", true);

nfa.setStartState(q0);

nfa.addTransition(q0, 'a', q1);
nfa.addTransition(q1, '@', q2);
```

