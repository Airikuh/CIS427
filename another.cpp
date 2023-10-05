// Standard C++ headers
#include <iostream>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
// Server Port/Socket/Addr related headers
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "sqlite3.h"
using namespace std;

#define SERVER_PORT  29270
#define MAX_PENDING  5
#define MAX_LINE     256

// Server Variables
struct sockaddr_in srv;
char buf[MAX_LINE];
socklen_t buf_len, addr_len;
int nRet;
int nClient[10] = { 0, };
int nSocket;
string infoArr[3];

sqlite3* db;
char* zErrMsg = 0;
const char* sql;
int rc;
string result;
string* ptr = &result;


typedef struct
{
    int socket;
    int id;
    string user;
    string password;
}userInfo;

typedef struct
{
    string ip;
    string user;
    int socket;
    pthread_t threadAwesome;
}loggedUser;

void* temp = malloc(sizeof(userInfo));
userInfo u;

vector<loggedUser> list;

fd_set fr;
fd_set fw;
fd_set fe;
int nMaxFd;
pthread_t thread_handles;
long thread;
string buildCommand(char*);
string extractInfo(char*, string);
bool extractInfo(char*, string*, string);
void* serverCommands(void*);
static int callback(void*, int, char**, char**);
string getPassword(char line[], int n);
void HandleNewConnection();
void HandleDataFromClient();

