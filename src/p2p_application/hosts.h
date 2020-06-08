#define BUFSIZE 100

//receives the message
string getMessage(int connection){
    // Vars
    char buffer[BUFSIZE+5];
    buffer[BUFSIZE] = 0;
    string recvd = "";
    int bytes_read;

    cerr << "--waiting to receive message--\n";

    bool printed = false;
    while ((bytes_read =  recv(connection, buffer, BUFSIZE, 0)) > 0){

        if (!printed) {
           printed = true;
           cerr << "--receiving message--";
        }

        if (bytes_read < 4){
            break;
        }

        if (bytes_read < BUFSIZE){
            buffer[bytes_read] = 0;
        }

        //cout << "current message is: " << buffer << endl;
        //cout << "string length: " << BUFSIZE << endl;

        recvd = recvd + string(buffer);

        //end of one message
        if (recvd.substr(recvd.size()-4, 4) == "\r\n\r\n" ){
            cerr << "--message received--\n";
            break;
        }
    }

    return recvd;
}


void split(string recvd, string delim,vector<string>& substrings){
    size_t curr_index, prev_index = 0;
    vector<string> ret;
    int step = delim.size();

    curr_index = recvd.find(delim);

    while (curr_index != string::npos) {
        substrings.push_back( recvd.substr(prev_index, curr_index - prev_index));
       
        prev_index = curr_index + step;
        curr_index = recvd.find(delim, prev_index);
    }
}

void send_set(int connection, unordered_set<int>& S  ){
                //send set
                string msg = "";
                int i = 0;
                int j = 1;
                for (int x : S){
                    msg = msg + to_string(x) + "\r\n";
                    i++;

                    //cerr << "inserted " << x << endl;
                    if ( i == 100000){
                        send(connection, msg.c_str(), msg.size(), 0);
                        i = 0;
                        msg = "";
                        cerr << "sent " << 100000*j << endl;
                        j++;
                    }
                }
                msg = msg + "\r\n";
                send(connection, msg.c_str() , msg.size(), 0 );
                cerr << "finished sending\n";

}

