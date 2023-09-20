#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>

#define PORT 9189

using namespace std;

class Server{
    int connection;
    int serverSocket;
    int newSocket;
    struct sockaddr_in address;
    int opt;
    int addressLen;

    //Need DATABASE Variables

public:
    Server(){
        opt = 1;
        addressLen = sizeof(address); 
    }


//Implement DATABASE



//Function to create and initalize New Server socket
    int createServer(){
//File Descriptor for server socket error handling
        if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            cout << "ERROR: Socket failed" << endl;
            exit(EXIT_FAILURE);
        }

//Set port error handling
        if (setsockopt(serverSocket, SOL_SOCKET,SO_REUSEADDR | SO_REUSEPORT, &opt,sizeof(opt))) {
            cout << "ERROR: Problem with Setting Socket Parameters! Check here!" << endl;
            exit(EXIT_FAILURE);
        }

        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(PORT);

//Address Configuration and Bind Socket Error Handling
        if (bind(serverSocket, (struct sockaddr*)&address,sizeof(address)) < 0) {
            cout << "ERROR: Problem with Bind!" << endl;
            exit(EXIT_FAILURE);
        }
        cout << "Server Successfully Initialized!" << endl;

//Listen Error Handling
        if (listen(serverSocket, 3) < 0) {
            cout << "ERROR: Problem with Listen. Listen failed" << endl;
            exit(EXIT_FAILURE);
        }
        cout << "Server Listening..." << endl;

//Accept Error Handling
        if ((newSocket = accept(serverSocket, (struct sockaddr*)&address,(socklen_t*)&addressLen)) < 0) {
            cout <<"ERROR: Problem with Accept" << endl;
            exit(EXIT_FAILURE);
        }
        //Server-Client Connection Now active
        connection = true; 
        //Can specify here which client later when we can accept more than 1 at a time
        cout << "Client Successfully Connected to Server" << endl;

        return 0;
    }


//Function to Send Message to Client
//Need call closeServer from here if QUIT is inputted!!!!!!
    int messageToClient(){
        cout << "To Client: " << endl;
        string message;
//Save written message typed in console
        getline(cin, message); 
    //Pointer to text data written to send to client
        char* hello = (char*) message.c_str(); 

//if input is "QUIT", CALL closeServer!!!!!

        //Send Message to Client through the socket
        send(newSocket, hello, strlen(hello), 0); 
        return 0;
    }


//Function to Recieve Message From Client
    int messageFromClient(){
        //As long as connection is active
        while (connection){ 
            //Buffer to save recieved message into
            char *buffer = new char[1024]; 
            read(newSocket, buffer, 1024); 
            //Can specify which client is sending the message later when we have > 1 at a time
            //Print to screen
            cout << "From Client: " << buffer << endl; 
//Can use this pointer to check message and call the Appropriate function(SWITCH STATEMENT)!
//formats for error handling




//Switch Statement Function Calls
//Default->Error Handling for incorrect message format 

//BUY
//SELL
//LIST
//BALANCE
//SHUTDOWN


            delete[] buffer;
            //Call Function to Respond to Client
            messageToClient(); 
        }
        return 0;
    }






//Function to Create Database




//Function to UPDATE Database







//Function for BUY

//Return Message: "200 OK" If Successful!





//Function for SELL


//Return Message: "200 OK" If Successful!






//Function for BALANCE {OWNER_ID}


//Return Message: "200 OK" If Successful!






//Function for LIST {OWNER_ID}



//Return Message: "200 OK" If Successful!













 //Function to SHUTDOWN the socket for client (call when client says specific thing, not done yet)
 //Can use client info specifications as parameters here when we have >1 client at a time later
//IF Client Message "SHUTDOWN"
    int closeSocket(){
        close(newSocket);
        //Print assignment specified return message
        cout << "200 OK" << endl;
        return 0;
    }

//Function to QUIT.  Need to have a specific word typed in to close server (not done yet)
//If SERVER INPUT "QUI"
    int closeServer(){
        shutdown(serverSocket, SHUT_RDWR);
        cout << "200 OK" << endl;
        return 0;
    }
};

int main(){
    Server server;
    server.createServer();
    server.messageFromClient();
    server.closeSocket();
    server.closeServer();
    return 0;
}