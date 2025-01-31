#pragma once

struct PairHash { //Персонализирана хешираща функция функция за stateMap, когато ключът е std::pair<State*, State*>
    template <typename T1, typename T2>
    std::size_t operator()(const std::pair<T1, T2>& p) const {
        auto hash1 = std::hash<T1>()(p.first);
        auto hash2 = std::hash<T2>()(p.second);
        return hash1 ^ (hash2 << 1);
    }
};