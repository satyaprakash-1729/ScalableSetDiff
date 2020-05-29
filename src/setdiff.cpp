//
//  setdiff.cpp
//  SetDiff
//
//  Created by Surya Addanki on 5/28/20.
//  Copyright © 2020 Surya Addanki. All rights reserved.
//

#include "MurmurHash3.h"
#include <iostream>
#include <vector>
#include <queue>

using namespace std;

class IBF
{
private:
    uint32_t key_size;
    uint32_t seed = 0;

    bool is_pure(int ind){
        uint32_t out[4];
        
        if(this->count[ind] != 1 and this->count[ind] != -1)
            return false;
        get_hashes(&(this->id_sum[ind]),out);
        
        return (out[0] == this->hash_sum[ind]);
    }
    
    void get_hashes(void* key, uint32_t out[]){
        MurmurHash3_x64_128(key,this->key_size,this->seed,out);
    }

    void get_indices(void* key, uint32_t out[]){
        get_hashes(key,out);
        for(int i = 1; i < 4; i++){
            out[i] = out[i]%this->size;
        }
    }
    

public:
    uint32_t size;
    vector<int32_t> count;
    vector<uint32_t> id_sum;
    vector<uint32_t> hash_sum;
    
    /**
     * size - number of IBF cells in the Invertible Bloom Filter
     * key_size - size of key in IBF
     */

    IBF(uint32_t size, uint32_t key_size): size(size), key_size(key_size){
        this->count.resize(size,0);
        this->id_sum.resize(size,0);
        this->hash_sum.resize(size,0);
    }

    IBF* subtract(IBF *B){
        IBF* ibf = new IBF(this->size,this->key_size);
        
        for(int i = 0; i < this->size; i++){
            ibf->count[i] = this->count[i] - B->count[i];
            ibf->id_sum[i] = this->id_sum[i]^B->id_sum[i];
            ibf->hash_sum[i] = this->hash_sum[i]^B->hash_sum[i];
        }
        return ibf;
    }
    
    void add_key(void* key){
        uint32_t out[4];
        get_indices(key,out);
        for(int i = 1; i < 4; i++){
            this->count[out[i]]++;
            this->id_sum[out[i]] ^= ((uint32_t*)key)[0];
            this->hash_sum[out[i]] ^= out[0];
        }
    }

    void encode(vector<int> arr){
        for(auto num : arr)
            this->add_key(&num);
    }
    
    bool decode(){
        queue<int> pureList;
        
        for(int i = 0; i < this->size; i++){
            if(this->is_pure(i))
                pureList.push(i);
        }

        while(!pureList.empty()){
            int ind = pureList.front();
            pureList.pop();

            if(!this->is_pure(ind))
                continue;

            uint32_t id = this->id_sum[ind];
            uint32_t hashSum = this->hash_sum[ind];

            uint32_t out[4];

            get_indices(&id,out);
            int c = this->count[ind];
            for(int i = 1; i < 4; i++){
                this->hash_sum[out[i]] ^= hashSum;
                this->count[out[i]] -= c;
                this->id_sum[out[i]] ^= id;

                if(this->is_pure((uint)out[i]))
                    pureList.push(out[i]);

            }
        }
         for(int i = 0; i < this->size; i++)
             if(this->count[i] != 0 or this->id_sum[i] != 0 or this->hash_sum[i] != 0)
                 return false;
        return true;
    }
    
    bool decode(vector<vector<int>> &ans){
        queue<int> pureList;
        
        for(int i = 0; i < this->size; i++){
            if(this->is_pure(i))
                pureList.push(i);
        }
        
        while(!pureList.empty()){
            int ind = pureList.front();
            pureList.pop();
            
            if(!this->is_pure(ind))
                continue;
            
            uint32_t id = this->id_sum[ind];
            uint32_t hashSum = this->hash_sum[ind];

            if(this->count[ind] == 1)
                ans[0].push_back(id);
            else
                ans[1].push_back(id);

            uint32_t out[4];

            get_indices(&id,out);
            int c = this->count[ind];
            
            for(int i = 1; i < 4; i++){
                this->hash_sum[out[i]] ^= hashSum;
                this->count[out[i]] -= c;
                this->id_sum[out[i]] ^= id;

                if(this->is_pure((uint)out[i]))
                    pureList.push(out[i]);

            }
        }
         for(int i = 0; i < this->size; i++)
             if(this->count[i] != 0 or this->id_sum[i] != 0 or this->hash_sum[i] != 0)
                 return false;
        return true;
    }


