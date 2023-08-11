#include <StompClientProtocol.h>
// #include <ConnectionHandler.h>
#include <Frame.h>
#include <event.h>
#include <atomic>
#include <iostream>
#include <queue>
#include <sstream>
using std::atomic_int;
using std::vector;


StompClientProtocol::StompClientProtocol() : nextSubscriptionId(0),
                                                                                     gameNameToSubscriptionId(), nextRecieptNumber(0), receiptIdToPrintMessage(), username(""),
                                                                                     gameNameUserNameToFramesList(), serverShouldTerminate(false){};

// returns a string in shape of the frame that is suitble for the command.
vector<string> StompClientProtocol::processFromKB(string &kbLine)
{ // check how to do a refrence
    // recognize the command and checks validity and sends to stringToFrame.

    // cout << "ENTERED PROCESS_FROM_KB with -" << kbLine << endl;
    // empty the queue
    // queue<string> emptyQueue;
    // framesAsStringsQ.swap(emptyQueue);
    // framesAsStringsQ.clear();
    vector<string> framesAsStringsQ;

    vector<string> linespace = split(kbLine, " ");
    string command = linespace.at(0); // recignoze the command

    // cout << "the command is: " << command << endl;


    // check validity of the input
    if (((command.compare("login") == 0) & (linespace.size() == 4) )||
        ((command.compare("logout") == 0) & (linespace.size() == 1)) ||
        ((command.compare("join") == 0) & (linespace.size() == 2)) ||
        ((command.compare("exit") == 0) & (linespace.size() == 2)))
    {

        // make a frame and return it to string.
        Frame frame = stringToFrameFromKB(command, linespace);
        string frameAsString = frameToString(frame);

        // framesAsStringsQ.push(frameAsString);
        framesAsStringsQ.push_back(frameAsString);
         // this is how we send to the run method the frame as string.
        // cout << "PUSHED THE FRAME TO THE -TO SEND- FRAMES Q" << endl;
        return framesAsStringsQ;
    }
    else if (((command.compare("report") == 0) & (linespace.size() == 2)))
    {
        return reportProcess(linespace.at(1)); // sending to reportProcess which adds to the q the ready frames as strings.
    }
    else if (((command.compare("summary") == 0) & (linespace.size() == 4)))
    {
        summaryProcess(linespace.at(1), linespace.at(2), linespace.at(3)); // gamename, username, file
        framesAsStringsQ.clear();
        return framesAsStringsQ;
    }
    else
    {
        Frame wrongFrame = generateWrongCommandFrame();
        string wrongFrameAsString = frameToString(wrongFrame);
        framesAsStringsQ.push_back(wrongFrameAsString);
        return framesAsStringsQ;
    }
}

Frame StompClientProtocol::stringToFrameFromKB(string &commandFromKB, vector<string> &splittedString)
{
    // gets the string, and the command and according to the command builds the frame
    // the string is a valid frame (was chacked at process)
    string command;
    unordered_map<string, string> headers;
    string body;

    if (commandFromKB.compare("login") == 0)
    {
        command = "CONNECT";
        string hostAndPort = splittedString.at(1);
        vector<string> hostAndPortVec = split(hostAndPort, ":");
        string host = hostAndPortVec.at(0);
        string port = hostAndPortVec.at(1);
        string username = splittedString.at(2);
        string passcode = splittedString.at(3);
        headers.emplace("accept-version", "1.2");
        headers.emplace("host", "stomp.cs.bgu.ac.il");
        headers.emplace("login", username);
        headers.emplace("passcode", passcode);
        return Frame(command, headers, "");
    }
    else if (commandFromKB.compare("join") == 0)
    {
        command = "SUBSCRIBE";
        string destination = splittedString.at(1);
        string subscriptionId = getNextSubscriptionId();
        gameNameToSubscriptionId.emplace(destination, subscriptionId); // add the <game name, subscription_id of client>
        string receipt_id = getNextRecieptNumber();
        string succesMessage = "Joined channel " + destination;
        receiptIdToPrintMessage.emplace(receipt_id, succesMessage);
        headers.emplace("destination", destination);
        headers.emplace("id", subscriptionId);
        headers.emplace("receipt-id", receipt_id);

        return Frame(command, headers, "");
    }

    else if (commandFromKB.compare("exit") == 0)
    {
        command = "UNSUBSCRIBE";
        string gamename = splittedString.at(1);
        string subsciption_id = gameNameToSubscriptionId[gamename];
        string receipt_id = getNextRecieptNumber();
        string succesMessage = "Exited channel" + gamename;
        receiptIdToPrintMessage.emplace(receipt_id, succesMessage);
        if (subsciption_id == ""){
            subsciption_id = "-1";
        }

        headers.emplace("id", subsciption_id);
        headers.emplace("receipt-id", receipt_id);

        return Frame(command, headers, "");
    }


    else if (commandFromKB.compare("logout") == 0)
    {

        command = "DISCONNECT";
        string receipt_id = getNextRecieptNumber();
        string succesMessage = "Logout succesfuly";
        receiptIdToPrintMessage.emplace(receipt_id, succesMessage);
        headers.emplace("receipt-id", receipt_id + "");

        return Frame(command, headers, "");
    }
    return generateWrongCommandFrame();
}

