#define _GNU_SOURCE
#define _POSIX_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <stdlib.h>

void get_info_files(struct stat* filebuff, char* filepath);

void myls( char *dir)
{
  DIR *d;
  struct dirent *w;
  struct stat buf;

  char n[2000];

 if ((d = opendir(dir)) == NULL) 
 {
  printf("This is not supposed to happen.\n");
  exit(0);
 }
  while ((w = readdir(d)) != NULL)
   {
    sprintf(n, "%s/%s", dir, w->d_name);
    if ((strcmp(w->d_name, ".") == 0) || (strcmp(w->d_name, "..")==0))
     continue;

   /* fprintf(stderr,"%s\n",n);*/
     if (lstat(n, &buf) < 0)
     {
       fprintf(stderr, "Cannot stat %s\n", n); 
       continue;
     }
    get_info_files(&buf,n);  
  }
  closedir(d);
 }

 int main(int argc, char *argv[])
 {
   struct stat buf;  
   if (argc != 2) 
   {
     printf("Usage: a.out filename directory\n");
     exit(0);
   }
  /* finding all occurance of filename under directory */
  if (lstat(argv[1], &buf) <0)
  {
    printf("stat %s failed (probably file does not exist).\n", argv[1]);
    exit(0);
  }
  if (S_ISDIR(buf.st_mode)) 
  {
   myls(argv[1]);  
  }
  else
  {
   get_info_files(&buf,argv[1]);
  }

  return 0;
 }

 void get_info_files(struct stat* filebuff,char* filepath)
 {
  struct passwd* user;
  struct group* grp;
  char* timebuff;
  char* filename;
  char* time;

  filename=strrchr(filepath,'/');
  /*time_t modificationtime;*/
  if (S_ISREG(filebuff->st_mode))
  {
   printf("-");
  }
  else if (S_ISDIR(filebuff->st_mode))
  {
   printf("d");
  }
  else if (S_ISBLK(filebuff->st_mode))
  {
   printf("b");
  }
  else if (S_ISCHR(filebuff->st_mode))
  {
   printf("c");
  }
  else if (S_ISFIFO(filebuff->st_mode))
  {
   printf("f");
  }
  else if (S_ISSOCK(filebuff->st_mode))
  {
    printf("s");
  }
  else if (S_ISLNK(filebuff->st_mode))
  {
    printf("l");
  }
  else
  {
    printf("u");
  }

 if (S_IRUSR & filebuff->st_mode)
 {
   printf("r");
 }
 else
 {
   printf("-");
 }
 if (S_IWUSR & filebuff->st_mode)
 {
   printf("w");
 }
 else
 {
  printf("-");
 }
 if (S_IXUSR & filebuff->st_mode)
 {
  printf("x");
 }
 else
 {
  printf("-");
 }
 if (S_IRGRP & filebuff->st_mode)
 {
  printf("r");
 }
 else
 {
  printf("-");
 }
 if (S_IWGRP & filebuff->st_mode)
 {
   printf("w");
 }
 else
 {
   printf("-");
 }
 if (S_IXGRP & filebuff->st_mode)
 {
   printf("x");
 }
 else
 {
  printf("-");
 }
 if (S_IROTH & filebuff->st_mode)
 {
   printf("r");
 }
 else
 {
  printf("-");
 }
 if (S_IWOTH & filebuff->st_mode)
 {
  printf("w");
 }
 else
 {
  printf("-");
 }
 if (S_IXOTH & filebuff->st_mode)
 {
   printf("x");
 }
 else
 {
   printf("-");
 }
 printf(" ");
 printf("%d",(int)filebuff->st_nlink);
 printf(" ");
 user=getpwuid(filebuff->st_uid);
 grp=getgrgid(filebuff->st_gid);
 if (user!=NULL)
 {
 printf("%s",user->pw_name);
 }
 else
 {
  printf("%d",filebuff->st_uid);
 }
 printf(" ");
 if (grp!=NULL)
 {
  printf("%s",grp->gr_name);
 }
 else
 {
  printf("%d",filebuff->st_gid);
 }
 printf(" ");
 printf("%d",(int)filebuff->st_size);
 printf(" ");
 timebuff=ctime(&(filebuff->st_mtime));
 /*timebuff[strlen(timebuff)-1]='\0';*/
 time=strrchr(timebuff,':');
 if (time!=NULL)
 {
  *time='\0';
 }

 printf("%s",timebuff+4);
 /*modificationtime=filebuff->st_mtime;
  *  strftime(timebuff,100,"%b %d %H:%M",gmtime(&modificationtime));
  *   printf(timebuff);*/
 printf(" ");
 printf("%s \n",(filename+1));
}
