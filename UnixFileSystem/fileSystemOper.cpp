#include <iostream>
#include<string.h>
#include<cstring>
#include<cstdio>
#include<string>
#include "diskPartition.h"
#include<stdio.h>
#include<stdlib.h>


using namespace std;


struct disk
{
	superblock Superblock;
	char *bitmapInode;
	char *bitmapBlocks;
	int freeBlockNumber;
	int freeInodeNumber;
};

disk Disk;

void readFromDisk(char* filename);

void list(char* path, char* filename);
void mkdir(char* path, char* filename);
void rmdir(char* path, char* filename);
void dumpe2fs(char* filename);
void write(char* systemFilePath, char* filename, char* linuxFilePath);
void read(char* systemFilePath, char* filename, char* linuxFilePath);
void del(char* path, char* filename);
void ln(char* path, char* filename, char* hardlinkPath);
void lnsym(char* path, char* filename, char* hardlinkPath);
void fsck(char *filename);


int freeInode();
int freeBlock();
string findStartAndEnd(FILE *fp, Inodes inode, char *filename, int *blockNumber);
void markInodeBitmap(FILE *fp, int location, char mark);
void markBlocksBitmap(FILE *fp, int location, char mark);
void writeInodeToFile(FILE *fp, Inodes inode, int location);
int findPathSize(char *Path);
void dividePath(char *path, char **pathArray, int size);
int get_inodeNumber(FILE *fp, int *InodeNumber, char* filename);
int pathCheck(FILE *fp, char *pathArray[], int size, char* name, int *targetInodeNumber);
void divideBlock(char *path, char **pathArray);
void recursiveSearch(FILE *fp, int InodeNumber, int blockArray[], int inodeArray[]);
void findDirAndFilesNumber(char* filename, int *filesNumber, int *dirNumber);
void recursiveWriteFiles(FILE *fp, int InodeNumber, char *filename);
void recursiveFindFilesNumber(FILE *fp, int InodeNumber, char *filename, int *dirNum, int *fileNum, int *hardNum, int *symNum);


int main(int argc, char *argv[]){

	if (argc < 3) {
    	
    	fprintf(stderr, "Usage: %s <block size> <number of inode> <file system name> \n", argv[0]);
    	
    	return 0;
    }
	   
	readFromDisk(argv[1]);


	if(strcmp(argv[2],"list") == 0){
		list(argv[3], argv[1]);
	}else if(strcmp(argv[2],"mkdir") == 0){
		mkdir(argv[3], argv[1]);	
	}else if(strcmp(argv[2],"rmdir") == 0){
		rmdir(argv[3], argv[1]);
	}else if(strcmp(argv[2],"dumpe2fs") == 0){
		dumpe2fs(argv[1]);
	}else if(strcmp(argv[2],"write") == 0){
		write(argv[3], argv[1], argv[4]);
	}else if(strcmp(argv[2],"read") == 0){
		read(argv[3], argv[1], argv[4]);
	}else if(strcmp(argv[2],"del") == 0){
		del(argv[3], argv[1]);
	}else if(strcmp(argv[2],"ln") == 0){
		ln(argv[3], argv[1], argv[4]);
	}else if(strcmp(argv[2],"lnsym") == 0){
		lnsym(argv[3], argv[1], argv[4]);
	}else if(strcmp(argv[2],"fsck") == 0){
		fsck(argv[1]);
	}else{

		fprintf(stderr, "Wrong command: %s\n", argv[2]);
		
		return 0;
	}

	
	return 0;
	

}



void fsck(char *filename){

	int InodeNumber = 0;

	FILE *fp;
	fp = fopen(filename,"r");

	if(!fp){
		printf("!!!!!!FILE '%s' is not open\n",filename);
		return;
	}


	int blockArray[Disk.Superblock.numberOfBlocks];
	int inodeArray[Disk.Superblock.numberOfInodes];
	
	for(int i = 0; i < Disk.Superblock.numberOfBlocks; i++){
		blockArray[i] = 0;
	}
	for(int i = 0; i < Disk.Superblock.numberOfInodes; i++){
		inodeArray[i] = 0;
	}


	inodeArray[0]++;

	recursiveSearch(fp, InodeNumber, blockArray, inodeArray);

	cout<<"\nINODES:\n\n";


	for(int i = 0; i < Disk.Superblock.numberOfInodes; i+=20){

		cout<<"\nInode number  : |";

		for(int j = i; j < i + 20 && j < Disk.Superblock.numberOfInodes; j++)
			printf("%3d|", j);

		
		cout<<"\nInode in use  : |";
		
		for(int j = i; j < i + 20 && j < Disk.Superblock.numberOfInodes; j++)
			printf("%3d|", inodeArray[j]);

		
		cout<<"\nFree Inode    : |";

		for(int j = i; j < i + 20 && j < Disk.Superblock.numberOfInodes; j++)
			if(Disk.bitmapInode[j] == '1')
				printf("%3c|", '0');
			else
				printf("%3c|", '1');
		cout<<endl;
		//i += 19; 
	}


	cout<<"\n\nBLOCKS:\n\n";

	for(int i = 0; i < Disk.Superblock.numberOfBlocks; i+=20){

		cout<<"\nBlock number  : |";

		for(int j = i; j < i + 20 && j < Disk.Superblock.numberOfBlocks; j++)
			printf("%3d|", j);

		
		cout<<"\nBlock in use  : |";
		
		for(int j = i; j < i + 20 && j < Disk.Superblock.numberOfBlocks; j++)
			printf("%3d|", blockArray[j]);

		
		cout<<"\nFree Block    : |";

		for(int j = i; j < i + 20 && j < Disk.Superblock.numberOfBlocks; j++)
			if(Disk.bitmapBlocks[j] == '1')
				printf("%3c|", '0');
			else
				printf("%3c|", '1');
		cout<<endl;
		//i += 19; 
	}


	fclose(fp);
}

