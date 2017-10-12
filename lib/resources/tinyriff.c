// tinyriff.c

#include <stdint.h>

const char * parse_chunk(const char *buf)
{
    uint32_t info = *(uint32_t*)buf;
	buf+=4;
	uint32_t sz = *(uint32_t *)buf;
	buf+=4;

	#ifdef TEST
	printf("chunk: %.4s size:%d\n",info,sz);
	#endif 

	const char *tgtbuf = buf+sz;

	if ( info == *(uint32_t*)"RIFF" || info == *(uint32_t*)"LIST") {
		const char *listname = buf;
		buf+=4;
		#ifdef TEST
		printf("list : %.4s\n",listname);
		#endif 

		while (buf<tgtbuf) {
			buf = parse_chunk(buf);
		}
	} else {
		// use chunk ! 
		buf += sz;
	}

	buf += sz%2;

	return buf;
}


#ifdef TEST
#include <stdio.h>
#include <stdlib.h>
#define SZ 10000
int main()
{
    printf("parsing file\n");
    void *buf=malloc(SZ); 
    FILE *f=fopen("go_ahead.wav","r"); 
    const size_t n=fread(buf,1,SZ,f);
    printf("%d bytes read.\n",n);

    parse_chunk(buf);
}

#endif 