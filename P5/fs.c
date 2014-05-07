/* CS537 - Spring 2014 - Program 5
 * CREATED BY:  
 * Xiang Zhi Tan (xtan@cs.wisc.edu)
 * Roy Fang (fang@cs.wisc.edu)
 */

#include "fs.h"

/** Global Variables **/

//file descriptor for the harddisk
int disk_fd;
//to store and manipulate the checkPoint region
//faster access than reading from memory.
checkPoint cp;


/** internal helpers **/
int getIMapNum(int pinum);
int getMapIndexNum(int inum);
int createEmptyDirectory(int inum, int pinum);
int getINode(int inum, iNode* node);
int getIMap(int inum, iMap* map);
int getNextId();
int insertINode(iNode node, int inum);
int createEmptyFile(int pinum);
int deleteINode(int inum);
int writeToEnd(char* buffer, int size);
int getINodeSize(iNode node);

/**
 * fs_init
 * description in header
 */
int fs_init(char* fileName){

    //check whether the file access
    if( access(fileName, F_OK) != -1 ){
        //file exists

        //open the file
        disk_fd = open(fileName, O_RDWR,S_IRWXU);
        if(disk_fd < 0){
            fprintf(stderr,"Error: Cannot open file %s\n",fileName);
            return -1;
        }

        //read the checkpoint region
        lseek(disk_fd, 0, SEEK_SET);
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
        cp.endPtr = sizeof(checkPoint); //0-4095 is the CR

        //create an Empty directory at that location
        createEmptyDirectory(0, 0);
    }
    return 0;
}

/**
 * fs_write
 * description in header
 */
int fs_write(int inum, char *buffer, int block){

    iNode node;
    if(getINode(inum, &node) == -1){
        //cannot find Inode
        return -1;
    }

    if(node.type == FS_DIRECTORY){
        //the iNode refers to Directory, fails
        return -1;
    }
    
    //save the data region to write
    node.dataPtrs[block] = cp.endPtr;
    //write to file
    writeToEnd(buffer, BUFFER_SIZE);
    //calculate the new size
    node.size = getINodeSize(node);
    //update the inum
    insertINode(node, inum);
    return 0;
}

int fs_read(int inum, char *buffer, int block){
    iNode node;
    if(getINode(inum, &node) == -1){
        //cannot find Inode
        return -1;
    }
    if(node.type == FS_REGULAR_FILE){
        //if its regular file,  just write it
        lseek(disk_fd, node.dataPtrs[block], SEEK_SET);
        read(disk_fd, buffer, BUFFER_SIZE);
        return 0;     
    }
    else{
        //if its a directory

        //find parent to get the name
        fs_dir_list childList;
        lseek(disk_fd, node.dataPtrs[0], SEEK_SET);
        read(disk_fd, &childList, sizeof(fs_dir_list));
        //get parent's id
        int pinum = childList.list[1].pair;
        iNode pnode;
        getINode(pinum, &pnode);
        fs_dir_list pList;
        //get parent's directory
        lseek(disk_fd, pnode.dataPtrs[0], SEEK_SET);
        read(disk_fd, &pList, sizeof(fs_dir_list));
        //loop through the parent's list to find the pairing.
        for(int i = 0; i < 64; i++){
            if(pList.list[i].pair < 0){
                continue;
            }
            if(pList.list[i].pair == inum){
                memcpy(buffer, &(pList.list[i]), sizeof(fs_dir));
                return 0;
            }
        }
        return -1;
    }
}

