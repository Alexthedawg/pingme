#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <iostream>

#define DOMAIN_LEN    256
#define HIST_LEN      50
#define HIST_TDL_LEN  5
#define OPT_STR       "f:m:"
#define READ_MODE     "r"
#define TL_DOMAIN_LEN 64
#define PARSE_DATA    "%s %*s %*s %*s %f %f %f %f %*s"
#define PARSE_OK_PING ".*rtt min/avg/max/mdev = .*"

struct entry {
  char website [DOMAIN_LEN];      // name of the website domain
  char tldomain [TL_DOMAIN_LEN];  // top level domain (i.e. 'com', 'net', 'gov', 'jp')
  double avg;                     // avg rtt ping time
  double min;                     // min ping time
  double max;                     // max ping time
  double mdev;                    // mean deviation (average deviation from the mean)
};

int errexit (const char* msg) {
  printf ("%s\n", msg);
  exit (EXIT_FAILURE);
}

int fcmp (const float a, const float b) {
  if (a > b) {
    return 1;
  } else if (a < b) {
    return -1;
  } else { return 0; }
}

int avgmin (const void *a, const void *b) {
  entry *entrya = (entry *)a;
  entry *entryb = (entry *)b;
  if (strcasecmp (entrya->tldomain, entryb->tldomain) == 0) {
    return fcmp(entrya->min, entryb->min);
  } else {
    return strcasecmp (entrya->tldomain, entryb->tldomain);
  }
}

int avgavg (const void *a, const void *b) {
  entry *entrya = (entry *)a;
  entry *entryb = (entry *)b;
  if (strcasecmp (entrya->tldomain, entryb->tldomain) == 0) {
    return fcmp(entrya->avg, entryb->avg);
  } else {
    return strcasecmp (entrya->tldomain, entryb->tldomain);
  }
}

int avgmax (const void *a, const void *b) {
  entry *entrya = (entry *)a;
  entry *entryb = (entry *)b;
  if (strcasecmp (entrya->tldomain, entryb->tldomain) == 0) {
    return fcmp (entrya->max, entryb->max);
  } else {
    return strcasecmp (entrya->tldomain, entryb->tldomain);
  }
}

int avgmdev (const void *a, const void *b) {
  entry *entrya = (entry *)a;
  entry *entryb = (entry *)b;
  if (strcasecmp (entrya->tldomain, entryb->tldomain) == 0) {
    return fcmp (entrya->mdev, entryb->mdev);
  } else {
    return strcasecmp (entrya->tldomain, entryb->tldomain);
  }
}

// sorts entries array based on mode: 0 = avgmin, 1 = avgavg, 2 = avgmax, 3 = avgmdev
void sort (entry entries[], const int mode, const int numentries) {
  switch (mode) {
    case 0:
      qsort (entries, numentries, sizeof (struct entry), avgmin);
      break;
    case 1:
      qsort (entries, numentries, sizeof (struct entry), avgavg);
      break;
    case 2:
      qsort (entries, numentries, sizeof (struct entry), avgmax);
      break;
    case 3:
      qsort (entries, numentries, sizeof (struct entry), avgmdev);
      break;
    default:
      errexit ("Invalid Mode Selected.");
  }
}

void report (entry entries[], const int mode, const int numentries) {
  int count = 0;
  double runningsum = 0;
  double runningcount = 0;
  char tldomain [TL_DOMAIN_LEN];
  memcpy (&tldomain, &entries[0].tldomain, sizeof (tldomain));
  switch (mode) {
    case 0:
      runningsum += entries[0].min;
      runningcount++;
      break;
    case 1:
      runningsum += entries[0].avg;
      runningcount++;
      break;
    case 2:
      runningsum += entries[0].max;
      runningcount++;
      break;
    case 3:
      runningsum += entries[0].mdev;
      runningcount++;
      break;
  }
  while (count < numentries) {
    char entrytl [TL_DOMAIN_LEN];
    memset (&entrytl, 0, sizeof (entrytl));
    memcpy (&entrytl, &entries[++count].tldomain, sizeof (entrytl));
    if (strcasecmp (&tldomain[0], &entrytl[0]) == 0) {
      switch (mode) {
        case 0:
          runningsum += entries[count].min;
          runningcount++;
          break;
        case 1:
          runningsum += entries[count].avg;
          runningcount++;
          break;
        case 2:
          runningsum += entries[count].max;
          runningcount++;
          break;
        case 3:
          runningsum += entries[count].mdev;
          runningcount++;
          break;
      }
    } else {
      double proportion = (runningsum / (double)runningcount) / 4;
      
      int tdllen = printf ("%s", &tldomain[0]);
      while (tdllen++ < HIST_TDL_LEN) {
        std::cout << " ";
      }
      for (int i = 0; i < proportion; i++) {
        std::cout << "-";
      }
      printf (" %f\n", runningsum / (double)runningcount);
      runningcount = 0;
      runningsum = 0;
      memset (&tldomain, 0, sizeof (tldomain));
      memcpy (&tldomain, &entries[count].tldomain, sizeof (tldomain));
    }
  }
}

