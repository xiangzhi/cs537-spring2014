#include "fs.h"

int disk_fd;
//to store and manipulate the checkPoint region
checkPoint cp;

/** internal helpers **/
int getIMapNum(int pinum);
int createEmptyDirectory(int fd_ptr);

void fs_init(char* fileName){

    //check whether the file access
    if( access(fileName, F_OK) != -1 ){
        //file exists

        //open the file
        disk_fd = open(fileName, O_RDWR|O_APPEND,S_IRWXU);
        if(disk_fd < 0){
            fprintf(stderr,"Error: Cannot open file %s\n",fileName);
            exit(1);
        }

        //read the checkpoint region
        int sum = read(disk_fd, &cp, sizeof(checkPoint));
        if(sum != sizeof(checkPoint)){
            fprintf(stderr,"Error: Unable to read checkPoint region\n");
            exit(1);
        }
    }
    else{
        //file doesn't exist
        //create the new file
        disk_fd = open(fileName, O_RDWR|O_CREAT,S_IRWXU);
        //initialize the checkpoint region
        //create the first inode
        int ptr = cp.endPtr = 4096; //0-4095 is the CR


        int dataPtr = ptr;
        //create an Empty directory at that location
        ptr = createEmptyDirectory(ptr);

        //creat the first Inode
        iNode node;
        node.type = FS_DIRECTORY;
        node.size = 4096;
        node.dataPtrs[0] = dataPtr;
        int nodePtr = ptr;

        lseek(disk_fd, ptr, SEEK_SET);
        write(disk_fd, &node, sizeof(iNode));
        ptr += (int)sizeof(iNode);

        //create the first Imap region
        iMap map;
        map.ptr[0] = nodePtr;
        for(int j = 1; j < 16; j++){
            map.ptr[j] = -1;
        }
        lseek(disk_fd, ptr, SEEK_SET);
        write(disk_fd, &map, sizeof(iMap));

        //save the pointer in checkpoint
        cp.mapPtrs[0] = ptr;
        ptr += sizeof(iMap);
        cp.endPtr = ptr;

    }
    fs_print();
}

int fs_close(){
    lseek(disk_fd, 0, SEEK_SET);
    //write back the critical region
    int stat = write(disk_fd, &cp, sizeof(checkPoint));
    //done
    return stat;
}

int fs_create(int pinum,  int type, char *name){
    return 0;
}

//returns the correct Imap number for any number
int getIMapNum(int pinum){
    return pinum >> 4;
}

//creates an emptry directory data at ptr
//return the pointer to the next free space
int createEmptyDirectory(int fd_ptr){

    //move fd head to fd_ptr;
    lseek(disk_fd, fd_ptr, SEEK_SET);

    //create an empty directory entry
    dir d;
    strncpy(d.name, ".", 60);
    d.pair = 0;
    write(disk_fd, &d, sizeof(dir));
    strncpy(d.name, "..", 60);
    write(disk_fd, &d, sizeof(dir));
    //initalize other entries to -1;
    for(int j = 0; j < 62; j++){
        dir d;
        strncpy(d.name, "", 60);
        d.pair = -1;
        write(disk_fd, &d, sizeof(dir));
    }
    return (fd_ptr + 4096);
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
            if(map.ptr[j] == -1){
                continue;
            }
            iNode node;
            //printf("location:%d\n", map.ptr[j]);
            lseek(disk_fd,map.ptr[j], SEEK_SET);
            read(disk_fd, &node, sizeof(iNode));
            if(node.type != 1 && node.type != 0){
                break;
            }
            printf("size:%d, type:%d\n",node.size, node.type);
            if(node.type == 0){
                int counter = node.size;
                int k = 0;
                do{
                    lseek(disk_fd, node.dataPtrs[k], SEEK_SET);
                    dir d;
                    read(disk_fd, &d, sizeof(dir));
                    while(d.pair >= 0){    
                        printf("name:%s par:%d\n", d.name, d.pair);
                        read(disk_fd, &d, sizeof(dir));
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
                printf("%s", ptr);
            }
        }
    }
}