#include "fs.h"

int disk_fd;
//to store and manipulate the checkPoint region
checkPoint cp;



/** internal helpers **/
int getIMapNum(int pinum);
int getMapEntryNum(int inum);
int createEmptyDirectory(int inum, int pinum);
int getINode(int inum, iNode* node);
int getIMap(int inum, iMap* map);
int getNextId();
void copyNode(iNode* node1, iNode* node2);
void copyMap(iMap* node1, iMap* node2);
void insertINode(iNode node, int inum);
void createEmptyFile(int pinum);

int fs_init(char* fileName){

    //check whether the file access
    if( access(fileName, F_OK) != -1 ){
        //file exists

        //open the file
        disk_fd = open(fileName, O_RDWR|O_APPEND,S_IRWXU);
        if(disk_fd < 0){
            fprintf(stderr,"Error: Cannot open file %s\n",fileName);
            return -1;
        }

        //read the checkpoint region
        int sum = read(disk_fd, &cp, sizeof(checkPoint));
        if(sum != sizeof(checkPoint)){
            fprintf(stderr,"Error: Unable to read checkPoint region\n");
            return -1;
        }
    }
    else{
        //file doesn't exist
        //create the new file
        disk_fd = open(fileName, O_RDWR|O_CREAT,S_IRWXU);

        //make sure we can create the file
        if(disk_fd < 0){
            fprintf(stderr,"Error: Cannot create file\n");
            return -1;
        }

        //initialize the checkpoint region
        //create the first inode
        cp.endPtr = 4096; //0-4095 is the CR

        //create an Empty directory at that location
        createEmptyDirectory(0, 0);
    }
    /*
    for(int j = 0; j < 100; j++){
        printf("num:%d, return%d\n", j, getMapEntryNum(j));
    }*/
    return 0;
    //fs_print();
}

int fs_write(int inum, char *buffer, int block){

    iNode node;
    if(getINode(inum, &node) == -1){
        //cannot find Inode
        return -1;
    }
    if(node.type == FS_DIRECTORY){
        return -1;
    }
    lseek(disk_fd, cp.endPtr, SEEK_SET);
    write(disk_fd, buffer, 4096);
    int region = cp.endPtr;
    node.dataPtrs[block] = region;
    //update the region
    insertINode(node, inum);
    return 0;
}

int fs_read(int inum, char *buffer, int block){
    iNode node;
    if(getINode(inum, &node) == -1){
        //cannot find Inode
        return -1;
    }    
    lseek(disk_fd, node.dataPtrs[block], SEEK_SET);
    read(disk_fd, buffer, 4096);
    return 0;
}

int fs_unlink(int pinum, char *name){
    iNode node;
    if(getINode(pinum, &node) == -1){
        //cannot find Inode
        return -1;
    }
    if(node.type != FS_DIRECTORY){
        //not a directory
        return -1;
    }

    fs_dir_list list;
    lseek(disk_fd, node.dataPtrs[0], SEEK_SET);
    read(disk_fd, &list, sizeof(fs_dir_list));

    int j;
    for(j = 0; j < 64; j++){
        if(list.list[j].pair < 0){
            continue;
        }
        if(strncmp(name, list.list[j].name, 60) == 0){
            strncpy(list.list[j].name, "", 60);
            list.list[j].pair = -1;
            break;
        }
    }
    if(j == 64){
        //didn't found it
        return -1;
    }
    lseek(disk_fd, cp.endPtr, SEEK_SET);
    int region = cp.endPtr;
    write(disk_fd, &list, sizeof(fs_dir_list));
    node.dataPtrs[0] = region;
    insertINode(node, pinum);
    return 0;
}

int fs_close(){
    lseek(disk_fd, 0, SEEK_SET);
    //write back the critical region
    int stat = write(disk_fd, &cp, sizeof(checkPoint));
    close(disk_fd);
    //done
    return stat;
}

