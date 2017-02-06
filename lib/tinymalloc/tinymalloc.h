/* tiny malloc library

   Taken from The C programming language, by Ritchie & Kernighan, Ed2 chap 8.7
   Adapted by Makapuf (designated inits, no sbrk)

   This implementation is tiny, not that fast and does not interact with any operating system features.
   Use by adding one or more chunks of memory to it with bfree(), then malloc / free within it.

   Using bitbox standard message / die interface.

   NOTE:  This is a single file library. Just include it once in your code and define IMPLEMENTATION_TINYMALLOC before

 */

// allocate memory. Always returns a valid pointer (or die trying :)
void *t_malloc(unsigned int nbytes) __attribute__( (warn_unused_result) );

void t_free(void *ap);                       // free a memory block

void t_addchunk(void *ptr, unsigned int sz); // declares memory as free from static chunk of mem.

int t_available();                           // get available mem (but not nec. in one chunk !)

// ----- IMPLEMENTATION -----------------------------------------------------------------------------------

/*

   Memory is managed by blocks :

   +------------+------------+----------- ...  -------+
 | next_block | block size | user-memory            |
 |+------------+------------+----------- ...  -------+
                            ^
   <------- header ---------> | address returned to user
 */

#ifdef TINYMALLOC_IMPLEMENTATION

typedef union header_u {                        // block header
	struct {
        union header_u *next;                   // next block if on free list
        unsigned int size;                      // size of this block
    } s;
    uint32_t x;                                 // will force alignment of blocks
} Header;

static Header base={.s.next=&base, .s.size=0 }; // empty list to get started
static Header *freep=&base;                     // start of free list

/* malloc: general-purpose storage allocator */
void *t_malloc(unsigned int nbytes)
{
	Header *p, *prevp;
	unsigned int nunits;


    nunits = (nbytes+sizeof(Header)-1)/sizeof(Header) + 1; // nb of 8bytes blocks

    message( "malloc %d bytes, %d units, %d real bytes\n",nbytes, nunits, nunits*sizeof(Header) );

    prevp=freep;

    for (p = prevp->s.next;; prevp = p, p = p->s.next) {
    	message("  checking %p (sz=%d))...",p,p->s.size);
        if (p->s.size >= nunits) {     // big enough
            if (p->s.size == nunits) { // exactly
            	message("size just ok");
            	prevp->s.next = p->s.next;
            } else {                   // allocate tail end
            	message("ok more than nec.\n");
            	p->s.size -= nunits;
            	p += p->s.size;
            	p->s.size = nunits;
            }
            freep = prevp;
            return (void *)(p+1);
        } else
        message("too small");
        message("\n");


        if (p == freep) { // wrapped around free list : out of memory
        	message("Out of memory.\n");
        	die (3,3);
        }
    }
}

void t_free(void *ap)
{
	Header *bp, *p;
    bp = (Header *)ap - 1;              // point to block header
    // XXX Check header is OK ? (next really in RAM, size not too big ie <64k )

    for (p = freep; !(bp > p && bp < p->s.next); p = p->s.next)
    	if ( p >= p->s.next && (bp > p || bp < p->s.next) )
            break;                      // freed block at start or end of arena

    if (bp + bp->s.size == p->s.next) { // join to upper nbr
    	bp->s.size += p->s.next->s.size;
    	bp->s.next = p->s.next->s.next;
    } else {
    	bp->s.next = p->s.next;
    }

    if (p + p->s.size == bp) { // join to lower nbr
    	p->s.size += bp->s.size;
    	p->s.next = bp->s.next;
    } else {
    	p->s.next = bp;
    }

    freep = p;
}

// Puts a block not already in free list to it (from static memory by example)
void t_addchunk(void *p, unsigned int n)
{
    // now build a block at given memory
	Header *hp = (Header *)p;
	hp->s.size=n/sizeof(Header);

    t_free( (void*)(hp+1) ); // and "free" this block
}

// gets available mem (but not in one chunk !)
int t_available()
{
	int nb;
	Header *p;
	for (nb=0,p=freep->s.next; p!=freep; p=p->s.next)
		nb += p->s.size;
	return nb*sizeof(Header);
}

void t_print_stack()
{
	Header *p, *prevp=freep;

	for (p = prevp->s.next; p!=freep; prevp = p, p = p->s.next) {
		message("chunk at %p size %u\n",p,p->s.size);
	}
	message("chunk at %p size %u\n",p,p->s.size);
	message("-\n");
}

#endif