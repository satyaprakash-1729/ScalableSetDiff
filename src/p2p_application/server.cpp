#include <sys/socket.h> 
#include <netinet/in.h> 
#include <cstdlib> 
#include <iostream> 
#include <unistd.h>
#include <fstream>
#include <unordered_map>
#include <functional>

#include "IBFSetDiff.h"
#include "hosts.h"
//#define BUFSIZE 100
#define FILECHUNK 5
#define PORT 9998

vector<IBFCell*> ibf1;
vector<IBFCell*> ibf2;

//receives the message
/*string getMessage(int connection){
    // Vars
    char buffer[BUFSIZE];
    string recvd = "";     
    int bytes_read;

    while ((bytes_read =  recv(connection, buffer, BUFSIZE, 0)) > 0){       
        cout << "bytes read: " << bytes_read << endl;

        if (bytes_read < 4){
            break;
        }

        if (bytes_read < BUFSIZE){
            buffer[bytes_read] = 0;
        }

        recvd = recvd + string(buffer);

        //end of one message
        if (recvd.substr(recvd.size()-4, 4) == "\r\n\r\n" ){
            break;
        } 
    }

    return recvd;
}*/

int main (int argc, char *argv[]) {
    int sockopt = 1;

    //unordered_set<int> B = {52,7,89,546,71,67,88,165,40,12,59,32,93,32};
    //unordered_set<int> B = {177,276,275,185,231,216,212,141,228,151,153};
    //unordered_set<int> B;
    unordered_set<int> B = getRandomSet(11, 100, 300);

   /* ifstream ifs;
    ifs.open("test_data/host2.data", ios::binary);

    hash<string> hash_func;

    char buf [FILECHUNK];
    int i = 0;
    while(ifs.read(buf, sizeof(buf)) || ifs.gcount()){
        string chunk(buf, ifs.gcount());
        int temp = hash_func (chunk); 
        temp = temp < 0 ? temp*-1 : temp;
        B.insert(temp);        
    }

    cout << "file hashes\n";
    for (int j : B) {cout << j << endl;}
    cout << "num hashes: " << B.size() << endl;
    */
 
    /*vector<hashFuncType> hashes;
    hashes.push_back(hashFunction1);
    hashes.push_back(hashFunction2);
    hashes.push_back(hashFunction3);
    */

    //create a socket
    int server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock == -1) {
      cout << "Socket creation failure\n";
      exit(EXIT_FAILURE);
    }

    //set sock option to resuse addr/port
    if (setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &sockopt, sizeof(sockopt))) 
    { 
        perror("Setsockopt failure\n"); 
        exit(EXIT_FAILURE); 
    } 
    //listen on any addr on port 9999
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);

    //binding sock to port
    if (bind(server_sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
      cout << "Port binding failure\n";
      exit(EXIT_FAILURE);
    }

    //listen
    if (listen(server_sock, 2) < 0) {
      cout << "Listening failure \n";
      exit(EXIT_FAILURE);
    }



    //this while loop here accepts many conections
    while (1){
        //accept connection
        auto addrlen = sizeof(addr);
        int connection = accept(server_sock, (struct sockaddr*)&addr, (socklen_t*)&addrlen);
        if (connection < 0) {
          cout << "Accept failure \n";
          exit(EXIT_FAILURE);
        }

        // Vars
        string recvd = "";     
        string msg = "";
        enum State { INITIAL,SEND_LOGU, ESTIMATE_DIFF_SIZE, CALCULATE_SET_DIFF, CALCULATE_ACTUAL_DIFF}; 

        State curr_state = INITIAL;
        int estimated_diff_size = 0;
        
        int logu2 = *max_element(B.begin(), B.end());
        hashFuncType Hc = &hashFunctionC;

        int logu = -1;
        
        Strata_IBF * strata2;


        //loop here to exchange messages
        while(1){
            recvd = getMessage(connection);

            //client ended connection
            if (recvd.size() == 0){
                break;
            }

            //cout << "The message was: " << recvd << endl;
            //cout << "The message length was: " << recvd.size() << endl;

            if (curr_state == INITIAL && recvd == "Host 1 Hello\r\n\r\n" ){
                msg = "Host 2 Hello\r\n\r\n";
                curr_state = SEND_LOGU;
                send(connection, msg.c_str(), msg.size(), 0);
                cerr << "logu sent\n";
            }
            
            else if (curr_state == SEND_LOGU && recvd != ""){
                //parse message for logu1
                //size_t end = recvd.find("\r\n\r\n"); 
                //int logu1 = stoi(recvd.substr(0, end));
                int logu1 = stoi(recvd);

                //send logu2
                msg = to_string(logu2) + "\r\n\r\n";
                send(connection, msg.c_str(), msg.size(), 0);
 
                //calculate logu
                logu = (int)(log(max(logu1, logu2))/log(2)) + 1;
                
                //estimate diff diff size
                cerr <<"before strata2 encoded\n";
                strata2 = new Strata_IBF(BETA, NUM_HASHES, logu);
                strata2->encode(B, Hc);
                cerr << "strata2 encoded\n";

                curr_state = ESTIMATE_DIFF_SIZE;
            }
            else if (curr_state == ESTIMATE_DIFF_SIZE && recvd != ""){
                //decode strata1
                Strata_IBF * strata1 = new Strata_IBF(BETA, NUM_HASHES, logu);

                vector<string> rows;
                split(recvd, "\r\n", rows);

                int i = 0, j = 0;
                for(string l : rows){
                    if (l == ""){continue;}
                    vector<string> cells;
                    split(l, "\t", cells);
                    for (string c : cells){
                        //cerr << "cell: " << c << endl;
                        //cerr << "strata1: " << (intptr_t) strata1 << endl;
                        //cerr << "strata1->ibfs: " << (intptr_t) strata1->ibfs << endl;
                        //cerr << "i: " << i << " j: " << j << endl;
                        //cerr << "strata1->ibfs[i][j] : " <<  strata1->ibfs[i][j] << endl;
        
                        vector<string> fields;
                        split(c, ",", fields);
                        //cerr << fields.size() << endl;
                        strata1->ibfs[i][j]->set(stoi(fields[0]),  stoi(fields[1]), stoi(fields[2])); 
                        j++;
                    }
                    i++;
                    j = 0;
                }

                int d = strata1->estimateLength(strata2, Hc);
                cout<<"Estimated Set Diff Size: "<<d<<endl;
                estimated_diff_size = d;
                estimated_diff_size = 35;

                /*cerr << "reconstructed strata1 \n";
                for (auto row : strata1->ibfs){
                   for (auto column : row){
                       cerr << column->getIdSum() << "," << column->getHashSum() << "," << column->getCount() << "\t";
                   }
                   cerr << endl;
                }*/

                //sending encoded strata2
                msg = "";
                for (auto row : strata2->ibfs){
                    for (IBFCell * cell : row){
                        msg = msg + to_string(cell->getIdSum()) + "," +  to_string(cell->getHashSum()) + "," + to_string(cell->getCount()) + ",\t";
                    }
                    msg = msg + "\r\n";
                }
                msg = msg + "\r\n";
                //cerr << "encoded strata2 vector sent\n" << msg << endl;
                send(connection, msg.c_str() , msg.size(), 0 );
                curr_state = CALCULATE_SET_DIFF;
            }

            /*else if (curr_state == ESTIMATE_DIFF_SIZE && recvd != ""){
                //parse message for set size and min hashes
                size_t curr_index, prev_index = 0;
                vector <int> their_hashes;
                int their_size = 0;

                //parse size
                curr_index = recvd.find("\r\n"); 
                their_size = stoi(recvd.substr(prev_index, curr_index - prev_index));
                prev_index = curr_index + 2;
                curr_index = recvd.find("\r\n", prev_index);

                //parse min hashes
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

                //sending own sizes and min hashes
                msg = "";
                vector <int> my_hashes;
                int hash = 0;
 
                //send own size 
                cout << "B's size is: " << B.size() << endl;
                msg = msg + to_string(B.size()) + "\r\n";

                //send own min hashes
                for(hashFuncType& hashFn: hashes){
                    hash = getMinHash(B, hashFn);
                    msg = msg + to_string(hash) + "\r\n";
                    my_hashes.push_back(hash);
                }

                msg = msg + "\r\n";
                send(connection, msg.c_str() , msg.size(), 0 );  

                cout << "their size: " << their_size << endl;
                for (int i : their_hashes){cout << "hash: " << i << endl;}

 
                //calculate diff size
                int m = 0;
                for(int i = 0; i < my_hashes.size(); i++){
                    if(my_hashes[i] == their_hashes[i]) m++;
                }

                float k = hashes.size();
                float r = float(m)/k;
                estimated_diff_size = ((1.-r)/(1.+r))*(float(their_size+B.size()));
                cout << "diff: " << estimated_diff_size << endl;
 
                curr_state = CALCULATE_SET_DIFF;
            }*/

            else if (curr_state == CALCULATE_SET_DIFF && recvd != ""){
                float alpha = ALPHA;
                int d = estimated_diff_size;
                int N = d*alpha;
                int k = NUM_HASHES;

                //vector<IBFCell*> ibf1(N);
                ibf1.resize(N);
                for (int i=0; i<N; i++) ibf1[i] = new IBFCell();

                //vector<IBFCell*> ibf2(N);
                ibf2.resize(N);
                for (int i=0; i<N; i++) ibf2[i] = new IBFCell();

                //hashFuncType Hc = &hashFunction4;
 
                vector<string> cell_strings;
                split(recvd, "\r\n", cell_strings);

                int i = 0;
                for(string s: cell_strings){
                    if (s == ""){continue;}
                    vector<string> fields;
                    split(s, "\t", fields);
                    ibf1[i] = new IBFCell(stoi(fields[0]), stoi(fields[1]), stoi(fields[2])); 
                    i++;
                }

                //cerr << "reconstructed ibf1\n";
                //for (IBFCell * cell : ibf1){
                //   cout<< to_string(cell->getIdSum()) + "\t" +  to_string(cell->getHashSum()) + "\t" + to_string(cell->getCount()) + "\t\r\n";
                //}   
                
                cerr << "before encode\n";
                //cerr << "ibf2: " << ibf2 << endl;
                /*cerr << "N is: " << N << endl;
                for (IBFCell * cell : ibf2){
                    cerr << cell << endl;
                }*/
                encode(B, N, k, Hc, ibf2);
                cerr << "after encode\n";

                //sending encoded ibf vector
                msg = "";
                for (IBFCell * cell : ibf2){
                   msg = msg + to_string(cell->getIdSum()) + "\t" +  to_string(cell->getHashSum()) + "\t" + to_string(cell->getCount()) + "\t\r\n";
                }               
                msg = msg + "\r\n";
                //cerr << "encoded ibf vector sent" << msg << endl;
                send(connection, msg.c_str() , msg.size(), 0 );


                //calculate set difference
                vector<IBFCell*> diff(N);
                subtract(ibf1, ibf2, diff, N);

                unordered_set<int> diff_A_B;
                unordered_set<int> diff_B_A;

                bool stat = decode(diff, diff_A_B, diff_B_A, k, Hc, N);
                if(!stat){
                    cout<<"Error finding difference... All differences might not be recorded.\n";
                }

                unordered_set<int> ans;
                ans.insert(diff_A_B.begin(), diff_A_B.end());
                ans.insert(diff_B_A.begin(), diff_B_A.end());

                cerr<<"set diff size: " << ans.size() << endl;
                cerr<<"---- SET DIFFERENCE ----\n";
                for(int x : ans){
                    cout<<x<<endl;
                }
                cerr << "---- diff_A_B ----\n";
                cerr << "size of diff_A_B" << diff_A_B.size() << endl;
                for(int x : diff_A_B){
                    cout<<x<<", ";
                }
                cout << endl;

                cerr << "---- diff_B_A ----\n";
                cerr << "size of diff_B_A" << diff_B_A.size() << endl;
                for(int x : diff_B_A){
                    cout<<x<<", ";
                }
                cout << endl;

                curr_state = CALCULATE_ACTUAL_DIFF; 
            }

            else if (curr_state == CALCULATE_ACTUAL_DIFF && recvd != ""){
                cerr << "\n\n---- ACTUAL DIFFERENCE ----" << endl;
                //parse set A
                unordered_set<int> A;
                unordered_set<int> diff;
                vector<string> elements;
                split(recvd, "\r\n", elements);

                for(string s: elements){
                    if (s == ""){continue;}
                    A.insert(stoi(s));
                }
 
                //send set B
                msg = "";
                for (int x : B){
                    msg = msg + to_string(x) + "\r\n";
                }
                msg = msg + "\r\n";
                //cerr << "encoded strata2 vector sent\n" << msg << endl;
                send(connection, msg.c_str() , msg.size(), 0 );
 
                unordered_set<int> diff_A_B;
                unordered_set<int> diff_B_A;
                
                for(int x: A) if(!B.count(x)){diff_A_B.insert(x);}
                for(int x: B) if(!A.count(x)){diff_B_A.insert(x);} 

                cerr << "---- diff_A_B ----\n";
                cerr << "size of diff_A_B" << diff_A_B.size() << endl;
                for(int x : diff_A_B){
                    cout<<x<<", ";
                }
                cout << endl;

                cerr << "---- diff_B_A ----\n";
                cerr << "size of diff_B_A" << diff_B_A.size() << endl;
                for(int x : diff_B_A){
                    cout<<x<<", ";
                }
                cout << endl;


            }

        }

        // Send a message to the connection
        string response = "Good talking to you\r\n\r\n";
        send(connection, response.c_str(), response.size(), 0);

        // Close the connections
        close(connection);
    }

    close(server_sock);

    return 0;

}
