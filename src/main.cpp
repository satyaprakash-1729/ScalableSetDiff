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
    key -= (key<<6);
    key ^= (key>>17);
    key -= (key<<9);
    key ^= (key<<4);
    key -= (key<<3);
    key ^= (key<<10);
    key ^= (key>>15);
    return key % N;
}

int hashFunction3(int key, int N){
    key = (key ^ 61) ^ (key >> 16);
    key = key + (key << 3);
    key = key ^ (key >> 4);
    key = key * 0x27d4eb2d;
    key = key ^ (key >> 15);
    return key % N;
}

int hashFunction4(int key, int N) {
    key = (key+0x7ed55d16) + (key<<12);
    key = (key^0xc761c23c) ^ (key>>19);
    key = (key+0x165667b1) + (key<<5);
    key = (key+0xd3a2646c) ^ (key<<9);
    key = (key+0xfd7046c5) + (key<<3);
    key = (key^0xb55a4f09) ^ (key>>16);
    return key % N;
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
    unordered_set<int> A = {72, 58, 78, 62, 90, 69, 57, 50, 98, 59, 52, 43, 57, 76, 33, 26, 66,
                            45, 21, 78, 61, 24, 55, 27, 14, 56, 34, 44, 25, 94, 34, 24, 26, 18,
                            82, 48, 23, 47, 43, 96, 18, 38, 48, 76, 37, 54, 42, 60, 10, 37, 20,
                            69, 89, 43, 57, 13, 93, 86, 18, 89, 99, 79, 68, 26, 32, 86, 31, 62,
                            63, 79, 68, 29, 26, 55, 90, 58, 25, 28, 99, 25, 82, 91, 70, 86, 26,
                            35, 92, 59, 37, 57, 70, 84, 19, 39, 93, 49, 89, 22, 71, 95, 62, 78,
                            62, 21, 66, 45, 63, 32, 99, 89, 62, 72, 15, 68, 11, 49, 68, 75, 95,
                            94, 53, 88, 35, 73, 44, 88, 31, 82, 47, 68, 20, 67, 46, 19, 37, 76,
                            71, 33, 61, 96, 69, 51, 35, 48, 43, 64, 43, 64, 96, 66, 33, 15, 75,
                            69, 20, 10, 86, 68, 98, 26, 99, 30, 59, 43, 65, 13, 49, 74, 81, 82,
                            77, 33, 77, 63, 11, 17, 34, 64, 48, 15, 69, 43, 83, 51, 98, 95, 60,
                            92, 15, 59, 30, 66, 84, 89, 66, 74, 32, 50, 89, 83};
    unordered_set<int> B = {95, 55, 77, 87, 34, 37, 70, 65, 54, 31, 15, 99, 15, 10, 28, 23, 78,
                            74, 34, 53, 91, 91, 13, 78, 93, 50, 87, 13, 96, 93, 32, 95, 44, 71,
                            89, 36, 69, 70, 49, 96, 22, 39, 49, 70, 23, 59, 56, 60, 54, 66, 16,
                            16, 13, 78, 48, 60, 37, 80, 93, 37, 88, 96, 71, 12, 59, 19, 46, 23,
                            95, 33, 33, 84, 76, 54, 42, 33, 22, 60, 86, 42, 15, 82, 17, 22, 42,
                            71, 18, 65, 79, 55, 44, 63, 44, 47, 22, 79, 12, 68, 44, 93, 27, 41,
                            76, 36, 64, 61, 72, 69, 70, 80, 24, 75, 16, 61, 46, 87, 57, 80, 55,
                            80, 10, 94, 45, 50, 43, 28, 20, 86, 75, 18, 71, 10, 63, 45, 44, 97,
                            14, 72, 89, 33, 82, 72, 93, 37, 21, 48, 89, 48, 73, 77, 95, 31, 75,
                            19, 99, 28, 55, 76, 35, 45, 58, 94, 74, 49, 91, 52, 61, 26, 27, 91,
                            73, 93, 68, 54, 25, 97, 86, 63, 16, 27, 50, 98, 76, 84, 66, 24, 79,
                            62, 87, 60, 62, 44, 90, 93, 65, 93, 26, 65, 88, 43};

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

    cout<<"---- SET DIFFERENCE VIA STL ----\n";
    unordered_set<int> setDiffInbuilt;
    set_difference(A.begin(), A.end(), B.begin(), B.end(), inserter(setDiffInbuilt, setDiffInbuilt.end()));
    for(int x : setDiffInbuilt){
        cout<<x<<" ";
    }
    cout<<endl;

    cout<<"-------------------------------\nCalculating Set Difference ...\n";
    unordered_set<int> setDiff = getSetDifference(A, B, N, hashes, Hc);
    cout<<"---- SET DIFFERENCE VIA IBF ----\n";
    for(int x : setDiff){
        if(!setDiffInbuilt.count(x)) {
            cout << "INCORRECT DIFFERENCE!\n";
            return 1;
        }
        cout<<x<<" ";
    }
    cout<<endl;
    cout<<"----------ACCURACY---------\n";
    cout<< (setDiff.size() / float(setDiffInbuilt.size())) * 100.<<"%\n--------------\n";
    return 0;
}