vector<string> StompClientProtocol::reportProcess(string json_path)
{
    // cout << "ENTERED REPORT PROCESS FOR PATH " << json_path << endl;

    vector<string> framesAsStringsQ;

    names_and_events namesAndEvents = parseEventsFile(json_path); // read the file
    unordered_map<string, string> headers;
    headers.emplace("destination", namesAndEvents.team_a_name + "_" + namesAndEvents.team_b_name);
    string body;

    for (Event event : namesAndEvents.events)
    {
        // cout << "PARSING NEW EVENT" << endl;
        body = ""; // clean up body for each use.

        body += "user:" + username + "\n";
        body += "team a:" + event.get_team_a_name() + "\n";
        body += "team b:" + event.get_team_b_name() + "\n";
        body += "event name:" + event.get_name() + "\n";
        body += "time:" + to_string(event.get_time()) + "\n";
        body += "general game updates:\n";
        for (auto keyValue : event.get_game_updates())
        {
            body += keyValue.first + ":" + keyValue.second + "\n";
        }
        body += "team a updates:\n";
        for (auto keyValue : event.get_team_a_updates())
        {
            body += keyValue.first + ":" + keyValue.second + "\n";
        }
        body += "team b updates:\n";
        for (auto keyValue : event.get_team_b_updates())
        {
            body += keyValue.first + ":" + keyValue.second + "\n";
        }
        body += "description:\n" + event.get_discription() + "\n";

        Frame finishedFrame = Frame("SEND", headers, body);
        string frameAsString = frameToString(finishedFrame);
        // cout << "THE READY FRAME IS:\n" << frameAsString << endl;

        // cout << "THE BODY OF THE FRAME IS:\n" << body << endl;

        // push to the queue so run could get it
        framesAsStringsQ.push_back(frameAsString);

        // cout << "framesAsStringsQ size now is: " << framesAsStringsQ.size() << endl;
        
    }

    // cout << "ABOUT TO SHIP ALL THE FRAMES" << endl;
    return framesAsStringsQ;
}

