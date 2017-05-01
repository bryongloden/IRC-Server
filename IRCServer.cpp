
const char * usage =
"                                                               \n"
"IRCServer:                                                   \n"
"                                                               \n"
"Simple server program used to communicate multiple users       \n"
"                                                               \n"
"To use it in one window type:                                  \n"
"                                                               \n"
"   IRCServer <port>                                          \n"
"                                                               \n"
"Where 1024 < port < 65536.                                     \n"
"                                                               \n"
"In another window type:                                        \n"
"                                                               \n"
"   telnet <host> <port>                                        \n"
"                                                               \n"
"where <host> is the name of the machine where talk-server      \n"
"is running. <port> is the port number you used when you run    \n"
"daytime-server.                                                \n"
"                                                               \n";

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
using namespace std;

#include "IRCServer.h"

int QueueLength = 5;

//test

int
IRCServer::open_server_socket(int port) {

	// Set the IP address and port for this server
	struct sockaddr_in serverIPAddress;
	memset( &serverIPAddress, 0, sizeof(serverIPAddress) );
	serverIPAddress.sin_family = AF_INET;
	serverIPAddress.sin_addr.s_addr = INADDR_ANY;
	serverIPAddress.sin_port = htons((u_short) port);

	// Allocate a socket
	int masterSocket =  socket(PF_INET, SOCK_STREAM, 0);
	if ( masterSocket < 0) {
		perror("socket");
		exit( -1 );
	}

	// Set socket options to reuse port. Otherwise we will
	// have to wait about 2 minutes before reusing the sae port number
	int optval = 1;
	int err = setsockopt(masterSocket, SOL_SOCKET, SO_REUSEADDR,
			     (char *) &optval, sizeof( int ) );

	// Bind the socket to the IP address and port
	int error = bind( masterSocket,
			  (struct sockaddr *)&serverIPAddress,
			  sizeof(serverIPAddress) );
	if ( error ) {
		perror("bind");
		exit( -1 );
	}

	// Put socket in listening mode and set the
	// size of the queue of unprocessed connections
	error = listen( masterSocket, QueueLength);
	if ( error ) {
		perror("listen");
		exit( -1 );
	}

	return masterSocket;
}

void
IRCServer::runServer(int port)
{
	int masterSocket = open_server_socket(port);

	initialize();

	while ( 1 ) {

		// Accept incoming connections
		struct sockaddr_in clientIPAddress;
		int alen = sizeof( clientIPAddress );
		int slaveSocket = accept( masterSocket,
					  (struct sockaddr *)&clientIPAddress,
					  (socklen_t*)&alen);

		if ( slaveSocket < 0 ) {
			perror( "accept" );
			exit( -1 );
		}

		// Process request.
		processRequest( slaveSocket );
	}
}

int
main( int argc, char ** argv )
{
	// Print usage if not enough arguments
	if ( argc < 2 ) {
		fprintf( stderr, "%s", usage );
		exit( -1 );
	}

	// Get the port from the arguments
	int port = atoi( argv[1] );

	IRCServer ircServer;

	// It will never return
	ircServer.runServer(port);

}

//
// Commands:
//   Commands are started y the client.
//
//   Request: ADD-USER <USER> <PASSWD>\r\n
//   Answer: OK\r\n or DENIED\r\n
//
//   REQUEST: GET-ALL-USERS <USER> <PASSWD>\r\n
//   Answer: USER1\r\n
//            USER2\r\n
//            ...
//            \r\n
//
//   REQUEST: CREATE-ROOM <USER> <PASSWD> <ROOM>\r\n
//   Answer: OK\n or DENIED\r\n
//
//   Request: LIST-ROOMS <USER> <PASSWD>\r\n
//   Answer: room1\r\n
//           room2\r\n
//           ...
//           \r\n
//
//   Request: ENTER-ROOM <USER> <PASSWD> <ROOM>\r\n
//   Answer: OK\n or DENIED\r\n
//
//   Request: LEAVE-ROOM <USER> <PASSWD>\r\n
//   Answer: OK\n or DENIED\r\n
//
//   Request: SEND-MESSAGE <USER> <PASSWD> <MESSAGE> <ROOM>\n
//   Answer: OK\n or DENIED\n
//
//   Request: GET-MESSAGES <USER> <PASSWD> <LAST-MESSAGE-NUM> <ROOM>\r\n
//   Answer: MSGNUM1 USER1 MESSAGE1\r\n
//           MSGNUM2 USER2 MESSAGE2\r\n
//           MSGNUM3 USER2 MESSAGE2\r\n
//           ...\r\n
//           \r\n
//
//    REQUEST: GET-USERS-IN-ROOM <USER> <PASSWD> <ROOM>\r\n
//    Answer: USER1\r\n
//            USER2\r\n
//            ...
//            \r\n
//

