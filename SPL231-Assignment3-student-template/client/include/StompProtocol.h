#pragma once
#include <string>
#include "../include/ConnectionHandler.h"
#include <map>
#include <vector>
#include "event.h"


// TODO: implement the STOMP protocol
class StompProtocol
{

public:
    StompProtocol(std::string username);
    std::vector<std::string> DealWithKeyboard(std::string &reciept);
    std::string DealWithFrame(std::string &reciept);
    std::string Connect(std::string &login,std::string &passcode);
    std::string Subscribe(std::string &dest);
    std::string Unsubscribe(int subscriptionId);
    std::string Disconnect();
    std::string Send(std::string &json_path);
    void DealWithReciept(std::string &reciept);
    std::string writeReport(Event &event);
    Event stringToEvent(std::string &str);
    std::string writeSummary(std::vector<Event> &events);
    

private:
    std::string username;
    int receipt;
    int disconnectReceiptNum;
    std::map<std::string,int> topicToSub;
    std::map<std::string,std::map<std::string,std::vector<Event>>> gameToUser;
    int joinReceiptNum;
    
};