void StompClientProtocol::summaryProcess(string gameName, string summaryUserName, string file)
{
    // cout << "IN SUMMARY: game name is: " << gameName << ", SUMMARY USER NAME  IS: "<< summaryUserName << endl;
    pair<string, string> gameNameUserName(gameName, summaryUserName);
    // cout << "THE PAIR I GOT IS: " << gameNameUserName.first << "+" << gameNameUserName.second << endl;
    if (gameNameUserNameToFramesList.count(gameNameUserName) == 0){
        cout << "The user is not subscribed to this game or there are no reports yet" << endl;
        return;
    }
    vector<Frame> framesToProcess = gameNameUserNameToFramesList[gameNameUserName];
    // cout << "IN SUMMARY : # OF FRAMES TO PROCESS: " << framesToProcess.size() << endl;
    vector<vector<string>> listOfTripltesList;
    map<string, string> teamAUpdates;
    map<string, string> teamBUpdates;
    map<string, string> generalUpdates;
    int i = 0;
    vector<string> splittedTeamNames = split(gameName, "_");
    string teamAName = splittedTeamNames.at(0);
    string teamBName = splittedTeamNames.at(1);

    while (unsigned(i) < framesToProcess.size())
    {  
        string bodyFromFrame = framesToProcess.at(i).getBody();
        // cout << "IN SUMMARY - THE BODY OF THE FRAME IS: \n" << bodyFromFrame << endl;
        vector<string> splittedBodyString = split(bodyFromFrame, "\n");
        
        vector <string> listOfTripltes;
        string eventNameLine = splittedBodyString.at(3);
        string eventName = split(eventNameLine, ":").at(1);
        string timeLine = splittedBodyString.at(4);
        string time = split(timeLine, ":").at(1);
        listOfTripltes.push_back(eventName);
        listOfTripltes.push_back(time);

        int j = 6;
        while (splittedBodyString.at(j) != "team a updates:")
        {
            string lineJ = splittedBodyString.at(j);
            vector<string> splittedLineJ = split(lineJ, ":");
            string key = splittedLineJ.at(0);
            string value = splittedLineJ.at(1);
            generalUpdates[key] = value;
            j++;
        }
        // now j points "team a updates"
        j++;

        while (splittedBodyString.at(j) != "team b updates:")
        {
            string lineJ = splittedBodyString.at(j);
            vector<string> splittedLineJ = split(lineJ, ":");
            string key = splittedLineJ.at(0);
            string value = splittedLineJ.at(1);
            teamAUpdates[key] = value;
            j++;
        }
        j++;
        while (splittedBodyString.at(j) != "description:")
        {
            string lineJ = splittedBodyString.at(j);
            vector<string> splittedLineJ = split(lineJ, ":");
            string key = splittedLineJ.at(0);
            string value = splittedLineJ.at(1);
            teamBUpdates[key] = value;
            j++;
        }
        j++;
        string description = splittedBodyString.at(j);
        listOfTripltes.push_back(description);
        listOfTripltesList.push_back(listOfTripltes);

        i++;
    }

    // now all the data is ready, we need to generate the output string
    string output;
    output += teamAName + " vs " + teamBName + "\n";
    output += "Game stats:\n";
    output += "General stats:\n";
    for (auto keyValue : generalUpdates)
    {
        output += keyValue.first + ": " + keyValue.second + "\n";
    }
    output += teamAName + " stats:\n";
    for (auto keyValue : teamAUpdates)
    {
        output += keyValue.first + ": " + keyValue.second + "\n";
    }
    output += teamBName + " stats:\n";
    for (auto keyValue : teamBUpdates)
    {
        output += keyValue.first + ": " + keyValue.second + "\n";
    }
    output += "Game event reports:\n";

    // run over the triplets
    int j = 0;
    while (unsigned(j) < listOfTripltesList.size()){
        output += listOfTripltesList.at(j).at(1) + " - " + listOfTripltesList.at(j).at(0) + ":\n";
        output += listOfTripltesList.at(j).at(2) + "\n\n";
        j++;
    }

    // cout << "I got to write to the file" << endl;
    ofstream outFile;
    outFile.open(file, std::ios::trunc); // trunc flag truncates the existing content of the file
    outFile << output;
    outFile.close();
    // cout << "I wrote to the FILE!!!!" << endl;

    
}

Frame StompClientProtocol::stringToFrameFromServer(string &stringFromServer)
{
    // gets the string, and the command and according to the command builds the frame
    // the string is a valid frame (was chacked at process)
    // cout << stringFromServer << endl;
    vector<string> beforeBodyAndAfterBody = split(stringFromServer, "\n\n"); // splits to until the body and body part
    vector<string> beforeBodySplittedStrings = split(beforeBodyAndAfterBody.at(0), "\n");
    string command = beforeBodySplittedStrings.at(0);
    unordered_map<string, string> headers;
    string body = beforeBodyAndAfterBody.at(1); // get the body part
    int index = 1;
    while (unsigned(index) < beforeBodySplittedStrings.size())
    { // while there are still headers
        string keyValue = beforeBodySplittedStrings.at(index);
        vector<string> keyValueVector = split(keyValue, ":");
        headers.emplace(keyValueVector.at(0), keyValueVector.at(1));
        index++;
    }

    return Frame(command, headers, body);
}

