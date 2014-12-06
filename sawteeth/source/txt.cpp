/* Copyright 2001, Jonas Norberg
Distributed under the terms of the MIT Licence. */

#include <stdlib.h>
#include <string.h>

#include "txt.h"

/*_EXPORT*/ txt::txt(char *buffer,int size)
{
	eof = false;
	error=JNG_OK;

	buff = buffer;
	bytes_left = size;
	
}

status_t txt::InitCheck()
{
	return error;
}

bool txt::Eof()
{
	return eof;
}

void txt::jntok(char *b, const char *m)
{
	bool cont=true;
	int mc=strlen(m);
	int c;
	while (*b!=0 && cont){
		for (c=0;c<mc;c++)
			if (*b == m[c]){
				*b=0;
				cont=false;
				break;
			}
		b++;
	}	
}

#if ST_FILEIO
void txt::String(const char *txt)
{
	fputs(txt,f);
	if (bin)
		fputc(0,f);
	else
		fputc('\n',f);
}

void txt::Comment(int val)
{
	if (!bin) fprintf(f,"%x",val);
}

void txt::Comment(const char *txt)
{
	if (!bin) fputs(txt,f);
}

void txt::Val(int val, int numbytes)
{
	if (bin){
		if (numbytes == 1)
			fputc(val,f);
		else
			for (int c=numbytes; c>0; c--)
				Val((val >> ((c-1) << 3)) & 255, 1);
	}else{
		fprintf(f,"%d\n",val);
	}
}
#endif

int txt::AtomRead()
{
	if (bytes_left > 0){
		uint8 tmp = *buff;
		buff ++;
		bytes_left --;
		return tmp;
	}else{
		eof = true;
		return 0;
	}		
}

int txt::NextVal(int numbytes)
{
	// binary file
	if (numbytes == 1){
		return AtomRead();
	}

	int tmp=0;
	for (int c=numbytes;c>0;c--)
		tmp |= AtomRead() << ((c-1) << 3);

	return tmp;
}

float txt::NextFloat()
{
	int n = NextVal(4);
	float *f = (float *)&n;
	return (*f);
}

char *txt::NextString()
{
	int tmp,c;
	for ( c = 0 ; (c < BUFSIZE-1) ; c++ ){
		tmp = AtomRead();
		if (eof){
			buf[c] = 0;
			return buf;
		}
		if (tmp == 0 || tmp == '\n') break;
		buf[c]=tmp;
	}

	if (c >= BUFSIZE-1) buf[0] = 0;
	buf[c]=0;
	
	return buf;
}

