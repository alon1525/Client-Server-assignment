#include "StompProtocol.h"
#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <vector>
#include <sstream>
#include "event.h"
#include <string.h>
#include <fstream>
using std::string;
using std::stringstream;

StompProtocol::StompProtocol(string username):username(),receipt(0), disconnectReceiptNum(0),topicToSub(),gameToUser(),joinReceiptNum(0)
{
    
}
    
std::vector<std::string> StompProtocol::DealWithKeyboard(std::string &message)
{
    std::vector<string> ans;
   
    std::istringstream messageToHandle(message);
    std::vector<string> fullmessage;
    std::string word;

    while (messageToHandle >> word)//we push the first word 
    {
        fullmessage.push_back(word); 
    }

    if(fullmessage[0] == "login")
    {
        ans.push_back("the client is already connected");
    }
    else if(fullmessage[0] == "join")
    {
        ans.push_back(Subscribe(fullmessage[1]));
        
    }
    else if(fullmessage[0] == "exit")
    {
        int subId = topicToSub[fullmessage[1]];
        topicToSub.erase(fullmessage[1]);
        ans.push_back(Unsubscribe(subId));
    }
    else if(fullmessage[0] == "report")
    {
        names_and_events event = parseEventsFile(fullmessage[1]);
        for(Event eve : event.events)
        {
            string gameName = eve.get_team_a_name() + "_" + eve.get_team_b_name();
            bool foundGame = false;
            bool foundUser = false;
            for(std::pair<std::string,std::map<std::string,std::vector<Event>>> value : gameToUser)
            {
                if(value.first == gameName)
                {
                    foundGame=true;
                    for(std::pair<std::string,std::vector<Event>> user : gameToUser[gameName])
                    {
                        if(user.first == username)
                        {
                            foundUser = true;
                            user.second.push_back(eve);
                        }
                    }
                    if(!foundUser)
                    {
                        std::vector<Event> events = std::vector<Event>();
                        events.push_back(eve);
                        gameToUser[gameName].insert({username,events});
                    }
                    
                }
            }
            if(foundGame==false)
            {
                std::vector<Event> events = std::vector<Event>();
                events.push_back(eve);
                std::map<std::string,std::vector<Event>> newUser = std::map<std::string,std::vector<Event>>();
                newUser.insert({username,events});
                gameToUser.insert({gameName,newUser});
            }
            string report = writeReport(eve);
            std::pair<string,string> newReport(report,username);
            ans.push_back(report);
        }
    }
    else if(fullmessage[0] == "summary")
    {
        std::map<std::string,std::vector<Event>> repository = gameToUser[fullmessage[1]];
        std::vector<Event> events = repository[fullmessage[2]];
        std::fstream file(fullmessage[3]);
        file << writeSummary(events);
        file.close();  
    }
    else if(fullmessage[0] == "logout")
    {
        ans.push_back(Disconnect());
    }
    else
    {
       ans.push_back("this is not a legal message");
    }
    
    return ans;
   }
   
  
std::string StompProtocol::DealWithFrame(std::string &frame)
{
    string ans;
   
    std::istringstream messageToHandle(frame);
    std::vector<string> fullmessage;
    std::string word;

    while (messageToHandle >> word)//we push the first word 
    {
        fullmessage.push_back(word); 
    }

    if(fullmessage[0] == "CONNECTED")
    {
        ans= "the client is already connected";
    }
    else if(fullmessage[0] == "RECEIPT" && stoi(fullmessage[2])==disconnectReceiptNum)
    {
        ans = "close";
    }
    else if(fullmessage[0] == "RECEIPT" && stoi(fullmessage[2])==joinReceiptNum)
    {
        int subId = stoi(fullmessage[3]);
        string dest = fullmessage[4];
        topicToSub.insert({dest,subId});
    }
    else if(fullmessage[0] == "MESSAGE")
    {
        Event eve = stringToEvent(frame);
        string gameName = eve.get_team_a_name() + "_" + eve.get_team_b_name();
        bool foundGame = false;
        bool foundUser = false;
            for(std::pair<std::string,std::map<std::string,std::vector<Event>>> value : gameToUser)
            {
                if(value.first == gameName)
                {
                    foundGame=true;
                    for(std::pair<std::string,std::vector<Event>> user : gameToUser[gameName])
                    {
                        if(user.first == username)
                        {
                            foundUser = true;
                            user.second.push_back(eve);
                        }
                    }
                    if(!foundUser)
                    {
                        std::vector<Event> events = std::vector<Event>();
                        events.push_back(eve);
                        gameToUser[gameName].insert({username,events});
                    }
                    
                }
            }
            if(foundGame==false)
            {
                std::vector<Event> events = std::vector<Event>();
                events.push_back(eve);
                std::map<std::string,std::vector<Event>> newUser = std::map<std::string,std::vector<Event>>();
                newUser.insert({username,events});
                gameToUser.insert({gameName,newUser});
            }
    }    
    else
    {
        ans= "this is not a legal message";
    }
    
    return ans;
}


string StompProtocol:: Connect(string &user,string &passcode){
    
    stringstream message;
    message << "CONNECT\n";
    message << "accept-version:1.2\n";
    message << "host:stomp.cs.bgu.ac.il\n";
    message << "login:" << user << "\n";
    message << "passcode:" << passcode << "\n\n";
    message << "\0";
    std::string login = message.str();
    return login;

}