int fs_create(int pinum,  int type, char *name){

    //check whether already exist already exist
    if( fs_lookup(pinum, name) != -1){
        return 0;
    }

    iNode node;
    if(getINode(pinum, &node) == -1){
        //cannot find Inode
        return -1;
    }    
    //make sure the node is a directory;
    if(node.type != FS_DIRECTORY){
        return -1;
    }


    //make sure the parent has enough space to store a new entry
    lseek(disk_fd, node.dataPtrs[0], SEEK_SET);
    fs_dir_list dirList;
    //get the parent's directory list
    read(disk_fd, &dirList, sizeof(fs_dir_list));
    int i;
    for(i = 0; i < 64; i++){
        if(dirList.list[i].pair == -1){
            return i;
        }
    }

    if(i == 64){
        //the folder is full
        return -1;
    }

    //get the id of the new instance
    int newId = pinum,getNextId();
    if(type == FS_DIRECTORY){
        createEmptyDirectory(pinum,newId);
    }
    else{
        createEmptyFile(newId);
    }

    //update the new information
    dirList.list[i].pair = newId;
    strncpy(dirList.list[i].name,name,60);
    //write the whole dirList to the file system
    lseek(disk_fd, cp.endPtr, SEEK_SET);
    //the whole disk is now 
    int region = cp.endPtr;
    write(disk_fd, &dirList, sizeof(fs_dir_list));
    node.dataPtrs[0] = region;

    //update the Inode
    insertINode(node, pinum);

    return 0;
}


void fs_print(){
    int endPtr = cp.endPtr;
    printf("endPointer:%d\n", endPtr);
    printf("each INode:\n");

    for(int i = 0; i < 1; i++){
        int id = cp.mapPtrs[i];
        printf("ptr %d: ptr->%d\n", i, id);
        //move to the IMap area
        lseek(disk_fd, id, SEEK_SET);
        iMap map;
        //read the whole are
        read(disk_fd, &map, sizeof(iMap));
        for(int j = 0; j < 16; j++){
            //printf("location:%d\n", map.ptr[j]);
            if(map.ptr[j] < 0){
                continue;
            }
            iNode node;
            lseek(disk_fd,map.ptr[j], SEEK_SET);
            read(disk_fd, &node, sizeof(iNode));
            printf("size:%d, type:%d\n",node.size, node.type);

            if(node.type != 1 && node.type != 0){
                break;
            }
            if(node.type == 0){
                int counter = node.size;
                int k = 0;
                do{
                    lseek(disk_fd, node.dataPtrs[k], SEEK_SET);
                    fs_dir d;
                    read(disk_fd, &d, sizeof(fs_dir));
                    while(d.pair >= 0){    
                        printf("name:%s par:%d\n", d.name, d.pair);
                        read(disk_fd, &d, sizeof(fs_dir));
                    }
                    k++;
                    counter -= 4096;
                }while(counter > 0);
            }
            else{
                //its afile
                void* ptr = malloc(node.size);
                void* writePtr = ptr;
                int counter = node.size;
                //int read = 0;
                int k = 0;
                while(counter > 4096){
                    //nothing yet
                }
                lseek(disk_fd, node.dataPtrs[k], SEEK_SET);
                read(disk_fd, writePtr, (uint)counter);
                printf("%p", ptr);
            }
        }
    }
}


int fs_stat(int inum, stat_t *m){
    iNode node;
    if(getINode(inum, &node) == -1){
        //cannot find Inode
        return -1;
    }

    m->size = node.size;
    m->type = node.type;
    return 0;
}


//lookup the filename at the parent directory given by pinum
// - pinum: parent's inode number
// - name: filename to be search
// - rtn: file's inode number
// status: as far as I know, correct
int fs_lookup(int pinum, char *name){

    iNode node;
    if(getINode(pinum, &node) == -1){
        //cannot find Inode
        return -1;
    }

    //the inode is not a directory
    if(node.type != FS_DIRECTORY){
        printf("not directory\n");
        return -1;
    }

    fs_dir dir;
    lseek(disk_fd, node.dataPtrs[0], SEEK_SET);
    for(int j = 0; j < 64; j++){
        read(disk_fd, &dir, sizeof(fs_dir));
        //reach the end of directory list;
        if(dir.pair < 0){
            continue;
        }
        if(strncmp(name, dir.name, 60) == 0){
            return dir.pair;
        }
    }    
    //cannot find it
    printf("cannot find\n");
    return -1;
}


/** helper functions **/

//returns the correct Imap number for any number
// - pinum: the inode number
int getIMapNum(int inum){
    return inum >> 4;
}

