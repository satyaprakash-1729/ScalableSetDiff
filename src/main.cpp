#include <bits/stdc++.h>

using namespace std;
typedef int (*hashFuncType)(int, int);

class IBFCell{
    int idSum;
    int hashSum;
    int count;

public:
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

    int getIdSum() {
        return idSum;
    }

    void setIdSum(int idSum1) {
        idSum = idSum1;
    }

    int getHashSum() {
        return hashSum;
    }

    void setHashSum(int hashSum1) {
        hashSum = hashSum1;
    }

    int getCount() {
        return count;
    }

    void setCount(int count1) {
        count = count1;
    }
};

int hashFunction1(int key, int N){
    return key % N;
}

int hashFunction2(int key, int N) {
    return ((key >> 8 & key << 24)^(key >> 24 & key << 8)) % N;
}

int hashFunction3(int key, int N){
    return ((key >> 4 & key << 8)^(key >> 8 & key << 4)) % N;
}

int hashFunction4(int key, int N) {
    long l = 2654435769L;
    return (key * l >> 32) % N ;
//    return ((key >> 16 & key << 16)^(key >>  & key << 8)) % N;
}

int getMinHash(unordered_set<int>& numSet, hashFuncType hashFn){
    int minHash = INT_MAX, temp;
    int ans = 0;
    for(int x: numSet){
        if(minHash > (temp = hashFn(x, INT_MAX))){
            minHash = temp;
            ans = x;
        }
    }
    return ans;
}

int estimateSetDiffSize(unordered_set<int>& A, unordered_set<int>& B, vector<hashFuncType>& hashes){
    int m = 0;
    for(hashFuncType& hashFn: hashes){
        if(getMinHash(A, hashFn) == getMinHash(B, hashFn)) m++;
    }
    float k = hashes.size();
    float r = float(m)/k;
    return ((1.-r)/(1.+r))*(float(A.size()+B.size()));
}

void encode(unordered_set<int>& numSet, int N,
        vector<hashFuncType>& hashes, hashFuncType Hc,
        vector<IBFCell*>& ibf){
    for(int num : numSet) {
        for(auto & hashe : hashes) {
            int idx = hashe(num, N);
            ibf[idx]->addEntry(num, Hc(num, N));
        }
    }
}

void subtract(vector<IBFCell*>& ibf1, vector<IBFCell*>& ibf2,
        vector<IBFCell*>& diff, int N){
    for(int i=0; i<N; i++){
        diff[i] = new IBFCell();
        diff[i]->setIdSum(ibf1[i]->getIdSum()^ibf2[i]->getIdSum());
        diff[i]->setHashSum(ibf1[i]->getHashSum()^ibf2[i]->getHashSum());
        diff[i]->setCount(ibf1[i]->getCount()-ibf2[i]->getCount());
    }
}

bool isPure(vector<IBFCell*>& diff, hashFuncType Hc, int i, int N){
    return abs(diff[i]->getCount())==1
            && Hc(diff[i]->getIdSum(), N)==diff[i]->getHashSum();
}

bool decode(vector<IBFCell*>& diff, unordered_set<int>& diff_A_B,
        unordered_set<int>& diff_B_A, vector<hashFuncType>& hashes,
        hashFuncType Hc, int N){
    queue<int> pureList;
    for(int i=0; i<N; i++){
        if(isPure(diff, Hc, i, N)){
            pureList.push(i);
        }
    }

    while(!pureList.empty()){
        int i = pureList.front();
        pureList.pop();

        if(!isPure(diff, Hc, i, N)) continue;

        int idSum = diff[i]->getIdSum();
        int count = diff[i]->getCount();
        if(count>0){
            diff_A_B.insert(idSum);
        }else{
            diff_B_A.insert(idSum);
        }

        for(auto & hashe : hashes) {
            int idx = hashe(idSum, N);
            diff[idx]->setIdSum(diff[idx]->getIdSum()^idSum);
            diff[idx]->setHashSum(diff[idx]->getHashSum()^Hc(idSum, N));
            diff[idx]->setCount(diff[idx]->getCount()-count);
        }
    }
    for(int i=0; i<N; i++){
        if(diff[i]->getCount() || diff[i]->getIdSum() || diff[i]->getHashSum()) return false;
    }
    return true;
}

unordered_set<int> getSetDifference(unordered_set<int>& A,
        unordered_set<int>& B, int N, vector<hashFuncType>& hashes,
        hashFuncType Hc){
    vector<IBFCell*> ibf1(N);
    vector<IBFCell*> ibf2(N);
    for (int i=0; i<N; i++) ibf1[i] = new IBFCell();
    for (int i=0; i<N; i++) ibf2[i] = new IBFCell();

    encode(A, N, hashes, Hc, ibf1);
    encode(B, N, hashes, Hc, ibf2);

    vector<IBFCell*> diff(N);
    subtract(ibf1, ibf2, diff, N);

    unordered_set<int> diff_A_B;
    unordered_set<int> diff_B_A;

    decode(diff, diff_A_B, diff_B_A, hashes, Hc, N);

    unordered_set<int> ans;
    ans.insert(diff_A_B.begin(), diff_A_B.end());
    ans.insert(diff_B_A.begin(), diff_B_A.end());

    return ans;
}

int main(int argc, char *argv[]) {
    unordered_set<int> A = {40,12,59,32,74,32,52,7,89,43,75,67,88,112};
    unordered_set<int> B = {52,7,89,546,71,67,88,165,40,12,59,32,93,32};

    vector<hashFuncType> hashes;
    hashes.push_back(hashFunction1);
    hashes.push_back(hashFunction2);
    hashes.push_back(hashFunction3);

    cout<<"Calculating estimated set difference size...\n";
    int d = estimateSetDiffSize(A, B, hashes);
    cout<<"Estimated Set Diff Size: "<<d<<endl;

    float alpha = 1.4;

    hashFuncType Hc = &hashFunction4;

    int N = d*alpha;

    cout<<"Calculating Set Difference ...\n";
    unordered_set<int> setDiff = getSetDifference(A, B, N, hashes, Hc);
    cout<<"---- SET DIFFERENCE ----\n";
    for(int x : setDiff){
        cout<<x<<endl;
    }

    return 0;
}