void
IRCServer::processRequest( int fd )
{
	// Buffer used to store the comand received from the client
	const int MaxCommandLine = 1024;
	char commandLine[ MaxCommandLine + 1 ];
	int commandLineLength = 0;
	int n;

	// Currently character read
	unsigned char prevChar = 0;
	unsigned char newChar = 0;

	//
	// The client should send COMMAND-LINE\n
	// Read the name of the client character by character until a
	// \n is found.
	//

	// Read character by character until a \n is found or the command string is full.
	while ( commandLineLength < MaxCommandLine &&
		read( fd, &newChar, 1) > 0 ) {

		if (newChar == '\n' && prevChar == '\r') {
			break;
		}

		commandLine[ commandLineLength ] = newChar;
		commandLineLength++;

		prevChar = newChar;
	}

	// Add null character at the end of the string
	// Eliminate last \r
	commandLineLength--;
        commandLine[ commandLineLength ] = 0;

	printf("RECEIVED: %s\n", commandLine);

	printf("The commandLine has the following format:\n");
	printf("COMMAND <user> <password> <arguments>. See below.\n");
	printf("You need to separate the commandLine into those components\n");
	printf("For now, command, user, and password are hardwired.\n");

	char * tempCommandLine = commandLine;
	char * command = 0;
	char * user = 0;
	char * password = 0;
	char * args = 0;

	command = strsep(&tempCommandLine, " ");
	user = strsep(&tempCommandLine, " ");
	password = strsep(&tempCommandLine, " ");
	args = strsep(&tempCommandLine, "\n");

	printf("command=%s\n", command);
	printf("user=%s\n", user);
	printf( "password=%s\n", password );
	printf("args=%s\n", args);

	if (password == 0) {
		const char * msg =  "DENIED\r\n";
		write(fd, msg, strlen(msg));
		close(fd);
		return;
	}

	if (!checkPassword(fd, user, password)) {
		if (!strcmp(command, "ADD-USER")) {
			addUser(fd, user, password, args);
			close(fd);
			return;
		}
		const char * msg =  "DENIED\r\n";
		const char * errMsg = "ERROR (Wrong password)\r\n";
		//write(fd, msg, strlen(msg));
		write(fd, errMsg, strlen(errMsg));
		close(fd);
		return;
	}

	if (!strcmp(command, "CREATE-ROOM")) {
		createRoom(fd, user, password, args);
	}
	else if (!strcmp(command, "ENTER-ROOM")) {
		enterRoom(fd, user, password, args);
	}
	else if (!strcmp(command, "LIST-ROOMS")) {
		listRooms(fd, user, password, args);
	}
	else if (!strcmp(command, "LEAVE-ROOM")) {
		leaveRoom(fd, user, password, args);
	}
	else if (!strcmp(command, "SEND-MESSAGE")) {
		sendMessage(fd, user, password, args);
	}
	else if (!strcmp(command, "GET-MESSAGES")) {
		getMessages(fd, user, password, args);
	}
	else if (!strcmp(command, "GET-USERS-IN-ROOM")) {
		getUsersInRoom(fd, user, password, args);
	}
	else if (!strcmp(command, "GET-ALL-USERS")) {
		getAllUsers(fd, user, password, args);
	}
	else {
		const char * msg =  "UNKNOWN COMMAND\r\n";
		write(fd, msg, strlen(msg));
	}

	// Send OK answer
	//const char * msg =  "OK\n";
	//write(fd, msg, strlen(msg));

	close(fd);
}

