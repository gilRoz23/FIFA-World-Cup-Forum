#pragma once
#include <string>
#include <unordered_map>
using namespace std;

class Frame
{
private:
    string command;
    unordered_map <string,string> headers;
    string body;

public:
    Frame(string command, unordered_map<string,string> headers, string body);

    string getCommand();
    unordered_map <string,string> getHeaders();
    string getBody();
    
};