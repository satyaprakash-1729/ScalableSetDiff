#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string>
using namespace std;

int main(int argc, char * argv[0]){

    ifstream ifs;
    ofstream ofs1;
    ofstream ofs2;

    int mod = 2;
    int N = 0;
    int max = 10;

    if (argc < 7){
        cerr << "need to include input file path, total rows of data in file, desired average number of rows to exclude, max number of rows to exclude, and two output file paths \n";
        return -1;
    }

    ifs.open(argv[1]);
    N = stoi(argv[2]);
    mod = N/stoi(argv[3]);
    max = stoi(argv[4]);
    ofs1.open(argv[5]);
    ofs2.open(argv[6]);

    srand (time(NULL));

    string buf;

    int diff1 = 0;
    int diff2 = 0;

    bool node1 = false;
    bool node2 = false;

    while(getline(ifs, buf))
    {
        if (rand()%mod || max <= diff1){
           ofs1 << "\t" << buf << endl;
        } else {
           diff1++;
        }
  
        if (rand()%mod || max <= diff2){
           ofs2 << "\t" << buf << endl;
        } else {
           diff2++;
        }

    }

}