void
IRCServer::initialize()
{
	system("rm -f *.rooms");
	system("rm -f *.roomstemp");
	system("rm -f password.txt");
	system("rm -f room.txt");
	system("rm -f *.messages");
	system("rm -f *.message");
	// Open password file

	// Initialize users in room

	// Initalize message list

}

bool
IRCServer::checkPassword(int fd, const char * user, const char * password) {
	// Here check the password
	string fileUser;
	string filePassword;

	ifstream myFile;
	myFile.open (PASSWORD_FILE);
	if (myFile.is_open()) {
		while (!myFile.eof()) {
			getline(myFile, fileUser);
			if (!fileUser.empty()) {
				getline(myFile, filePassword);
			 		if (strcmp(fileUser.c_str(), user)==0 && strcmp(filePassword.c_str(), password)==0) {
						return true;
					}
			}
		}
		myFile.close();
	}
	else return false;

	return false;
}

void
IRCServer::createRoom(int fd, const char * user, const char * password, const char * args)
{
	const char * msgOK =  "OK\r\n";
	const char * msgDENIED = "DENIED\r\n";
	string fileRoomName;

	ifstream myFileIn;
	myFileIn.open(ROOM_FILE);
	if (myFileIn.is_open()) {
		while (!myFileIn.eof()) {
			getline(myFileIn, fileRoomName);
			if (!fileRoomName.empty()) {
			 		if (strcmp(fileRoomName.c_str(), args)==0) {
						write(fd, msgDENIED, strlen(msgDENIED));
						myFileIn.close();
						return;
					}
			}
		}
		myFileIn.close();
	}

	ofstream myFileOut;
	myFileOut.open (ROOM_FILE, ios::app);
	if (myFileOut.is_open()) {
		myFileOut << args << "\n\n";
		myFileOut.close();

		write(fd, msgOK, strlen(msgOK));
	}
	else write(fd, msgDENIED, strlen(msgDENIED));

	ofstream myrooms;
	string rooms(args);
	rooms.append(".messages");
	myrooms.open (rooms.c_str());
//	if (myFileOut.is_open()){

//	}
	return;
}

void
IRCServer::addUser(int fd, const char * user, const char * password, const char * args)
{
	// Here add a new user. For now always return OK.

	const char * msgOK =  "OK\r\n";
	const char * msgDENIED = "DENIED\r\n";

	ofstream myFile;
	myFile.open (PASSWORD_FILE, ios::app);
	if (myFile.is_open()) {
		myFile << user << "\n";
		myFile << password << "\n\n";
		myFile.close();

		ofstream userFile;
		string userName(user);
		userName.append(".rooms");

		userFile.open(userName.c_str());

		write(fd, msgOK, strlen(msgOK));
	}
	else write(fd, msgDENIED, strlen(msgDENIED));

	return;
}

void
IRCServer::enterRoom(int fd, const char * user, const char * password, const char * args)
{
	const char * msgOK =  "OK\r\n";
	const char * msgDENIED = "DENIED\r\n";
	const char * msgERROR = "ERROR (No room)\r\n";
	string fileRoom;

	bool isInRoom = false;

	ifstream userFile;
	string userName(user);
	userName.append(".rooms");
	string roomName;

	userFile.open(userName.c_str());
	if (userFile.is_open()) {
		while(!userFile.eof()) {
			getline(userFile, roomName);
			if (!roomName.empty()) {
				if (0 == strcmp(roomName.c_str(), args)) {
					isInRoom = true;
				}
			}
		}
	}

	if (isInRoom) {
		write(fd, msgOK, strlen(msgOK));
		return;
	}

	ifstream myFile;
	myFile.open (ROOM_FILE);
	if (myFile.is_open()) {
		while (!myFile.eof()) {
			getline(myFile, fileRoom);
			if (!fileRoom.empty()) {
				if (strcmp(fileRoom.c_str(), args)==0) {
						ofstream userFile;
						string userName(user);
						userName.append(".rooms");

						userFile.open(userName.c_str(), ios::app);
						userFile << fileRoom.c_str() << "\n";

						userFile.close();
						myFile.close();

						write(fd, msgOK, strlen(msgOK));
						return;
				}
			}
		}
	}

	write(fd, msgERROR, strlen(msgERROR));
	return;
}

