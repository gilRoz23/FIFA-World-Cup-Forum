#pragma once

// #include <ConnectionHandler.h>
#include <vector>
#include <list>
#include <string>
#include <thread>
#include <queue>
#include <fstream>
#include <unordered_map>
#include <map>
using namespace std;

class Frame;
class Game;

// TODO: implement the STOMP protocol
class StompClientProtocol
{
private:
    int nextSubscriptionId;
    int nextRecieptNumber;
    unordered_map<string, string> gameNameToSubscriptionId;
    unordered_map<string, string> receiptIdToPrintMessage;
    string username;
    // vector<string> framesAsStringsQ;
    map<pair<string, string>, vector<Frame>> gameNameUserNameToFramesList;
    bool serverShouldTerminate;

public:
    StompClientProtocol();
    vector<string> processFromKB(string &kbLine);
    Frame stringToFrameFromKB(string &commandFromKB, vector<string> &splittedString);
    Frame stringToFrameFromServer(string &stringFromServer);
    string frameToString(Frame &frame);
    void runServerListener();
    void runKeyBoardListener();
    void processFromServer(Frame &frameFromServer);
    string getNextSubscriptionId();
    string getNextRecieptNumber();
    Frame generateWrongCommandFrame();
    vector<string> split(const string &str, string delimiter);
    void setUserName(string user);
    vector<string> reportProcess(string json_path);
    void summaryProcess(string gameName, string summaryUserName, string json_path);
    bool getServerShouldTerminate();
};
