#include "util.h"

using namespace std;
typedef int (*hashFuncType)(int, int);


#ifndef ALPHA
    #define ALPHA 4
#endif

#ifndef BETA
    #define BETA 80
#endif

#ifndef NUM_HASHES
    #define NUM_HASHES 3
#endif

#ifndef SET1_SIZE
    #define SET1_SIZE 10
#endif

#ifndef SET2_SIZE
    #define SET2_SIZE 10
#endif

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

int hashFunctionK(int key, int N) {
    hash<int> h;
    return h(key+1)%N;
}

int recHash(int cnt, int key, int N){
    if(!cnt) return hashFunctionK(key, N);
    return hashFunctionK(recHash(cnt-1, key, N), N);
}

int hashFunctionC(int key, int N) {
    hash<int> h;
    return recHash(7, key, N);
}

vector<int> getDiffHashedInd(int num, int cnt, int N){
    unordered_set<int> inds;
    for(int i=0; i<cnt; i++){
        int ind = hashFunctionK(num, N);
        while(inds.count(ind)){
            ind = hashFunctionK(ind, N);
        }
        inds.insert(ind);
    }
    return vector<int>(inds.begin(), inds.end());
}

void encode(unordered_set<int>& numSet, int N,
        int k, hashFuncType Hc,
        vector<IBFCell*>& ibf){
    for(int num : numSet) {
        vector<int> idxs = getDiffHashedInd(num, k, N);
        for(int idx: idxs){
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
        unordered_set<int>& diff_B_A, int k,
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
        vector<int> idxs = getDiffHashedInd(idSum, k, N);
        for(int idx: idxs){
            diff[idx]->setIdSum(diff[idx]->getIdSum()^idSum);
            diff[idx]->setHashSum(diff[idx]->getHashSum()^Hc(idSum, N));
            diff[idx]->setCount(diff[idx]->getCount()-count);
            if (isPure(diff, Hc, idx, N))
                pureList.push(idx);
        }
    }
    for(int i=0; i<N; i++){
        if(diff[i]->getCount()!=0 || diff[i]->getIdSum()!=0 || diff[i]->getHashSum()!=0) return false;
    }
    return true;
}

vector<unordered_set<int>> getSetDifference(unordered_set<int>& A,
        unordered_set<int>& B, int N, int k,
        hashFuncType Hc){
    vector<IBFCell*> ibf1(N);
    vector<IBFCell*> ibf2(N);
    for (int i=0; i<N; i++) ibf1[i] = new IBFCell();
    for (int i=0; i<N; i++) ibf2[i] = new IBFCell();

    encode(A, N, k, Hc, ibf1);
    encode(B, N, k, Hc, ibf2);

    vector<IBFCell*> diff(N);
    subtract(ibf1, ibf2, diff, N);

    unordered_set<int> diff_A_B;
    unordered_set<int> diff_B_A;

    bool stat = decode(diff, diff_A_B, diff_B_A, k, Hc, N);
    if(!stat){
        cout<<"Error finding difference... All differences might not be recorded.\n";
    }

    vector<unordered_set<int>> ans;
    ans.push_back(diff_A_B);
    ans.push_back(diff_B_A);

    return ans;
}

class Strata_IBF
{
public:
    vector<vector<IBFCell*>> ibfs;
    int ibfSize;
    int logu;
    int numHashes;

    Strata_IBF(int ibfSize, int numHashes, int logu){
        this->logu = logu;
        this->ibfSize = ibfSize;
        this->numHashes = numHashes;
        for(int i=0; i<logu; i++){
            vector<IBFCell*> ibf1(ibfSize);
            for(int j=0; j<ibfSize; j++)
                ibf1[j] = new IBFCell();
            this->ibfs.push_back(ibf1);
        }
    }

    static int getNumTrailingZeros(int x){
        if(!x) return 0;
        int mask = 1;
        int i = 0;
	    while (mask <= x){
	        if ((x & mask) != 0)
	            return i;
	        i++;
	        mask = mask<<1;
	    }
        return i;
    }

    void encode(unordered_set<int>& numSet, hashFuncType Hc){
        for(int num: numSet){
            vector<int> idxs = getDiffHashedInd(num, this->numHashes, this->ibfSize);
            int hashed = Hc(num, (int)(1<<this->logu));
            int trails = getNumTrailingZeros(hashed);
            for(int idx: idxs){
                this->ibfs[trails][idx]->addEntry(num, Hc(num, this->ibfSize));
            }
        }
    }

    int estimateLength(Strata_IBF* strata1, hashFuncType Hc){
        int ans = 0;
        for(int i=this->logu-1; i>=0; i--){
            vector<IBFCell*> diff(this->ibfSize);
            subtract(this->ibfs[i], strata1->ibfs[i], diff, this->ibfSize);
            unordered_set<int> s1;
            unordered_set<int> s2;
            if(decode(diff, s1, s2, this->numHashes, Hc, this->ibfSize)){
                ans += (int)(s1.size() + s2.size());
            }else{
                return (1<<(i+1))*ans;
            }
        }
        return ans;
    }
};

void parseAndPopulate(int argc, char *argv[], int& alpha, int& beta, int& num_hashes,
                        int& set1size, int& set2size, string& file1path,
                        string& file2path, bool& verbose){
    for(int i=1; i<argc; i++){
        if(string(argv[i]) == "--beta"){
            beta = stoi(string(argv[i+1]));
        }
        else if(string(argv[i]) == "--alpha"){
            alpha = stoi(string(argv[i+1]));
        }
        else if(string(argv[i]) == "--num_hashes"){
            num_hashes = stoi(string(argv[i+1]));
        }
        else if(string(argv[i]) == "--set1size"){
            set1size = stoi(string(argv[i+1]));
        }
        else if(string(argv[i]) == "--set2size"){
            set2size = stoi(string(argv[i+1]));
        }
        else if(string(argv[i]) == "--file1path"){
            file1path = string(argv[i+1]);
        }
        else if(string(argv[i]) == "--file2path"){
            file2path = string(argv[i+1]);
        }
        else if(string(argv[i]) == "--verbose"){
            verbose = true;
            i--;
        }
        else{
            cout<<"Usage: ./IBFSetDiff --beta BETA --alpha ALPHA --num_hashes NUM_HASHES --file1path path --file2path path --set1size size --set2size size --verbose\n";
            exit(1);
        }
        i++;
    }
}

int main(int argc, char *argv[]) {
    int alpha = ALPHA;
    int beta = BETA;
    int num_hashes = NUM_HASHES;
    int set1size = SET1_SIZE;
    int set2size = SET2_SIZE;
    bool verbose = false;
    string file1path = "";
    string file2path = "";

    if(argc==2){
        string arg1 = string(argv[1]);
        if(arg1=="--help")
            cout<<"Usage: ./IBFSetDiff --beta BETA --alpha ALPHA --num_hashes NUM_HASHES --file1path path --file2path path --set1size size --set2size size --verbose\n";
        return 1;
    }
    parseAndPopulate(argc, argv, alpha, beta, num_hashes, set1size, set2size, file1path, file2path, verbose);

    unordered_set<int> A;
    unordered_set<int> B;

    if(file1path=="" || file2path==""){
        A = getRandomSet(set1size, 10000, 90000);
        B = getRandomSet(set2size, 10000, 90000);
    }else{
        A = getDatasetFromFile(file1path);
        B = getDatasetFromFile(file2path);
    }
    if(verbose){
        cout << "ALPHA: " << alpha << " BETA: " << beta << " NUM_HASHES: " << num_hashes << endl;
        printSet("A", A);
        printSet("B", B);
    }

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
    const clock_t begin_time = clock();
    hashFuncType Hc = &hashFunctionC;

    int logu1 = *max_element(A.begin(), A.end());
    int logu2 = *max_element(B.begin(), B.end());
    int logu = (int)(log(max(logu1, logu2)) / log(2))+1;

    if(verbose){
        cout<<"LOGU: "<<logu<<endl;
        cout<<"Calculating estimated set difference size...\n";
    }

    auto strata1 = new Strata_IBF(beta, num_hashes, logu);
    auto strata2 = new Strata_IBF(beta, num_hashes, logu);

    strata1->encode(A, Hc);
    strata2->encode(B, Hc);

    int d = strata1->estimateLength(strata2, Hc);
    if(verbose)
        cout<<"Estimated Set Diff Size: "<<d<<endl;

    if(d) {
        int N = (int)((float)d * alpha);
        if(verbose){
            cout<< "N: "<<N<<endl;
            cout << "Calculating Set Difference ...\n";
        }
        vector<unordered_set<int>> setDiff = getSetDifference(A, B, N, num_hashes, Hc);
        cout << "---- SET DIFFERENCE A-B ----\n";
        for (int x : setDiff[0]) {
            cout << x << endl;
        }
        cout << "---- SET DIFFERENCE B-A ----\n";
        for (int x : setDiff[1]) {
            cout << x << endl;
        }
        cout<<"Retrieved Difference Size: "<<setDiff[0].size()+setDiff[1].size()<<"\n";
    }else{
        cout<<"Sets Identical...\n";
    }
    if(verbose)
        std::cout << "Time Taken: " << float( clock () - begin_time ) /  CLOCKS_PER_SEC << "\n";
    int ans = 0;
    for(int x: A) if(!B.count(x)) ans++;
    for(int x: B) if(!A.count(x)) ans++;
    cout<<"Actual Difference Size: "<<ans<<endl;
    return 0;
}