void
IRCServer::leaveRoom(int fd, const char * user, const char * password, const char * args)
{
	const char * msgOK =  "OK\r\n";
	const char * msgDENIED = "DENIED\r\n";
	const char * msgDEBUG = "DEBUG #0\r\n";
	const char * msgERROR = "ERROR (No user in room)\r\n";
	bool isInRoom = false;

	string fileRoom;
	string roomName;

	ifstream myFile;
	myFile.open (ROOM_FILE);
	if (myFile.is_open()) {
		while (!myFile.eof()) {
			getline(myFile, fileRoom);
			if (!fileRoom.empty()) {
				if (strcmp(fileRoom.c_str(), args)==0) {
						ifstream userFile;
						ofstream tempFile;
						string userName(user);
						string tempName(user);
						userName.append(".rooms");
						tempName.append(".roomstemp");

						userFile.open(userName.c_str());
						tempFile.open(tempName.c_str());
						//write(fd, "DEBUG #1\r\n", strlen(msgDEBUG));
						while (!userFile.eof()) {
							//write(fd, "DEBUG #2\r\n", strlen(msgDEBUG));
							getline(userFile, roomName);
							if (!roomName.empty()) {
								//write(fd, "DEBUG #3\r\n", strlen(msgDEBUG));
								if (strcmp(roomName.c_str(), args) != 0) {
									//write(fd, "DEBUG #4\r\n", strlen(msgDEBUG));
									tempFile << roomName.c_str() << "\n";
								} else {
									isInRoom = true;
								}
							}
						}
						// userFile << "\n";

						userFile.close();
						tempFile.close();
						myFile.close();

						ofstream userFile_2;
						ifstream tempFile_2;
						string userName_2(user);
						string tempName_2(user);
						userName_2.append(".rooms");
						tempName_2.append(".roomstemp");

						userFile_2.open(userName_2.c_str());
						tempFile_2.open(tempName_2.c_str());
						while (!tempFile_2.eof())
						{
							getline(tempFile_2, roomName);
							userFile_2 << roomName.c_str() << "\n";
						}

						userFile_2.close();
						tempFile_2.close();

						if (isInRoom == true) {
							write(fd, msgOK, strlen(msgOK));
						} else {
							write(fd, msgERROR, strlen(msgERROR));
						}
						return;
				}
			}
		}
		write(fd, msgDENIED, strlen(msgDENIED));
	}
	return;
}

