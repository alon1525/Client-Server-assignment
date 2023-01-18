#include <stdlib.h>
#include "StompProtocol.h"
#include <thread>
#include "../include/ConnectionHandler.h"
#include <string.h>

void keyboardHandler(bool shouldTerminate,ConnectionHandler &connectionHandler, StompProtocol &protocol  )
{   
     while (1) {
        const short bufsize = 1024;
        char buf[bufsize];
        std::cin.getline(buf, bufsize);
		std::string keyboardCommand(buf);
		std::vector<std::string> keyboardCommands = protocol.DealWithKeyboard(keyboardCommand);
        for(std::string command : keyboardCommands)
        {
            if (!connectionHandler.sendLine(command)|shouldTerminate) {
            std::cout << "Disconnected. Exiting...\n" << std::endl;
            connectionHandler.close();
            std::terminate();
            break;
            }
        }
        
     }
}


void socketHandler(bool shouldTerminate,ConnectionHandler &connectionHandler , StompProtocol &protocol )
{
    while (1) {
        std::string answer;
        if(connectionHandler.getFrameAscii(answer,'\0'))
        {   
            answer = protocol.DealWithFrame(answer);
            std::cout<<answer<<std::endl;
            if(answer=="close")
            {
                shouldTerminate = true;
            }
        }
        if(shouldTerminate)
        {
            connectionHandler.close();
            std::terminate();
            break;
        }
    }
}

int main(int argc, char *argv[]) {
	// not enough arguments were sent to the server
    if (argc < 5) {
        std::cerr << "not enough arguments" << std::endl;
        return -1;
    }
	std::string login = argv[1];
    std::string hostAndPort = argv[2];
    int index = hostAndPort.find(":");
    short port = stoi(hostAndPort.substr(index+1));
    std::string host = hostAndPort.substr(0,index);
	std:: string username = argv[3];
    std:: string password = argv[4];
    bool shouldTerminate = false;

    ConnectionHandler connectionHandler(host, port);
    if (!connectionHandler.connect()) {
        std::cerr << "Cannot connect to " << host << ":" << port << std::endl;
        return 1;
    }


	if(login!="login")
	{
   		std::cerr << "please login before" << std::endl;
        return -1;
	}
    
    StompProtocol protocol = StompProtocol(username);
    std::string frame = protocol.Connect(username,password);
    std::string packet(frame);
    connectionHandler.sendFrameAscii(packet,'\0');//send the Connect Frame

    std::thread keyBoardThread(keyboardHandler,std::ref(shouldTerminate),std::ref(connectionHandler),std::ref(protocol));
    std::thread serverSocketThread(socketHandler,std::ref(shouldTerminate), std::ref(connectionHandler), std::ref(protocol));

	return 0;
}


