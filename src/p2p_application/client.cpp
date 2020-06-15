#include <sys/socket.h> // For socket functions
#include <netinet/in.h> // For sockaddr_in
#include <cstdlib> // For exit() and EXIT_FAILURE
#include <iostream> // For cout
#include <unistd.h> // For read
#include <string>
#include <arpa/inet.h> 
#include <functional>

#include "IBFSetDiff.h"
#include "hosts.h"
//#define BUFSIZE 100
#define PORT 9998

int main (int argc, char *argv[]) {
    unordered_set<int> A;
    unordered_set<int> ans;

    float alpha = ALPHA;
    int beta = BETA;
    int num_hashes = NUM_HASHES;

    string filepath = "";
    string mode = "both";

    //parses parameter arguments 
    parseAndPopulate(argc, argv, alpha, beta, num_hashes, filepath, mode);

    if (filepath == ""){
        cerr << "please enter filepath\n";
        return -1;
    }

    cout << "alpha: " << alpha  << " beta: " << beta << " num_hashes: " << num_hashes << endl;

    //opens input file and hashes rows of data
    ifstream ifs;
    ifs.open(filepath, ios::binary);

    hash<string> hash_func;
    cout << "reading file hashes\n";
    string buf;
    int i = 0;
    while(getline(ifs, buf)){
        int temp = hash_func (buf);
        A.insert(temp);
        i++;
    }

    cout << "num hashes: " << A.size() << endl;
    cout << "num hashes read: " << i << endl;
    i = 0;

    //create a socket
    int client_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (client_sock == -1) {
      cout << "Socket creation failure\n";
      exit(EXIT_FAILURE);
    }

    //connect to port
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
 
    //convert addr to binary
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);

    //attempt connection
    if (connect(client_sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) 
    { 
        printf("Connection failure\n"); 
        exit(EXIT_FAILURE); 
    } 

    //char buffer[BUFSIZE];
    string recvd = "";
    string msg = "";
    int bytes_read;
    enum State {INITIAL, SEND_LOGU, ESTIMATE_DIFF_SIZE, CALCULATE_SET_DIFF, CALCULATE_ACTUAL_DIFF};
    State curr_state = INITIAL;

    vector <int> my_hashes;
    int estimated_diff_size = 0;

    vector<IBFCell*> ibf1;
    hashFuncType Hc = &hashFunctionC;
    int logu1 = *max_element(A.begin(), A.end());
    int logu = -1;
    Strata_IBF * strata1;

    //many messages back and forth with server
    while(1){
        //send hello message to server
        if (curr_state == INITIAL && recvd == ""){
            msg = "Host 1 Hello\r\n\r\n";
            send(client_sock , msg.c_str() , msg.size(), 0 );
            cout << "host 1 hello send \n";
        }
        //start set diff algorithms
        else if (curr_state == INITIAL && recvd == "Host 2 Hello\r\n\r\n"){
           if (mode == "naive"){
               send_set(client_sock, A);
               curr_state = CALCULATE_ACTUAL_DIFF;
           }
           else { //mode == IBF or both
               //send logu1
               msg = to_string(logu1) + "\r\n\r\n";
               send(client_sock , msg.c_str() , msg.size(), 0 );
               curr_state = SEND_LOGU;
               cerr << "logu sent\n";
           }
        }

        else if (curr_state == SEND_LOGU && recvd != "" ){
            //parse for logu2
            int logu2 = stoi(recvd);
   
            //calculate logu
            logu = (int) (log(max(logu1, logu2))/log(2)) + 1;

            //encode own strata estimator
            strata1 = new Strata_IBF (beta, num_hashes, logu);
            strata1->encode(A, Hc);
            cerr << "after encode strata1\n";

            //send strata1 
            msg = "";
            for (auto row : strata1->ibfs){
                for (IBFCell * cell : row){
                    msg = msg + to_string(cell->getIdSum()) + "," +  to_string(cell->getHashSum()) + "," + to_string(cell->getCount()) + ",\t";
                }
                msg = msg + "\r\n";
            }
            msg = msg + "\r\n";
            send(client_sock, msg.c_str() , msg.size(), 0 );

            curr_state = ESTIMATE_DIFF_SIZE;
        }

        else if (curr_state == ESTIMATE_DIFF_SIZE && recvd != "" ){
            //decode strata2
            Strata_IBF * strata2 = new Strata_IBF(beta, num_hashes, logu);

            vector<string> rows;
            split(recvd, "\r\n", rows);

            int i = 0, j = 0;
            for(string l : rows){
                if (l == ""){continue;}
                vector<string> cells;
                split(l, "\t", cells);
                for (string c : cells){
                    vector<string> fields;
                    split(c, ",", fields);
                    strata2->ibfs[i][j]->set(stoi(fields[0]), stoi(fields[1]), stoi(fields[2]));
                    j++;
                }
                i++;
                j=0;
            }

            //estimated diff size
            int d = strata1->estimateLength(strata2, Hc);
            cout<<"Estimated Set Diff Size: "<<d<<endl;
            estimated_diff_size = d;

            if (d == 0){//set default diff size if estimate is 0
                estimated_diff_size = 40;
                cerr << "ESTIMATED DIFF ERROR d=0, using d=40\n";
            }

            //encode and send ibf1
            d = estimated_diff_size;
            int N = d*alpha;
            int k = num_hashes;

            ibf1.resize(N);
            for (int i=0; i<N; i++) ibf1[i] = new IBFCell();
            encode(A, N, k, Hc, ibf1);

            //sending encoded ibf vector
            msg = "";
            for (IBFCell * cell : ibf1){
               msg = msg + to_string(cell->getIdSum()) + "\t" +  to_string(cell->getHashSum()) + "\t" + to_string(cell->getCount()) + "\t\r\n";
            }
            msg = msg + "\r\n";
            send(client_sock, msg.c_str() , msg.size(), 0 );
  
            curr_state = CALCULATE_SET_DIFF;
        }
       
        //perform IBF set diff 
        else if (curr_state == CALCULATE_SET_DIFF && recvd != ""){
            int d = estimated_diff_size;
            int N = d*alpha;
            int k = num_hashes;

            //parse received ibf2
            vector<IBFCell*> ibf2(N);
            for (int i=0; i<N; i++) ibf2[i] = new IBFCell();
            vector<string> cell_strings;
            split(recvd, "\r\n", cell_strings);

            int i = 0;
            for(string s: cell_strings){
                if (s == ""){continue;}
                vector<string> fields;
                split(s, "\t", fields);
                ibf2[i] = new IBFCell(stoi(fields[0]), stoi(fields[1]), stoi(fields[2]));
                i++;
            }
 
            //calculate set difference
            vector<IBFCell*> diff(N);
            subtract(ibf1, ibf2, diff, N);

            unordered_set<int> diff_A_B;
            unordered_set<int> diff_B_A;

            decode(diff, diff_A_B, diff_B_A, k, Hc, N);

            ans.insert(diff_A_B.begin(), diff_A_B.end());
            ans.insert(diff_B_A.begin(), diff_B_A.end());
            

            cout<<"---- CALCULATED SET DIFFERENCE ----\n";
            cerr << "\n---- diff_A_B ----\n";
            cerr << "size of diff_A_B: " << diff_A_B.size() << endl;
            for(int x : diff_A_B){
                cout<<x<<", ";
            }
            cout << endl;

            cerr << "\n---- diff_B_A ----\n";
            cerr << "size of diff_B_A: " << diff_B_A.size() << endl;
            for(int x : diff_B_A){
                cout<<x<<", ";
            }
            cout << endl << endl;

            if (mode == "IBF"){
                return 0;
            }

            curr_state = CALCULATE_ACTUAL_DIFF;
            send_set(client_sock, A);
        }

        //naive set difference algorithm
        else if (curr_state == CALCULATE_ACTUAL_DIFF && recvd != ""){
            cerr << "\n---- NAIVE SET DIFFERENCE ----\n";
            //parse set B
            unordered_set<int> B;
            unordered_set<int> diff;
            vector<string> elements;
            split(recvd, "\r\n", elements);
            cerr << "parsing set " << endl;
            for(string s: elements){
                if (s == ""){continue;}
                B.insert(stoi(s));
            }
            cerr << "finished parsing\n" << "set size: " << B.size() << endl;

            unordered_set<int> diff_A_B;
            unordered_set<int> diff_B_A;

            for(int x: A) if(!B.count(x)){diff_A_B.insert(x);}
            for(int x: B) if(!A.count(x)){diff_B_A.insert(x);}

            cerr << "\n---- diff_A_B ----\n";
            cerr << "size of diff_A_B: " << diff_A_B.size() << endl;
            for(int x : diff_A_B){
                cout<<x<<", ";
            }
            cout<< endl;

            cerr << "\n---- diff_B_A ----\n";
            cerr << "size of diff_B_A: " << diff_B_A.size() << endl;
            for(int x : diff_B_A){
                cout<<x<<", ";
            }
                
            cout<<endl;

            unordered_set<int> actual_ans;
            actual_ans.insert(diff_A_B.begin(), diff_A_B.end());
            actual_ans.insert(diff_B_A.begin(), diff_B_A.end());

            if (mode == "both"){
                cerr << "\nboth methods are equal: " << ((actual_ans == ans) ? "true" : "false") << endl;
            }

            return 0;
        }

        
        recvd = getMessage(client_sock);
        if (recvd.size() == 0){
            break;
        }
     }
}