void
IRCServer::sendMessage(int fd, const char * user, const char * password, const char * args)
{
	const char * msgOK =  "OK\r\n";
	const char * msgDENIED = "DENIED\r\n";
	const char * msgERROR = "ERROR (user not in room)\r\n";
	// write(fd, msgDENIED, strlen(msgDENIED));
	//write(fd, msgOK, strlen(msgOK));

	char * tempCommandLine = strdup(args);
	//tempCommandLine = strArgs.c_str();
	char * arg1;
	char * arg2;

	arg1 = strsep(&tempCommandLine, " ");
	arg2 = strsep(&tempCommandLine, "\n");
	// Check if user is in the room first
	const char * msgDEBUG = "DEBUG #0\r\n";
	string fileUser;
	string filePassword;
	int howManyUsers = 0;
	vector<string> users;
	bool isInRoom = false;

	ifstream myFile;
	myFile.open (PASSWORD_FILE);
	if (myFile.is_open()) {
		while (!myFile.eof()) {
			getline(myFile, fileUser);
			if (!fileUser.empty()) {
				getline(myFile, filePassword);
				howManyUsers++;
				users.push_back(fileUser);
				//write(fd, "DEBUG #1\r\n", strlen(msgDEBUG));
				//write(fd, fileUser.c_str(), fileUser.length());
				//write(fd, "\r\n", strlen("\r\n"));
				//write(fd, filePassword.c_str(), filePassword.length());
				//write(fd, "\r\n", strlen("\r\n"));
			}
		}
		myFile.close();
		sort(users.begin(), users.end());
		int i=0;
		for (i=0; i<howManyUsers; i++) {
			//write(fd, "DEBUG #1\r\n", strlen(msgDEBUG));
			//write(fd, users[i].c_str(), users[i].length());
			//write(fd, "\r\n", strlen("\r\n"));
			ifstream userFile;
			string userName;
			userName = users[i];
			userName.append(".rooms");
			string roomName;
			//write(fd, "DEBUG #1\r\n", strlen(msgDEBUG));
			//write(fd, userName.c_str(), strlen(userName.c_str()));
			//write(fd, "\r\n", strlen("\r\n"));
			userFile.open(userName.c_str());
			if (userFile.is_open()) {
				// write(fd, "DEBUG #1\r\n", strlen(msgDEBUG));
				while (!userFile.eof()) {
					getline(userFile, roomName);
					if (!roomName.empty()) {
					// 	write(fd, arg1, strlen(arg1));
					// 	write(fd, "\r\n^^^^^^^^^^^^^^\r\n", strlen("\r\n^^^^^^^^^^^^^^\r\n"));
					// 	write(fd, roomName.c_str(), strlen(roomName.c_str()));
					// 	write(fd, "\r\n^^^^^^^^^^^^^^\r\n", strlen("\r\n^^^^^^^^^^^^^^\r\n"));
					// 	write(fd, user, strlen(user));
					// 	write(fd, "\r\n^^^^^^^^^^^^^^\r\n", strlen("\r\n^^^^^^^^^^^^^^\r\n"));
					// 	write(fd, userName.c_str(), strlen(users[i].c_str()));
					// 	write(fd, "\r\n^^^^^^^^^^^^^^\r\n", strlen("\r\n^^^^^^^^^^^^^^\r\n"));
						if (0 == strcmp(roomName.c_str(), arg1) && 0 == strcmp(users[i].c_str(), user)) {
							//write(fd, "DEBUG #1\r\n", strlen(msgDEBUG));
							isInRoom = true;
						//write(fd, users[i].c_str(), users[i].length());

						//write(fd, roomName.c_str(), roomName.length());
						//write(fd, "\r\n", strlen("\r\n"));
						}
					}
				}
			}
		//	write(fd, msgOK, strlen(msgOK));
			userFile.close();
		}
		//write(fd, "\r\n", strlen("\r\n"));
	}
	else {
		//write(fd, "DEBUG #1\r\n", strlen(msgDEBUG));
		write(fd, msgDENIED, strlen(msgDENIED));
		return;
	}
	if (isInRoom != true) {
		//write(fd, "DEBUG #2\r\n", strlen(msgDEBUG));
		write(fd, msgERROR, strlen(msgERROR));
		return;
	}

	//const char * msgOK =  "OK\r\n";
	//const char * msgDENIED = "DENIED\r\n";
	// write(fd, msgDENIED, strlen(msgDENIED));
	fstream roomopen;
	string roomname(arg1);
	roomname.append(".messages");
	roomopen.open(roomname.c_str());
	int currentlinenumber=0;
	string stringofline;
// 	write(fd, "DEBUG #0.8\r\n DEBUG #1\r\n",strlen(msgERROR)+strlen(msgERROR+2));
// 	write(fd, "DEBUG #1.1\r\n",strlen(msgERROR+2));
// 	write(fd, "OK\r\n",strlen(msgOK));
// 	while(!roomopen.eof()){
// 		write(fd, "DEBUG #2\r\n",strlen(msgERROR));
// 		getline(roomopen,stringofline);
// 		currentlinenumber++;
// //		if (!stringofline.empty){
//
// //		}
// 	}

	while(getline(roomopen, stringofline)) {
		if (stringofline.empty()) {
			break;
		}
		else {
			currentlinenumber++;
		}
	}
	roomopen.close();
	roomopen.open(roomname.c_str(), ios::app);
	//write(fd, "DEBUG #3\r\n",strlen(msgERROR));
	if (roomopen.is_open()){
		//write(fd, "DEBUG #4\r\n",strlen(msgERROR));
		roomopen << currentlinenumber << " " << user << " " << arg2 << "\n";
		roomopen.close();
//		while(roomopen!=EOF){
//			getline(roomopen, roomname);
	}
	write(fd, msgOK, strlen(msgOK));
//			write(fd, arg2, strlen(arg2));
//	write(fd, msgOK, strlen(msgOK));
	return;
}

