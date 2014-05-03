
int disk_fd

void init(char[] fileName){
    disk_fd = open(fileName, O_WRONGLY|O_CREAT|O_TRUNC,S_IRWXU);

    if(disk_fd < 0){
        fprintf(stderr,"Error: Cannot open file %s\n",fileName);
        exit(1);
    }
}