void lnsym(char* path, char* filename, char* symboliclinkPath){

	if(path == NULL){		// path girilmemis yanlis kullanim
		fprintf(stderr, "Wrong usage!!!\n");
		return ;
	}

	int freeInodeNum = freeInode();

	if(freeInodeNum == -1){			//bos inode yok
		fprintf(stderr, "There is no free space!!!\n");
		return ;
	}

	int size = findPathSize(path);
	int symboliclinkPath_size = findPathSize(symboliclinkPath);

	if(size == 0 || symboliclinkPath_size == 0){		// path yanlis girilmis yanlıs kullanim
		fprintf(stderr, "Wrong usage!!!\n");
		return ;
	}

	char *pathArray[size];
	char *symboliclinkPathArray[size];

	dividePath(path, pathArray, size);
	dividePath(symboliclinkPath, symboliclinkPathArray, size);


	FILE *fp;
	fp = fopen(filename,"rw+");

	if(!fp){
		printf("!!!!!!FILE '%s' is not open\n",filename);
		return;
	}

	char name[256];
	char symboliclink_name[256];
	int targetInodeNumber;
	int symboliclinkInodeNumber;

	int InodeNumber = pathCheck(fp, pathArray, size, name, &targetInodeNumber);
	int symboliclinkParentInodeNumber = pathCheck(fp, symboliclinkPathArray, symboliclinkPath_size, symboliclink_name, &symboliclinkInodeNumber);

	
	if(InodeNumber == -2 || symboliclinkParentInodeNumber == -2){		// path yanlis girilmis yanlıs kullanim
		fprintf(stderr, "There is no '%s' dirrectory!!!\n",name);
		fclose(fp);
		return ;
	}

	if(targetInodeNumber == -1){		//ayni isimde directory var mi kontrolu
		fprintf(stderr, "There is no '%s' file!!!\n",name);
		fclose(fp);
		return ;
	}

	if(symboliclinkInodeNumber != -1){		//ayni isimde directory var mi kontrolu
		fprintf(stderr, "write : cannot create file '%s': File exists!!!\n",name);
		fclose(fp);
		return ;
	}

	char symbolicInodeInformation[300];
	sprintf(symbolicInodeInformation,"%s %d",symboliclink_name, freeInodeNum); 

	char newInodeInformation[300];
	sprintf(newInodeInformation,"%s %d",name,targetInodeNumber); 

	fseek(fp, Disk.Superblock.InodePosition + (targetInodeNumber * sizeof(Inodes)), SEEK_SET); 		//root inode alındı

	Inodes nod;
	fread(&nod, sizeof(Inodes), 1, fp);

	fseek(fp, Disk.Superblock.InodePosition + (symboliclinkParentInodeNumber * sizeof(Inodes)), SEEK_SET); 		//root inode alındı

	Inodes inode;
	fread(&inode, sizeof(Inodes), 1, fp);

	Inodes node;
	
	fseek(fp, Disk.Superblock.InodePosition + (freeInodeNum * sizeof(Inodes)), SEEK_SET);
	
	fread(&node, sizeof(Inodes), 1, fp);
	node.type = nod.type;
	node.parent = symboliclinkParentInodeNumber;
	node.fileSize = Disk.Superblock.blockSize;
	strcpy(node.fileName, symboliclink_name);
	node.isSymbolicLink = 1;
	int blockNum = 0;

	while(inode.blocks[blockNum] != -1){
		blockNum++;
	}

	blockNum--;

	fseek(fp, Disk.Superblock.blockPosition + (inode.blocks[blockNum] * Disk.Superblock.blockSize), SEEK_SET);
	char block[Disk.Superblock.blockSize];

	fread(block, Disk.Superblock.blockSize, 1, fp);

	int freeblock = freeBlock();


	int num = 0;

	if(strlen(block) + strlen(symbolicInodeInformation) + 2 > Disk.Superblock.blockSize){
		
		if(freeblock == -1){
			fprintf(stderr, "There is no free space!!!\n");
			fclose(fp);
			return;
		}

		markBlocksBitmap(fp, freeblock, '1');

		inode.blocks[blockNum + 1] = freeblock;

		inode.fileSize += Disk.Superblock.blockSize;
		blockNum++;

		num++;
	}

	int freeblock2 = freeBlock();
	char temp = ' ';

	if(freeblock2 == -1){
		fprintf(stderr, "There is no free space!!!\n");
		markBlocksBitmap(fp, freeblock, '0');
		fclose(fp);
		return;
	}

	markBlocksBitmap(fp, freeblock2, '1');

	node.blocks[0] = freeblock2;

	if(num == 1)
		fseek(fp, Disk.Superblock.blockPosition + (inode.blocks[blockNum] * Disk.Superblock.blockSize), SEEK_SET);
	else{
		fseek(fp, Disk.Superblock.blockPosition + (inode.blocks[blockNum] * Disk.Superblock.blockSize) + strlen(block), SEEK_SET);
		
		if(strlen(block) != 0)
			fwrite(&temp, 1, 1, fp);

	}


	for(int i = 0; i < strlen(symbolicInodeInformation); i++ ){	
		fwrite(&symbolicInodeInformation[i], 1, 1, fp);
	}

	temp = '\0';

	fwrite(&temp, 1, 1, fp);

	fseek(fp, Disk.Superblock.blockPosition + (node.blocks[0] * Disk.Superblock.blockSize), SEEK_SET);
	
	for(int i = 0; i < strlen(newInodeInformation); i++ ){	
		fwrite(&newInodeInformation[i], 1, 1, fp);
	}

	fwrite(&temp, 1, 1, fp);
	

	time_t c_time;

	c_time = time(0);

	node.lastmodification = c_time;
	inode.lastmodification = c_time;

	markInodeBitmap(fp, freeInodeNum, '1');

	writeInodeToFile(fp, node, node.InodeNumber);
	writeInodeToFile(fp, inode, inode.InodeNumber);
	

	fclose(fp);

}
void ln(char* path, char* filename, char* hardlinkPath){

	if(path == NULL){		// path girilmemis yanlis kullanim
		fprintf(stderr, "Wrong usage!!!\n");
		return ;
	}


	int size = findPathSize(path);
	int hardlinkPath_size = findPathSize(hardlinkPath);

	if(size == 0 || hardlinkPath_size == 0){		// path yanlis girilmis yanlıs kullanim
		fprintf(stderr, "Wrong usage!!!\n");
		return ;
	}

	char *pathArray[size];
	char *hardlinkPathArray[size];

	dividePath(path, pathArray, size);
	dividePath(hardlinkPath, hardlinkPathArray, size);

	

	FILE *fp;
	fp = fopen(filename,"rw+");

	if(!fp){
		printf("!!!!!!FILE '%s' is not open\n",filename);
		return;
	}


	char name[256];
	char hardlink_name[256];
	int targetInodeNumber;
	int hardlinkInodeNumber;

	int InodeNumber = pathCheck(fp, pathArray, size, name, &targetInodeNumber);
	int hardlinkParentInodeNumber = pathCheck(fp, hardlinkPathArray, hardlinkPath_size, hardlink_name, &hardlinkInodeNumber);
	
	if(InodeNumber == -2 || hardlinkParentInodeNumber == -2){		// path yanlis girilmis yanlıs kullanim
		fprintf(stderr, "There is no '%s' dirrectory!!!\n",name);
		fclose(fp);
		return ;
	}

	if(targetInodeNumber == -1){		//ayni isimde directory var mi kontrolu
		fprintf(stderr, "write : cannot create file '%s': File exists!!!\n",name);
		fclose(fp);
		return ;
	}

	if(hardlinkInodeNumber != -1){		//ayni isimde directory var mi kontrolu
		fprintf(stderr, "write : cannot create file '%s': File exists!!!\n",name);
		fclose(fp);
		return ;
	}

	char newInodeInformation[300];
	sprintf(newInodeInformation,"%s %d",hardlink_name,targetInodeNumber); 

	fseek(fp, Disk.Superblock.InodePosition + (hardlinkParentInodeNumber * sizeof(Inodes)), SEEK_SET); 		//root inode alındı

	Inodes inode;
	fread(&inode, sizeof(Inodes), 1, fp);

	int blockNum = 0;

	while(inode.blocks[blockNum] != -1){
		blockNum++;
	}

	blockNum--;

	fseek(fp, Disk.Superblock.blockPosition + (inode.blocks[blockNum] * Disk.Superblock.blockSize), SEEK_SET);
	char block[Disk.Superblock.blockSize];

	fread(block, Disk.Superblock.blockSize, 1, fp);

	int freeblock = freeBlock();


	int num = 0;

	if(strlen(block) + strlen(newInodeInformation) + 2 > Disk.Superblock.blockSize){
		
		if(freeblock == -1){
			fprintf(stderr, "There is no free space!!!\n");
			fclose(fp);
			return;
		}

		markBlocksBitmap(fp, freeblock, '1');

		inode.blocks[blockNum + 1] = freeblock;

		inode.fileSize += Disk.Superblock.blockSize;
		blockNum++;

		num++;
	}
	char temp = ' ';
	if(num == 1)
		fseek(fp, Disk.Superblock.blockPosition + (inode.blocks[blockNum] * Disk.Superblock.blockSize), SEEK_SET);
	else{
		fseek(fp, Disk.Superblock.blockPosition + (inode.blocks[blockNum] * Disk.Superblock.blockSize) + strlen(block), SEEK_SET);
		
		if(strlen(block) != 0)
			fwrite(&temp, 1, 1, fp);

	}


	for(int i = 0; i < strlen(newInodeInformation); i++ ){	
		fwrite(&newInodeInformation[i], 1, 1, fp);
	}

	temp = '\0';

	fwrite(&temp, 1, 1, fp);

	fseek(fp, Disk.Superblock.InodePosition + (targetInodeNumber * sizeof(Inodes)), SEEK_SET); 		//root inode alındı

	Inodes node;
	fread(&node, sizeof(Inodes), 1, fp);
	node.hardLinkNumber++;

	time_t c_time;

	c_time = time(0);

	//node.lastmodification = c_time;
	inode.lastmodification = c_time;

	//markInodeBitmap(fp, freeInodeNum, '1');

	writeInodeToFile(fp, node, node.InodeNumber);
	writeInodeToFile(fp, inode, inode.InodeNumber);
	

	fclose(fp);

}