void
IRCServer::getMessages(int fd, const char * user, const char * password, const char * args)
{
	const char * msgOK =  "OK\r\n";
	const char * msgDENIED = "DENIED\r\n";
	const char * msgERROR = "ERROR (User not in room)\r\n";
	const char * msgNO = "NO-NEW-MESSAGES\r\n";
	// write(fd, msgDENIED, strlen(msgDENIED));
	//write(fd, msgOK, strlen(msgOK));

	char * tempCommandLine = strdup(args);
	//tempCommandLine = strArgs.c_str();
	char * arg1 = 0;
	char * arg2 = 0;

	arg1 = strsep(&tempCommandLine, " ");
	arg2 = strsep(&tempCommandLine, "\n");
	// Check if user is in the room first
	const char * msgDEBUG = "DEBUG #0\r\n";
	string fileUser;
	string filePassword;
	int howManyUsers = 0;
	vector<string> users;
	bool isInRoom = false;

	ifstream myFile;
	myFile.open (PASSWORD_FILE);
	if (myFile.is_open()) {
		while (!myFile.eof()) {
			getline(myFile, fileUser);
			if (!fileUser.empty()) {
				getline(myFile, filePassword);
				howManyUsers++;
				users.push_back(fileUser);
				//write(fd, "DEBUG #1\r\n", strlen(msgDEBUG));
				//write(fd, fileUser.c_str(), fileUser.length());
				//write(fd, "\r\n", strlen("\r\n"));
				//write(fd, filePassword.c_str(), filePassword.length());
				//write(fd, "\r\n", strlen("\r\n"));
			}
		}
		myFile.close();
		sort(users.begin(), users.end());
		int i=0;
		for (i=0; i<howManyUsers; i++) {
			//write(fd, "DEBUG #1\r\n", strlen(msgDEBUG));
			//write(fd, users[i].c_str(), users[i].length());
			//write(fd, "\r\n", strlen("\r\n"));
			ifstream userFile;
			string userName;
			userName = users[i];
			userName.append(".rooms");
			string roomName;
			//write(fd, "DEBUG #1\r\n", strlen(msgDEBUG));
			//write(fd, userName.c_str(), strlen(userName.c_str()));
			//write(fd, "\r\n", strlen("\r\n"));
			userFile.open(userName.c_str());
			if (userFile.is_open()) {
				// write(fd, "DEBUG #1\r\n", strlen(msgDEBUG));
				while (!userFile.eof()) {
					getline(userFile, roomName);
					if (!roomName.empty()) {
						//write(fd, arg2, strlen(arg2));
						//write(fd, "^^^^^^^^^^^^^^\r\n", strlen("^^^^^^^^^^^^^^\r\n"));
						if (0 == strcmp(roomName.c_str(), arg2) && 0 == strcmp(users[i].c_str(), user)) {
							isInRoom = true;
						//write(fd, users[i].c_str(), users[i].length());

						//write(fd, roomName.c_str(), roomName.length());
						//write(fd, "\r\n", strlen("\r\n"));
						}
					}
				}
			}
			userFile.close();


		}
		//write(fd, "\r\n", strlen("\r\n"));
	}
	else {
		//write(fd, "DEBUG #1\r\n", strlen(msgDEBUG));
		write(fd, msgDENIED, strlen(msgDENIED));
		return;
	}
	if (isInRoom != true) {
		//write(fd, "DEBUG #2\r\n", strlen(msgDEBUG));
		write(fd, msgERROR, strlen(msgERROR));
		return;
	}

	ifstream roomopen;
	string roomname(arg2);
	roomname.append(".messages");
	roomopen.open(roomname.c_str());
	int currentlinenumber=0;
	string stringofline;
// 	write(fd, "DEBUG #0.8\r\n DEBUG #1\r\n",strlen(msgERROR)+strlen(msgERROR+2));
// 	write(fd, "DEBUG #1.1\r\n",strlen(msgERROR+2));
// 	write(fd, "OK\r\n",strlen(msgOK));
// 	while(!roomopen.eof()){
// 		write(fd, "DEBUG #2\r\n",strlen(msgERROR));
// 		getline(roomopen,stringofline);
// 		currentlinenumber++;
// //		if (!stringofline.empty){
//
// //		}
// 	}
	int fileMessageNum = atoi(arg1)+1;
	bool didWrite = false;
	while(getline(roomopen, stringofline)) {
		if (stringofline.empty()) {
			break;
		}
		else {
			stringofline.append("\r\n");
			if (currentlinenumber >= fileMessageNum) {
				write(fd, stringofline.c_str(), strlen(stringofline.c_str()));
				didWrite = true;
			}
			currentlinenumber++;
		}
	}
	if(didWrite) {
		write(fd, "\r\n", strlen("\r\n"));
	} else {
		write(fd, msgNO, strlen(msgNO));
	}
	roomopen.close();
// 	roomopen.open(roomname.c_str(), ios::app);
// 	write(fd, "DEBUG #3\r\n",strlen(msgERROR));
// 	if (roomopen.is_open()){
// 		write(fd, "DEBUG #4\r\n",strlen(msgERROR));
// 		roomopen << currentlinenumber << " " << user << " " << arg2 << "\n";
// 		roomopen.close();
// //		while(roomopen!=EOF){
// //			getline(roomopen, roomname);
// 	}

//			write(fd, arg2, strlen(arg2));
//	write(fd, msgOK, strlen(msgOK));
	return;

	//write(fd, msgNO, strlen(msgNO));

}