/**
 * fs_unlink
 * description in header
 */
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

    int i;
    int j;
    bool found = false;
    fs_dir_list list;
    //loop through the parent's directory list to find the pairing
    for(i = 0; i < 14; i++){
        if(node.dataPtrs[i] == -1){
            continue;
        }
        lseek(disk_fd, node.dataPtrs[i], SEEK_SET);
        read(disk_fd, &list, sizeof(fs_dir_list));

        for(j = 0; j < 64; j++){
            if(list.list[j].pair < 0){
                continue;
            }
            //check if the name is the same
            if(strncmp(name, list.list[j].name, 60) == 0){
                //if yes,  try deleting the inode first
                int stat = deleteINode(list.list[j].pair);
                //if can fail because its a directory and 
                //the chidlren is not deleted
                if(stat == -1){
                    return -1;
                }
                //delete the information
                strncpy(list.list[j].name, "", 60);
                list.list[j].pair = -1;
                found = true;
                break;
            }
        }

        if(found){
            break;
        }

    }

    //if both counter is MAXED out, means they cannot find it
    if(i == 14 && j == 64){
        //didn't found it
        return 0;
    }

    //update the new region for the new directory list
    node.dataPtrs[i] = cp.endPtr;

    //write data to the region
    lseek(disk_fd, cp.endPtr, SEEK_SET);    
    write(disk_fd, &list, sizeof(fs_dir_list));
    cp.endPtr += sizeof(fs_dir_list);
    //update i_Node
    insertINode(node, pinum);
    return 0;
}

/**
 * fs_close
 * description in header
 */
int fs_close(){
    fs_fsync();
    close(disk_fd);
    return 0;
}

/**
 * fs_create
 * description in header
 */
int fs_create(int pinum,  int type, char *name){
    //check whether already exist already exist
    if( fs_lookup(pinum, name) != -1){
        return 0;
    }

    iNode node;
    //make sure we can find the parent
    if(getINode(pinum, &node) == -1){
        return -1;
    }    
    //make sure the node is a directory;
    if(node.type != FS_DIRECTORY){
        return -1;
    }

    //get the id of the new instance
    int newId = getNextId();
    //cannot find an Id for it
    if(newId == -1){
        return -1;
    }

    int i;
    int j;
    fs_dir_list dirList;

    //loop through the parent's directory list to find an 
    //empty spot to insert the new file
    for(i = 0; i < 14; i++){
        //the data region is not initialized
        if(node.dataPtrs[i] == -1){
            //initialize it
            for(j = 0; j < 64; j++){
                strncpy(dirList.list[j].name,"",60);
                dirList.list[j].pair = -1;
            }
            j = 0;
            break;
        }
        lseek(disk_fd, node.dataPtrs[i], SEEK_SET);
        read(disk_fd, &dirList, sizeof(fs_dir_list));
        for(j = 0; j < 64; j++){
            //found an empty part
            if(dirList.list[j].pair < 0){
                break;
            }
        }

        if(j < 64 && dirList.list[j].pair < 0){
            break;
        }
    }

    //if MAXED out, means the folder is full
    if(i == 14 && j == 64){
        return -1;
    }

    //depend on type, create different elements
    if(type == FS_DIRECTORY){
        createEmptyDirectory(newId, pinum);
    }
    else{
        createEmptyFile(newId);
    }

    //update the new information
    dirList.list[j].pair = newId;
    strncpy(dirList.list[j].name,name,60);

    //write the new parent's directory list
    node.dataPtrs[i] = cp.endPtr;
    writeToEnd((char*)&dirList, sizeof(fs_dir_list));

    //update the Inode
    insertINode(node, pinum);
    return 0;
}

/**
 * fs_stat
 * description in header
 */
int fs_stat(int inum, stat_t *m){
    iNode node;
    if(getINode(inum, &node) == -1){
        //cannot find Inode
        return -1;
    }

    //copy the needed informations
    m->size = node.size;
    m->type = node.type;
    return 0;
}

/**
 * fs_fsync
 * description in header
 */
int fs_fsync(){
    lseek(disk_fd, 0, SEEK_SET);
    //write back the critical region
    int stat = write(disk_fd, &cp, sizeof(checkPoint));
    if(stat < 0){
        return -1;
    }
    return fsync(disk_fd);
}

/**
 * fs_lookup
 * description in header
 */