void dumpe2fs(char* filename){
	
	fprintf(stdout, "Number of i-node	:	%d\n",Disk.Superblock.numberOfInodes);
	fprintf(stdout, "Number of block	:	%d\n",Disk.Superblock.numberOfBlocks);
	fprintf(stdout, "Number of free i-node	:	%d\n",Disk.freeInodeNumber);
	fprintf(stdout, "Number of free block	:	%d\n",Disk.freeBlockNumber);
	
	
	fprintf(stdout, "Block size	:	%d\n",Disk.Superblock.blockSize);


	FILE *fp;
	fp = fopen(filename,"r");

	if(!fp){
		printf("!!!!!!FILE '%s' is not open\n",filename);
		return;
	}

	char root[] = "root";

	int dirNum = 0, fileNum = 0, hardNum = 0, symNum = 0;

	recursiveFindFilesNumber(fp, 0, root, &dirNum, &fileNum, &hardNum, &symNum);

	fprintf(stdout, "Number of files	:	%d\n",fileNum);
	fprintf(stdout, "Number of directory	:	%d\n",dirNum);
	fprintf(stdout, "Number of symbolicLink	:	%d\n",symNum);
	fprintf(stdout, "Number of hardlink	:	%d\n\n",hardNum);


	recursiveWriteFiles(fp, 0, root);

}

void del(char* path, char* filename){

	if(path == NULL){		// path girilmemis yanlis kullanim
		fprintf(stderr, "Wrong usage!!!\n");
		return ;
	}

	int size = findPathSize(path);

	if(size == 0){		// path yanlis girilmis yanlıs kullanim
		fprintf(stderr, "Wrong usage!!!\n");
		return ;
	}

	char *pathArray[size];

	dividePath(path, pathArray, size);


	FILE *fp;
	fp = fopen(filename,"rw+");

	if(!fp){
		printf("!!!!!!FILE '%s' is not open\n",filename);
		return;
	}

	char name[256];
	int targetInodeNumber;

	int InodeNumber = pathCheck(fp, pathArray, size, name, &targetInodeNumber);

	if(InodeNumber == -2){		// path yanlis girilmis yanlıs kullanim
		fprintf(stderr, "There is no '%s' dirrectory!!!\n",name);
		fclose(fp);
		return ;
	}

	if(targetInodeNumber == -1){		//ayni isimde directory var mi kontrolu
		fprintf(stderr, "There is no '%s' file!!!\n",name);
		fclose(fp);
		return ;
	}

	fseek(fp, Disk.Superblock.InodePosition + (targetInodeNumber * sizeof(Inodes)), SEEK_SET); 

	Inodes node;
	fread(&node, sizeof(Inodes), 1, fp);

	if(node.type != 0 && node.isSymbolicLink == 0){
		fprintf(stderr, "'%s' is not a file!!!\n",name);
		fclose(fp);
		return;
	}

	char temp;
	

	fseek(fp, Disk.Superblock.InodePosition + (InodeNumber * sizeof(Inodes)), SEEK_SET); 		//root inode alındı

	Inodes inode;
	fread(&inode, sizeof(Inodes), 1, fp);


	int blockNumber;
	int num = 0;
	temp = '\0';
	string block = findStartAndEnd(fp, inode, name, &blockNumber);



	fseek(fp, Disk.Superblock.blockPosition + (inode.blocks[blockNumber] * Disk.Superblock.blockSize), SEEK_SET);
	
	char Block[Disk.Superblock.blockSize];

	if(block != ""){

		strcpy(Block, block.c_str());
		Block[strlen(Block)] = '\0';
					
		for(int i = strlen(Block); i < Disk.Superblock.blockSize; i++)
			Block[i] = '\0';

		fwrite(Block, Disk.Superblock.blockSize, 1, fp);


		num = 1;
	}else{
		for(int i = strlen(Block); i < Disk.Superblock.blockSize; i++)
			Block[i] = '\0';
		fwrite(Block, Disk.Superblock.blockSize, 1, fp);
		fseek(fp, Disk.Superblock.blockPosition + (inode.blocks[blockNumber] * Disk.Superblock.blockSize), SEEK_SET);
	}

	if(num == 1 && blockNumber != 0){
		markInodeBitmap(fp, inode.blocks[blockNumber], '0');
		inode.blocks[blockNumber] = -1;
		
	}

	if(node.hardLinkNumber > 0 && strcmp(node.fileName, name) != 0){
		node.hardLinkNumber--;

	}
	int p = node.parent;
	int control = get_inodeNumber(fp, &p, node.fileName);

	if((node.hardLinkNumber == 0  &&  strcmp(node.fileName, name) == 0) || (node.hardLinkNumber == 0 && control == -1)){
		blockNumber = 0;

		while(node.blocks[blockNumber] != -1){
			fseek(fp, Disk.Superblock.blockPosition + (node.blocks[blockNumber] * Disk.Superblock.blockSize), SEEK_SET);

			for(int i = 0; i < Disk.Superblock.blockSize; i++){
				fwrite(&temp, 1, 1, fp);
			}

			markBlocksBitmap(fp, node.blocks[blockNumber], '0');
		
			node.blocks[blockNumber] = -1;

			blockNumber++;
		}

		markInodeBitmap(fp, node.InodeNumber, '0');
		node.fileSize = 0;
	}/*else{
		node.hardLinkNumber--;
	}*/
	

	time_t c_time;

	c_time = time(0);

	inode.lastmodification = c_time;
	
	node.fileName[0] = '\0';
	writeInodeToFile(fp, inode, inode.InodeNumber);
	writeInodeToFile(fp, node, node.InodeNumber);


	fclose(fp);
}