    ~IBF(){
        count.clear();
        id_sum.clear();
        hash_sum.clear();
    }
};

class Strata_IBF
{
    IBF **ibfs;
    int num;
    
private:
    /**
     * Helper function to calculate number of trailing zeroes in a key.
     */
    uint32_t trailing_zeroes(uint32_t n) {
        uint32_t bits = 0, x = n;

        if (x) {
            /* assuming `x` has 32 bits: lets count the low order 0 bits in batches */
            /* mask the 16 low order bits, add 16 and shift them out if they are all 0 */
            if (!(x & 0x0000FFFF)) { bits += 16; x >>= 16; }
            /* mask the 8 low order bits, add 8 and shift them out if they are all 0 */
            if (!(x & 0x000000FF)) { bits +=  8; x >>=  8; }
            /* mask the 4 low order bits, add 4 and shift them out if they are all 0 */
            if (!(x & 0x0000000F)) { bits +=  4; x >>=  4; }
            /* mask the 2 low order bits, add 2 and shift them out if they are all 0 */
            if (!(x & 0x00000003)) { bits +=  2; x >>=  2; }
            /* mask the low order bit and add 1 if it is 0 */
            bits += (x & 1) ^ 1;
        }
        return bits;
    }
    
public:
    /**
     * size - number of IBF cells in each Invertible Bloom Filter
     * key_size - size of key in IBF
     * num - log(U) - max number of bits in a key (Needed to estimate d - estimate of difference
     */
    Strata_IBF(uint32_t size, uint32_t key_size, uint32_t num):num(num){
        ibfs = (IBF **) malloc(num*sizeof(IBF*));
        for(int i = 0; i < num; i++){
            ibfs[i] = new IBF(size,key_size);
        }
    }
    
    ~Strata_IBF(){
        for(int i = 0; i < num; i++)
            free(ibfs[i]);
        free(ibfs);
    }
    
    /**
     * Initializes the Strata Estimator with keys in 'v'
     */
    void encode(vector<int> &v){
        for(int num : v){
            uint32_t n = this->trailing_zeroes(num);
            ibfs[n]->add_key(&num);
        }
    }
    
    /**
     * Method to estimate the dfference between two sets both encoded in Strata Estimators
     */
    int estimate(Strata_IBF* sibf){
        int count = 0;
        vector<vector<int>> ans(2);
        for(int i = num - 1; i >= 0; i--){
            ans[0].clear(); ans[1].clear();
            IBF *diff = this->ibfs[i]->subtract(sibf->ibfs[i]);
            
            if(!diff->decode(ans)){
                return (1 << (i+1))*count;
            }
            count += (ans[0].size() + ans[1].size());
        }
        return count;
    }
    
};

int main(){
    
    vector<int> v1{1,2,3,4,5,6,7,8,9,10};
    vector<int> v2{112,88,67,43,89,75,7,52,32,59};

//    for(int i = 0; i < 10; i++){
//        v1.push_back(i);
//        v2.push_back(i+5);
//    }
//    cout << "A : ";
//    for(int i = 0; i < 10; i++) cout << v1[i] << " "; cout << endl;
//    cout << "B : ";
//    for(int i = 0; i < 10; i++) cout << v2[i] << " "; cout << endl;
    
    vector<vector<int>> ans(2);

    Strata_IBF* sibf1 = new Strata_IBF(80,4,7);
    Strata_IBF* sibf2 = new Strata_IBF(80,4,7);

    sibf1->encode(v1);
    sibf2->encode(v2);
    
    int c = sibf1->estimate(sibf2);

    cout << "difference estimate : " << c << endl;
    
    cout << "picking 2*estimate :" << endl;
    
    IBF *ibf1, *ibf2;
    
    ibf1 = new IBF(100,4);
    ibf2 = new IBF(100,4);
    ibf1->encode(v1);
    ibf2->encode(v2);
    
    IBF *ibfd = ibf1->subtract(ibf2);
    
    ibfd->decode(ans);
    cout << "A - B : ";
    for(int i = 0; i < ans[0].size(); i++){
        cout << ans[0][i] << " ";
    }
    cout << endl;
    
    cout << "B - A : ";
    for(int i = 0; i < ans[1].size(); i++){
        cout << ans[1][i] << " ";
    }
    cout << endl;
    
    return 0;
}
