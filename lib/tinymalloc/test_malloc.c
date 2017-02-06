#include <stdint.h>
#include <stdio.h>

#define die(a,b) return;
#define message printf

void t_print_stack(); // debug

#define IMPLEMENTATION_TINYMALLOC
#include "tinymalloc.h"

int main(void)
{

	char a[32],b[200],c[3000]; // mem is made of 3 chunks

	t_malloc(100); // die 

	t_print_stack();

	t_addchunk(&a,sizeof(a));
	t_print_stack();
	t_addchunk(&b,sizeof(b));
	t_print_stack();
	t_addchunk(&c,sizeof(c));
	t_print_stack();

	void *p1=t_malloc(50);
	t_print_stack();
	void *p2=t_malloc(1000);
	t_print_stack();
	void *p3=t_malloc(150);
	t_print_stack();

	t_free(p2);p2=0;
	p2=t_malloc(2000);
	t_free(p3);p3=0;
	t_print_stack();
	p2=t_malloc(2000);
	t_free(p2);p2=0;
	t_free(p1);p2=0;
	t_print_stack();
}