void list(char* path, char* filename){


	if(path == NULL){		// path girilmemis yanlis kullanim
		fprintf(stderr, "Wrong usage!!!\n");
		return ;
	}
	char name[256];
	int targetInodeNumber;
	int size = findPathSize(path);

		
	FILE *fp;
	fp = fopen(filename,"r");

	if(!fp){
		printf("!!!!!!FILE '%s' is not open\n",filename);
		return;
	}

	if(size == 0){		// path yanlis girilmis yanlıs kullanim
		targetInodeNumber = 0; 
	}else{

		char *pathArray[size];

		dividePath(path, pathArray, size);

		int InodeNumber = pathCheck(fp, pathArray, size, name, &targetInodeNumber);

		
		if(InodeNumber == -2){		// path yanlis girilmis yanlıs kullanim
			fprintf(stderr, "There is no '%s' dirrectory!!!\n",name);
			fclose(fp);
			return ;
		}

		if(targetInodeNumber == -1){		//ayni isimde directory var mi kontrolu
			fprintf(stderr, "There is no '%s' dirrectory!!!\n",name);
			fclose(fp);
			return ;
		}
	}

	
	fseek(fp, Disk.Superblock.InodePosition + (targetInodeNumber * sizeof(Inodes)), SEEK_SET); 

	Inodes node;
	fread(&node, sizeof(Inodes), 1, fp);

	while(node.isSymbolicLink == 1){
			
		fseek(fp, Disk.Superblock.blockPosition + (node.blocks[0] * Disk.Superblock.blockSize), SEEK_SET);
		
		char block[Disk.Superblock.blockSize];
		
		fread(block, sizeof(block), 1, fp);
		
		char *token; 
		char *token2; 

		token = strtok (block," ");
		token2 = strtok (NULL, " ");

		fseek(fp, Disk.Superblock.InodePosition + (atoi(token2) * sizeof(Inodes)), SEEK_SET);

		fread(&node, sizeof(node), 1, fp);

		if(Disk.bitmapInode[node.InodeNumber] == 0){
			fprintf(stderr, "write : cannot create file, this is symbolic link file and its linked file deleted!!!\n");
			fclose(fp);
			return ;
		}

	
	}

	if(node.type != 1){
		fprintf(stderr, "'%s' is not a dirrectory!!!\n",name);
		fclose(fp);
		return;
	}

	
	fseek(fp, Disk.Superblock.blockPosition + (node.blocks[0] * Disk.Superblock.blockSize), SEEK_SET);
	char temp;
		
	fread(&temp, 1, 1, fp);

	if(temp == '\0'){
		fprintf(stderr, "'%s' dirrectory is empty!!!\n",name);
		fclose(fp);
		return ;
	}



	for(int i = 0; i < 10; i++){
		if(node.blocks[i] != -1){
			fseek(fp, Disk.Superblock.blockPosition + (node.blocks[i] * Disk.Superblock.blockSize), SEEK_SET);
			char block[Disk.Superblock.blockSize];
			
			fread(block, Disk.Superblock.blockSize, 1, fp);

			char *token;
			char *token2;

			token = strtok (block," ");
			token2 = strtok (NULL, " ");
			
			while (token != NULL){

				fseek(fp, Disk.Superblock.InodePosition + (atoi(token2) * sizeof(Inodes)), SEEK_SET); 
				
				Inodes nod;

				fread(&nod, sizeof(nod), 1, fp);
				
				cout<<nod.fileName<<"\t"<<nod.fileSize<<"\t"<<ctime(&(nod.lastmodification))<<endl;

				token = strtok (NULL, " ");
				token2 = strtok (NULL, " ");

			}

		}

	}

	fclose(fp);
}


