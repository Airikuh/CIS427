## INTRODUCTION: 

This program was designed to run with C++. The Platform we used was Visual Studio. This is an online Pokémon card trading application that allows for multiple clients to connect to the server simultaneously. The client and server components of this application will communicate with each other using TCP sockets program. The commands you will be able to execute within this application are: BUY, SELL, LIST, BALANCE, QUIT, SHUTDOWN, LOGIN, LOGOUT, DEPOSIT, WHO, and LOOKUP.


## Running Instructions: 

To run the code you will need to follow these steps: 

Connect first to global protectant. Then you can run Bitvise SSH Client
After connected to UMICH Server- Make sure you have uploaded both server and client files and the sqlite files (3 Sqlite files total) to the umich server: Bitvise SSH.
You will need a New Terminal console x 2 for client and server windows 

Server:
g++ server.cpp -std=c++11 -ldl -pthread sqlite3.o -o server

./server

Client:
g++ client.cpp -o client

./client 127.0.0.1
*Must include IP address to compile


Users you can log in to:

UserID and Password:

root
root01

j_doe
Passwrd4

j_smith		
Pass456

c_brown
Snoopy

l_van
Football

l_blanket
Security23


## COMMANDS:

### BUY:
In order to buy a Pokemon card you must follow these steps: 

Client > BUY (followed by the Pokemon name, card type, rarity, quantity, card cost, and the owners ID)

Buy Example: BUY Pikachu Electric Common 19.99 2 2

### SELL:
To sell a Pokemon card you must follow these steps: 

Client > SELL (followed by the Pokemon name, card type, rarity, quantity, card cost, and the user ID you want to sell to)

Sell Example: SELL Pikachu Electric Common 19.99 2 4


### LIST: 
To list all records in the Pokemon card table of all Users, you will need to be logged in to a Root users account.

A LIST command issued by a non-root user such as John should return only John’s records.

Here’s LIST example for Non-root users:
Client sends: LIST 
Server sends: 200 OK 

The list of records in the Pokémon cards table for current user, John:
ID Card Name Type Rarity Count OwnerID
1 Pikachu Electric Common 2 John
5 Charizard Fire Rare 6 John

Scenario 2, if a Root user is currently logged in:

Client sends: LIST 
Server sends: 200 OK 
The list of records in the cards database:
ID Card Name Type Rarity Count OwnerID
1 Pikachu Electric Common 2 John
2 Charizard Fire Rare 1 Jane
5 Charizard Fire Rare 6 John


### BALANCE:
Display the USD balance for the selected user A client-server interaction with the BALANCE command looks like:
 Client sends: BALANCE  Server sends: BALANCE  c: 200 OK Balance for user Jane Smith: $90.00


### SHUTDOWN:
If a client sends a SHUTDOWN command to the server, it will terminate all connected clients and shut down the server. 

However, only the root user has the authority to execute a server shutdown. 
Client > SHUTDOWN
Client > 200 OK

Server > Received: SHUTDOWN
		Closed database
		Client Terminated Connection
		Socket Closed

If a non-root user attempts to send a shutdown command, the server will refuse it and send a 401 status code, and an error message accordingly.


### QUIT:
Only terminate the client. The client exits when it receives the confirmation message from the server.
 
A client-server interaction with the QUIT command looks like:
client: QUIT
client: 200 OK


### LOGIN:
-To login to the server, you must enter LOGIN, followed by a space, followed by a UserID, followed by a space, followed by a Password

LOGIN Example:
Client sends: LOGIN j_doe Passwrd4 
Server sends: 200 OK 

### WHO:
-List all active users, including the UserID and the user’s IP addresses. This command is only allowed for the root users.

WHO Example:

Client sends: WHO 
Server sends: 200 OK  
The list of the active users: 
John  141.215.69.202 
root   127.0.0.1

### LOOKUP:
-To lookup a Pokemon card name in the list, you must enter LOOKUP followed by a space, followed by a name

LOOKUP Example:
Client sends: LOOKUP Char 
Server sends: 200 OK
Found 1 match
ID Card Name Type Rarity Count Owner
2 Charizard Fire Rare 1 John

Client sends: LOOKUP random_card
Server sends: 404 Your search did not match any records.


### LOGOUT:
To log out of the server, the client enters LOGOUT . The user is also not allowed to send BUY, SELL, LIST, BALANCE, and SHUTDOWN commands after logging out, but it can still send the QUIT commands. This command results in the server terminating the allocated socket and thread for this client. 

LOGOUT Example:
Client sends: LOGOUT
Server sends: Recieved: LOGOUT 


### DEPOSIT:
Deposit USD to the user’s account/record. A user can deposit an amount of USD into their account. A client that wants too deposit an amount of USD to their account must follow the steps below: 

DEPOSIT + a space + the USD amount 

An example of using DEPOSIT: 
Client sends: DEPOSIT 40.50
Server sends: Deposit successfully. New balance $140.50


## STUDENTS’ ROLES: 

Erika Valle-Baird: Lead developer/documenter
Aiden Chimienti: developer/documenter
Abeer Waza: developer/documenter


## BUG’S IN THE CODE: 




## Test Cases:

<img width="592" alt="Screen Shot 2023-10-19 at 11 03 02 AM" src="https://github.com/Airikuh/CIS427/assets/114634736/32c02bda-a7ce-4883-acfd-cbb6c1a12803">
<img width="591" alt="Screen Shot 2023-10-19 at 11 03 14 AM" src="https://github.com/Airikuh/CIS427/assets/114634736/64bb8f08-40d6-4906-8231-1724ccec9747">

                                                                            