int main(int argc, char* argv[]) {
#pragma region Database Setup
    // Open Database and Connect to Database
    rc = sqlite3_open("cis427PokeCardssss.sqlite", &db);
    // Check if Database was opened successfully
    if (rc) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        return(0);
    }
    else {
        fprintf(stderr, "Opened database successfully\n");
    }


    // Create sql Users table creation command
    sql = "create table if not exists Users\
    (\
        ID INTEGER PRIMARY KEY AUTOINCREMENT,\
        email TEXT NOT NULL,\
        first_name TEXT,\
        last_name TEXT,\
        user_name TEXT NOT NULL,\
        password TEXT,\
        usd_balance DOUBLE NOT NULL\
    );";

    // Execute Users table creation
    rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);


    // Create sql Pokemon_cards table creation command
    sql = "create table if not exists Pokemon_cards (\
        ID INTEGER PRIMARY KEY AUTOINCREMENT,\
        card_name varchar(10) NOT NULL,\
        card_balance DOUBLE,\
        card_type TEXT NOT NULL,\
        rarity TEXT NOT NULL,\
        owner_id TEXT,\
        FOREIGN KEY(owner_id) REFERENCES Users(ID)\
    );";

    // Execute Pokemon_cards table creation
    rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);


    // Checks if the root exists in the database. If no user is found, create it
    sql = "SELECT IIF(EXISTS(SELECT 1 FROM Users WHERE  Users.user_name='root'), 'USER_PRESENT', 'USER_NOT_PRESENT') result;";
    rc = sqlite3_exec(db, sql, callback, ptr, &zErrMsg);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }
    else if (result == "USER_NOT_PRESENT") {
        // Create the root user:
        fprintf(stdout, "Root user is not present. Attempting to add the user.\n");

        // Adds the root user
        sql = "INSERT INTO Users VALUES (1, 'cis427@abc.com', 'Root', 'User', 'root', 'root01', 100);";
        rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);

        if (rc != SQLITE_OK) {
            fprintf(stderr, "SQL error: %s\n", zErrMsg);
            sqlite3_free(zErrMsg);
        }
        else {
            fprintf(stdout, "\tA root user was added successfully.\n");
        }
    }
    else if (result == "USER_PRESENT") {
        cout << "\tThe root user already exists in the Users table.\n";
    }
    else {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        cout << "Error returned Result = " << result << endl;
    }

    // Checks if john exists in the database. If no user is found, create it
    sql = "SELECT IIF(EXISTS(SELECT 1 FROM Users WHERE  Users.user_name='jdoe'), 'USER_PRESENT', 'USER_NOT_PRESENT') result;";
    rc = sqlite3_exec(db, sql, callback, ptr, &zErrMsg);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }
    else if (result == "USER_NOT_PRESENT") {
        fprintf(stdout, "john is not present. Attempting to add the user.\n");

        // Adds john to the database
        sql = "INSERT INTO Users VALUES (2, 'j.doe@abc.com', 'john', 'doe', 'jdoe', 'Passwrd4', 100);";
        rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);

        if (rc != SQLITE_OK) {
            fprintf(stderr, "SQL error: %s\n", zErrMsg);
            sqlite3_free(zErrMsg);
        }
        else {
            fprintf(stdout, "\tA new user (john) was added successfully.\n");
        }
    }
    else if (result == "USER_PRESENT") {
        cout << "\tjohn already exists in the Users table.\n";
    }
    else {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        cout << "Error returned Result = " << result << endl;
    }

    // Check if jane exists in the database. If no user is found, create it
    sql = "SELECT IIF(EXISTS(SELECT 1 FROM Users WHERE  Users.user_name='j_smith'), 'USER_PRESENT', 'USER_NOT_PRESENT') result;";
    rc = sqlite3_exec(db, sql, callback, ptr, &zErrMsg);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }
    else if (result == "USER_NOT_PRESENT") {
        fprintf(stdout, "jane is not present. Attempting to add the user.\n");

        // Adds jane
        sql = "INSERT INTO Users VALUES (3, 'j.smith@abc.com', 'jane', 'smith', 'j_smith', 'pass456', 90);";
        rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);

        if (rc != SQLITE_OK) {
            fprintf(stderr, "SQL error: %s\n", zErrMsg);
            sqlite3_free(zErrMsg);
        }
        else {
            fprintf(stdout, "\tA new user (jane) was added successfully.\n");
        }
    }
    else if (result == "USER_PRESENT") {
        cout << "\tjane already exists in the Users table.\n";
    }
    else {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        cout << "Error returned Result = " << result << endl;
    }


    
    // Checks if charlie exists in the database. If no user is found, create it
    sql = "SELECT IIF(EXISTS(SELECT 1 FROM Users WHERE  Users.user_name='c_brown'), 'USER_PRESENT', 'USER_NOT_PRESENT') result;";
    rc = sqlite3_exec(db, sql, callback, ptr, &zErrMsg);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }
    else if (result == "USER_NOT_PRESENT") {
        fprintf(stdout, "charlie is not present. Attempting to add the user.\n");

        // Add charlie to the Database
        sql = "INSERT INTO Users VALUES (4, 'c.brown@abc.com', 'charlie', 'brown', 'c_brown', 'Snoopy', 90);";
        rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);

        if (rc != SQLITE_OK) {
            fprintf(stderr, "SQL error: %s\n", zErrMsg);
            sqlite3_free(zErrMsg);
        }
        else {
            fprintf(stdout, "\tA new user (charlie) was added successfully.\n");
        }
    }
    else if (result == "USER_PRESENT") {
        cout << "\tcharlie already exists in the Users table.\n";
    }
    else {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        cout << "Error returned Result = " << result << endl;
    }



    // Check if lucy exists in the database. If no user is found, create it
    sql = "SELECT IIF(EXISTS(SELECT 1 FROM Users WHERE  Users.user_name='l_van'), 'USER_PRESENT', 'USER_NOT_PRESENT') result;";
    rc = sqlite3_exec(db, sql, callback, ptr, &zErrMsg);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }
    else if (result == "USER_NOT_PRESENT") {
        fprintf(stdout, "lucy is not present. Attempting to add the user.\n");

        // Add lucy to the database
        sql = "INSERT INTO Users VALUES (5, 'l.van@abc.com', 'lucy', 'van', 'l_van', 'Football', 70);";
        rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);

        if (rc != SQLITE_OK) {
            fprintf(stderr, "SQL error: %s\n", zErrMsg);
            sqlite3_free(zErrMsg);
        }
        else {
            fprintf(stdout, "\tA new user (lucy) was added successfully.\n");
        }
    }
    else if (result == "USER_PRESENT") {
        cout << "\tlucy already exists in the Users table.\n";
    }
    else {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        cout << "Error returned Result = " << result << endl;
    }

    // Checks if linus exists in the database. If no user is found, create it
    sql = "SELECT IIF(EXISTS(SELECT 1 FROM Users WHERE  Users.user_name='l_blanket'), 'USER_PRESENT', 'USER_NOT_PRESENT') result;";
    rc = sqlite3_exec(db, sql, callback, ptr, &zErrMsg);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }
    else if (result == "USER_NOT_PRESENT") {
        fprintf(stdout, "linus is not present. Attempting to add the user.\n");

        // Add linus to the Database
        sql = "INSERT INTO Users VALUES (6, 'l.blanket@abc.com', 'linus', 'blanket', 'l_blanket', 'security23', 90);";
        rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);

        if (rc != SQLITE_OK) {
            fprintf(stderr, "SQL error: %s\n", zErrMsg);
            sqlite3_free(zErrMsg);
        }
        else {
            fprintf(stdout, "\tA new user (linus) was added successfully.\n");
        }
    }
    else if (result == "USER_PRESENT") {
        cout << "\tlinus already exists in the Users table.\n";
    }
    else {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        cout << "Error returned Result = " << result << endl;
    }






    // Checks if Pikachu exists in the database. If no user is found, create it
    sql = "SELECT IIF(EXISTS(SELECT 1 FROM Pokemon_cards WHERE  Pokemon_cards.card_name='Pikachu'), 'CARD_PRESENT', 'CARD_NOT_PRESENT') result;";
    rc = sqlite3_exec(db, sql, callback, ptr, &zErrMsg);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }
    else if (result == "CARD_NOT_PRESENT") {
        fprintf(stdout, "Pikachu is not present. Attempting to add the card.\n");

        // Adds Pikachu to the database
        sql = "INSERT INTO Pokemon_cards VALUES (1, 'Pikachu', 19.99, 'Electric', 'Common', 1);";
        rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);

        if (rc != SQLITE_OK) {
            fprintf(stderr, "SQL error: %s\n", zErrMsg);
            sqlite3_free(zErrMsg);
        }
        else {
            fprintf(stdout, "\tA new card (Pikachu) was added successfully.\n");
        }
    }
    else if (result == "CARD_PRESENT") {
        cout << "\tcard already exists in the Pokemon_cards table.\n";
    }
    else {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        cout << "Error returned Result = " << result << endl;
    }


    // Checks if Pikachu exists in the database. If no user is found, create it
    sql = "SELECT IIF(EXISTS(SELECT 1 FROM Pokemon_cards WHERE  Pokemon_cards.card_name='Pikachu'), 'CARD_PRESENT', 'CARD_NOT_PRESENT') result;";
    rc = sqlite3_exec(db, sql, callback, ptr, &zErrMsg);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }
    else if (result == "CARD_NOT_PRESENT") {
        fprintf(stdout, "Pikachu is not present. Attempting to add the card.\n");

        // Adds Pikachu to the database
        sql = "INSERT INTO Pokemon_cards VALUES (1, 'Pikachu', 19.99, 'Electric', 'Common', 1);";
        rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);

        if (rc != SQLITE_OK) {
            fprintf(stderr, "SQL error: %s\n", zErrMsg);
            sqlite3_free(zErrMsg);
        }
        else {
            fprintf(stdout, "\tA new card (Pikachu) was added successfully.\n");
        }
    }
    else if (result == "CARD_PRESENT") {
        cout << "\tcard already exists in the Pokemon_cards table.\n";
    }
    else {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        cout << "Error returned Result = " << result << endl;
    }

    // Checks if Charizard exists in the database. If no user is found, create it
    sql = "SELECT IIF(EXISTS(SELECT 1 FROM Pokemon_cards WHERE  Pokemon_cards.card_name='Charizard'), 'CARD_PRESENT', 'CARD_NOT_PRESENT') result;";
    rc = sqlite3_exec(db, sql, callback, ptr, &zErrMsg);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }
    else if (result == "CARD_NOT_PRESENT") {
        fprintf(stdout, "Charizard is not present. Attempting to add the card.\n");

        // Adds Pikachu to the database
        sql = "INSERT INTO Pokemon_cards VALUES (2, 'Charizard', 15.49, 'Fire', 'Rare', 1);";
        rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);

        if (rc != SQLITE_OK) {
            fprintf(stderr, "SQL error: %s\n", zErrMsg);
            sqlite3_free(zErrMsg);
        }
        else {
            fprintf(stdout, "\tA new card (Charizard) was added successfully.\n");
        }
    }
    else if (result == "CARD_PRESENT") {
        cout << "\tCharizard already exists in the Pokemon_cards table.\n";
    }
    else {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        cout << "Error returned Result = " << result << endl;
    }


    // Checks if Bulbasaur exists in the database. If no user is found, create it
    sql = "SELECT IIF(EXISTS(SELECT 1 FROM Pokemon_cards WHERE  Pokemon_cards.card_name='Bulbasaur'), 'CARD_PRESENT', 'CARD_NOT_PRESENT') result;";
    rc = sqlite3_exec(db, sql, callback, ptr, &zErrMsg);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }
    else if (result == "CARD_NOT_PRESENT") {
        fprintf(stdout, "Bulbasaur is not present. Attempting to add the card.\n");

        // Adds Pikachu to the database
        sql = "INSERT INTO Pokemon_cards VALUES (3, 'Bulbasaur', 11.9, 'Grass', 'Common', 3);";
        rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);

        if (rc != SQLITE_OK) {
            fprintf(stderr, "SQL error: %s\n", zErrMsg);
            sqlite3_free(zErrMsg);
        }
        else {
            fprintf(stdout, "\tA new card (Bulbasaur) was added successfully.\n");
        }
    }
    else if (result == "CARD_PRESENT") {
        cout << "\tBulbasaur already exists in the Pokemon_cards table.\n";
    }
    else {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        cout << "Error returned Result = " << result << endl;
    }


    // Checks if Squirtle exists in the database. If no user is found, create it
    sql = "SELECT IIF(EXISTS(SELECT 1 FROM Pokemon_cards WHERE  Pokemon_cards.card_name='Squirtle'), 'CARD_PRESENT', 'CARD_NOT_PRESENT') result;";
    rc = sqlite3_exec(db, sql, callback, ptr, &zErrMsg);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }
    else if (result == "CARD_NOT_PRESENT") {
        fprintf(stdout, "Squirtle is not present. Attempting to add the card.\n");

        // Adds Pikachu to the database
        sql = "INSERT INTO Pokemon_cards VALUES (4, 'Squirtle', 18.99, 'Water', 'Uncommon', 4);";
        rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);

        if (rc != SQLITE_OK) {
            fprintf(stderr, "SQL error: %s\n", zErrMsg);
            sqlite3_free(zErrMsg);
        }
        else {
            fprintf(stdout, "\tA new card (Squirtle) was added successfully.\n");
        }
    }
    else if (result == "CARD_PRESENT") {
        cout << "\tSquirtle already exists in the Pokemon_cards table.\n";
    }
    else {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        cout << "Error returned Result = " << result << endl;
    }


    // Checks if Jigglypuff exists in the database. If no user is found, create it
    sql = "SELECT IIF(EXISTS(SELECT 1 FROM Pokemon_cards WHERE  Pokemon_cards.card_name='Jigglypuff'), 'CARD_PRESENT', 'CARD_NOT_PRESENT') result;";
    rc = sqlite3_exec(db, sql, callback, ptr, &zErrMsg);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }
    else if (result == "CARD_NOT_PRESENT") {
        fprintf(stdout, "Jigglypuff is not present. Attempting to add the card.\n");

        // Adds Pikachu to the database
        sql = "INSERT INTO Pokemon_cards VALUES (5, 'Jigglypuff', 24.99, 'Normal', 'Common', 5);";
        rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);

        if (rc != SQLITE_OK) {
            fprintf(stderr, "SQL error: %s\n", zErrMsg);
            sqlite3_free(zErrMsg);
        }
        else {
            fprintf(stdout, "\tA new card (Jigglypuff) was added successfully.\n");
        }
    }
    else if (result == "CARD_PRESENT") {
        cout << "\tJigglypuff already exists in the Pokemon_cards table.\n";
    }
    else {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        cout << "Error returned Result = " << result << endl;
    }