//returns the correct entry num
int getMapEntryNum(int inum){
    return (inum & 0xF);
}

void createEmptyFile(int pinum){
    iNode node;
    node.size = 0;
    node.type = FS_REGULAR_FILE;
    for(int i = 0; i < 14; i++){
        node.dataPtrs[i] = -1;
    }
    return insertINode(node, pinum);
}

//creates an emptry directory data at end of Ptr
// - inum: inode number of the directory
// - pinum: parent's inode number
// this increments the cp.endPtr;
//return the pointer to the next free space
int createEmptyDirectory(int inum, int pinum){

    //create the directory data region
    fs_dir_list dirList;
    int i;
    for(i = 0; i < 64; i++){
        dirList.list[i].pair = -1;
        strncpy(dirList.list[i].name, "", 60);
    }

    strncpy(dirList.list[0].name, ".", 60);
    dirList.list[0].pair = inum;
    strncpy(dirList.list[1].name, "..", 60);
    dirList.list[1].pair = pinum;

    //move fd head to END
    lseek(disk_fd, cp.endPtr, SEEK_SET);
    write(disk_fd, &dirList, sizeof(fs_dir_list));

    int dataRegion = cp.endPtr;
    cp.endPtr += sizeof(fs_dir_list);

    iNode node;
    for(int i = 0; i < 14; i++){
        node.dataPtrs[i] = -1;
    }
    node.dataPtrs[0] = dataRegion;
    node.type = FS_DIRECTORY;
    node.size = 4096;
    insertINode(node, pinum);
    return 1;
}

int getINode(int inum, iNode* node){

    iMap map;
    if(getIMap(inum, &map) == -1){
        return -1;
    }

    int ptr = map.ptr[getMapEntryNum(inum)];
    //check whether the inode was created
    if(ptr == -1){
        //yap,  there is an inode;
        printf("no inode\n");
        return -1;
    }

    //now go read the inode
    iNode _node;
    lseek(disk_fd, ptr, SEEK_SET);
    read(disk_fd, &_node, sizeof(iNode));

    copyNode(node, &_node);
    return 0;
}

void insertINode(iNode node, int inum){

    lseek(disk_fd, cp.endPtr, SEEK_SET);
    write(disk_fd, &node, sizeof(iNode));
    int nodePtr = cp.endPtr;
    cp.endPtr += sizeof(iNode);

    //insert the Inode Number into the IMap and CP;
    iMap map;
    //iMap not initialize, need to create new one
    if(getIMap(inum, &map) == -1){
        for(int i = 0; i < 16; i++){
            map.ptr[i] = -1;
        }
    }
    int entryNum = getMapEntryNum(inum);
    map.ptr[entryNum] = nodePtr;
    int endPtr = cp.endPtr;
    lseek(disk_fd, cp.endPtr, SEEK_SET);
    write(disk_fd, &map, sizeof(iMap));

    //update CP
    int iMapNum = getIMapNum(inum);
    cp.mapPtrs[iMapNum] = endPtr;

    //update end pointer
    cp.endPtr += sizeof(iMap);

}

int getIMap(int inum, iMap* map){
    //get the which IMap the parent is int
    int iMapNum = getIMapNum(inum);
    printf("iMapNum:%d\n", iMapNum);
    //make sure the imap has been initialize
    if(cp.mapPtrs[iMapNum] == 0){
        //the imap is not initialize
        printf("wrong imap\n");

        return -1;
    }
    iMap _map;
    //read Imap
    lseek(disk_fd, cp.mapPtrs[iMapNum], SEEK_SET);
    read(disk_fd, &_map, sizeof(iMap));

    copyMap(map, &_map); 
    return 0;   
}

void copyMap(iMap* map1, iMap* map2){
    for(int i = 0; i < 16; i++){
        map1->ptr[i] = map2->ptr[i];
    }    
}

void copyNode(iNode* node1, iNode* node2){
    node1->type = node2->type;
    node1->size = node2->size;
    for(int i = 0; i < 14; i++){
        node1->dataPtrs[i] = node2->dataPtrs[i];
    }
}

int getNextId(){
    iNode node;
    for(int i = 0; i < 4096; i++){
        if(getINode(i, &node) == -1){
            return i;
        }
    }
    return -1;
}
