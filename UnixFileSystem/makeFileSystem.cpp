#include <iostream>
#include <string.h>
#include "diskPartition.h"
#include <iostream>
#include<string.h>
#include<cstring>
#include<cstdio>
#include<string>
#include "diskPartition.h"
#include<stdio.h>
#include<stdlib.h>

 

using namespace std;

int findnumberOfBlock(int inodeNum, int blockSize);
superblock createSuperblock(int numberOfInodes, int blockSize);
void writeToFile(superblock Superblock, char* filename);
Inodes createRootDirectory();




int main(int argc, char *argv[])
{

	if (argc != 4) {
    	fprintf(stderr, "Usage: %s <block size> <number of inode> <file system name> \n", argv[0]);
    	return 0;
    }

    createRootDirectory();

	superblock Superblock = createSuperblock(atoi(argv[2]), atoi(argv[1]));


 	writeToFile(Superblock, argv[3]);


	superblock sup;


	return 0;

}




int findnumberOfBlock(int inodeNum, int blockSize){

	int num = (inodeNum * sizeof(Inodes)) + inodeNum + sizeof(superblock);
	
	if(num + blockSize > DISKSIZE){
		fprintf(stderr, "DISK OVERSIZE!!!\n");
		return;
	}

	int inodeSizes = inodeNum * sizeof(Inodes);

	int inodeBitmapSize = inodeNum;

	int temp_size = DISKSIZE - (inodeSizes + sizeof(superblock) + inodeBitmapSize);

	int blockNumber = temp_size / blockSize;

	int temp = temp_size % blockSize;

	if(blockNumber > temp){
		while(blockNumber > temp){
			blockNumber--;

			temp += blockSize;
		}
	}


	return blockNumber;
}

superblock createSuperblock(int numberOfInodes, int blockSize){

	superblock Superblock;

  	Superblock.numberOfInodes = numberOfInodes;
 	Superblock.blockSize = blockSize * 1024;
 	Superblock.InodeSize = sizeof(Inodes);
 	Superblock.numberOfBlocks = findnumberOfBlock(Superblock.numberOfInodes, Superblock.blockSize);
 	Superblock.InodePosition = SUPERBLOCKSIZE + Superblock.numberOfInodes + Superblock.numberOfBlocks;
 	Superblock.blockPosition = Superblock.InodePosition + (Superblock.numberOfInodes * Superblock.InodeSize);

 	return Superblock;

}

Inodes createRootDirectory(){

	Inodes root;

	root.type = 1;
	strcpy(root.fileName ,"root");
	root.InodeNumber = 0;

	time_t c_time;

	c_time = time(0);

	root.lastmodification = c_time;
	root.parent = 0;
	

    return root;

}

void writeToFile(superblock Superblock, char* filename){


	char inodes[Superblock.numberOfInodes];
 	char blocks[Superblock.numberOfBlocks];

 	
 	inodes[0] = '1';

 	for(int i = 1; i < Superblock.numberOfInodes; i++){
 		inodes[i] = '0';
 	}

 	blocks[0] = '1';

 	for(int i = 1; i < Superblock.numberOfBlocks; i++){
 		blocks[i] = '0';
 	}


	FILE *fp;
	fp = fopen(filename,"w");
	
	if(!fp){
		printf("!!!!!!FILE '%s' is not open\n",filename);
		return;
	}

	fwrite(&Superblock, sizeof(superblock), 1, fp);
	
	fwrite(inodes, sizeof(inodes), 1, fp);
	
	fwrite(blocks, sizeof(blocks), 1, fp);

	Inodes nod = createRootDirectory();
	
	nod.fileSize = Superblock.blockSize;
	nod.isSymbolicLink = 0;
	nod.hardLinkNumber = 0;
	nod.blocks[0] = 0;

	for(int i = 1; i < 10; i++)
		nod.blocks[i] = -1;

	fwrite(&nod, sizeof(nod), 1, fp);


	for(int i = 1; i < Superblock.numberOfInodes; i++){
		
		Inodes node;

		node.InodeNumber = i;
		node.isSymbolicLink = 0;
		node.hardLinkNumber = 0;

		for(int i = 0; i < 10; i++)
			node.blocks[i] = -1;

		fwrite(&node, sizeof(node), 1, fp);
	}
	
	char temp = '\0';

	fwrite(&temp, 1, 1, fp);
	
	fseek(fp, DISKSIZE - 1, SEEK_SET);

	fwrite(&temp, 1, 1, fp);
	fclose(fp);

}