#pragma endregion

    // Setup passive open // Initialize the socket
    nSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (nSocket < 0) {
        cout << "Socket not Opened\n";
        sqlite3_close(db);
        cout << "Closed DB" << endl;
        exit(EXIT_FAILURE);
    }
    else {
        cout << "Socket Opened: " << nSocket << endl;
    }

    // Set Socket Options
    int nOptVal = 1;
    int nOptLen = sizeof(nOptVal);
    nRet = setsockopt(nSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&nOptVal, nOptLen);
    if (!nRet) {
        cout << "The setsockopt call successful\n";
    }
    else {
        cout << "Failed setsockopt call\n";
        sqlite3_close(db);
        cout << "Closed Database" << endl;
        close(nSocket);
        cout << "Closed socket: " << nSocket << endl;
        exit(EXIT_FAILURE);
    }

    // Build address data structure
    srv.sin_family = AF_INET;
    srv.sin_port = htons(SERVER_PORT);
    srv.sin_addr.s_addr = INADDR_ANY;
    memset(&(srv.sin_zero), 0, 8);

    //Bind the socket to the local port
    nRet = (bind(nSocket, (struct sockaddr*)&srv, sizeof(srv)));
    if (nRet < 0) {
        cout << "Failed to bind to local port\n";
        sqlite3_close(db);
        cout << "Closed Database " << endl;
        close(nSocket);
        cout << "Closed socket: " << nSocket << endl;
        exit(EXIT_FAILURE);
    }
    else {
        cout << "Successfully bound to local port\n";
    }


    //Listen to the request from client
    nRet = listen(nSocket, MAX_PENDING);
    if (nRet < 0) {
        cout << "Failed to start listen to local port\n";
        sqlite3_close(db);
        cout << "Closed Database " << endl;
        close(nSocket);
        cout << "Closed socket: " << nSocket << endl;
        exit(EXIT_FAILURE);
    }
    else {
        cout << "Started listening to local port\n";
    }


    struct timeval tv;

    cout << "\nWaiting for connections ...\n";


    while (1)
    {
        tv.tv_sec = 1;
        tv.tv_usec = 0;

        //Set the FD_SET. This needs to be done every time
        FD_ZERO(&fr);
        FD_SET(nSocket, &fr);
        nMaxFd = nSocket;

        for (int nIndex = 0; nIndex < 5; nIndex++)
        {
            if (nClient[nIndex] > 0)
            {
                FD_SET(nClient[nIndex], &fr);
            }
            if (nClient[nIndex] > nMaxFd)
                nMaxFd = nClient[nIndex];
        }

        nRet = select(nMaxFd + 1, &fr, NULL, NULL, &tv);

        //After above call, every bit is reset by select call in fr
        if (nRet < 0)
        {
            cout << endl << "select api call failed. Will exit";
            return (EXIT_FAILURE);
        }
        else
        {
            //There is some client waiting either to connect or some new data came from existing client.
            if (FD_ISSET(nSocket, &fr))
            {
                //Handle New connection
                HandleNewConnection();
            }
            else
            {
                //Check what existing client got the new data
                HandleDataFromClient();
            }
        }
    }



    for (int l = 0; l < 10; l++) {
        close(nClient[l]);
    }


    sqlite3_close(db);
    cout << "Closed DB" << endl;
    close(nSocket);
    cout << "Closed socket: " << nSocket << endl;
    exit(EXIT_SUCCESS);
}

