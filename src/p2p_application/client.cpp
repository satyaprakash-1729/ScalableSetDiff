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
    //unordered_set<int> A = {40,12,59,32,74,32,52,7,89,43,75,67,88,112};
    //unordered_set<int> A = {159, 100, 281, 171, 270, 137, 183, 119, 163, 260, 283};
    //unordered_set<int> A = getRandomSet(11, 100, 300);
    unordered_set<int> A;

    if (argc < 3){
        cerr << "please enter all arguments -- ./CLIENT /path/to/filename mode\n";
        return -1;
    }

    string mode (argv[2]);    
    ifstream ifs;
    //ifs.open("test_files/host1.data", ios::binary);
    //ifs.open("test_files/dataset1.dat", ios::binary);
    ifs.open(argv[1], ios::binary);

    hash<string> hash_func;
    cout << "reading file hashes\n";
    string buf;
    int i = 0;
    while(getline(ifs, buf)){
        int temp = hash_func (buf);
        A.insert(temp);
    }

    //for (int j : A) {cout << j << endl;}
    cout << "num hashes: " << A.size() << endl;


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

    char buffer[BUFSIZE];
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

    //many messages ack and forth with server
    while(1){
        //server ended connection
        if (curr_state == INITIAL && recvd == ""){
            msg = "Host 1 Hello\r\n\r\n";
            send(client_sock , msg.c_str() , msg.size(), 0 );
            cout << "host 1 hello send \n";
        }
        
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
            //size_t end = recvd.find("\r\n\r\n");
            //cerr << "before logu2 received\n"; 
            int logu2 = stoi(recvd);
            //cerr << "after logu2 received\n";
   
            //calculate logu
            logu = (int) (log(max(logu1, logu2))/log(2)) + 1;

            //encode own vector
            cerr << "before encode strata1\n";
            strata1 = new Strata_IBF (BETA, NUM_HASHES, logu);
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
            //cerr << "encoded strata1 sent\n" << msg << endl;
            //cerr << "msg length: " << msg.size() << endl;
            send(client_sock, msg.c_str() , msg.size(), 0 );

            curr_state = ESTIMATE_DIFF_SIZE;
        }

        else if (curr_state == ESTIMATE_DIFF_SIZE && recvd != "" ){
            //decode strata2
            Strata_IBF * strata2 = new Strata_IBF(BETA, NUM_HASHES, logu);

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

            /*cerr << "reconstructed strata2 \n";
            for (auto row : strata2->ibfs){
               for (auto column : row){
                   cerr << column->getIdSum() << "," << column->getHashSum() << "," << column->getCount() << "\t";
               }
               cerr << endl;
            }*/

            //estimated diff size
            int d = strata1->estimateLength(strata2, Hc);
            cout<<"Estimated Set Diff Size: "<<d<<endl;
            estimated_diff_size = d;
            estimated_diff_size = 100;

            //encode and send ibf1
            float alpha = ALPHA;
            d = estimated_diff_size;
            int N = d*alpha;
            int k = NUM_HASHES;

            ibf1.resize(N);
            for (int i=0; i<N; i++) ibf1[i] = new IBFCell();
            encode(A, N, k, Hc, ibf1);

            //sending encoded ibf vector
            msg = "";
            for (IBFCell * cell : ibf1){
               msg = msg + to_string(cell->getIdSum()) + "\t" +  to_string(cell->getHashSum()) + "\t" + to_string(cell->getCount()) + "\t\r\n";
            }
            msg = msg + "\r\n";
            //cerr << "encoded ibf vector sent" << msg << endl;
            //cerr << "msg length: " << msg.size() << endl;
            send(client_sock, msg.c_str() , msg.size(), 0 );
  
            curr_state = CALCULATE_SET_DIFF;
        }
        
        /*else if (curr_state == INITIAL && recvd == "Host 2 Hello\r\n\r\n"){
            curr_state = ESTIMATE_DIFF_SIZE;

            //send min hashes
            msg = "";
            int hash = 0;

            cout << "A's size is: " << A.size() << endl;

            msg = msg + to_string(A.size()) + "\r\n";

            for(hashFuncType& hashFn: hashes){
                hash = getMinHash(A, hashFn);
                msg = msg + to_string(hash) + "\r\n";
                my_hashes.push_back(hash);
            }

            msg = msg + "\r\n";
            send(client_sock , msg.c_str() , msg.size(), 0 );            
        }*/

        /*else if (curr_state == ESTIMATE_DIFF_SIZE && recvd != "" ){

            //parse message for min hashes
            size_t curr_index, prev_index = 0;
            vector <int> their_hashes;
            int their_size = 0;

            //parse their size
            curr_index = recvd.find("\r\n");
            their_size = stoi(recvd.substr(prev_index, curr_index - prev_index));
            prev_index = curr_index + 2;
            curr_index = recvd.find("\r\n", prev_index);

            while (curr_index != string::npos) {
                string substring = recvd.substr(prev_index, curr_index - prev_index);
                cerr << "substr: " << substring << endl;
                if (substring.size() == 0){
                  break;
                }
                their_hashes.push_back(stoi(substring));

                prev_index = curr_index + 2;
                curr_index = recvd.find("\r\n", prev_index);
            }
            
            cout << "their size: " << their_size << endl; 
            for (int i : their_hashes){cout << "hash: " << i << endl;}

            //calculate size diff
            int m = 0;
            for(int i = 0; i < my_hashes.size(); i++){
                if(my_hashes[i] == their_hashes[i]) m++;
            }

            float k = hashes.size();
            float r = float(m)/k;
            estimated_diff_size = ((1.-r)/(1.+r))*(float(their_size+A.size()));
            cout << "diff: " << estimated_diff_size << endl;

            curr_state = CALCULATE_SET_DIFF;

            //encode own vector
            float alpha = 1.4;
            int d = estimated_diff_size;
            int N = d*alpha;

            ibf1.resize(N);
            for (int i=0; i<N; i++) ibf1[i] = new IBFCell();
            encode(A, N, hashes, Hc, ibf1);

            //sending encoded ibf vector
            msg = "";
            for (IBFCell * cell : ibf1){
               msg = msg + to_string(cell->getIdSum()) + "\t" +  to_string(cell->getHashSum()) + "\t" + to_string(cell->getCount()) + "\t\r\n";
            }
            msg = msg + "\r\n";
            cerr << "encoded ibf vector sent" << msg << endl;
            cerr << "msg length: " << msg.size() << endl;
            send(client_sock, msg.c_str() , msg.size(), 0 );
        }*/

        else if (curr_state == CALCULATE_SET_DIFF && recvd != ""){
            float alpha = ALPHA;
            int d = estimated_diff_size;
            int N = d*alpha;
            int k = NUM_HASHES;

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

            /*cout << "reconstructed ibf2\n";
            for (IBFCell * cell : ibf2){
                cout<< to_string(cell->getIdSum()) + "\t" +  to_string(cell->getHashSum()) + "\t" + to_string(cell->getCount()) + "\t\r\n";
            }*/

            //calculate set diff           
            //calculate set difference
            vector<IBFCell*> diff(N);
            subtract(ibf1, ibf2, diff, N);

            unordered_set<int> diff_A_B;
            unordered_set<int> diff_B_A;

            decode(diff, diff_A_B, diff_B_A, k, Hc, N);

            /*unordered_set<int> ans;
            ans.insert(diff_A_B.begin(), diff_A_B.end());
            ans.insert(diff_B_A.begin(), diff_B_A.end());
            */

            cout<<"---- CALCULATED SET DIFFERENCE ----\n";
            /*for(int x : ans){
                cout<<x<<endl;
            }*/
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
                //send actual set A
                /*msg = "";
                i = 0;
                int j = 1;
                for (int x : A){
                    msg = msg + to_string(x) + "\r\n";
                    i++;

                    //cerr << "inserted " << x << endl;
                    if ( i == 100000){
                        send(client_sock, msg.c_str(), msg.size(), 0);
                        i = 0;
                        msg = "";
                        cerr << "sent " << 100000*j << endl;
                        j++;
                    }
                }
                msg = msg + "\r\n";
                send(client_sock, msg.c_str() , msg.size(), 0 );
                cerr << "finished sending\n";*/
        }
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

                return 0;
        }

        
        recvd = getMessage(client_sock);
        //cout << "Message recevied: " << recvd << endl;
        if (recvd.size() == 0){
            break;
        }
     }

    close(client_sock);
    return 0;

}