void write(char* path, char* filename, char* linuxFilePath){
	
	if(path == NULL){		// path girilmemis yanlis kullanim
		fprintf(stderr, "Wrong usage!!!\n");
		return ;
	}

	int freeInodeNum = freeInode();

	if(freeInodeNum == -1){			//bos inode yok
		fprintf(stderr, "There is no free space!!!\n");
		return ;
	}


	int size = findPathSize(path);

	if(size == 0){		// path yanlis girilmis yanlıs kullanim
		fprintf(stderr, "Wrong usage!!!\n");
		return ;
	}

	char *pathArray[size];

	dividePath(path, pathArray, size);


	FILE *fp;
	fp = fopen(filename,"rw+");

	if(!fp){
		printf("!!!!!!FILE '%s' is not open\n",filename);
		return;
	}

	char name[256];
	int targetInodeNumber;
	int blockNumberOffillFile = 0;
	char temp = ' ';

	int InodeNumber = pathCheck(fp, pathArray, size, name, &targetInodeNumber);

	if(InodeNumber == -2){		// path yanlis girilmis yanlıs kullanim
		fprintf(stderr, "There is no '%s' dirrectory!!!\n",name);
		fclose(fp);
		return ;
	}


	Inodes node;

	if(targetInodeNumber != -1){		//ayni isimde directory var mi kontrolu
		
		fseek(fp, Disk.Superblock.InodePosition + (targetInodeNumber * sizeof(Inodes)), SEEK_SET);
		
		fread(&node, sizeof(node), 1, fp);


		while(node.isSymbolicLink == 1){
			fseek(fp, Disk.Superblock.blockPosition + (node.blocks[0] * Disk.Superblock.blockSize), SEEK_SET);
		
			char block[Disk.Superblock.blockSize];
			
			fread(block, sizeof(block), 1, fp);
			
			char *token; 
			char *token2; 

			token = strtok (block," ");
			token2 = strtok (NULL, " ");

			fseek(fp, Disk.Superblock.InodePosition + (atoi(token2) * sizeof(Inodes)), SEEK_SET);

			fread(&node, sizeof(node), 1, fp);
			InodeNumber = node.parent;



			if(Disk.bitmapInode[node.InodeNumber] == '0' || strcmp(node.fileName, token) != 0 ){
				fprintf(stderr, "write : cannot create file, this is symbolic link file and its linked file deleted!!!\n");
				fclose(fp);
				return ;
			}
			
		}

		int i = 0;
		while(node.blocks[i] != -1){
			blockNumberOffillFile++;
			i++;
		}
		
	}


	char newInodeInformation[300];
	sprintf(newInodeInformation,"%s %d",name,freeInodeNum); 

	fseek(fp, Disk.Superblock.InodePosition + (InodeNumber * sizeof(Inodes)), SEEK_SET); 		//root inode alındı

	Inodes inode;
	fread(&inode, sizeof(Inodes), 1, fp);

	FILE *pf;
	pf = fopen(linuxFilePath,"r");

	if(!fp){
		printf("!!!!!!FILE '%s' is not open\n",filename);
		return;
	}

	fseek(pf, 0L, SEEK_END);
	int filesize = ftell(pf);

	int blockNumberOfFile = filesize / Disk.Superblock.blockSize;

	if(filesize % Disk.Superblock.blockSize > 0)
		blockNumberOfFile++;
	
	if(blockNumberOfFile > 10){
		fprintf(stderr, "This file size more than 10 blocks!!!\n");
		return;
	}

	if(Disk.freeBlockNumber + blockNumberOffillFile < blockNumberOfFile){
		fprintf(stderr, "There is no free space!!!\n");
		return;
	}


	if(targetInodeNumber == -1){	
		fseek(fp, Disk.Superblock.InodePosition + (freeInodeNum * sizeof(Inodes)), SEEK_SET);

		fread(&node, sizeof(Inodes), 1, fp);
		strcpy(node.fileName, name);
		
		int blockNum = 0;

		while(inode.blocks[blockNum] != -1){
			blockNum++;
		}

		blockNum--;

		fseek(fp, Disk.Superblock.blockPosition + (inode.blocks[blockNum] * Disk.Superblock.blockSize), SEEK_SET);
		char block[Disk.Superblock.blockSize];

		fread(block, Disk.Superblock.blockSize, 1, fp);

		int freeblock = freeBlock();

		int num = 0;

		if(strlen(block) + strlen(newInodeInformation) + 2 > Disk.Superblock.blockSize){
			
			if(freeblock == -1){
				fprintf(stderr, "There is no free space!!!\n");
				fclose(fp);
				return;
			}

			markBlocksBitmap(fp, freeblock, '1');

			inode.blocks[blockNum + 1] = freeblock;

			inode.fileSize += Disk.Superblock.blockSize;
			blockNum++;

			num++;
		}

		if(num == 1)
			fseek(fp, Disk.Superblock.blockPosition + (inode.blocks[blockNum] * Disk.Superblock.blockSize), SEEK_SET);
		else{
			fseek(fp, Disk.Superblock.blockPosition + (inode.blocks[blockNum] * Disk.Superblock.blockSize) + strlen(block), SEEK_SET);
			
			if(strlen(block) != 0)
				fwrite(&temp, 1, 1, fp);

		}

		for(int i = 0; i < strlen(newInodeInformation); i++ ){	
			fwrite(&newInodeInformation[i], 1, 1, fp);
		}

		temp = '\0';

		fwrite(&temp, 1, 1, fp);
	}else{
		int i = 0; 
		temp = '\0';
		while(node.blocks[i] != -1){

			fseek(fp, Disk.Superblock.blockPosition + (node.blocks[i] * Disk.Superblock.blockSize), SEEK_SET);

			for(int i = 0; i < Disk.Superblock.blockSize; i++){
				fwrite(&temp, 1, 1, fp);
			}

			markBlocksBitmap(fp, node.blocks[i], '0');

			node.blocks[i] = -1;
			
		}
	}	
	

	node.fileSize = filesize;

	for(int i = 0; i < blockNumberOfFile; i++){
		node.blocks[i] = freeBlock();

		markBlocksBitmap(fp, node.blocks[i], '1');

	}	

	int j = 0;
	int k = 0;
	

	fseek(pf, 0L, SEEK_SET);
	fseek(fp, Disk.Superblock.blockPosition + (node.blocks[j] * Disk.Superblock.blockSize), SEEK_SET);

	for(int i = 0; i < filesize; i++, k++){

		if(k + 1 == Disk.Superblock.blockSize){
			j++;
			k = 0;
			fseek(fp, Disk.Superblock.blockPosition + (node.blocks[j] * Disk.Superblock.blockSize), SEEK_SET);
		}

		fread(&temp, 1, 1, pf);
		fwrite(&temp, 1, 1, fp);
	}

	temp = '\0';

	fwrite(&temp, 1, 1, fp);
	
	time_t c_time;
	c_time = time(0);

		
	
	if(targetInodeNumber == -1){

		node.parent = inode.InodeNumber;
		inode.lastmodification = c_time;

		markInodeBitmap(fp, freeInodeNum, '1');
		writeInodeToFile(fp, inode, inode.InodeNumber);
	}

	node.lastmodification = c_time;
	node.type = 0;

	writeInodeToFile(fp, node, node.InodeNumber);

	fclose(fp);
	fclose(pf);

}

void read(char* path, char* filename, char* linuxFilePath){

	if(path == NULL){		// path girilmemis yanlis kullanim
		fprintf(stderr, "Wrong usage!!!\n");
		return ;
	}

	int freeInodeNum = freeInode();

	if(freeInodeNum == -1){			//bos inode yok
		fprintf(stderr, "There is no free space!!!\n");
		return ;
	}


	int size = findPathSize(path);

	if(size == 0){		// path yanlis girilmis yanlıs kullanim
		fprintf(stderr, "Wrong usage!!!\n");
		return ;
	}

	char *pathArray[size];

	dividePath(path, pathArray, size);


	FILE *fp;
	fp = fopen(filename,"r");

	if(!fp){
		printf("!!!!!!FILE '%s' is not open\n",filename);
		return;
	}

	char name[256];
	int targetInodeNumber;

	int InodeNumber = pathCheck(fp, pathArray, size, name, &targetInodeNumber);

	if(InodeNumber == -2){		// path yanlis girilmis yanlıs kullanim
		fprintf(stderr, "There is no '%s' dirrectory!!!\n",name);
		fclose(fp);
		return ;
	}

	if(targetInodeNumber == -1){		//ayni isimde directory var mi kontrolu
		fprintf(stderr, "read : There is no '%s' file!!!\n",name);
		fclose(fp);
		return ;
	}


	Inodes node;
	
	fseek(fp, Disk.Superblock.InodePosition + (targetInodeNumber * sizeof(Inodes)), SEEK_SET);
	
	fread(&node, sizeof(Inodes), 1, fp);

	if(node.type != 0){
		fprintf(stderr, "'%s' is not a file!!!\n",name);
		fclose(fp);
		return;
	}

	while(node.isSymbolicLink == 1){
			
		fseek(fp, Disk.Superblock.blockPosition + (node.blocks[0] * Disk.Superblock.blockSize), SEEK_SET);
		
		char block[Disk.Superblock.blockSize];
		
		fread(block, sizeof(block), 1, fp);
		
		char *token; 
		char *token2; 

		token = strtok (block," ");
		token2 = strtok (NULL, " ");

		fseek(fp, Disk.Superblock.InodePosition + (atoi(token2) * sizeof(Inodes)), SEEK_SET);

		fread(&node, sizeof(node), 1, fp);

		if(Disk.bitmapInode[node.InodeNumber] == 0 || strcmp(node.fileName, token) != 0){
			fprintf(stderr, "write : cannot create file, this is symbolic link file and its linked file deleted!!!\n");
			fclose(fp);
			return ;
		}

	
	}

	FILE *pf;
	pf = fopen(linuxFilePath,"w");

	if(!fp){
		printf("!!!!!!FILE '%s' is not open\n",linuxFilePath);
		return;
	}


	int blockNum = 0;
	int k;
	char temp;
	fseek(fp, Disk.Superblock.blockPosition + (node.blocks[blockNum] * Disk.Superblock.blockSize), SEEK_SET);

	for(int i = 0; i < node.fileSize; i++){
		if(k + 1 == Disk.Superblock.blockSize){
			blockNum++;
			k = 0;
			fseek(fp, Disk.Superblock.blockPosition + (node.blocks[blockNum] * Disk.Superblock.blockSize), SEEK_SET);
		}

		fread(&temp, 1, 1, fp);
		fwrite(&temp, 1, 1, pf);

	}
	

	fclose(fp);
	fclose(pf);

}

