//
// Created by satya on 6/1/20.
//

#include "util.h"

std::unordered_set<int> getRandomSet(int size, int start, int end){
    std::unordered_set<int> ans;
    for(int i=0; i<size; i++){
        ans.insert(rand()%((start - end) + 1) + start);
    }
    return ans;
}

void printSet(const std::string& name, std::unordered_set<int>& A){
    std::cout<<name<<": ";
    for(int x: A){
        std::cout<<x<<" ";
    }
    std::cout<<std::endl;
}

std::unordered_set<int> getDatasetFromFile(const std::string& filename){
    std::unordered_set<int> ans;
    std::ifstream fin(filename);
    int num;
    while(fin>>num){
        ans.insert(num);
    }
    return ans;
}
