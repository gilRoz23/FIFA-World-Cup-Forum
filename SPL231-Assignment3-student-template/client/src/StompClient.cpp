#include <iostream>
#include <ConnectionHandler.h>
#include <StompClientProtocol.h>
#include <Frame.h>
#include <vector>
#include <string>
#include <thread>

using namespace std;

vector<string> split(const string& str, char delimiter) {
	vector<string> lines;
	std::stringstream ss(str);
	string line;
	while (std::getline(ss, line, delimiter)) {
		lines.push_back(line);
	}
	return lines;
}



int main(int argc, char *argv[]) {
    // TODO: implement the STOMP client


    // im in a loop while(1):
    // waiting for a message from keyboard
    // verify that the first string is login
    // if not - return 1
    while (1){
        const short bufsize = 1024;
        char buf[bufsize];
        std::cin.getline(buf, bufsize);
        std::string line(buf);
		
		vector<string> lines = split(line, ' ');
        if (lines.at(0) != "login"){
            return 1;
        }
        else{
        // else:
        // split according to ' ' check if the string are valid
        // try to create Socket with host:port (string[1] - לפצל)
        // if couldnt connect- return 1 (like in echo)
    
            // get the host and port strings
            vector<string> hostAndPort = split(lines.at(1),':');
            string host = hostAndPort.at(0);
            string rawPort = hostAndPort.at(1);
            short port = (short)std::stoi(rawPort);
            
            //try to make a connection handler
            ConnectionHandler connectionHandler(host, port);
            if (!connectionHandler.connect()) {
                std::cerr << "Cannot connect to " << host << ":" << port << std::endl;
                return 1;
            }
          
            //sending login frame:
            string command = "CONNECT";
            unordered_map<string, string> headers;
            string username = lines.at(2);
            string passcode = lines.at(3);

            headers.emplace("accept-version", "1.2");
            headers.emplace("host", "stomp.cs.bgu.ac.il");
            headers.emplace("login", username);
            headers.emplace("passcode", passcode);
            Frame connectFrame =  Frame(command, headers, "");

            string frameAsString = connectionHandler.stompClientProtocol.frameToString(connectFrame); // adds \0
            connectionHandler.sendFrameAscii(frameAsString, '\0'); // sends until the \0
            string answer; // create empty string to be filled in getframeascii
            connectionHandler.getFrameAscii(answer , '\0');
            
            // cout << "PRINT THE ANSWER FROM THE SERVER - " << answer << endl;

            Frame responseFrame = connectionHandler.stompClientProtocol.stringToFrameFromServer(answer);
            if (responseFrame.getCommand() != "CONNECTED"){ //if its not CONNECTED its probably ERROR
                cout << (responseFrame.getBody()) << endl;
                cout << "Disconnecting - good bye" << endl;
                return (1);
            }
            else{ // connected succesfully
                cout << "login succeful" << endl;
                connectionHandler.setUserName(username);
            }
            thread keyBoardListenerThread (&ConnectionHandler::runKeyBoardListener, &connectionHandler);
            thread serverListenerThread (&ConnectionHandler::runServerListener, &connectionHandler);
            keyBoardListenerThread.join();
            serverListenerThread.join();
            // cout <<"BOTH THREADS ARE DEAD" << endl;


            
            
            // else ( sockets connected - i can start talking with the server)
            // convert the string of the keyboard to a frame (the process) (is in the protocol - send process)
            // encode the frame to string and send it to sendLine (or something like that..)
            // waiting on readsome(blocked) to receive bytes and decode them to a frame
            

    

        }
        

	}
    return 0;
}

// ./bin/StompWCIClient 127.0.0.1 7777
// login 127.0.0.1:7777 benzi benzi
// join Germany_Japan
// report data/events1.json
// summary Germany_Japan benzi data/benzi.txt

// why i do summary, i get the same stats for different teams. the general stats are ok, but possesion and goals are not.


