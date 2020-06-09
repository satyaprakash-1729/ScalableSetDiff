//
// Created by satya on 6/1/20.
//
#ifndef IBFSETDIFF_UTIL_H
#include <bits/stdc++.h>

#define IBFSETDIFF_UTIL_H

std::unordered_set<int> getRandomSet(int size, int start, int end);
void printSet(const std::string& name, std::unordered_set<int>& A);
std::unordered_set<int> getDatasetFromFile(const std::string& filename);

#endif //IBFSETDIFF_UTIL_H