void
IRCServer::getUsersInRoom(int fd, const char * user, const char * password, const char * args)
{
	const char * msgOK =  "OK\r\n";
	const char * msgDENIED = "DENIED\r\n";
	//write(fd, msgDENIED, strlen(msgDENIED));

	const char * msgDEBUG = "DEBUG #0\r\n";
	string fileUser;
	string filePassword;
	int howManyUsers = 0;
	vector<string> users;

	ifstream myFile;
	myFile.open (PASSWORD_FILE);
	if (myFile.is_open()) {
		while (!myFile.eof()) {
			getline(myFile, fileUser);
			if (!fileUser.empty()) {
				getline(myFile, filePassword);
				howManyUsers++;
				users.push_back(fileUser);
				//write(fd, "DEBUG #1\r\n", strlen(msgDEBUG));
				//write(fd, fileUser.c_str(), fileUser.length());
				//write(fd, "\r\n", strlen("\r\n"));
				//write(fd, filePassword.c_str(), filePassword.length());
				//write(fd, "\r\n", strlen("\r\n"));
			}
		}
		myFile.close();
		sort(users.begin(), users.end());
		for (int i=0; i<howManyUsers; i++) {
			//write(fd, "DEBUG #1\r\n", strlen(msgDEBUG));
			//write(fd, users[i].c_str(), users[i].length());
			//write(fd, "\r\n", strlen("\r\n"));
			ifstream userFile;
			string userName;
			userName = users[i];
			userName.append(".rooms");
			string roomName;
      //write(fd, "DEBUG #1\r\n", strlen(msgDEBUG));
			//write(fd, userName.c_str(), strlen(userName.c_str()));
			//write(fd, "\r\n", strlen("\r\n"));
			userFile.open(userName.c_str());
			if (userFile.is_open()) {
				// write(fd, "DEBUG #1\r\n", strlen(msgDEBUG));
				while (!userFile.eof()) {
					getline(userFile, roomName);
					if (!roomName.empty()) {
						if (0 == strcmp(roomName.c_str(), args)) {

						write(fd, users[i].c_str(), users[i].length());

						//write(fd, roomName.c_str(), roomName.length());
						write(fd, "\r\n", strlen("\r\n"));
						}
					}
				}
			}
			userFile.close();


		}
		write(fd, "\r\n", strlen("\r\n"));
	}
	else write(fd, msgDENIED, strlen(msgDENIED));

}

