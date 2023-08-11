#include "../include/Frame.h"
#include <string>
#include <unordered_map>
using namespace std;


Frame::Frame(string command, unordered_map<string,string> headers, string body): command(command),
    headers(headers),body(body){}

string Frame::getCommand(){
    return command;
};
unordered_map <string,string> Frame::getHeaders(){
    return headers;
};
string Frame::getBody(){
    return body;
};