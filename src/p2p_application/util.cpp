//
// Created by satya on 6/1/20.
//

#include "util.h"
#include "time.h"
#include <unistd.h>

std::unordered_set<int> getRandomSet(int size, int start, int end){
    sleep(1);
    srand(time(NULL));

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
