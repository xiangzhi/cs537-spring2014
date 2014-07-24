/* CS537 - Spring 2014
 * Xiang Zhi Tan (xtan@cs.wisc.edu)
 * 9069081090
 * Program Assignment 1
 * A program to sort an input file by its key 
 * and store the output into an output file
 * "Usage: fastsort -i inputfile -o outputfile"
 */


#include "sort.h" // for struct rec_t
#include <stdio.h> // for fprintf
#include <stdlib.h> // for malloc / free / qsort
#include <sys/stat.h> // for fstat
#include <fcntl.h> // for open
#include <unistd.h> // for close

// function prototypes
void usage();
int compare (const void* p1, const void* p2);

/*
 * An error function to show the correct usage of the program and exit
 */
void usage() 
{
    fprintf(stderr, "Usage: fastsort -i inputfile -o outputfile\n");
    exit(1);
}

/*
 * Main function for this program
 */
int main(int argc, char *argv[])
{
  /* declaring variables */
  // name of input file
  char* inputName;
  // name of output file
  char* outputName;
  // character to store the opt character in getopt
  int opt_c;
  //to store the information for inputfile
  struct stat info;
  //to store the file descripter for the inputfile
  int input_fd;
  // to store the file descripter for the outputfile
  int output_fd;
  // used to check return values
  int check;
  // to store the number of rec_t in inputfile
  int count;
  // a loop counter
  int i;
  // store the base to the allocated region to store the rec_t
  rec_t* base;
  // pointer to the current rec_t in the allocated region
  rec_t* pointer;
  // a temporary rec_t to store values.
  rec_t temp_rec_t;

  /*Initialize variables*/
  // input name and output are declared to makesure there it is always declared
  inputName = "invalid";
  outputName = "invalid";

  // using getopt to get all the arguments
  while((opt_c = getopt(argc, argv, "i:o:")) != -1){
    switch(opt_c){
      case 'i':
        // copy the input file Name
        inputName = optarg;
        break;
      case 'o':
        // copy the output file Name
        outputName = optarg;
        break;
      // if the argument is not specify by the optstring
      // show the usage and exit program
      // invalid code is thrown by '?' character
      case '?':
        usage();
      default:
        usage();
    }
  }

  // check optind to see whether there is invalid argument that is not processed
  if(optind != 5){
    usage();
  }

  // open the input file
  input_fd = open(inputName, O_RDONLY);
  // check whther the input file has open correctly
  if (input_fd < 0){
    fprintf(stderr, "Error: Cannot open file %s\n", inputName);
    exit(1);
  }

  //  open and create output file
  output_fd = open(outputName, O_WRONLY|O_CREAT|O_TRUNC, S_IRWXU);
  // check whether the output file has open correctly
  if (output_fd < 0) {
    fprintf(stderr, "Error: Cannot open file %s\n", outputName);
    exit(1);
  }

  // get the file size of the input file
  check = fstat(input_fd, &info);
  // check whether fstat has worked correctly
  if(check < 0){
    fprintf(stderr, "Error: Cannot find file size of %s\n", inputName);
    exit(1);
  }

  // allocate space for the whole file
  base = (rec_t*) malloc(info.st_size);
  // change the pointer to the base of the allocated space
  pointer = base;
  // initialize the counter.
  count = 0;

  // copy the files into the allocated space
  while (1) { 
    // read only the size of the rec_t and store into temp_rec_t
    check = read(input_fd, &temp_rec_t, sizeof(rec_t));
    // check whether we have reach the end of file
    if (check == 0) //  0 indicates EOF
      break;
    // if less than 0 means error in read()
    if (check < 0) {
        perror("read");
        exit(1);
    }
    // increment counter
    count++;
    // store the content of temp_rec_t into the poiner
    *pointer = temp_rec_t;
    // increment the pointer to the next location
    pointer = (rec_t*) ((char*)pointer + sizeof(rec_t));
  }

  // using the qsort,  we sort the whole allocated space using the compare
  // function
  qsort(base, count, sizeof(rec_t), compare);

  // move the pointer back to the base
  pointer = base;

  // we used our count to guide the writing of output file
  // we loop through the allocate space and write rec_t into output file
  for(i = 0; i < count; i++){
    check = write(output_fd, pointer, sizeof(rec_t));
    // check whether write was successful.
    if (check != sizeof(rec_t)) {
      perror("ouput write");
      exit(1);
    }
    // increment the pointer to the next item in the space
    pointer = (rec_t*) ((char*)pointer + sizeof(rec_t));
  }

  // free the used space
  free(base);

  // close both our files
  close(input_fd);
  close(output_fd);

  exit(0);
}

/*
 * A compar function for qsort().
 * it takes 2 rec_t and compare their difference
 * return an integer according to the specification of the compar function in
 * man page of qsort()
 */
int compare (const void* p1, const void* p2){
  // get the key for p1 and p2 and get the difference
  return ((rec_t*) p1)->key - ((rec_t*) p2)->key;
}