string findStartAndEnd(FILE *fp, Inodes inode, char *filename, int *blockNumber){

	char *token; 
	char *token2; 
	int num;
	string block = ""; 
	char Block[Disk.Superblock.blockSize];
	char Block2[Disk.Superblock.blockSize];

	for(int i = 0; i < 10; i++){

		if(inode.blocks[i] != -1){

			fseek(fp, Disk.Superblock.blockPosition + (inode.blocks[i] * Disk.Superblock.blockSize), SEEK_SET);
			fread(Block, Disk.Superblock.blockSize, 1, fp);


			token = strtok (Block," ");
			token2 = strtok (NULL, " ");

			while (token != NULL){

				if(strcmp(token, filename) == 0){
					num = i;
				}

			    token = strtok (NULL, " ");
			  	token2 = strtok (NULL, " ");

			}
		}
	}

	*blockNumber = num;

	fseek(fp, Disk.Superblock.blockPosition + (inode.blocks[num] * Disk.Superblock.blockSize), SEEK_SET);
	fread(Block2, Disk.Superblock.blockSize, 1, fp);


	token = strtok (Block2," ");
	token2 = strtok (NULL, " ");

	while (token != NULL){

		if(strcmp(token, filename) != 0){
			block += token;
			block += ' ';
			block += token2;
			block += ' ';
		}

	    token = strtok (NULL, " ");
	  	token2 = strtok (NULL, " ");

	}

	return block;
}

void rmdir(char* path, char* filename){

	if(path == NULL){		// path girilmemis yanlis kullanim
		fprintf(stderr, "Wrong usage!!!\n");
		return ;
	}

	int size = findPathSize(path);

	if(size == 0){		// path yanlis girilmis yanlıs kullanim
		fprintf(stderr, "Wrong usage!!!\n");
		return ;
	}

	char *pathArray[size];

	dividePath(path, pathArray, size);


	FILE *fp;
	fp = fopen(filename,"rw+");

	if(!fp){
		printf("!!!!!!FILE '%s' is not open\n",filename);
		return;
	}

	char name[256];
	int targetInodeNumber;

	int InodeNumber = pathCheck(fp, pathArray, size, name, &targetInodeNumber);

	if(InodeNumber == -2){		// path yanlis girilmis yanlıs kullanim
		fprintf(stderr, "There is no '%s' dirrectory!!!\n",name);
		fclose(fp);
		return ;
	}

	if(targetInodeNumber == -1){		//ayni isimde directory var mi kontrolu
		fprintf(stderr, "There is no '%s' dirrectory!!!\n",name);
		fclose(fp);
		return ;
	}


	fseek(fp, Disk.Superblock.InodePosition + (targetInodeNumber * sizeof(Inodes)), SEEK_SET); 

	Inodes node;
	fread(&node, sizeof(Inodes), 1, fp);

	if(node.type != 1 || node.isSymbolicLink == 1){
		fprintf(stderr, "'%s' is not a dirrectory!!!\n",name);
		fclose(fp);
		return;
	}

	fseek(fp, Disk.Superblock.blockPosition + (node.blocks[0] * Disk.Superblock.blockSize), SEEK_SET);

	char temp;
	fread(&temp, 1, 1, fp);

	if(temp != '\0'){
		fprintf(stderr, "rmdir: failed to remove '%s': Directory not empty!!!\n",node.fileName);
		return;
	}


	fseek(fp, Disk.Superblock.InodePosition + (InodeNumber * sizeof(Inodes)), SEEK_SET); 		//root inode alındı

	Inodes inode;
	fread(&inode, sizeof(Inodes), 1, fp);


	int blockNumber;
	int num = 0;
	temp = '\0';
	string block = findStartAndEnd(fp, inode, name, &blockNumber);



	fseek(fp, Disk.Superblock.blockPosition + (inode.blocks[blockNumber] * Disk.Superblock.blockSize), SEEK_SET);
	
	char Block[Disk.Superblock.blockSize];

	if(block != ""){

		strcpy(Block, block.c_str());
		Block[strlen(Block)] = '\0';
					
		for(int i = strlen(Block); i < Disk.Superblock.blockSize; i++)
			Block[i] = '\0';

		fwrite(Block, Disk.Superblock.blockSize, 1, fp);


		num = 1;
	}else{
		for(int i = strlen(Block); i < Disk.Superblock.blockSize; i++)
			Block[i] = '\0';
		fwrite(Block, Disk.Superblock.blockSize, 1, fp);
		fseek(fp, Disk.Superblock.blockPosition + (inode.blocks[blockNumber] * Disk.Superblock.blockSize), SEEK_SET);
	}




	if(num == 1 && blockNumber != 0){
		markInodeBitmap(fp, inode.blocks[blockNumber], '0');
		inode.blocks[blockNumber] = -1;
		
	}



	markBlocksBitmap(fp, node.blocks[0], '0');
	
	node.blocks[0] = -1;

	markInodeBitmap(fp, node.InodeNumber, '0');


	time_t c_time;

	c_time = time(0);

	inode.lastmodification = c_time;

	writeInodeToFile(fp, inode, inode.InodeNumber);
	writeInodeToFile(fp, node, node.InodeNumber);


	fclose(fp);

}



