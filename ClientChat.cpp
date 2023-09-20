#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/socket.h>


#define PORT 9189

using namespace std;

class Client{
    int connection;
    int newSocket;
    int clientSocket;
    struct sockaddr_in serverAdress;

public:
    Client(){
        newSocket = 0;
    }

//Need to connect using I.P Specs, not implemented yet


//Function to Connect New Socket with Server 
    int connectToServer(){
//Create File Descriptor of Socket Error Handling 
        if ((newSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            cout << "ERROR: Socket creation error!!" << endl;
            return -1;
        }

        serverAdress.sin_family = AF_INET;
        serverAdress.sin_port = htons(PORT);

//Address Conversion to Binary Error Handling
        if (inet_pton(AF_INET, "127.0.0.1", &serverAdress.sin_addr)<= 0) {
            cout << "ERROR: Invalid address. Address not supported" << endl;
            return -1;
        }

//Server Connection Error Handling
        if ((clientSocket = connect(newSocket, (struct sockaddr*)&serverAdress,sizeof(serverAdress)))< 0) {
            cout << "ERROR: Connection Failed" << endl;
            return -1;
        }
        connection = true; 
        cout << "Connected to Server" << endl;
        return 0;
    }




//Function to send client message through socket
    int messageToClient(){
        //Can use client details as a parameter here when sending msgs to server when have >1 client
        cout << "To Server: " << endl;
        string message;
        getline(cin, message); 
        char* data = (char*) message.c_str(); 
        //Send message through the socket
        send(newSocket, data, strlen(data), 0); 
        return 0;
    }

//Function to recieve server message
    int messageFromClient(){
        //Loop to send messages to server until finished with session
        while (connection){ 
            //Buffer to hold the message recieved message from server
            char *buffer = new char[1024]; 
            read(newSocket, buffer, 1024); 
            //Print message to screen
            cout << "Server: " << buffer << endl; 
            delete[] buffer;
            //Call function to respond to server
            messageToClient(); 
        }
        return 0;
    }

//Close connection with server. 
    int closeSocket(){
        close(clientSocket);
        cout << "Socket Connection Successfully Closed" << endl;
        return 0;
    }
};


int main(){
    Client client;
//Function Calls to Send, recieve, and close sockets
    client.connectToServer();
    client.messageToClient();
    client.messageFromClient();
    client.closeSocket();
    return 0;
}