#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define ARG_COUNT      3
#define CLEAN_OUTPUT   "rm -f output.txt"
#define MAX_DOMAIN_LEN 255
#define OPT_STR        "f:n:v"
#define PREPEND_SITE   "printf '%s ' >> output.txt"
#define PI_TO_FI       "ping %s -i 0.2 -c 5 -q | tail -1 >> output.txt"
#define READ_MODE      "r"
#define WRITE_MODE     "w"

int errexit (const char* msg) {
  printf ("%s\n", msg);
  exit (EXIT_FAILURE);
}

void readfile (const char *file, const int numlines, const int verbose) {
  int check;
  char site [MAX_DOMAIN_LEN];
  char prepend [MAX_DOMAIN_LEN + sizeof (PREPEND_SITE)];
  char command [MAX_DOMAIN_LEN + sizeof (PI_TO_FI)];
  int line = 0;
  FILE *fptr = fopen (file, READ_MODE);
  
  system (CLEAN_OUTPUT);
  
  if (fptr == NULL) {
    errexit ("Error Reading File.");
  }
  
  while (line < numlines) {
    // getting line number
    if (fscanf (fptr, "%d,", &check) == 0) {
      errexit ("File Read Error.");
    }
    
    // checking line number
    if (check != ++line) {
      errexit ("Line Read Error.");
    }
    
    // saving domain name
    memset (site, 0, sizeof (site));
    if (fscanf (fptr, "%[^\n]", site) == 0) {
      errexit ("Line Read Error.");
    }
    if (verbose == 1) {
      puts (site);
    }
    
    // prepending site info
    memset (prepend, 0, sizeof (prepend));
    if (sprintf (prepend, PREPEND_SITE, site) < 0) {
      errexit ("File Write Error.");
    }
    system (prepend);
    
    // getting ping output
    memset (command, 0, sizeof (command));
    if (sprintf (command, PI_TO_FI, site) < 0) {
      errexit ("File Write Error.");
    }
    system (command);
  }
  
  fclose (fptr);
}

int main (int argc, char **argv) {
  int opt, numlines;
  int fflag, nflag, vflag;
  char *file;
  opt = 0;
  
  numlines = 0;
  fflag = 0;
  nflag = 0;
  vflag = 0;
  
  while ((opt = getopt (argc, argv, OPT_STR)) != EOF) {
    switch (opt) {
      case 'f': 
        fflag = fflag + 1;
        file = optarg;
        break;
      case 'n':
        nflag = nflag + 1;
        numlines = atoi (optarg);
        break;
      case 'v':
        vflag = vflag + 1;
        break;
      default:
        errexit ("Invalid Argument(s). Usage: ./proj5 -n LINES -f FILE [-v]");
    }
  }
  
  readfile (file, numlines, vflag > 0);
  
  exit (EXIT_SUCCESS);
}