void mkdir(char* path, char* filename){

	
	if(path == NULL){		// path girilmemis yanlis kullanim
		fprintf(stderr, "Wrong usage!!!\n");
		return ;
	}

	int freeInodeNum = freeInode();

	if(freeInodeNum == -1){			//bos inode yok
		fprintf(stderr, "There is no free space!!!\n");
		return ;
	}


	int size = findPathSize(path);

	if(size == 0){		// path yanlis girilmis yanlıs kullanim
		fprintf(stderr, "Wrong usage!!!\n");
		return ;
	}

	char *pathArray[size];

	dividePath(path, pathArray, size);


	FILE *fp;
	fp = fopen(filename,"rw+");

	if(!fp){
		printf("!!!!!!FILE '%s' is not open\n",filename);
		return;
	}

	char name[256];
	int targetInodeNumber;

	int InodeNumber = pathCheck(fp, pathArray, size, name, &targetInodeNumber);

	if(InodeNumber == -2){		// path yanlis girilmis yanlıs kullanim
		fprintf(stderr, "There is no '%s' dirrectory!!!\n",name);
		fclose(fp);
		return ;
	}

	if(targetInodeNumber != -1){		//ayni isimde directory var mi kontrolu
		fprintf(stderr, "mkdir : cannot create directory '%s': File exists!!!\n",name);
		fclose(fp);
		return ;
	}


	char newInodeInformation[300];
	sprintf(newInodeInformation,"%s %d",name,freeInodeNum); 

	fseek(fp, Disk.Superblock.InodePosition + (InodeNumber * sizeof(Inodes)), SEEK_SET); 		//root inode alındı

	Inodes inode;
	fread(&inode, sizeof(Inodes), 1, fp);

	Inodes node;
	
	fseek(fp, Disk.Superblock.InodePosition + (freeInodeNum * sizeof(Inodes)), SEEK_SET);
	
	fread(&node, sizeof(Inodes), 1, fp);
	node.type = 1;
	node.fileSize = Disk.Superblock.blockSize;
	strcpy(node.fileName, name);
				
	int blockNum = 0;

	while(inode.blocks[blockNum] != -1){
		blockNum++;
	}

	blockNum--;

	fseek(fp, Disk.Superblock.blockPosition + (inode.blocks[blockNum] * Disk.Superblock.blockSize), SEEK_SET);
	char block[Disk.Superblock.blockSize];

	fread(block, Disk.Superblock.blockSize, 1, fp);

	int freeblock = freeBlock();


	int num = 0;

	if(strlen(block) + strlen(newInodeInformation) + 2 > Disk.Superblock.blockSize){
		
		if(freeblock == -1){
			fprintf(stderr, "There is no free space!!!\n");
			fclose(fp);
			return;
		}

		markBlocksBitmap(fp, freeblock, '1');

		inode.blocks[blockNum + 1] = freeblock;

		inode.fileSize += Disk.Superblock.blockSize;
		blockNum++;

		num++;
	}

	int freeblock2 = freeBlock();
	char temp = ' ';

	if(freeblock2 == -1){
		fprintf(stderr, "There is no free space!!!\n");
		markBlocksBitmap(fp, freeblock, '0');
		fclose(fp);
		return;
	}

	markBlocksBitmap(fp, freeblock2, '1');

	node.blocks[0] = freeblock2;

	if(num == 1)
		fseek(fp, Disk.Superblock.blockPosition + (inode.blocks[blockNum] * Disk.Superblock.blockSize), SEEK_SET);
	else{
		fseek(fp, Disk.Superblock.blockPosition + (inode.blocks[blockNum] * Disk.Superblock.blockSize) + strlen(block), SEEK_SET);
		
		if(strlen(block) != 0)
			fwrite(&temp, 1, 1, fp);

	}


	for(int i = 0; i < strlen(newInodeInformation); i++ ){	
		fwrite(&newInodeInformation[i], 1, 1, fp);
	}

	temp = '\0';

	fwrite(&temp, 1, 1, fp);

	fseek(fp, Disk.Superblock.blockPosition + (node.blocks[0] * Disk.Superblock.blockSize), SEEK_SET);
	fwrite(&temp, 1, 1, fp);
	
	
	node.parent = inode.InodeNumber;

	time_t c_time;

	c_time = time(0);

	node.lastmodification = c_time;
	inode.lastmodification = c_time;

	markInodeBitmap(fp, freeInodeNum, '1');

	writeInodeToFile(fp, node, node.InodeNumber);
	writeInodeToFile(fp, inode, inode.InodeNumber);
	

	fclose(fp);

}




void readFromDisk(char* filename){
	

	Disk.freeInodeNumber = 0;
	Disk.freeBlockNumber = 0;

	FILE *fp;
	fp = fopen(filename,"r");
	
	if(!fp){
		printf("!!!!!!FILE '%s' is not open\n",filename);
		return;
	}

	fread(&Disk.Superblock, sizeof(superblock), 1, fp);

	Disk.bitmapInode = new char[Disk.Superblock.numberOfInodes];
	Disk.bitmapBlocks = new char[Disk.Superblock.numberOfBlocks];

	fread(Disk.bitmapInode, Disk.Superblock.numberOfInodes, 1, fp);
	
	for(int i = 0; i < Disk.Superblock.numberOfInodes; i++)
		if(Disk.bitmapInode[i] == '0')
			Disk.freeInodeNumber++;

	fread(Disk.bitmapBlocks, Disk.Superblock.numberOfBlocks, 1, fp);	
	
	for(int i = 0; i < Disk.Superblock.numberOfBlocks; i++)
		if(Disk.bitmapBlocks[i] == '0')
			Disk.freeBlockNumber++;

	fclose(fp);

}



int freeInode(){

	for(int i = 0; i < Disk.Superblock.numberOfInodes; i++)
		if(Disk.bitmapInode[i] == '0')
			return i;
	
	return -1;

}

int freeBlock(){

	for(int i = 0; i < Disk.Superblock.numberOfBlocks; i++)
		if(Disk.bitmapBlocks[i] == '0')
			return i;
	
	return -1;

}

void markInodeBitmap(FILE *fp, int location, char mark){

	fseek(fp, sizeof(Disk.Superblock) + location, SEEK_SET);

	fwrite(&mark, sizeof(mark), 1, fp);

	Disk.bitmapInode[location] = mark;
	
	if(mark == '1')
		Disk.freeInodeNumber--;
	else
		Disk.freeInodeNumber++;
}

void markBlocksBitmap(FILE *fp, int location, char mark){

	fseek(fp, sizeof(Disk.Superblock) + Disk.Superblock.numberOfInodes + location, SEEK_SET);

	fwrite(&mark, sizeof(mark), 1, fp);

	Disk.bitmapBlocks[location] = mark;

	if(mark == '1')
		Disk.freeBlockNumber--;
	else
		Disk.freeBlockNumber++;
}	

void writeInodeToFile(FILE *fp, Inodes inode, int location){

	fseek(fp, Disk.Superblock.InodePosition + (location * Disk.Superblock.InodeSize) , SEEK_SET);

	fwrite(&inode, sizeof(inode), 1, fp);

}




int findPathSize(char *Path){

	char path[1000];
	strcpy(path,Path);

	char *token;
	int size = 0;

	token = strtok (path,"/");

	while (token != NULL){

		size++;

		token = strtok (NULL, "/");

	}

	return size;
}

void dividePath(char *path, char **pathArray, int size){

	char *token = strtok (path,"/");

	int i = 0;

	pathArray[i] = token;

  	while (token != NULL){

	  	i++;

	    token = strtok (NULL, "/");
	  	
	  	pathArray[i] = token;

	}


}




int get_inodeNumber(FILE *fp, int *InodeNumber, char* filename){
	
	
	fseek(fp, Disk.Superblock.InodePosition + (*InodeNumber * sizeof(Inodes)), SEEK_SET);

	Inodes inode;
	fread(&inode, sizeof(inode), 1, fp);

	if(inode.type != 1)
		return -1;
	
	while(inode.isSymbolicLink == 1){
		
		fseek(fp, Disk.Superblock.blockPosition + (inode.blocks[0] * Disk.Superblock.blockSize), SEEK_SET);
		char block[Disk.Superblock.blockSize];
		fread(block, sizeof(block), 1, fp);
		char *token; 
		char *token2; 

		token = strtok (block," ");
		token2 = strtok (NULL, " ");

		fseek(fp, Disk.Superblock.InodePosition + (atoi(token2) * sizeof(Inodes)), SEEK_SET);

		fread(&inode, sizeof(inode), 1, fp);

		*InodeNumber = atoi(token2);
		if(strcmp(inode.fileName,token) != 0 && inode.blocks[0] == -1)
			return -1;
	}


	int i = 0;


	while(inode.blocks[i] != -1){
		fseek(fp, Disk.Superblock.blockPosition + (inode.blocks[i] * Disk.Superblock.blockSize), SEEK_SET);

		char block[Disk.Superblock.blockSize];
		fread(block, sizeof(block), 1, fp);

		char *token; 
		token = strtok (block," ");
		char *token2; 
		token2 = strtok (NULL, " ");

		while (token != NULL){

			if(strcmp(token, filename) == 0){
				return atoi(token2);
			}

		    token = strtok (NULL, " ");
		  	token2 = strtok (NULL, " ");

		}

		i++;
	}

	return -1;

}

