#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <mutex>
#include <string>


// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")


#define DEFAULT_BUFLEN 16384
#define DEFAULT_PORT "27015"

using namespace std;

void printtimeline(SOCKET);
void printalltimeline(SOCKET);
void DeleteMessage(SOCKET);

int __cdecl main(int argc, char **argv) 
{
	WSADATA wsaData;
	SOCKET ConnectSocket = INVALID_SOCKET;
	struct addrinfo *result = NULL,
		*ptr = NULL,
		hints;
	char *sendbuf = "";
	char recvbuf[DEFAULT_BUFLEN];
	int iResult;
	int recvbuflen = DEFAULT_BUFLEN;
	bool ifconnect=FALSE;

	// Validate the parameters
	if (argc != 2) {
		printf("usage: %s server-name\n", argv[0]);
		return 1;
	}

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed with error: %d\n", iResult);
		return 1;
	}

	ZeroMemory( &hints, sizeof(hints) );
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	// Resolve the server address and port
	iResult = getaddrinfo(argv[1], DEFAULT_PORT, &hints, &result);
	if ( iResult != 0 ) {
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		return 1;
	}

	// Attempt to connect to an address until one succeeds
	for(ptr=result; ptr != NULL ;ptr=ptr->ai_next) {

		// Create a SOCKET for connecting to server
		ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, 
			ptr->ai_protocol);
		if (ConnectSocket == INVALID_SOCKET) {
			printf("socket failed with error: %ld\n", WSAGetLastError());
			WSACleanup();
			return 1;
		}

		// Connect to server.
		iResult = connect( ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (iResult == SOCKET_ERROR) {
			closesocket(ConnectSocket);
			ConnectSocket = INVALID_SOCKET;
			continue;
		}	
		break;
	}


	freeaddrinfo(result);

	if (ConnectSocket == INVALID_SOCKET) {
		printf("Unable to connect to server!\n");
		WSACleanup();
		return 1;
	}

	// Send the username------------------------------------------------------
	cout<<"Hello, welcome. Your username?"<<endl;
	string username;
	cin>>username;

	sendbuf=(char*)username.c_str();

	iResult = send(ConnectSocket, sendbuf, (int)strlen(sendbuf)+1, 0 );
	if (iResult == SOCKET_ERROR) {
		printf("send failed with error: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		return 1;
	}

	//Receive log info-----------------------------------------------------------
	iResult=recv(ConnectSocket, recvbuf, recvbuflen, 0);
	if(iResult>0){
		if(recvbuf[0]=='0'&&recvbuf[1]=='0'){
			cout<<"Successful login."<<endl;
			ifconnect=TRUE;
		}
		else{
			cout<<"The authentication fails. The user is already being used by another client."<<endl;
			return 0;
		}
	}

	//Main menu--------------------------------------------------------------------

	while(ifconnect){
		cout<<"==================Main menu====================="<<endl;
		cout<<"1. Send a message."<<endl;
		cout<<"2. Delete a message."<<endl;
		cout<<"3. Print user timeline."<<endl;
		cout<<"4. Print timeline of friends."<<endl;
		cout<<"5. Follow someone."<<endl;
		cout<<"6. Unfollow someone."<<endl;
		cout<<"7. Log out." <<endl;
		cout<<"-------------------------------------------------"<<endl;
		char choice;
		cout<<"Your choice number: "<<endl;
		cin>>choice;
		sendbuf[0]=choice;
		if(send(ConnectSocket,sendbuf,(int)strlen(sendbuf)+1,0)==SOCKET_ERROR){
			printf("send failed with error: %d\n", WSAGetLastError());
			closesocket(ConnectSocket);
			WSACleanup();
			return 1;
		}
		string s;
		int size;
		switch (choice){
		case '7'://Log out----------------------------------------------------------
			cout<<"Log out. Bye."<<endl;
			// shutdown the connection since no more data will be sent
			iResult = shutdown(ConnectSocket, SD_BOTH);
			if (iResult == SOCKET_ERROR) {
				printf("shutdown failed with error: %d\n", WSAGetLastError());
				closesocket(ConnectSocket);
				WSACleanup();
				return 1;
			}
			return 0;
			break;
		case '1'://Send message-------------------------------------------------------
			cout<<"Please type in your Message: "<<endl;
			//string s;
			getchar();
			getline(cin,s);
			sendbuf=(char*)s.c_str();
			if(send(ConnectSocket,sendbuf,(int)strlen(sendbuf)+1,0)==SOCKET_ERROR){
				printf("send failed with error: %d\n", WSAGetLastError());
				closesocket(ConnectSocket);
				WSACleanup();
				return 1;
			}
			break;

		case '2'://Deleting message----------------------------------------------------
			printtimeline(ConnectSocket);
			DeleteMessage(ConnectSocket);
			break;
		case '3'://Print user timeline-------------------------------------------------
			printtimeline(ConnectSocket);
			break;
		case '4'://Print friends' timeline---------------------------------------------
			printalltimeline(ConnectSocket);
			break;
		case '5': //Follow--------------------------------------------------------------
			cout<<"Who do you want to follow?"<<endl;
			//string s;
			cin>>s;
			sendbuf=(char*)s.c_str();
			if(send(ConnectSocket,sendbuf,(int)strlen(sendbuf)+1,0)==SOCKET_ERROR){
				printf("send failed with error: %d\n", WSAGetLastError());
				closesocket(ConnectSocket);
				WSACleanup();
				return 1;
			}
			iResult=recv(ConnectSocket,recvbuf,recvbuflen,0);
			if(iResult>0){
				recvbuf[iResult]='\0';
				if(strcmp(recvbuf,"50")==0){
					cout<<"Sorry, the user ["<<s<<"] doesn't exist."<<endl;
				}
				else if(strcmp(recvbuf,"51")==0){
					cout<<"Sorry, you can't follow yourself."<<endl;
				}
				else if(strcmp(recvbuf,"52")==0){
					cout<<"Sorry, you have already followed ["<<s<<"]."<<endl;
				}
				else{
					cout<<"You follow ["<<s<<"] successfully."<<endl;
				}
			}
			break;

		case '6'://Unfollow---------------------------------------------------------------
			cout<<"Who do you want to unfollow?"<<endl;
			cin>>s;
			sendbuf=(char*)s.c_str();
			if(send(ConnectSocket,sendbuf,(int)strlen(sendbuf)+1,0)==SOCKET_ERROR){
				printf("send failed with error: %d\n", WSAGetLastError());
				closesocket(ConnectSocket);
				WSACleanup();
				return 1;
			}
			iResult=recv(ConnectSocket,recvbuf,recvbuflen,0);
			if(iResult>0){
				recvbuf[iResult]='\0';
				if(strcmp(recvbuf,"60")==0){
					cout<<"Sorry, the user ["<<s<<"] doesn't exist."<<endl;
				}
				else if(strcmp(recvbuf,"61")==0){
					cout<<"Sorry, you can't unfollow yourself."<<endl;
				}
				else if(strcmp(recvbuf,"62")==0){
					cout<<"Sorry, ["<<s<<"] is not your friend."<<endl;
				}
				else{
					cout<<"You unfollow ["<<s<<"] successfully."<<endl;
				}
			}
			break;

		default:
			cout<<"Wrong Selection"<<endl;
			break;
		}
		cout<<endl;
	}

	// cleanup

	closesocket(ConnectSocket);
	WSACleanup();

	getchar();
	return 0;
}

