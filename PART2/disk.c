#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include "disk.h"

// define DEBUG for messages about all written blocks
// #define DEBUG


static FILE *diskfile;
static int nblocks = 0;
static int nreads = 0;
static int nwrites = 0;

// Data structures for the cache
#define EMPTY -1
typedef struct __cache_entry {
	int disk_block_number;  // identifies the block number in disk
	int dirty_bit;   // this value is 1 if the block has been written, 0 otherwise 
	char *datab;    // a pointer to a disk data block cached in memory
} cache_entry;		

cache_entry *cache;	// cache metadata
// char * cache_memory;	 // cache data space

static int cache_nblocks = 0;
static int cachehits = 0;
static int cachemisses = 0;

int disk_init( const char *filename, int n ) {
    diskfile = fopen( filename, "r+" );
    if ( diskfile != NULL && n == -1 ) {
        fseek( diskfile, 0L, SEEK_END );
        n = ftell( diskfile );
        fprintf( stderr, "filesize=%d, %d\n", n, n / DISK_BLOCK_SIZE );
        n = n / DISK_BLOCK_SIZE;
    }
    if ( !diskfile )
        diskfile = fopen( filename, "w+" );
    if ( !diskfile )
        return 0;

    ftruncate( fileno( diskfile ), n * DISK_BLOCK_SIZE );

    nblocks = n;
    nreads = 0;
    nwrites = 0;

   // TODO: cache initialization 
	
#ifdef DEBUG
    printf( "Cache blocks %d\n", cache_nblocks );
#endif

	
    srand( 0 );	// to generate always the same sequence of blocks to evict

    return 1;
}

int disk_size( ) {
    return nblocks;
}

static void sanity_check( int blocknum, const void *data ) {
    if (blocknum < 0) {
        printf( "ERROR: blocknum (%d) is negative!\n", blocknum );
        abort();
    }

    if (blocknum >= nblocks) {
        printf( "ERROR: blocknum (%d) is too big!\n", blocknum );
        abort();
    }

    if (!data) {
        printf( "ERROR: null data pointer!\n" );
        abort();
    }
}

void disk_read( int blocknum, char *data ) {
    sanity_check( blocknum, data );

    if ( fseek( diskfile, blocknum * DISK_BLOCK_SIZE, SEEK_SET )<0 )
        perror("disk_read seek");

    if (fread( data, DISK_BLOCK_SIZE, 1, diskfile ) == 1) {
        nreads++;
    } else {
        printf( "ERROR: couldn't access simulated disk: %s\n",
                strerror( errno ) );
        abort();
    }
}

void disk_write( int blocknum, const char *data ) {
#ifdef DEBUG
    printf( "Writing block %d\n", blocknum );
#endif
    sanity_check( blocknum, data );

    if ( fseek( diskfile, blocknum * DISK_BLOCK_SIZE, SEEK_SET )<0 )
        perror("disk_write seek");

    if ( fwrite( data, DISK_BLOCK_SIZE, 1, diskfile ) == 1) {
        nwrites++;
    } else {
        printf( "ERROR: couldn't access simulated disk: %s\n",
                strerror( errno ) );
        abort();
    }
}

// Searches the cache for a block; returns its position in the cache or -1 otherwise
int search_cache( int data_block_num )
{
	int entry_num = -1;
	// TODO

	return entry_num;
}

// allocates a cache_entry where to place the new block
int entry_selection()
{
	int entry_num;
	
	// TODO
	// note: the function rand() generates a random number
	
	return entry_num;
}

// Cache aware read
void disk_read_data( int blocknum, char *data ) {
 	sanity_check( blocknum, data );
	int cache_index;
#ifdef DEBUG
    printf( "disk_read_data for block %d \n", blocknum );
#endif

	// TODO

}

// Cache aware write
void disk_write_data( int blocknum, const char *data ) {
	sanity_check( blocknum, data );
	int cache_index;

#ifdef DEBUG
    printf( "disk_write_data for block %d \n", blocknum );
#endif

	// TODO

}

// Writes the cache's metadata
void cache_debug() {
	for( int i = 0; i < cache_nblocks; i++ ) {
    		// TODO
	}
}


// flushes the modified data blocks to disk
void disk_flush() {
	for( int i = 0; i < cache_nblocks; i++ ) {
		// TODO
	}
}


void disk_close( ) {
  if (diskfile)  {
		// TODO: flushes the cache and frees the allocated memory
	
		// Writes statistics
  	printf( "%d disk block reads\n", nreads );   
  	printf( "%d disk block writes\n", nwrites );  
		printf( "%d cache hits, %d cache misses\n", cachehits, cachemisses),
    
		fclose( diskfile );
    diskfile = 0;

	}
    
}