int fs_lookup(int pinum, char *name){

    iNode node;
    if(getINode(pinum, &node) == -1){
        //cannot find Inode
        return -1;
    }

    //the inode is not a directory
    if(node.type != FS_DIRECTORY){
        return -1;
    }

    fs_dir_list list;
    //loop through the whole parent's directory to find the pairing.
    for(int i = 0 ; i < 14; i++){
        if(node.dataPtrs[i] == -1){
            continue;
        }

        lseek(disk_fd, node.dataPtrs[i], SEEK_SET);
        read(disk_fd, &list, sizeof(fs_dir_list));

        for(int j = 0; j < 64; j++){
    
            if(list.list[j].pair < 0){
                continue;
            }

            if(strncmp(name, list.list[j].name, 60) == 0){
                return list.list[j].pair;
            }
        }
    }

    return -1;
}


/** helper functions **/

/**
 * getIMapNum
 * returns the correct Imap number of any number
 * - pinum: the inode number
 * - return: the Imap number
 */
int getIMapNum(int inum){
    return inum >> 4;
}

/**
 * getMapIndexNum
 * returns the correct map index number
 * - inum: the inode number
 * - return: the map entry index
 */
int getMapIndexNum(int inum){
    return (inum & 0xF);
}

/**
 * createEmptyFile
 * create an Empty File and insert it
 * - inum: iNode number of the file
 * return: 0 for success, -1 if failed
 */
int createEmptyFile(int inum){
    iNode node;
    node.size = 0;
    node.type = FS_REGULAR_FILE;
    for(int i = 0; i < 14; i++){
        node.dataPtrs[i] = -1;
    }
    return insertINode(node, inum);
}


/**
 * createEmptyDirectory
 * creates an emptry directory data and insert it
 * - inum: inode number of the directory
 * - pinum: parent's inode number
 * - return: 0 for success, -1 if failed
 */
int createEmptyDirectory(int inum, int pinum){

    //create the directory data region
    fs_dir_list dirList;
    int i;
    for(i = 0; i < 64; i++){
        dirList.list[i].pair = -1;
        strncpy(dirList.list[i].name, "", 60);
    }
    //create the . and ..
    strncpy(dirList.list[0].name, ".", 60);
    dirList.list[0].pair = inum;
    strncpy(dirList.list[1].name, "..", 60);
    dirList.list[1].pair = pinum;

    int dataRegion = cp.endPtr;
    writeToEnd( (char*)&dirList, sizeof(fs_dir_list));

    iNode node;
    for(int i = 0; i < 14; i++){
        node.dataPtrs[i] = -1;
    }
    node.dataPtrs[0] = dataRegion;
    node.type = FS_DIRECTORY;
    node.size = BUFFER_SIZE;
    return insertINode(node, inum);
}

/**
 * getINode
 * tries getting the INode of the INode number given
 * - inum: the INode number
 * - node: to store the Inode
 * return: 0 for success, -1 if failed
 */
int getINode(int inum, iNode* node){

    iMap map;
    if(getIMap(inum, &map) == -1){
        //printf("no imap in getInode\n");
        return -1;
    }

    int ptr = map.ptr[getMapIndexNum(inum)];
    //check whether the inode was created
    if(ptr == -1){
        //yap,  there is an inode;
        //printf("no inode in getInode\n");
        return -1;
    }

    //now go read the inode
    lseek(disk_fd, ptr, SEEK_SET);
    int status = read(disk_fd, node, sizeof(iNode));
    //make sure the read was successful
    if(status < 0){
        return -1;
    }
    return 0;
}

/**
 * getINode
 * tries deleting the iNode and related files
 * - inum: the INode number
 * return: 0 for success, -1 if failed
 */
