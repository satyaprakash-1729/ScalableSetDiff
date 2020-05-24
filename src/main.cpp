#include <bits/stdc++.h>

using namespace std;
typedef int (*hashFuncType)(int, int);

class IBFCell{
public:
    int idSum;
    int hashSum;
    int count;

    IBFCell(){
        idSum = 0;
        hashSum = 0;
        count = 0;
    }

    void addEntry(int num, int hash){
        idSum ^= num;
        hashSum ^= hash;
        count++;
    }
};

int hashFunction1(int key, int N){
    return key % N;
}

int hashFunction2(int key, int N) {
    long l = 2654435769L;
    return (key * l >> 32) % N ;
}

int hashFunction3(int key, int N){
    return (4*key + 5)%N;
}

int hashFunction4(int key, int N) {
    return key % N;
}

unordered_set<int> getSetDifference(unordered_set<int>& A, unordered_set<int>& B, int N,
        vector<hashFuncType>& hashes, hashFuncType Hc){
    vector<IBFCell*> ibf1(N);
    vector<IBFCell*> ibf2(N);

    for(int num : A) {
        for(auto & hashe : hashes) {
            int idx = hashe(num, N);
            if(!ibf1[idx]) ibf1[idx] = new IBFCell();
            ibf1[idx]->addEntry(num, Hc(num, N));
        }
    }

    for(auto x=B.begin(); x!=B.end(); x++) {
        for(int i=0; i<hashes.size(); i++) {
            int idx = hashes[i](*x, N);
            if(!ibf2[idx]) ibf2[idx] = new IBFCell();
            ibf2[idx]->addEntry(*x, Hc(*x, N));
        }
    }

    return unordered_set<int>({});
}

int main() {
    unordered_set<int> A = {1, 2, 3};
    unordered_set<int> B = {5, 2, 1};

    int d = 5;
    float alpha = 1.4;

    vector<hashFuncType> hashes;
    hashes.push_back(hashFunction1);
    hashes.push_back(hashFunction2);
    hashes.push_back(hashFunction3);

    hashFuncType Hc = &hashFunction4;

    int N = d*alpha;

    unordered_set<int> setDiff = getSetDifference(A, B, N, hashes, Hc);

    return 0;
}