string StompClientProtocol::frameToString(Frame &frame)
{
    // gets frame and just makes it as a long line with \n
    string output;
    output += frame.getCommand();
    output += "\n";
    for (auto keyValue : frame.getHeaders())
    {
        output += keyValue.first + ":" + keyValue.second + "\n";
    }
    output += "\n";
    if (frame.getBody() != "")
    {
        output += frame.getBody();
    }
    output += "\0";
    return output;
}

void StompClientProtocol::processFromServer(Frame &frameFromServer)
{
    // i know that the frame is valid
    // frame if without \0 already
    // cout << "ENTERED PROCESS_FROM_SERVER" << endl;
    string command = frameFromServer.getCommand();
    unordered_map<string, string> headers = frameFromServer.getHeaders();
    string body = frameFromServer.getBody(); // without the \0
    // cout << "WITH THE COMMAND: " << command << ", AND BODY: " << body << endl; 
    // recognize the frame it got from the server
    if (command.compare("CONNECTED") == 0)
    {
        cout << "login succeful" << endl;
    }

    // MESSAGE
    else if (command.compare("MESSAGE") == 0)
    {
        // parse into the Game Object which will save all the details.
        string gameName = frameFromServer.getHeaders()["destination"];
        vector<string> splittedBody = split(frameFromServer.getBody(), "\n");
        string userHeaderAndValue = splittedBody.at(0); // get user: meni
        string senderUserName = split(userHeaderAndValue, ":").at(1);
        pair<string, string> gameNameUserName(gameName, senderUserName);
        // vector<Frame> listOfFrames = gameNameUserNameToFramesList[gameNameUserName];
        gameNameUserNameToFramesList[gameNameUserName].push_back(frameFromServer);
        // listOfFrames.push_back(frameFromServer);
        // gameNameUserNameToFramesList[gameNameUserName] = listOfFrames; ------ is this update is necessery????

        // get the gamename and username. make them a pair and check in a map from this pairs vector of string for updates
    }

    // RECEIPT
    //  -- print "map[repeipt-id]"
    else if (command.compare("RECEIPT") == 0)
    {
        string receiptId = frameFromServer.getHeaders()["receipt-id"];
        string printMessage = receiptIdToPrintMessage[receiptId];
        if (printMessage == "Logout succesfuly"){
            serverShouldTerminate = true;
        }

        cout << printMessage << endl;
    }
    // ERROR
    //  -- print
    else if (command.compare("ERROR") == 0)
    {

        cout << frameFromServer.getHeaders()["message"] << endl;        
        // cout << "AFTER PRINTING THE BODY OF THE ERROR" << endl;
    }

    // do accordingly to the command what it should do.
}

string StompClientProtocol::getNextSubscriptionId()
{
    int output = nextSubscriptionId;
    nextSubscriptionId++;
    return to_string(output);
}

string StompClientProtocol::getNextRecieptNumber()
{
    int output = nextRecieptNumber;
    nextRecieptNumber++;
    return to_string(output);
}

Frame StompClientProtocol::generateWrongCommandFrame()
{
    unordered_map<string, string> headers;
    string command = "WRONG-COMMAND";
    return Frame(command, headers, "");
}
// return Frame(command, headers, body);

// vector<string> StompClientProtocol::split(const string &str, char delimiter)
// {
//     vector<string> lines;
//     std::stringstream ss(str);
//     string line;
//     while (std::getline(ss, line, delimiter))
//     {
//         lines.push_back(line);
//     }
//     return lines;
// }

vector<string> StompClientProtocol::split(const string &str, string delimiter)
{
    // cout <<"enter to split - splitiing the string: " << str << endl;
    vector<string> lines;
    size_t index = str.find(delimiter);
    string temp_string = str;
    while (index != string::npos) // string::npos if could not find
    {
        lines.push_back(temp_string.substr(0,index));
        temp_string.erase(0, index + delimiter.length());
        index = temp_string.find(delimiter);
    }
    lines.push_back(temp_string);
    return lines;
}

void StompClientProtocol::setUserName(string user)
{
    username = user;
}

bool StompClientProtocol::getServerShouldTerminate(){
    return serverShouldTerminate;

}
