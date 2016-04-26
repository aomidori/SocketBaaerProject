#ifndef DATABASE_H
#define DATABASE_H

#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <string>
#include <ctime>

using namespace std;

typedef struct User;
typedef struct Message;

struct User{
	char *name;
	int status;  //0: offline,  1: online
	int userID;
	vector<Message> messagelist;
	vector<User> following;
};

struct Message{
	int messageID;
	SYSTEMTIME publishtime;
	char *text;
};

#endif