string StompProtocol:: Subscribe(std::string &dest){
    
    stringstream message;
    message << "SUBSCRIBE\n";
    message << "destination:/" << dest << "\n";
    message << "id:" << topicToSub[dest] << "\n";
    receipt++;
    joinReceiptNum = receipt-1;
    message << "receipt:" << receipt-1 << "\n\n";
    message << "\0";
    std::string sub = message.str();
    return sub;

}

string StompProtocol:: Unsubscribe(int subscriptionId){
    
    stringstream message;
    message << "UNSUBSCRIBE\n";
    message << "id:" << subscriptionId << "\n\n";
    message << "\0";
    std::string unsub = message.str();
    
    return unsub;

}

string StompProtocol:: Disconnect(){
    
    stringstream message;
    message << "DISCONNECT\n";
    receipt++;
    disconnectReceiptNum = receipt-1;
    message << "receipt:" << receipt-1 << "\n\n";
    message << "\0";
    std::string dis = message.str();
    
    return dis;

}

string StompProtocol::writeReport(Event &event)
{
    stringstream message;
    message << "SEND\n";
    message << "destination:/topic/" << event.get_team_a_name()<<"_"<<event.get_team_b_name()<<"\n";
    message << "user:"<< username<<"\n";
    message << "team a:" << event.get_team_a_name() << "\n";
    message << "team b:" << event.get_team_b_name() << "\n";
    message << "event name: " << event.get_name() << "\n";
    message << "time: " << event.get_time() << "\n";
    message << "general game updates:" << "\n";//check
    for(std::pair<string,string> update : event.get_game_updates())
    {
        message << "   " << update.first << ": " << update.second << "\n";
    }  

    message << "team a updates:" << "\n";
    for(std::pair<string,string> update : event.get_team_a_updates())
    {
        message << "   " << update.first << ": " << update.second << "\n";
    }

    message << "team b updates:" << "\n";
    for(std::pair<string,string> update : event.get_team_b_updates())
    {
        message << "   " << update.first << ": " << update.second << "\n";
    }
    
    message << "description:\n" << event.get_discription() << "\n";

    message << "\0";
    std::string sub = message.str();
    return sub;

}

Event StompProtocol::stringToEvent(string &str)
{
    std::stringstream ss(str);
    string line;
    std::getline(ss,line);
    std::getline(ss,line);
    std::getline(ss,line);
    int index = line.find(":");
    string user = line.substr(index+1);
    std::getline(ss,line);
    index = line.find(":");
    string team_a_name = line.substr(index+1);
    std::getline(ss,line);
    index = line.find(":");
    string team_b_name = line.substr(index+1);
    std::getline(ss,line);
    index = line.find(":");
    string name = line.substr(index+1);
    std::getline(ss,line);
    index = line.find(":");
    int time = stoi(line.substr(index+1));
    std::getline(ss,line);
    std::getline(ss,line);
    index = line.find(":");
    std::map<std::string, std::string> game_updates;
    while(line.substr(0,index)!="team a updates")
    {
        int i=0;
        while(line.find(" ") != std::string::npos)
        {
            line.erase(i,1);
        }
        index = line.find(":");
        game_updates.insert({line.substr(0,index),line.substr(index+1)});
        std::getline(ss,line);
    }
    std::getline(ss,line);
    index = line.find(":");
    std::map<std::string, std::string> team_a_updates;
    while(line.substr(0,index)!="team b updates")
    {
        int i=0;
        while(line.find(" ")!= std::string::npos)
        {
            line.erase(i,1);
        }
        index = line.find(":");
        team_a_updates.insert({line.substr(0,index),line.substr(index+1)});
        std::getline(ss,line);
    }
    std::getline(ss,line);
    index = line.find(":");
    std::map<std::string, std::string> team_b_updates; 
    while(line.substr(0,index)!="description")
    {
        int i=0;
        while(line.find(" ")!= std::string::npos)
        {
            line.erase(i,1);
        }
        index = line.find(":");
        team_b_updates.insert({line.substr(0,index),line.substr(index+1)});
        std::getline(ss,line);
    }
    index = str.find("description:");
    int endIndex = str.find("\0");
    string description = str.substr(index+12,endIndex-index+1);
    return Event(team_a_name,team_b_name,name,time,game_updates,team_a_updates,team_b_updates,description);
    
    
}

std::string StompProtocol::writeSummary(std::vector<Event> &events)
{
    stringstream message;
    
    message << events[0].get_team_a_name() << " vs " << events[0].get_team_b_name();
    message<< "game stats:\n";
    message<< "general stats:\n";
    for(Event event : events)
    {
        for(std::pair<std::string,std::string> update : event.get_game_updates()){
            message << update.first <<  ": " << update.second << "\n";}
        
    }
    message<< events[0].get_team_a_name() <<"stats:\n";
    for(Event event : events)
    {
        for(std::pair<std::string,std::string> update : event.get_team_a_updates()){
            message << update.first <<  ": " << update.second << "\n";}
        
    }
    message<< events[0].get_team_b_name() <<"stats:\n";
    for(Event event : events)
    {
        for(std::pair<std::string,std::string> update : event.get_team_b_updates()){
            message << update.first <<  ": " << update.second << "\n";}
        
    }
    message<< "game event reports:\n";
    for(Event event : events)
    {
        message << event.get_time() <<  " - " << event.get_team_b_name() << "\n";
        message << event.get_discription();
        
    }
    
    message << "\0";
    std::string sub = message.str();
    return sub;
}