// Parses command from buffer sent from client 
string buildCommand(char line[]) {
    string command = "";
    size_t len = strlen(line);
    for (size_t i = 0; i < len; i++) {
        if (line[i] == '\n')
            continue;
        if (line[i] == ' ')
            break;
        command += line[i];
    }
    return command;
}

// Enters the command info into an array. This array contains the type of card, amount of card, price per unit of card, and the user ID.
// Returns true if successful, otherwise returns false 
string extractInfo(char line[], string command) {
    int l = command.length();
    int spaceLocation = l + 1;
    int i = spaceLocation;
    string info = "";

    while (line[i] != '\n') {
        if (line[i] == 0)
            return "";
        if (line[i] == ' ')
            return info;
        info += line[i];
        i++;
    }
    return info;
}

void* serverCommands(void* userData) {
    cout << "Username: " << ((userInfo*)userData)->user << endl;;
    int clientIndex = ((userInfo*)userData)->socket;
    int clientID = nClient[((userInfo*)userData)->socket];

    nClient[clientIndex] = -1;
    cout << clientID << endl;
    int buf_len;
    string u = ((userInfo*)userData)->user;
    int idINT = ((userInfo*)userData)->id;
    string id = to_string(idINT);
    string command;
    bool rootUsr;

    if (idINT == 1) {
        rootUsr = true;
    }
    else {
        rootUsr = false;
    }

    while (1)
    {
        char Buff[256] = { 0, };

        while ((buf_len = (recv(clientID, Buff, sizeof(Buff), 0)))) {
            //Print out received message
            cout << "SERVER: Received: " << Buff << endl;
            //Parse message for initial command
            command = buildCommand(Buff);
            //cout << command << endl;
            if (command == "LOGIN") {
                send(clientID, "You are already logged in!", 27, 0);
            }
            else if (command == "BUY") {
                //cout << "Received: BUY!" << endl;
                //send(clientID, "You sent the BUY command!", 26, 0);


                // Checks if the client used the command properly
                if (!extractInfo(Buff, infoArr, command)) {
                    send(clientID, "400 Invalid Command: Missing information\n EX. Command: BUY card_name #_to_buy price userID", sizeof(Buff), 0);
                    cout << "extraction Error" << endl;
                }
                else {
                    // Check if selected user exists in Users table 
                    string sql = "SELECT IIF(EXISTS(SELECT 1 FROM Users WHERE Users.ID=" + (string)id + "), 'PRESENT', 'NOT_PRESENT') result;";
                    rc = sqlite3_exec(db, sql.c_str(), callback, ptr, &zErrMsg);
                    cout << "RC is equal to: " << rc << endl;

                    //Check if SQL executed correctly
                    if (rc != SQLITE_OK) {
                        fprintf(stderr, "SQL error: %s\n", zErrMsg);
                        sqlite3_free(zErrMsg);
                        //send(clientID, "SQL error", 10, 0);
                    }
                    else if (result == "PRESENT") {
                        // USER EXISTS
                        fprintf(stdout, "User Exists in Users Table.\n");

                        // Calculate Card price
                        double cardPrice = stod(infoArr[1]) * stod(infoArr[2]);
                        cout << "Card Price: " << cardPrice << endl;

                        // Get the usd balance of the user
                        sql = "SELECT usd_balance FROM Users WHERE Users.ID=" + (string)id;
                        rc = sqlite3_exec(db, sql.c_str(), callback, ptr, &zErrMsg);
                        string usd_balance = result;
                        cout << "Current User Balance: " << usd_balance << endl;

                        //Check if SQL executed correctly
                        if (rc != SQLITE_OK) {
                            fprintf(stderr, "SQL error: %s\n", zErrMsg);
                            sqlite3_free(zErrMsg);
                            //send(clientID, "SQL error", 10, 0);
                        }
                        else if (stod(usd_balance) >= cardPrice) {
                            // User has enough in balance to make the purchase
                            // Update usd_balance with new balance
                            double difference = stod(usd_balance) - cardPrice;
                            string sql = "UPDATE Users SET usd_balance=" + to_string(difference) + " WHERE ID =" + id + ";";
                            rc = sqlite3_exec(db, sql.c_str(), callback, 0, &zErrMsg);
                            cout << "User Balance Updated: " << difference << endl;

                            //Check if SQL executed correctly
                            if (rc != SQLITE_OK) {
                                fprintf(stderr, "SQL error: %s\n", zErrMsg);
                                sqlite3_free(zErrMsg);
                                //send(clientID, "SQL error", 10, 0);
                            }

                            // Add new record or update record to Card table
                            // Checks if record already exists in Pokemon_cards
                            sql = "SELECT IIF(EXISTS(SELECT 1 FROM Pokemon_cards WHERE Pokemon_cards.card_name='" + infoArr[0] + "' AND Pokemon_cards.owner_id='" + id + "'), 'RECORD_PRESENT', 'RECORD_NOT_PRESENT') result;";
                            rc = sqlite3_exec(db, sql.c_str(), callback, ptr, &zErrMsg);

                            if (rc != SQLITE_OK) {
                                fprintf(stderr, "SQL error: %s\n", zErrMsg);
                                sqlite3_free(zErrMsg);
                                //send(clientID, "SQL error", 10, 0);
                            }
                            else if (result == "RECORD_PRESENT") {
                                // A record exists, so update the record
                                sql = "UPDATE Pokemon_cards SET card_balance= card_balance +" + infoArr[1] + " WHERE Pokemon_cards.card_name='" + infoArr[0] + "' AND Pokemon_cards.owner_id='" + id + "';";
                                rc = sqlite3_exec(db, sql.c_str(), NULL, NULL, &zErrMsg);
                                cout << "Added " << infoArr[1] << " Card to " << infoArr[0] << " for " << id << endl;

                                //Check if SQL executed correctly
                                if (rc != SQLITE_OK) {
                                    fprintf(stderr, "SQL error: %s\n", zErrMsg);
                                    sqlite3_free(zErrMsg);
                                    //send(clientID, "SQL error", 10, 0);
                                }
                            }
                            else {
                                // A record does not exist, so add a record
                                sql = "INSERT INTO Pokemon_cards(card_name, card_balance, owner_id) VALUES ('" + infoArr[0] + "', '" + infoArr[1] + "', '" + id + "');";
                                rc = sqlite3_exec(db, sql.c_str(), NULL, NULL, &zErrMsg);
                                cout << "New record created:\n\tCard Name: " << infoArr[0] << "\n\tCard Balance: " << infoArr[1] << "\n\tUserID: " << id << endl;

                                //Check if SQL executed correctly
                                if (rc != SQLITE_OK) {
                                    fprintf(stderr, "SQL error: %s\n", zErrMsg);
                                    sqlite3_free(zErrMsg);
                                    //send(clientID, "SQL error", 10, 0);
                                }
                            }

                            // Get the new usd_balance
                            sql = "SELECT usd_balance FROM Users WHERE Users.ID=" + id;
                            rc = sqlite3_exec(db, sql.c_str(), callback, ptr, &zErrMsg);
                            usd_balance = result;

                            //Check if SQL executed correctly
                            if (rc != SQLITE_OK) {
                                fprintf(stderr, "SQL error: %s\n", zErrMsg);
                                sqlite3_free(zErrMsg);
                                //send(clientID, "SQL error", 10, 0);
                            }

                            // Get the new card_balance
                            sql = "SELECT card_balance FROM Pokemon_cards WHERE Pokemon_cards.card_name='" + infoArr[0] + "' AND Pokemon_cards.owner_id='" + id + "';";
                            rc = sqlite3_exec(db, sql.c_str(), callback, ptr, &zErrMsg);

                            //Check if SQL executed correctly
                            if (rc != SQLITE_OK) {
                                fprintf(stderr, "SQL error: %s\n", zErrMsg);
                                sqlite3_free(zErrMsg);
                                //send(clientID, "SQL error", 10, 0);
                            }
                            string card_balance = result;

                            // The command completed successfully, return 200 OK, the new usd_balance and new card_balance
                            string tempStr = "200 OK\n   BOUGHT: New balance: " + card_balance + " " + infoArr[0] + ". USD balance $" + usd_balance;
                            send(clientID, tempStr.c_str(), sizeof(buf), 0);
                        }
                        else {
                            cout << "SERVER: Not enough balance. Purchase Aborted." << endl;
                            send(clientID, "400 Invalid Command: not enough USD", sizeof(Buff), 0);
                        }
                    }
                    else {
                        // USER DOES NOT EXIST
                        fprintf(stdout, "SERVER: User Does Not Exist in Users Table. Aborting Buy\n");
                        string tempStr = "400 Invalid Command: user " + id + " does not exist";
                        send(clientID, tempStr.c_str(), sizeof(Buff), 0);
                    }
                }

                cout << "SERVER> Successfully executed BUY command\n\n";
            }
            else if (command == "SELL") {
                cout << " Received: SELL" << endl;
                // Check if the client used the command properly
                if (!extractInfo(Buff, infoArr, command)) {
                    cout << "Invalid Command: Missing information" << endl;
                    send(clientID, "400 Invalid Command: Missing information\n EX. Command: SELL card_name Card_price Card_amnt userID", sizeof(Buff), 0);
                }
                else {
                    // Check if the selected user exists in Users table 
                    string sql = "SELECT IIF(EXISTS(SELECT 1 FROM Users WHERE Users.ID=" + id + "), 'PRESENT', 'NOT_PRESENT') result;";
                    rc = sqlite3_exec(db, sql.c_str(), callback, ptr, &zErrMsg);

                    if (rc != SQLITE_OK) {
                        fprintf(stderr, "SQL error: %s\n", zErrMsg);
                        sqlite3_free(zErrMsg);
                        //send(clientID, "SQL error", 10, 0);
                    }
                    else if (result == "PRESENT") {
                        // Check if the user owns the selected card
                        sql = "SELECT IIF(EXISTS(SELECT 1 FROM Pokemon_cards WHERE Pokemon_cards.card_name='" + infoArr[0] + "' AND Pokemon_cards.owner_id='" + id + "'), 'RECORD_PRESENT', 'RECORD_NOT_PRESENT') result;";
                        rc = sqlite3_exec(db, sql.c_str(), callback, ptr, &zErrMsg);

                        if (rc != SQLITE_OK) {
                            fprintf(stderr, "SQL error: %s\n", zErrMsg);
                            sqlite3_free(zErrMsg);
                            //send(clientID, "SQL error", 10, 0);
                        }
                        else if (result == "RECORD_NOT_PRESENT") {
                            cout << "SERVER: Invalid Card Selection. Aborting Sell\n";
                            send(clientID, "400 Invalid Command: Invalid Card Selection, This User does not Own that Pokemon Card.", sizeof(Buff), 0);
                        }
                        else {
                            // Check if the user has enough of the selected card to sell
                            double numCardsToSell = stod(infoArr[1]);
                            // Get the number of cards the user owns of the selected card
                            sql = "SELECT card_balance FROM Pokemon_cards WHERE Pokemon_cards.card_name='" + infoArr[0] + "' AND Pokemon_cards.owner_id='" + id + "';";
                            rc = sqlite3_exec(db, sql.c_str(), callback, ptr, &zErrMsg);

                            if (rc != SQLITE_OK) {
                                fprintf(stderr, "SQL error: %s\n", zErrMsg);
                                sqlite3_free(zErrMsg);
                                //send(clientID, "SQL error", 10, 0);
                            }

                            double card_balance = stod(result);
                            // Not enough cards in balance to sell
                            if (card_balance < numCardsToSell) {
                                cout << "SERVER: Invalid Quantity. User does not own that many cards. Aborting sell.\n";
                                send(clientID, "400 Invalid Command: User does not have that quantity.", sizeof(Buff), 0);
                            }
                            else {
                                // Get dollar amount to sell
                                double cardPrice = stod(infoArr[1]) * stod(infoArr[2]);

                                /* Update Users table */
                                // Add new amount to user's balance
                                sql = "UPDATE Users SET usd_balance= usd_balance +" + to_string(cardPrice) + " WHERE Users.ID='" + id + "';";
                                rc = sqlite3_exec(db, sql.c_str(), NULL, NULL, &zErrMsg);

                                if (rc != SQLITE_OK) {
                                    fprintf(stderr, "SQL error: %s\n", zErrMsg);
                                    sqlite3_free(zErrMsg);
                                    //send(clientID, "SQL error", 10, 0);
                                }

                                /* Update Pokemon_cards table */
                                // Remove the sold cards from Pokemon_cards
                                sql = "UPDATE Pokemon_cards SET card_balance= card_balance -" + to_string(numCardsToSell) + " WHERE Pokemon_cards.card_name='" + infoArr[0] + "' AND Pokemon_cards.owner_id='" + id + "';";
                                rc = sqlite3_exec(db, sql.c_str(), NULL, NULL, &zErrMsg);

                                if (rc != SQLITE_OK) {
                                    fprintf(stderr, "SQL error: %s\n", zErrMsg);
                                    sqlite3_free(zErrMsg);
                                    //send(clientID, "SQL error", 10, 0);
                                }


                                // Get new usd_balance
                                sql = "SELECT usd_balance FROM Users WHERE Users.ID=" + id;
                                rc = sqlite3_exec(db, sql.c_str(), callback, ptr, &zErrMsg);
                                string usd_balance = result;

                                // Get new card_balance
                                sql = "SELECT card_balance FROM Pokemon_cards WHERE Pokemon_cards.card_name='" + infoArr[0] + "' AND Pokemon_cards.owner_id='" + id + "';";
                                rc = sqlite3_exec(db, sql.c_str(), callback, ptr, &zErrMsg);
                                string card_balance = result;

                                // Sell command completed successfully
                                string tempStr = "200 OK\n   SOLD: New balance: " + card_balance + " " + infoArr[0] + ". USD $" + usd_balance;
                                send(clientID, tempStr.c_str(), sizeof(Buff), 0);
                            }
                        }
                    }
                    else {
                        fprintf(stdout, "SERVER> User Does Not Exist  in Users Table. Aborting Sell.\n");
                        send(clientID, "400 Invalid Command: user does not exist.", sizeof(Buff), 0);
                    }
                }
                cout << "Successfully executed SELL command for User\n\n";
                //send(clientID, "You sent the SELL command!", 27, 0);
            }
            else if (command == "LIST") {
                //cout << "List command!" << endl;
                if (idINT == 1) {
                    //cout << " Received: LIST" << endl;
                    result = "";
                    // List all records in Pokemon_cards table for owner_id = 1
                    string sql = "SELECT * FROM Pokemon_cards";

                    /* Execute SQL statement */
                    rc = sqlite3_exec(db, sql.c_str(), callback, ptr, &zErrMsg);

                    if (rc != SQLITE_OK) {
                        fprintf(stderr, "SQL error: %s\n", zErrMsg);
                        sqlite3_free(zErrMsg);
                        //send(nClient, "SQL error", 10, 0);
                    }

                    string sendStr;

                    if (result == "") {
                        sendStr = "200 OK\n   No records in the Pokemon Cards Database.";
                    }
                    else {
                        sendStr = "200 OK\n   The list of records in the Card database:\nCardID  card_name Card_Amount  Card_Type  Card_Rarity  UserID\n   " + result;
                    }
                    send(clientID, sendStr.c_str(), sizeof(Buff), 0);
                }
                else {
                    //cout << " Received: LIST" << endl;
                    result = "";
                    // List all records in Pokemon_cards table for owner_id = 1
                    string sql = "SELECT * FROM Pokemon_cards WHERE Pokemon_cards.owner_id=" + id;

                    /* Execute SQL statement */
                    rc = sqlite3_exec(db, sql.c_str(), callback, ptr, &zErrMsg);

                    if (rc != SQLITE_OK) {
                        fprintf(stderr, "SQL error: %s\n", zErrMsg);
                        sqlite3_free(zErrMsg);
                        //send(nClient, "SQL error", 10, 0);
                    }

                    string sendStr;

                    if (result == "") {
                        sendStr = "200 OK\n   No records in the Card Database.";
                    }
                    else {
                        sendStr = "200 OK\n   The list of records in the Card database:\nCardID  card_name Card_Amount  Card_Type  Card_Rarity  UserID\n   " + result;
                    }
                    send(clientID, sendStr.c_str(), sizeof(Buff), 0);
                }
                //send(clientID, "You sent the LIST command!", 27, 0);
            }
            else if (command == "BALANCE") {
                //cout << "Received: BALANCE" << endl;
                string sql = "SELECT IIF(EXISTS(SELECT 1 FROM Users WHERE Users.ID=" + id + "), 'PRESENT', 'NOT_PRESENT') result;";

                /* Execute SQL statement */
                rc = sqlite3_exec(db, sql.c_str(), callback, ptr, &zErrMsg);

                if (rc != SQLITE_OK) {
                    fprintf(stderr, "SQL error: %s\n", zErrMsg);
                    sqlite3_free(zErrMsg);
                    //send(nClient, "SQL error", 10, 0);
                }
                else if (result == "PRESENT") {
                    // outputs balance for user 1
                    sql = "SELECT usd_balance FROM Users WHERE Users.ID=" + id;
                    rc = sqlite3_exec(db, sql.c_str(), callback, ptr, &zErrMsg);
                    string usd_balance = result;

                    // get full user name
                    sql = "SELECT first_name FROM Users WHERE Users.ID=" + id;
                    rc = sqlite3_exec(db, sql.c_str(), callback, ptr, &zErrMsg);
                    string user_name = result;

                    sql = "SELECT last_name FROM Users WHERE Users.ID=" + id;
                    rc = sqlite3_exec(db, sql.c_str(), callback, ptr, &zErrMsg);
                    user_name += " " + result;

                    string tempStr = "200 OK\n   Balance for user " + user_name + ": $" + usd_balance;
                    send(clientID, tempStr.c_str(), sizeof(Buff), 0);
                }
                else {
                    cout << "SERVER: User does not exist. Aborting Balance.\n";
                    send(clientID, "User does not exist.", sizeof(Buff), 0);
                }
                //send(clientID, "You sent the BALANCE command!", 30, 0);
            }
            else if (command == "QUIT") {
                //cout << "Quit command!" << endl;
                send(clientID, "200 OK", 27, 0);
                for (int i = 0; i < list.size(); i++) {
                    if (list.at(i).user == u)
                        list.erase(list.begin() + i);
                }
                nClient[clientIndex] = 0;
                close(clientID);
                pthread_exit(userData);

                return userData;
            }
            else if (command == "SHUTDOWN" && rootUsr) {
                send(clientID, "200 OK", 7, 0);
                sqlite3_close(db);
                cout << "Closed Database" << endl;
                close(clientID);
                cout << "Client Terminated Connection:  " << clientID << endl;
                for (int i = 0; i < list.size(); i++) {
                    if (list.at(i).user == u)
                        list.erase(list.begin() + i);
                }
                for (int i = 0; i < list.size(); i++) {
                    //nClient[list.at(i).socket] = 0;
                    close(nClient[list.at(i).socket]);
                    pthread_cancel((list.at(i)).threadAwesome);
                }
                close(nSocket);
                cout << "Socket Closed: " << nSocket << endl;
                pthread_exit(userData);

                //return userData;
                exit(EXIT_SUCCESS);
                //send(clientID, "You sent the SHUTDOWN command!", 31, 0);
            }
            else if (command == "LOGOUT") {
    	   //cout << "Received: LOGOUT" << endl;
                send(clientID, "200 OK", 7, 0);
                for (int i = 0; i < list.size(); i++) {
                    if (list.at(i).user == u)
                        list.erase(list.begin() + i);
                }
                nClient[clientIndex] = clientID;
                pthread_exit(userData);
                return userData;
            }
            else if (command == "DEPOSIT") {
                //cout << "Received: DEPOSIT" << endl;
                string sql = "SELECT IIF(EXISTS(SELECT 1 FROM Users WHERE Users.ID=" + id + "), 'PRESENT', 'NOT_PRESENT') result;";

                /* Execute SQL statement */
                rc = sqlite3_exec(db, sql.c_str(), callback, ptr, &zErrMsg);

                if (rc != SQLITE_OK) {
                    fprintf(stderr, "SQL error: %s\n", zErrMsg);
                    sqlite3_free(zErrMsg);
                    //send(nClient, "SQL error", 10, 0);
                }
                else if (result == "PRESENT") {
                    // outputs balance for user 1
                    string deposit = "";

                    for (int i = (command.length() + 1); i < strlen(Buff); i++) {
                        if (Buff[i] == '\n')
                            break;
                        deposit += Buff[i];
                    }

                    sql = "UPDATE Users SET usd_balance= usd_balance +" + deposit + " WHERE Users.ID='" + id + "';";
                    rc = sqlite3_exec(db, sql.c_str(), callback, ptr, &zErrMsg);

                    sql = "SELECT usd_balance FROM Users WHERE Users.ID=" + id;
                    rc = sqlite3_exec(db, sql.c_str(), callback, ptr, &zErrMsg);
                    string usd_balance = result;

                    // get full user name
                    sql = "SELECT first_name FROM Users WHERE Users.ID=" + id;
                    rc = sqlite3_exec(db, sql.c_str(), callback, ptr, &zErrMsg);
                    string user_name = result;

                    sql = "SELECT last_name FROM Users WHERE Users.ID=" + id;
                    rc = sqlite3_exec(db, sql.c_str(), callback, ptr, &zErrMsg);
                    user_name += " " + result;

                    string tempStr = "200 OK\n   New balance for user " + user_name + ": $" + usd_balance;
                    send(clientID, tempStr.c_str(), sizeof(Buff), 0);
                }
                else {
                    cout << "SERVER> User does not exist. Aborting Balance.\n";
                    send(clientID, "User does not exist.", sizeof(Buff), 0);
                }
                //send(clientID, "You sent the DEPOSIT command!", 30, 0);
            }
            else if (command == "WHO" && rootUsr) {
                //cout << "Who command!" << endl;
                string result = "200 OK\nThe list of the active Users:\n";
                for (int i = 0; i < list.size(); i++) {
                    result += (list.at(i).user + " " + list.at(i).ip + "\n");
                    /*cout << list.at(i).user << endl;
                    cout << list.at(i).ip << endl;*/
                }

                send(clientID, result.c_str(), sizeof(Buff), 0);

                //send(clientID, "You sent the WHO command!", 26, 0);
            }
            else if (command == "LOOKUP") {
                //cout << "Received LOOKUP" << endl;
                string searchTerm = "";
                string sendStr;
                result = "";
                for (int i = (command.length() + 1); i < strlen(Buff); i++) {
                    if (Buff[i] == '\n')
                        break;
                    searchTerm += Buff[i];
                }
                string sql = "SELECT COUNT(card_name) FROM (SELECT * FROM Pokemon_cards WHERE owner_id = " + id + ") WHERE card_name LIKE '%" + searchTerm + "%';";
                rc = sqlite3_exec(db, sql.c_str(), callback, ptr, &zErrMsg);
                string count = result;
                result = "";
                sql = "SELECT card_name, card_balance FROM (SELECT * FROM Pokemon_cards WHERE owner_id = " + id + ") WHERE card_name LIKE '%" + searchTerm + "%'";
                rc = sqlite3_exec(db, sql.c_str(), callback, ptr, &zErrMsg);

                if (result == "") {
                    sendStr = "404 Your search did not match any records";
                }
                else {
                    sendStr = "200 OK\n   Found:" + count + "\ncard_name Card_Amount\n   " + result;
                }
                send(clientID, sendStr.c_str(), sizeof(Buff), 0);

                //send(clientID, "You sent the LOOKUP command!", 29, 0);
            }
            // Default response to Invalid Command
            else {
                cout << "SERVER: INVALID COMMAND" << endl;
                send(clientID, "400 Invalid Command", 20, 0);
            }
        }

        for (int i = 0; i < list.size(); i++) {
            if (list.at(i).user == u)
                list.erase(list.begin() + i);
        }

        cout << endl << "Error at client socket\n";
        nClient[clientIndex] = 0;
        close(clientID);
        pthread_exit(userData);

        return userData;
    }
}

