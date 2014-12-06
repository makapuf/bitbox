/* Copyright 2001, Jonas Norberg
Distributed under the terms of the MIT Licence. */

#ifndef TXT_H
#define TXT_H

#include <stdio.h>
#include "types.h"

#define BUFSIZE 300	

class txt
{
public:
	txt(char *buffer, int size);
	status_t InitCheck();
	
	bool IsMemory();

	// call this to create a binary file
	void IsBin(bool b = true);
	
	// read
	int NextVal(int numbytes = 1);
	float NextFloat();
	char *NextString();	// NOT malloc:ed

	bool Eof();
	
private:

	int AtomRead();
			
	void jntok(char *b, const char *m);

	bool open_for_reading;
	bool eof;

	char buf[BUFSIZE];
	status_t error;

//	läsa från minne
	char *buff;
	int bytes_left;
};

#endif
