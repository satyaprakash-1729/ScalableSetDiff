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
            if (isPure(diff, Hc, idx, N))
                pureList.push(idx);
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

    bool stat = decode(diff, diff_A_B, diff_B_A, hashes, Hc, N);
    if(!stat){
        cout<<"Error finding difference... All differences might not be recorded.\n";
    }

    unordered_set<int> ans;
    ans.insert(diff_A_B.begin(), diff_A_B.end());
    ans.insert(diff_B_A.begin(), diff_B_A.end());

    return ans;
}

int main(int argc, char *argv[]) {
    unordered_set<int> A = {40,12,59,32,74,32,52,7,89,43,75,67,88,112};
    unordered_set<int> B = {52,7,89,546,71,67,88,165,40,12,59,32,93,32};

    /* //These two data sets produce an incorrect set difference
    unordered_set<int> A = {40,12,59,32,74,32,52,7,89,43,75,67,88,112};
    unordered_set<int> B = {1048819952,439126708,128102478,314044322,772079190,1506566877,1529299921,1555170102,
    1298148561,316488765,1558007614,477837041,216917387,2139210544,1386845307,110710049,
    1914170756,606903873,1823265971,1529077132,1129940662,1976335180,84452123,316088603,
    79618302,2112888028,1030659457, 1555467755,289848869,2652690,128336225,1549418590,
    23322993,1265009319,1032716911,434993659,646518113,119204574};
    */

    /*//These two data sets produce a segfault because in estimateSetDiffSize m==3, so r==1, so 0 is returned
    //This makes N == 0, which results in problems (easy fix is to have a minimum N be a constant > 0)
    //This happens when getMinHash(A, hashFn) == getMinHash(B, hashFn) for all hash functions
    //This can happen when the two sets are similar or exactly the same.
    unordered_set<int> A = {
    792341777,79618302,1555467755,1529077132,2652690,1788677552,128336225,606903873,
    1823265971,734002505,110710049,1950176370,289848869,867774395,1558007614,2112888028,
    1030659457,1549418590,23322993,434993659,646518113,316088603,1976335180,84452123,
    1265009319,216917387,119204574,316488765,1129940662,1298148561,1555170102,1529299921,
    1032716911,477837041,1386845307,1506566877,1073559280,2139210544,314044322,1914170756,
    128102478,439126708,1346878174,1048819952};
    unordered_set<int> B = {  
    79618302,1555467755,1529077132,2652690,128336225,606903873,1823265971,110710049,
    289848869,1558007614,2112888028,1030659457,1549418590,23322993,434993659,646518113,
    316088603,1976335180,84452123,1265009319,216917387,119204574,316488765,1129940662,
    1298148561,1555170102,1529299921,1032716911,477837041,1386845307,1506566877,2139210544,
    772079190,314044322,1914170756,128102478,1048819952}; 
    */
    
    /*//Even though they are unordered sets, the order seems to matter
    //These two sets work, and the only difference is moving the last element of A to the front
    unordered_set<int> A = {
    1048819952,792341777,79618302,1555467755,1529077132,2652690,1788677552,128336225,606903873,
    1823265971,734002505,110710049,1950176370,289848869,867774395,1558007614,2112888028,
    1030659457,1549418590,23322993,434993659,646518113,316088603,1976335180,84452123,
    1265009319,216917387,119204574,316488765,1129940662,1298148561,1555170102,1529299921,
    1032716911,477837041,1386845307,1506566877,1073559280,2139210544,314044322,1914170756,
    128102478,439126708,1346878174};
    unordered_set<int> B = {  
    79618302,1555467755,1529077132,2652690,128336225,606903873,1823265971,110710049,
    289848869,1558007614,2112888028,1030659457,1549418590,23322993,434993659,646518113,
    316088603,1976335180,84452123,1265009319,216917387,119204574,316488765,1129940662,
    1298148561,1555170102,1529299921,1032716911,477837041,1386845307,1506566877,2139210544,
    772079190,314044322,1914170756,128102478,1048819952}; 
    */
    
    /*//if A and B are same set, the same error occurs
    unordered_set <int> A {0,1,2,3,4,5,6};
    unordered_set <int> B {0,1,2,3,4,5,6};
    */
    
    /*//Finally, negative numbers are not supported
    unordered_set <int> A {-1,0,1,2,3,4,5};
    unordered_set <int> B {0,1,2,3,4,5,6};
    */

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