bool extractInfo(char line[], string info[], string command) {
    int l = command.length();
    int spaceLocation = l + 1;

    for (int i = 0; i < 3; i++) {
        info[i] = "";

        // Parses the information
        for (int j = spaceLocation; j < strlen(line); j++) {

            if (line[j] == 0)
                return false;
            if (line[j] == ' ')
                break;
            if (line[j] == '\n')
                break;
            info[i] += line[j];

            // Makes sure that only numbers are entered into the array for index 1, 2 and 3
            if (i > 0) {
                if (((int)line[j] > 57 || (int)line[j] < 46) && (int)line[j] != 47)
                    return false;
            }
        }
        if (info[i] == "") {
            fill_n(info, 3, 0);
            return false;
        }

        spaceLocation += info[i].length() + 1;

    }

    return true;

}

static int callback(void* ptr, int count, char** data, char** azColName) {

    if (count == 1) {
        result = data[0];
    }
    else if (count > 1) {
        for (int i = 0; i < count; i++) {

            if (result == "") {
                result = data[i];
            }
            else {
                result = result + " " + data[i];
            }

            // new line btwn every record
            if (i == 3)
            {
                result += "\n  ";
            }

        }
    }
    return 0;
}

void HandleNewConnection()
{
    // nNewClient will be a new file descriptor and now the client communication will take place 
    // using this file descriptor/socket only
    int nNewClient = accept(nSocket, (struct sockaddr*)&srv, &addr_len);

    //If you accept the value in second parameter, then it will be 
    if (nNewClient < 0) {
        perror("Error during accepting connection");
    }
    else {

        void* temp = &nNewClient;
        cout << nNewClient << endl;

        int nIndex;
        for (nIndex = 0; nIndex < 5; nIndex++)
        {
            if (nClient[nIndex] == 0)
            {
                nClient[nIndex] = nNewClient;
                if (nNewClient > nMaxFd)
                {
                    nMaxFd = nNewClient + 1;
                }
                break;
            }
        }

        if (nIndex == 5)
        {
            cout << endl << "Server busy. Cannot accept anymore connections";
        }

        printf("New connection, socket fd is %d, ip is: %s, port: %d\n", nNewClient, inet_ntoa(srv.sin_addr), ntohs(srv.sin_port));
        send(nClient[nIndex], "You have successfully connected to the server!", 47, 0);
    }

}

