#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>

static int unsigned const buffersize = 512*32;
static int unsigned const verbose = 0;

// Compare two file names. Return 0 if file contents match, non-zero if unsure or differences found.
// This function is seperate from directory scanning functions since the only API calls needed to
// compare two files are C standard library functions, whereas directory scanning functions are
// typically compiler-specific.
int compareFiles(char *sn,char *dn)

{
  FILE          *sf,*df;
  char          *sb,*db;
  unsigned long size,difference;

  difference=1;
  if (verbose) printf("Request to open [%s] and [%s].\n",sn,dn);
  if (strcmp(sn,dn)) { // half-hearted attempt to make sure file names are different, really need an inode check that respects linked files.
    if (sb=malloc(buffersize)) {  // allocate source buffer
      if (db=malloc(buffersize)) {  // allocate destination buffer
        if ((sf=fopen(sn,"rb"))) {  // open source file for input
          if ((df=fopen(dn,"rb"))) {  // open detination file for input
            fseek(sf,0L,SEEK_END);    // Seek to end of source
            fseek(df,0L,SEEK_END);    // Seek to end of destination
            if ((size=ftell(sf))==ftell(df)) {  // Continue if file sizes match
              fseek(sf,0L,SEEK_SET);  // Rewind to beginning of file
              fseek(df,0L,SEEK_SET);  // Rewind to beginning of file
              if (verbose) printf("Sizes match, difference: %lu!\n",difference);
              if (verbose) printf("Size is %lu and buffer size is %u\n",size,buffersize);
              // The following block is executed iteratively for each full block
              // in both files, exiting the loop until all blocks have been read
              // or a mismatch located.
              for (difference=0;size>buffersize&&!difference;size-=buffersize) {
                if (verbose) printf("Should compare a whole block; %lu.\n",size);
                fread(sb,1,buffersize,sf);  // Error checking really should go here!
                fread(db,1,buffersize,df);  // Error checking really should go here!
                difference=memcmp(sb,db,buffersize);
                if (verbose) puts("Compared block");
              }
              // The following block is executed if there is a partial buffer
              // remaining and no differences have been found
              if (size&&!difference) {
                if (verbose) printf("Should compare partial block, size is %lu\n",size);
                fread(sb,1,size,sf);  // Error checking really should go here!
                fread(db,1,size,df);  // Error checking really should go here!
                difference=memcmp(sb,db,size);
                if (verbose) puts("Compared partial");
              }
              if (verbose) printf("Found difference to be %lu\n",difference);
              if (!difference) { // Differences found, close and remove file
                fclose(df);
                printf("\nYou have a copy of %s in %s\n\n",sn,dn);
                if (1) {
                  printf("\n\nRemoving %s!!!!\n\n",sn);
                  if (size=unlink(sn)) {
                    printf("Unlinking returned %li\n",size);
                  }
                }
              }
            } else printf("Failed size comparison: %lu, %lu, %lu\n",size,ftell(sf),ftell(df));
            if (difference) fclose(df); // No differences found, close file.
          } else puts("Could not open destination for reading");
          fclose(sf);
        } else puts("Could not open source for reading");
        free(db);
      }
      free(sb);
    }
  }
  return difference;
}

void dirTest(char *s,char *d,char *name)

{
  char          *sn,*dn;
  DIR           *dp;
  struct dirent *ep;
  char          *newpath;
  struct stat   sstat,dstat;

  printf("------------------->%i[%s]\n",stat(s,&sstat),s);
  printf("\nBeginning directory scan of [%s] [%s] [%s]\n",s,d,name);
  if (dp = opendir(s)) {
    while (ep = readdir (dp)) {
      switch (ep->d_type) {
        case DT_DIR:
          if (strcmp(ep->d_name,".")&&strcmp(ep->d_name,"..")) {
//            printf("Folder: %s%s\n",name,ep->d_name);
            if (newpath=malloc(strlen(ep->d_name)+2+strlen(name))) {
              strcpy(newpath,name);
              strcat(newpath,ep->d_name);
              printf("Should recurse: [%s]\n",newpath);
              if (sn=malloc(strlen(s)+strlen(newpath)+2)) {
                if (dn=malloc(strlen(d)+strlen(newpath)+2)) {
                  strcpy(sn,s);
                  strcat(sn,"/");
                  strcat(sn,newpath);
                  strcpy(dn,d);
                  strcat(dn,"/");
                  strcat(dn,newpath);
                  printf("Should recurse: [%s] [%s]\n",sn,dn);
                  dirTest(sn,dn,"");
                  free(dn);
                }
                free(sn);
              }
              free(newpath);
            } else puts("Could not allocate buffer for recursion");
          }
          break;
        case DT_REG:
          printf("---->>>> %u [%s%s]\n",ep->d_type,name,ep->d_name);
          if (sn=malloc(strlen(s)+strlen(ep->d_name)+2)) {
            if (dn=malloc(strlen(d)+strlen(ep->d_name)+2)) {
              strcpy(sn,s);
              strcat(sn,"/");
              strcat(sn,ep->d_name);
              strcpy(dn,d);
              strcat(dn,"/");
              strcat(dn,ep->d_name);
              printf(" -  -  -  -  -  -  -  -  -  - Should examine: [%s] [%s]\n",sn,dn);
              compareFiles(sn,dn);
              free(dn);
            }
            free(sn);
          }
          break;
      }
    }
    (void) closedir (dp);
  } else {
      printf("Couldn't open the directory [%s]\n",s);
      compareFiles(s,d);
  }
  printf("Ending directory scan of [%s] [%s] [%s]\n",s,d,name);
}

int main(int argc,char **argv)

{
  int     i;
  char    *sfn,*dfn;

  puts("\n\n\n\n\n");
  sfn=dfn=0;
  if (3!=argc) {
    printf("Usage:\
%s file1 file2\
  Compares contents of file1 and file2 if files have the same size and content, deleting file1 if they are the same.\n\
",argv[0]);
  } else {
//    compareFiles(argv[1],argv[2]);
    dirTest(argv[1],argv[2],"");
  }

  return 0;
}