void getentries (entry entries[], const char *file, const int numentries) {
  char *line = NULL;
  size_t len = 0;
  FILE *fptr = fopen (file, READ_MODE);
  if (fptr == NULL) {
    errexit ("File Read Error.");
  }
  regex_t pingok;
  if (regcomp (&pingok, PARSE_OK_PING, 0) != 0) {
    errexit ("Regex Compilation Error.");
  }
  
  memset (entries, 0, numentries * sizeof (struct entry));
  
  // creating website entries
  int currentline = 0;
  while (getline (&line, &len, fptr) != EOF) {
    if (regexec (&pingok, line, 0, NULL, 0) == 0) {
      struct entry myentry;
      
      memset (&myentry, 0, sizeof (struct entry));
      void *ptr = strtok (line, " ");
      memcpy (&myentry.website, ptr, strcspn (line, "\0"));
      
      ptr = strtok (NULL, "=");
      ptr = strtok (NULL, "/");
      myentry.min = atof ((char *)ptr);
      ptr = strtok (NULL, "/");
      myentry.avg = atof ((char *)ptr);
      ptr = strtok (NULL, "/");
      myentry.max = atof ((char *)ptr);
      ptr = strtok (NULL, "/");
      myentry.mdev = atof ((char *)ptr);
            
      char tldomain [TL_DOMAIN_LEN];
      ptr = strtok (line, ".");
      while (ptr != NULL) {
        strcpy (tldomain, (char *)ptr);
        ptr = strtok (NULL, ".");
      }
      memcpy (&myentry.tldomain, tldomain, strcspn(tldomain, "\0"));
      
      memcpy (&entries[currentline++], &myentry, sizeof (struct entry));     
    }
  }
  
}

void sortfile (const char *file, const int mode) {
  FILE *fptr = fopen (file, READ_MODE);
  char *line = NULL;
  size_t len = 0;
  int numentries = 0;
  regex_t reegex;
  
  if (fptr == NULL) {
    errexit ("File Read Error.");
  }
  if (regcomp (&reegex, PARSE_OK_PING, 0) != 0) {
    errexit ("Regex Compilation Error.");
  }
  while (getline (&line, &len, fptr) != EOF) {
    if ((regexec (&reegex, line, 0, NULL, 0) == 0)) {
      numentries++;
    }
  }
  
  regfree (&reegex);
  free (line);
  fclose (fptr);
  
  entry entries [numentries];
  getentries (entries, file, numentries);
  
  sort (entries, mode, numentries);
  
  report (entries, mode, numentries);
  
  exit (EXIT_SUCCESS);
}

int main (int argc, char **argv) {
  int opt, mode;
  int fflag, mflag;
  char *file;
  
  opt = 0;
  mode = 0;
  fflag = 0;
  mflag = 0;
  
  // OPT_STR has two args: -f, the file to read from and -m, the mode to sort lines
  // -m the argument following '-m' can be 0, 1, 2, or 3 which sorts by the min, avg, max, or mdev
  while ((opt = getopt (argc, argv, OPT_STR)) != EOF) {
    switch (opt) {
      case 'f':
        fflag = fflag + 1;
        file = optarg;
        break;
      case 'm':
        mflag = mflag + 1;
        mode = atoi (optarg);
        break;
      default:
        return errexit ("Invalid Argument(s). Usage: ./sort -m MODE -f FILE");
    }
  }
  
  if (fflag != 1 && mflag != 1) { errexit ("Incorrect Args."); }
  
  sortfile (file, mode);

  exit (EXIT_SUCCESS);
}
