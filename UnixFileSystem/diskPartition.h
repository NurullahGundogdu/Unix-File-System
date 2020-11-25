#include <time.h>




#define SUPERBLOCKSIZE 24
#define INODESIZE 328
#define DISKSIZE 1048576    // 1MB


struct superblock{  

 	int numberOfBlocks;
 	int numberOfInodes;
 	int blockSize;
 	int InodeSize;
 	int InodePosition;
 	int blockPosition;

};



struct Inodes{  

	char fileName[256];
	int type;    			// file = 0,  directory = 1  symboliklink = 2
	int InodeNumber;
	time_t lastmodification;
	int blocks[10];
	int fileSize;
	int parent;
	int isSymbolicLink;
	int hardLinkNumber;
 
};