void
IRCServer::getAllUsers(int fd, const char * user, const char * password,const  char * args)
{
	// const char * msgOK =  "OK\r\n";
	// const char * msgDENIED = "DENIED\r\n";
	// string fileUser;
	// string filePassword;
	//
	// ifstream myFile;
	// myFile.open (PASSWORD_FILE);
	// if (myFile.is_open()) {
	// 	while (!myFile.eof()) {
	// 		getline(myFile, fileUser);
	// 		if (!fileUser.empty()) {
	// 			getline(myFile, filePassword);
	// 		 	write(fd, fileUser.c_str(), fileUser.length());
	// 			write(fd, "\r\n", strlen("\r\n"));
	// 		 	//write(fd, filePassword.c_str(), filePassword.length());
	// 			//write(fd, "\r\n", strlen("\r\n"));
	// 		}
	// 	}
	// 	myFile.close();
	// }
	// else write(fd, msgDENIED, strlen(msgDENIED));

	const char * msgOK =  "OK\r\n";
	const char * msgDENIED = "DENIED\r\n";
	const char * msgDEBUG = "DEBUG #0\r\n";
	string fileUser;
	string filePassword;
	int howManyUsers = 0;
	vector<string> users;

	ifstream myFile;
	myFile.open (PASSWORD_FILE);
	if (myFile.is_open()) {
		while (!myFile.eof()) {
			getline(myFile, fileUser);
			if (!fileUser.empty()) {
				getline(myFile, filePassword);
				howManyUsers++;
				users.push_back(fileUser);
				//write(fd, fileUser.c_str(), fileUser.length());
				//write(fd, "\r\n", strlen("\r\n"));
				//write(fd, filePassword.c_str(), filePassword.length());
				//write(fd, "\r\n", strlen("\r\n"));
			}
		}
		myFile.close();
		sort(users.begin(), users.end());
		for (int i=0; i<howManyUsers; i++) {
			//write(fd, "DEBUG #1\r\n", strlen(msgDEBUG));
			write(fd, users[i].c_str(), users[i].length());
			write(fd, "\r\n", strlen("\r\n"));
		}
		write(fd, "\r\n", strlen("\r\n"));
	}
	else write(fd, msgDENIED, strlen(msgDENIED));

}

void
IRCServer::listRooms(int fd, const char * user, const char * password,const  char * args)
{
	// const char * msgOK =  "OK\r\n";
	// const char * msgDENIED = "DENIED\r\n";
	// string fileRoom;
	//
	// ifstream myFile;
	// myFile.open (ROOM_FILE);
	// if (myFile.is_open()) {
	// 	while (!myFile.eof()) {
	// 		getline(myFile, fileRoom);
	// 		if (!fileRoom.empty()) {
	// 		 	write(fd, fileRoom.c_str(), fileRoom.length());
	// 			write(fd, "\r\n", strlen("\r\n"));
	// 		}
	// 	}
	// 	myFile.close();
	// }
	// else write(fd, msgDENIED, strlen(msgDENIED));

	const char * msgOK =  "OK\r\n";
	const char * msgDENIED = "DENIED\r\n";
	string fileRoom;
	vector<string> rooms;
	int howManyRooms = 0;

	ifstream myFile;
	myFile.open (ROOM_FILE);
	if (myFile.is_open()) {
		while (!myFile.eof()) {
			getline(myFile, fileRoom);
			if (!fileRoom.empty()) {
				howManyRooms++;
				rooms.push_back(fileRoom);
			}
		}
		myFile.close();
		sort(rooms.begin(), rooms.end());
		for (int i = 0; i < howManyRooms; i++) {
			write(fd, rooms[i].c_str(), rooms[i].length());
			write(fd, "\r\n", strlen("\r\n"));
		}
		write(fd, "\r\n", strlen("\r\n"));
	}
	else write(fd, msgDENIED, strlen(msgDENIED));
}