void printtimeline(SOCKET cnctSocket){
	SOCKET ConnectSocket=cnctSocket;
	char recvbuf[DEFAULT_BUFLEN];
	int iResult;
	int recvbuflen = DEFAULT_BUFLEN;

	cout<<"--Your own timeline-----------------------:"<<endl;
	iResult=recv(ConnectSocket,recvbuf,recvbuflen,0);
	if(iResult>0){
		recvbuf[iResult]='\0';
		cout<<recvbuf<<endl;
	}

}

void printalltimeline(SOCKET cnctSocket){
	SOCKET ConnectSocket=cnctSocket;
	char recvbuf[DEFAULT_BUFLEN];
	int iResult;
	int recvbuflen = DEFAULT_BUFLEN;
	cout<<"--Your friends' timeline-----------------------:"<<endl;
	iResult=recv(ConnectSocket,recvbuf,recvbuflen,0);
	if(iResult>0){
		recvbuf[iResult]='\0';
		cout<<recvbuf<<endl;
	}
}

void DeleteMessage(SOCKET cnctSocket){
	SOCKET ConnectSocket=cnctSocket;
	char recvbuf[DEFAULT_BUFLEN];
	int iResult;
	int recvbuflen = DEFAULT_BUFLEN;
	cout<<"Message ID of the message you want to delete, "<<endl;
	cout<<"(0 if you have no message to delete): ";
	int id;
	cin>>id;
	string s=to_string(id);
	const char *sendbuf=s.c_str();
	if(send(ConnectSocket,sendbuf,(int)strlen(sendbuf)+1,0)==SOCKET_ERROR){
		printf("send failed with error: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		exit(1);
	}
	iResult=recv(ConnectSocket,recvbuf,recvbuflen,0);
	if(iResult>0){
		recvbuf[iResult]='\0';
		if(recvbuf[0]!='0'){
			cout<<"The message ID is wrong. You don't have such message."<<endl;
		}
		else{
			cout<<"Done!"<<endl;
		}
	}
}