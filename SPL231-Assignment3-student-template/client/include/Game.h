#pragma once
#include <string>
#include <vector>
#include <map>
#include <list>
using namespace std;

class Event;

class Game
{
private:
    string username;
    string team_a_name;
    string team_b_name;
    
    map<string,string> generalStats; //stat_name,stat_value
    map<string,string> team_a_stats; //stat_name,stat_value
    map<string,string> team_b_stats; //stat_name,stat_value
    list<string> gameEventReports; // "eventTime - eventName: \nevent - description"

public:
    map<string,string> getGeneralStats(); 
    map<string,string> getTeam_a_stats(); 
    map<string,string> getTeam_b_stats(); 
    list<string> getGameEventReports();
    
    
}