int pathCheck(FILE *fp, char *pathArray[], int size, char* name, int *targetInodeNumber){

	int temp;
	int InodeNumber = 0;

	for(int i = 0; i < size; i++){

		strcpy(name, pathArray[i]);

		temp = get_inodeNumber(fp, &InodeNumber, pathArray[i]);

		*targetInodeNumber = temp;

		if(temp == -1 && i + 1 == size)
			return InodeNumber;

		if(temp == -1)
			return -2;

		if(i + 1 == size)
			return InodeNumber;
		InodeNumber = temp;
	}

	

}


void divideBlock(char *path, char **pathArray){

	char *token = strtok (path," ");

	int i = 0;

	pathArray[i] = token;

  	while (token != NULL){

	  	i++;

	    token = strtok (NULL, " ");
	  	
	  	pathArray[i] = token;

	}

}


void recursiveSearch(FILE *fp, int InodeNumber, int blockArray[], int inodeArray[]){
	
	fseek(fp, Disk.Superblock.InodePosition + (InodeNumber * sizeof(Inodes)), SEEK_SET);

	Inodes inode;
	fread(&inode, sizeof(Inodes), 1, fp);

	int blockNumber = 0;
	
	while(inode.blocks[blockNumber] != -1){
		
		if(inode.type == 1 && inode.isSymbolicLink != 1){
			
			fseek(fp, Disk.Superblock.blockPosition + (inode.blocks[blockNumber] * Disk.Superblock.blockSize), SEEK_SET);
			char block[Disk.Superblock.blockSize];

			fread(block, Disk.Superblock.blockSize, 1, fp);
			
			char *pathArray[Disk.Superblock.numberOfInodes];
			
			divideBlock(block, pathArray);

			int i = 0;

			while (pathArray[i] != NULL){

				
				int inodenumber = atoi(pathArray[i + 1]);
				fseek(fp, Disk.Superblock.InodePosition + (inodenumber * sizeof(Inodes)), SEEK_SET); 		//root inode alındı

				Inodes node;
				fread(&node, sizeof(Inodes), 1, fp);

				if(strcmp(node.fileName, pathArray[i]) == 0){
					inodeArray[inodenumber]++;

					recursiveSearch(fp, inodenumber, blockArray, inodeArray);
				}
				

			  	i += 2;
			}
		}

		blockArray[inode.blocks[blockNumber]]++;

		blockNumber++;
	}
}

void findDirAndFilesNumber(char* filename, int *filesNumber, int *dirNumber){

	FILE *fp;
	fp = fopen(filename,"r");

	if(!fp){
		printf("!!!!!!FILE '%s' is not open\n",filename);
		return;
	}


	for(int i = 0; i < Disk.Superblock.numberOfInodes; i++){
		if(Disk.bitmapInode[i] == '1'){
			fseek(fp, Disk.Superblock.InodePosition + (i * sizeof(Inodes)), SEEK_SET); 
			Inodes inode;
			fread(&inode, sizeof(Inodes), 1, fp);

			if(inode.type == 0){
				*filesNumber++;
			}else if(inode.type == 1){
				*dirNumber++;
			}
		}
	}
}

void recursiveWriteFiles(FILE *fp, int InodeNumber, char *filename){
	
	fseek(fp, Disk.Superblock.InodePosition + (InodeNumber * sizeof(Inodes)), SEEK_SET);

	Inodes inode;
	fread(&inode, sizeof(Inodes), 1, fp);

	int blockNumber = 0;
	if(inode.isSymbolicLink !=1){
		while(inode.blocks[blockNumber] != -1){
			
			if(inode.type == 1){
				
				fseek(fp, Disk.Superblock.blockPosition + (inode.blocks[blockNumber] * Disk.Superblock.blockSize), SEEK_SET);
				char block[Disk.Superblock.blockSize];

				fread(block, Disk.Superblock.blockSize, 1, fp);
				
				char *pathArray[Disk.Superblock.numberOfInodes];
				
				divideBlock(block, pathArray);

				int i = 0;

				while (pathArray[i] != NULL){
					
					int inodenumber = atoi(pathArray[i + 1]);

					recursiveWriteFiles(fp, inodenumber, pathArray[i]);

				  	i += 2;
				}
			}

			blockNumber++;
		}
	}
	
	if(strcmp(inode.fileName, filename) != 0)
		printf("%12s\t","Hardlink");
	else if(inode.isSymbolicLink == 1)
		printf("%12s\t","Symboliclink");
	else if(inode.type == 0)
		printf("%12s\t","File");
	else if(inode.type == 1)
		printf("%12s\t","Directory");

	printf("i-node Number : %3d\t",inode.InodeNumber);
	printf("blocks : ");
	
	for(int j = 0; j < 10; j++){
		if(inode.blocks[j] != -1)
			printf("%d ", inode.blocks[j] );
	}

	printf("\tFilename : %s\n",filename);


}		

void recursiveFindFilesNumber(FILE *fp, int InodeNumber, char *filename, int *dirNum, int *fileNum, int *hardNum, int *symNum){
	
	fseek(fp, Disk.Superblock.InodePosition + (InodeNumber * sizeof(Inodes)), SEEK_SET);

	Inodes inode;
	fread(&inode, sizeof(Inodes), 1, fp);

	int blockNumber = 0;
	if(inode.isSymbolicLink !=1){
		while(inode.blocks[blockNumber] != -1){
			
			if(inode.type == 1){
				
				fseek(fp, Disk.Superblock.blockPosition + (inode.blocks[blockNumber] * Disk.Superblock.blockSize), SEEK_SET);
				char block[Disk.Superblock.blockSize];

				fread(block, Disk.Superblock.blockSize, 1, fp);
				
				char *pathArray[Disk.Superblock.numberOfInodes];
				
				divideBlock(block, pathArray);

				int i = 0;

				while (pathArray[i] != NULL){
					
					int inodenumber = atoi(pathArray[i + 1]);

					recursiveFindFilesNumber(fp, inodenumber, pathArray[i], dirNum, fileNum, hardNum, symNum);

				  	i += 2;
				}
			}

			blockNumber++;
		}
	}
	
	if(strcmp(inode.fileName, filename) != 0)
		*hardNum += 1;
	else if(inode.isSymbolicLink == 1)
		*symNum += 1;
	else if(inode.type == 0)
		*fileNum += 1;
	else if(inode.type == 1)
		*dirNum += 1;




}	