void HandleDataFromClient()
{
    string command;
    temp = &u;

    for (int nIndex = 0; nIndex < 5; nIndex++)
    {
        if (nClient[nIndex] > 0)
        {
            if (FD_ISSET(nClient[nIndex], &fr))
            {
                //Read the data from client
                char sBuff[256] = { 0, };
                int nRet = recv(nClient[nIndex], sBuff, 256, 0);
                if (nRet < 0)
                {
                    //This happens when client closes connection abruptly
                    cout << endl << "Error at client socket";
                    for (int i = 0; i < list.size(); i++) {
                        if (list.at(i).user == u.user)
                            list.erase(list.begin() + i);
                    }
                    close(nClient[nIndex]);
                    nClient[nIndex] = 0;
                }
                else
                {

                    command = buildCommand(sBuff);
                    cout << command << endl;

                    if (command == "LOGIN") {
                        string info = extractInfo(sBuff, command);
                        loggedUser tempStruct;
                        u.user = info;
                        int passLength = command.length() + info.length();
                        string passInfo = getPassword(sBuff, passLength);

                        u.password = passInfo;
                        u.socket = nIndex;
                        tempStruct.socket = nIndex;
                        struct sockaddr_in client_addr;
                        socklen_t addrlen;


                        cout << "Assigned user info. Username: " << info << " Socket Index: " << u.socket << endl;

                        string commandSql = "SELECT IIF(EXISTS(SELECT * FROM Users WHERE user_name = '" + info + "' AND password = '" + passInfo + "') , 'USER_PRESENT', 'USER_NOT_PRESENT') result;";
                        sql = commandSql.c_str();
                        sqlite3_exec(db, sql, callback, 0, &zErrMsg);



                        if (result == "USER_PRESENT") {
                            cout << "Logging in... " << endl;
                            send(nClient[nIndex], "200 OK", 7, 0);

                            getpeername(nClient[nIndex], (struct sockaddr*)&client_addr, &addrlen);
                            tempStruct.ip = "";
                            cout << "IP address: " << inet_ntoa(client_addr.sin_addr) << endl;
                            //for (int i = 0; i < sizeof(inet_ntoa(srv.sin_addr)) + 1; i++) {
                            tempStruct.ip = inet_ntoa(client_addr.sin_addr);
                            cout << tempStruct.ip << endl;
                            //}
                            tempStruct.user = u.user;

                            list.push_back(tempStruct);

                            commandSql = "SELECT ID FROM Users WHERE user_name = '" + info + "' AND password = '" + passInfo + "'";
                            sql = commandSql.c_str();
                            sqlite3_exec(db, sql, callback, 0, &zErrMsg);
                            u.id = stoi(result);

                            pthread_create(&(list.at(list.size() - 1).threadAwesome), NULL, serverCommands, temp);

                        }
                        else {
                            cout << "Username or Password Invalid!" << endl;
                        }
                    }
                    else if (command == "QUIT") {
                        cout << "Quit command!" << endl;
                        send(nClient[nIndex], "200 OK", 27, 0);
                        close(nClient[nIndex]);
                        nClient[nIndex] = 0;
                    }
                    else if (command == "BUY" || command == "SELL" || command == "LOOKUP" || command == "DEPOSIT" || command == "SHUTDOWN" || command == "LIST" || command == "WHO" || command == "BALANCE" || command == "LOGOUT" || command == "NULL") {
                        send(nClient[nIndex], "Guest Users can only use the login and quit commands.", 54, 0);
                    }
                    else {
                        cout << endl << "Received data from:" << nClient[nIndex] << "[Message: " << sBuff << " size of array: " << strlen(sBuff) << "] Error 400" << endl;
                        send(nClient[nIndex], "Command does not exist.", 24, 0);
                    }
                    break;
                }
            }
        }
    }
}

string getPassword(char line[], int n) {

    int spaceLocation = n + 2;
    int i = spaceLocation;
    string info = "";

    while (line[i] != '\n') {
        if (line[i] == 0)
            return "";
        if (line[i] == ' ')
            return info;
        info += line[i];
        i++;
    }

    return info;
}