int deleteINode(int inum){

    //check whether its a direcotry
    iNode node;
    if(getINode(inum, &node) == -1){
        //try deleting and INode that never exists
        return 0;
    };

    if(node.type == FS_DIRECTORY){
        //if its a directory
        int i;
        int j;
        fs_dir_list dirList;
        //make sure its empty
        for(i = 0; i < 14; i++){
            if(node.dataPtrs[i] == -1){
                continue;
            }

            lseek(disk_fd, node.dataPtrs[i], SEEK_SET);
            //get the parent's directory list
            read(disk_fd, &dirList, sizeof(fs_dir_list));

            //to avoid the first 2 entries, . and ..
            if(i == 0){
                j = 2;
            }
            else{
                j = 0;
            }

            for(; j < 64; j++){
                if(dirList.list[j].pair != -1){
                    return -1;
                }
            }
        }
    }


    iMap map;
    //update the IMaps
    getIMap(inum, &map);

    //update the map pointer
    map.ptr[getMapIndexNum(inum)] = -1;

    cp.mapPtrs[getIMapNum(inum)] = cp.endPtr;
    writeToEnd( (char*)&map, sizeof(iMap));
    return 0;
}

/**
 * insetINode
 * tries insert or update if exist and Inode
 * with the iNode number
 * - inum: the INode number
 * - node: the node to be stored
 * return: 0 for success, -1 if failed
 */
int insertINode(iNode node, int inum){
    int nodePtr = cp.endPtr;
    int status = writeToEnd( (char*)&node, sizeof(iNode));
    if(status < 0){
        return -1;
    }
    //insert the Inode Number into the IMap and CP;
    iMap map;
    //iMap not initialize, need to create new one
    if(getIMap(inum, &map) == -1){
        for(int i = 0; i < 16; i++){
            map.ptr[i] = -1;
        }
    }
    //make sure the map is pointing at the new node
    map.ptr[getMapIndexNum(inum)] = nodePtr;

    //make sure the cp is pointing at the new map
    cp.mapPtrs[getIMapNum(inum)] = cp.endPtr;
    //write the new map to the end
    status = writeToEnd( (char*)&map, sizeof(iMap));
    if(status < 0){
        return -1;
    }
    return 0;
}

/**
 * getMap
 * tries getting the IMap of the INode number given
 * - inum: the INode number
 * - map: to store the map
 * return: 0 for success, -1 if failed
 */
int getIMap(int inum, iMap* map){
    //get the which IMap the parent is int
    int iMapNum = getIMapNum(inum);
    //make sure the imap has been initialize
    if(cp.mapPtrs[iMapNum] == 0){
        //the imap is not initialize
        return -1;
    }
    //read Imap
    lseek(disk_fd, cp.mapPtrs[iMapNum], SEEK_SET);
    int status = read(disk_fd, &map, sizeof(iMap));
    if(status < 0){
        return -1;
    }
    return 0;   
}

/**
 * getNextId
 * returns the next available ID
 * return: id when success, -1 if cannot find one
 */
int getNextId(){
    iNode node;
    for(int i = 0; i < BUFFER_SIZE; i++){
        if(getINode(i, &node) == -1){
            return i;
        }
    }
    return -1;
}

/**
 * writeToEnd
 * write the buffer to the end of the data disk
 * - buffer : the buffer to be written
 * - size: the size to be written
 * return: whether its successful
 */
int writeToEnd(char* buffer, int size){
    int status = lseek(disk_fd, cp.endPtr, SEEK_SET);
    if(status < 0){
        return -1;
    }
    //try writing it
    int wrote = write(disk_fd, buffer, size);
    if(wrote < 0){
        return wrote;
    } 
    //update pointer
    cp.endPtr += wrote;
    return wrote;
}

/**
 * getINodeSize
 * gets the size of the INode according to the specifications
 * - iNode: the iNode
 * return: size of the file stored by the INode
 */
int getINodeSize(iNode node){
    int i;
    int j;
    char* buffer[BUFFER_SIZE];
    bool done = false;
    //loop through all the data pointer from the back
    for(i = 14 - 1; i >= 0 && !done; i--){
        if(node.dataPtrs[i] == -1){
            continue;
        }

        lseek(disk_fd, node.dataPtrs[i], SEEK_SET);
        read(disk_fd, buffer, BUFFER_SIZE);
        for(j = BUFFER_SIZE - 1; j >= 0 && !done; j++){
            if( buffer[j] != '\0'){
                done = true;
                break;
            }
        }
        if(done){
	       break;
        }
    }
    //calculate and return the size
    return (i * BUFFER_SIZE) + (j + 1);

}

