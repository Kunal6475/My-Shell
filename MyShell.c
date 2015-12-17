#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

void shell_loop();
char* read_shellcommand();
char** get_command_arguments(char* commandline);
void myshell_cd(char** args);
void myshell_pwd(char** args);
void myshell_set(char** args);
void myshell_exit(char** args);
void builtincommand_execute(char** args);
int myshell_noofbuiltin();
void myshell_executepipe(char** args);
void myshell_executeoutputredirection(char** args);
void myshell_executeinputredirection(char** args);
void myshell_executeinoutredirection(char** args);
void myshell_executelibraryfunction(char** args);
void myshell_execute_mylsfunction(char** args);
void myshell_execute_backgroundfunction(char** args);

/*Variable to keep track whether shell should continue running. if status is zero the shell would exit*/
int status=1;
/*Variable to keep track whether pipe function needs to be executed*/
int execute_pipe=0;
/*Variable to keep track whether output redirection function needs to be executed*/
int execute_output_redirection=0;
/*Variable to keep track whether input redirection function needs to be executed*/
int execute_input_redirection=0;
/*Variable to keep track whether both input output redirection function needs to be executed*/
int execute_in_out_redirection=0;
/*Variable to keep track whether built in command function needs to be executed*/
int execute_builtin_command=0;
/*Variable to keep track whether first argument is for output*/
int output_first=1;
/*Variable to keep track whether myls command needs to be executed*/
int execute_myls_command=0;
/*Variable to keep track whether background process command needs to be executed*/
int execute_background_command=0;
/*Variable to keep track the number of pipes more than 1*/
int no_pipes_2=0;

char* builtin_commands[] = {"exit","cd","pwd","set"};
void (*builtin_commands_function[])(char**)={ &myshell_exit, &myshell_cd, &myshell_pwd, &myshell_set};

int main()
{

 shell_loop();

 return EXIT_SUCCESS;
}

/*Loop to display the command prompt*/
void shell_loop()
{
 char* read_line;
 char** command_arguments=NULL;
 
 do
 {
 printf("$ ");
 read_line=read_shellcommand();
 /*Every time the shell starts make the command arguments NULL*/
 command_arguments=NULL;
 /* If the user pressed ctrl D then it should not try to parse the command*/
  if (read_line!=NULL)
  {
     command_arguments=get_command_arguments(read_line);
     /* int i=0;  This part is keep to check what are the command arguments that we got
     while(command_arguments[i]!=NULL)
     {
      printf("%s\n",command_arguments[i]);
      i++;
     }*/
     if (execute_pipe==1)
     {
      myshell_executepipe(command_arguments); 
     }
     else if (execute_output_redirection==1)
     {
      myshell_executeoutputredirection(command_arguments);
     }
     else if (execute_input_redirection==1)
     {
      myshell_executeinputredirection(command_arguments);
     }
     else if (execute_in_out_redirection==1)
     {
      myshell_executeinoutredirection(command_arguments);
     }
     else if (execute_builtin_command==1)
     {
      builtincommand_execute(command_arguments);
     }
     else if (execute_myls_command==1)
     {
      myshell_execute_mylsfunction(command_arguments);
     }
     else if (execute_background_command==1)
     {
      myshell_execute_backgroundfunction(command_arguments);
     }
     else
     {
      myshell_executelibraryfunction(command_arguments);
     }
  }
  /*check if read line is not null then free.The situation when user press ctrl D at start.*/
  if (read_line!=NULL)
  {
  free(read_line);
  }
  /*check if command arguments is not null then free.The situation user pressed ctrl D at start.*/
  if (command_arguments!=NULL)
  {
  free(command_arguments);
  }
 } while(status);

}

/* This function will read the commands entered by the user*/
char* read_shellcommand()
{
/* Our shell can take commands upto 80 characters*/
int buffer_size=81;
int position=0;
int character;
char* buffer=malloc(sizeof(char*)*buffer_size);

do
{
   character=getchar();   
   /* if position is 0 & EOF is there we need to exit*/
   if (position==0 && character==EOF)
   {
   status=0;
   return NULL;
   }
   /* if EOF or nextline character comes then make it null terminated string*/
   if (character==EOF || character=='\n')
   {
   buffer[position]='\0';
   return buffer;
   }
   else
   {
   buffer[position]=character;
   position ++;
   }

} while(position<80);

if (position==80)
{
buffer[position]='\n';
}
return buffer;
}

/*This function will parse the commands into arguments*/
char** get_command_arguments(char *commandline)
{
 int buffer_size=81;
 int position=0;
 char** arguments=malloc(sizeof(char)*buffer_size);
 char* token;
 char* token_delimiter_pipe;
 char* token_delimiter;
 char* token_delimiter_output=">";
 char* token_delimiter_input="<";
 char* token_background_delimiter="&";
 
 int counter;

 if (arguments==NULL)
 {
  fprintf(stderr,"Unable to tokenize the commands\n");
  return arguments;
 }

 /*It would detect whether background process character is present in the command arguments*/

  if (commandline[strlen(commandline)-1]=='&')
  {
   execute_background_command=1;
   token=strtok(commandline,token_background_delimiter);
   if (token!=NULL)
   {
    arguments[position]=token;
    position++;
    arguments[position]=NULL;
    return arguments;
   }
  }
 

 /*It would do the splitting of arguments when piping condition is there*/
 if (strchr(commandline,'|')!=NULL)
 {
  execute_pipe=1;
  token_delimiter_pipe="|";
  if (((strrchr(commandline,'|'))-(strchr(commandline,'|')))>0)
  {
   no_pipes_2=1;
  }
  token=strtok(commandline,token_delimiter_pipe);
  
  if (token!=NULL)
  {
   arguments[position]=token;
   position ++;
   token=strtok(NULL,token_delimiter_pipe);
   if (token!=NULL)
   {
    arguments[position]=token;
    position ++;
    token=strtok(NULL,token_delimiter_pipe);
    if (token!=NULL)
    {
     arguments[position]=token;
     position++;
    }
   }
  }
  arguments[position]=NULL;
  return arguments;
 }

 /*It would do the splitting of arguments if only output redirection is present*/
 if ((strchr(commandline,'>')!=NULL) && (strchr(commandline,'<')==NULL))
 {
  execute_output_redirection=1;
  token=strtok(commandline,token_delimiter_output);
  
  if (token!=NULL)
  {
   arguments[position]=token;
   position++;
   token=strtok(NULL,token_delimiter_output);
   if (token!=NULL)
   {
    arguments[position]=token;
    position++;
   }
  }
  arguments[position]=NULL;
  return arguments;
 }

 /*It would do the splitting of arguments if only input redirection is present*/
 if ((strchr(commandline,'<')!=NULL) && (strchr(commandline,'>')==NULL))
 {
  execute_input_redirection=1;
  token=strtok(commandline,token_delimiter_input);

  if (token!=NULL)
  {
   arguments[position]=token;
   position++;
   token=strtok(NULL,token_delimiter_input);
   if (token!=NULL)
   {
    arguments[position]=token;
    position++;
   }
  }
  arguments[position]=NULL;
  return arguments;
 }

 if ((strchr(commandline,'<')!=NULL) && (strchr(commandline,'>')!=NULL))
 {
  execute_in_out_redirection=1;
  /*reset the output first variable*/
  output_first=0;
  /*if output redirection symbol appears first then we should set output first variable to 1*/
  if (((strchr(commandline,'>'))-(strchr(commandline,'<')))<0)
  {
   output_first=1;
  }
  if (output_first)
  {
   token=strtok(commandline,token_delimiter_output);
   if (token!=NULL)
   {
   arguments[position]=token;
   position++;
   token=strtok(NULL,token_delimiter_input);
    if (token!=NULL)
    {
     arguments[position]=token;
     position++;
     token=strtok(NULL,token_delimiter_input);
     if (token!=NULL)
     {
      arguments[position]=token;
      position++;
     }
    }
   }
  }
  else
  {
   token=strtok(commandline,token_delimiter_input);
   if (token!=NULL)
   {
    arguments[position]=token;
    position++;
    token=strtok(NULL,token_delimiter_output);
     if (token!=NULL)
     {
      arguments[position]=token;
      position++;
      token=strtok(NULL,token_delimiter_output);
      if (token!=NULL)
      {
       arguments[position]=token;
       position++;
      }
     }   
    }  
  }
  arguments[position]=NULL;
  return arguments;
 }


 /*It would do spliting into tokens based on space or carriage return or tab */
 token_delimiter=" \t\r";

 token=strtok(commandline,token_delimiter);

 while (token!=NULL)
 {
 arguments[position]=token;
 position++;

 token=strtok(NULL,token_delimiter);
 }

 arguments[position]=NULL;

 for (counter=0;counter<myshell_noofbuiltin();counter++)
 {
  if (strcmp(arguments[0],builtin_commands[counter])==0)
  {
   execute_builtin_command=1;
   break;
  }
 }
 
 if (strcmp(arguments[0],"myls")==0)
 {
  execute_myls_command=1;
 }

 return arguments;
}

/*This function will return the number of builtin commands in the shell */
int myshell_noofbuiltin()
{
int number;
number=(sizeof(builtin_commands))/(sizeof(char*));
return number;
}

/*This function would call the builtin function according to the argument provided.
 * If null is there then it would not do anything*/
void builtincommand_execute(char** command_arguments)
{
  int counter=0;
  execute_builtin_command=0;
  if (command_arguments[0]==NULL)
  {
  return ;
  }

  for (counter=0;counter<myshell_noofbuiltin();counter++)
  {
    if (strcmp(command_arguments[0],builtin_commands[counter])==0)
    {
     (*builtin_commands_function[counter])(command_arguments);
    }
  }
}

/*The implementation of cd command*/
void myshell_cd(char** arguments)
{
  if(arguments[1]==NULL)
  {
   fprintf(stderr,"Invalid arguments to cd\n");
  }
  else
  {
    if (chdir(arguments[1])!=0)
     {
      perror("Unable to change directory");
     }
  }
}

/*The implementation of exit command*/
void myshell_exit(char** arguments)
{
 /* if status is set to 0 the shell will exit*/
 status=0;
}

/*The implementation of pwd command.The maximum length of the path is 2000.*/
void myshell_pwd(char** arguments)
{
 char buffer[2000];
 if (arguments[1]!=NULL)
 {
  fprintf(stderr,"pwd:ignoring non option arguments\n");
 }
 if(getcwd(buffer,sizeof(buffer))!=NULL)
 {
 printf("%s\n",buffer);
 }
 else
 {
 fprintf(stderr,"Path too long\n");
 }
}

/*The implementation of set command*/
void myshell_set(char** arguments)
{
 if(arguments[1]==NULL)
 {
  fprintf(stderr,"Invalid argument to set\n");
 }
 else
 {
 char* environment_variable=malloc(sizeof(char*)*7);
 environment_variable=strncpy(environment_variable,arguments[1],6);
 environment_variable[6]='\0';
 if (strcmp(environment_variable,"MYPATH")!=0)
 {
  fprintf(stderr,"Only MYPATH enviorment variable can be set\n");
  free(environment_variable);
 }
 else
 {
  char* buffer=arguments[1];
  char* token;
  char* environment_value;
  char* token_delimiter="=";

  token=strtok(buffer,token_delimiter);

   if (token!=NULL)
   {
    environment_value=strtok(NULL,token_delimiter);
    if (environment_value!=NULL)
    {
      if (setenv(environment_variable,environment_value,1)!=0)
      {
         perror("Set Environment");
      }
      free(environment_variable);
    }
    else
    {
    fprintf(stderr,"Incorrect syntax of set\n");
    free(environment_variable);
    }
 
  }
 }
 }
}

/*This function will execute the pipe*/
void myshell_executepipe(char** args)
{
 int fd[2];
 int fd_pipe2[2];
 int status;
 int position=0;
 char* myargs[6];
 char* token;
 char* token_delimiter=" \t\r";

 execute_pipe=0;
 
 /*This is for finding out the arguments how it is passed*/
/* int i=0;
 while (args[i]!=NULL)
 {
 fprintf(stderr,"%d  %s",i,args[i]);
 i++;
 }*/
 if (no_pipes_2==1)
 {
  if(args[0]==NULL||args[1]==NULL||args[2]==NULL)
  {
   fprintf(stderr,"Invalid number of arguments for pipe\n");
   execute_pipe=0;
   no_pipes_2=0;
   return;
  }
 }
 else
 {
  if(args[0]==NULL || args[1]==NULL)
  {
   fprintf(stderr,"Invalid number of arguments for pipe\n");
   execute_pipe=0;
   return;
  }
 }

 if (no_pipes_2==1)
 {
  no_pipes_2=0;
  if (fork()==0)
  {
   pipe(fd);
   pipe(fd_pipe2);
   if (fork()==0)
   {
    close(STDOUT_FILENO);
    dup(fd[1]);
    close(fd[0]);
    token=strtok(args[0],token_delimiter);
    while(token!=NULL && position<5)
    {
     myargs[position]=token;
     position++;
     token=strtok(NULL,token_delimiter);
    }
    myargs[position]=NULL;
    if(execvp(myargs[0],myargs)==-1)
    {
     perror("Unable to start args[0]");
     exit(EXIT_SUCCESS);
    }
   }
   
   if (fork()==0)
   {
    close(STDIN_FILENO);
    dup(fd[0]);
    close(fd[1]);
    close(STDOUT_FILENO);
    dup(fd_pipe2[1]);
    close(fd_pipe2[0]);
    token=strtok(args[1],token_delimiter);
    while(token!=NULL && position<5)
    {
     myargs[position]=token;
     position++;
     token=strtok(NULL,token_delimiter);
    }
    myargs[position]=NULL;
    if (execvp(myargs[0],myargs)==-1)
    {
     perror("Unable to start args[1]");
     exit(EXIT_SUCCESS);
    }
   }
   close(fd[0]);
   close(fd[1]);
   close(STDIN_FILENO);
   dup(fd_pipe2[0]);
   close(fd_pipe2[1]);
   wait(&status);
  /* fprintf(stderr,"Child with process Id %d exited\n",pid);*/
   wait(&status);
  /* fprintf(stderr,"Child with process Id %d exited\n",pid);*/
   
   token=strtok(args[2],token_delimiter);
   while (token!=NULL && position<5)
   {
    myargs[position]=token;
    position++;
    token=strtok(NULL,token_delimiter);
   }
   myargs[position]=NULL;
   if (execvp(myargs[0],myargs)==-1)
   {
    perror("Unable to start args[2]");
    exit(EXIT_SUCCESS);
   }
  }
   wait(&status);
   if (status>>8==0xff)
   {
    fprintf(stderr,"Command could not be successfully executed\n");
   }
  }

 else
 {
  pipe(fd);

  if (fork()==0)
  {
   close(STDIN_FILENO);
   dup(fd[0]);
   close(fd[1]);
   token=strtok(args[1],token_delimiter);
   while(token!=NULL && position<5)
   {
    myargs[position]=token;
    position++;
    token=strtok(NULL,token_delimiter);
   }
   myargs[position]=NULL;
    if (execvp(myargs[0],myargs)==-1)
   {
        perror("Unable to start");
	exit(EXIT_SUCCESS);
   }
 }

  if (fork()==0)
  {
   close(STDOUT_FILENO);
   dup(fd[1]);
   close(fd[0]);
   token=strtok(args[0],token_delimiter);
   while(token!=NULL && position<5)
   {
    myargs[position]=token;
    position++;
    token=strtok(NULL,token_delimiter);
   }
   myargs[position]=NULL;
   if (execvp(myargs[0],myargs)==-1)
   {
      perror("Unable to start");
      exit(EXIT_SUCCESS);
   }
  }
  close(fd[0]);
  close(fd[1]);
  wait(&status);
  wait(&status);
}
 
}

/*Implement the functionality of output redirection*/
void myshell_executeoutputredirection(char** args)
{
 char* token;
 char* token_delimiter=" \t\r";
 int position=0;
 char* myargs[6];
 int status;
 execute_output_redirection=0;

 if (args[0]==NULL || args[1]==NULL)
 {
  fprintf(stderr,"Invalid number of arguments for output redirection\n");
  return;
 }
 
 token=strtok(args[0],token_delimiter);
 while(token!=NULL && position<5)
 {
  myargs[position]=token;
  position++;
  token=strtok(NULL,token_delimiter);
 }
 myargs[position]=NULL;
 if (fork()==0)
 {
  close(STDOUT_FILENO);
  open(args[1],O_RDWR|O_CREAT|O_TRUNC,0777);
  if (execvp(myargs[0],myargs)==-1)
  {
   perror("Unable to start");
   exit(EXIT_SUCCESS);
  }
 }
 else
 {
  wait(&status);
  if (status>>8==0xff)
  {
   printf("Output redirection failed\n");
  }
 }
} 

/*Implement the functionality of input redirection*/
void myshell_executeinputredirection(char** args)
{
 char* token;
 char* token_delimiter=" \t\r";
 int position=0;
 char* myargs[6];
 int status;
 execute_input_redirection=0;

 if (args[0]==NULL || args[1]==NULL)
 {
  fprintf(stderr,"Invalid number of arguments for input redirection\n");
  return;
 }
 
 token=strtok(args[0],token_delimiter);
 while (token!=NULL && position<5)
 {
  myargs[position]=token;
  position++;
  token=strtok(NULL,token_delimiter);
 }
 myargs[position]=NULL;

 if (fork()==0)
 {
  close(STDIN_FILENO);
  if (open(args[1],O_RDONLY)==-1)
  {
   perror("File can not be opened");
   exit(EXIT_SUCCESS);
  }
  if (execvp(myargs[0],myargs)==-1)
  {
   perror("Unable to start");
   exit(EXIT_SUCCESS);
  }
 }
 else
 {
  wait(&status);
  if (status>>8==0xff)
  {
   printf("Input redirection failed\n");
  }
 }
} 

/*This function implements input output redirection*/
void myshell_executeinoutredirection(char** args)
{
 char* token;
 char* token_delimiter=" \t\r";
 int position=0;
 char* myargs[6];
 int status;
 execute_in_out_redirection=0;

 if (args[0]==NULL|| args[1]==NULL|| args[2]==NULL)
 {
  fprintf(stderr,"Invalid arguments for input output redirection\n");
  return;
 }

 token=strtok(args[0],token_delimiter);
 while(token!=NULL && position<5)
 {
  myargs[position]=token;
  position++;
  token=strtok(NULL,token_delimiter);
 }
 myargs[position]=NULL;

 if (fork()==0)
 {
  if (output_first)
  {
   close(STDOUT_FILENO);
   if (open(args[1],O_RDWR|O_CREAT|O_TRUNC,0777)==-1)
   {
    perror("Can not open the file");
    exit(EXIT_SUCCESS);
   }
   close(STDIN_FILENO);
   if (open(args[2],O_RDONLY)==-1)
   {
    perror("Can not open file");
    exit(EXIT_SUCCESS);
   }
   if (execvp(myargs[0],myargs)==-1)
   {
    perror("Unable to start");
    exit(EXIT_SUCCESS);
   }
  }
  else
  {
   close(STDIN_FILENO);
   if (open(args[1],O_RDONLY)==-1)
   {
    perror("Can not open the file");
    exit(EXIT_SUCCESS);
   }
   close(STDOUT_FILENO);
   if (open(args[2],O_RDWR|O_CREAT|O_TRUNC,0777)==-1)
   {
    perror("Can not open file");
    exit(EXIT_SUCCESS);
   }
   if (execvp(myargs[0],myargs)==-1)
   {
    perror("Unable to start");
    exit(EXIT_SUCCESS);
   }
  }
 }
 else
 {
  wait(&status);
  if (status>>8==0xff)
  {
   fprintf(stderr,"Unable to execute input output redirection\n");
   return;
  }
 }
}

/*This function will contain the implementation of the execution of library functions*/
void myshell_executelibraryfunction(char** args)
{
 int status;
 if (fork()==0)
 {
 if (execvp(args[0],args)==-1)
 {
  perror("Unable to start");
  exit(EXIT_SUCCESS);
 }
 }
 else
 {
  wait(&status);
  if (status>>8==0xff)
  {
   fprintf(stderr,"Unable to execute the command\n");
  }
 }
}

/*This function will contain the implementation of myls command*/
void myshell_execute_mylsfunction(char** args)
{
 int status;
 char myls_cmd_token[2000];
 char* mylspath;
 char* mylstoken="myls";
 char* myargs[3];
 
 execute_myls_command=0;
 mylspath=getenv("MYPATH");
 if (mylspath==NULL)
 {
  fprintf(stderr,"Please set MYPATH first using set command\n");
  return;
 }
 sprintf(myls_cmd_token,"%s/%s",mylspath,mylstoken);

  if (fork()==0)
 {
  myargs[0]=myls_cmd_token;
  if (args[1]!=NULL)
  {
   myargs[1]=args[1];
  }
  else
  {
    myargs[1]="./";
  }
  myargs[2]=NULL;
  if (execv(myargs[0],myargs)==-1)
  {
   perror("Unable to start");
   exit(EXIT_SUCCESS);
  }
 }
 else
 {
  wait(&status);
  if (status>>8==0xff)
  {
   fprintf(stderr,"Unable to execute myls command\n");
  }
 }

}

/*This function will implement the background process execution*/
void myshell_execute_backgroundfunction(char** args)
{
 char* myargs[8];
 char* buffer[4];
 int status;
 int pid;
 int position=0;
 int output_redirection=0;
 int input_redirection=0;
 int in_out_redirection=0;
 int out_first=0;
 char* token;
 char* token_delimiter_output=">";
 char* token_delimiter_input="<";
 char* token_delimiter=" \t\r";

 execute_background_command=0;
 if (args[0]==NULL)
 {
  fprintf(stderr,"Invalid arguments for executing background command\n");
  return;
 }
 
 if ((strchr(args[0],'>')!=NULL) && (strchr(args[0],'<')==NULL))
  {
    output_redirection=1;
    token=strtok(args[0],token_delimiter_output);
    if (token!=NULL)
    {
     buffer[position]=token;
     position++;
     token=strtok(NULL,token_delimiter_output);
     if (token!=NULL)
     {
      buffer[position]=token;
      position++;
     }
    }
   buffer[position]=NULL;
 }

 if ((strchr(args[0],'<')!=NULL) && (strchr(args[0],'>')==NULL))
  {
   input_redirection=1;
   token=strtok(args[0],token_delimiter_input);
   if (token!=NULL)
   {
    buffer[position]=token;
    position++;
    token=strtok(NULL,token_delimiter_input);
    if (token!=NULL)
    {
     buffer[position]=token;
     position++;
    }
   }
   buffer[position]=NULL;
  }

  if ((strchr(args[0],'<')!=NULL) && (strchr(args[0],'>')!=NULL))
  {
   in_out_redirection=1;
    /*reset the output first variable*/
    out_first=0;
   /*if output redirection symbol appears first then we should set output first variable to 1*/
   if (((strchr(args[0],'>'))-(strchr(args[0],'<')))<0)
   {
    out_first=1;
   }
   if (out_first)
   {
    token=strtok(args[0],token_delimiter_output);
    if (token!=NULL)
    {
     buffer[position]=token;
     position++;
     token=strtok(NULL,token_delimiter_input);
      if (token!=NULL)
      {
       buffer[position]=token;
       position++;
       token=strtok(NULL,token_delimiter_input);
       if (token!=NULL)
       {
        buffer[position]=token;
        position++;
       }
     }
   }
  }
 else
 {
  token=strtok(args[0],token_delimiter_input);
  if (token!=NULL)
  {
   buffer[position]=token;
   position++;
   token=strtok(NULL,token_delimiter_output);
   if (token!=NULL)
   {
    buffer[position]=token;
    position++;
    token=strtok(NULL,token_delimiter_output);
    if (token!=NULL)
    {
     buffer[position]=token;
     position++;
    }
   }   
  }  
 }
  buffer[position]=NULL;
 }

 position=0;
 if (input_redirection==1 || output_redirection==1 || in_out_redirection==1)
 {
  token=strtok(buffer[0],token_delimiter);
  while(token!=NULL && position<5)
  {
   myargs[position]=token;
   position++;
   token=strtok(NULL,token_delimiter);
  }
 }
 else
 {
  token=strtok(args[0],token_delimiter);
   while(token!=NULL && position<5)
   {
    myargs[position]=token;
    position++;
    token=strtok(NULL,token_delimiter);
  }
 }
 myargs[position]=NULL;

 /*This process will create another child process which will execute the command.To prevent the command process being zombie process*/
 if ((pid=fork())==0)
 {
   if (fork()==0)
   {
    if (input_redirection==1)
    {
     close(STDIN_FILENO);
     if (open(buffer[1],O_RDONLY)==-1)
     {
      perror("File can not be opened");
      exit(EXIT_SUCCESS);
     }
     if (execvp(myargs[0],myargs)==-1)
     {
      perror("Unable to start");
      exit(EXIT_SUCCESS);
     }
    }
    else if (output_redirection==1)
    {
     close(STDOUT_FILENO);
     if (open(buffer[1],O_RDWR|O_CREAT|O_TRUNC,0777)==-1)
     {
      perror("File can not be opened");
      exit(EXIT_SUCCESS);
     }
     close(STDIN_FILENO);
     if (open("/dev/null",O_RDWR)==-1)
     {
      perror("File could not be opened\n");
      exit(EXIT_SUCCESS);
     }
     if (execvp(myargs[0],myargs)==-1)
     {
      perror("Unable to start");
      exit(EXIT_SUCCESS);
     }
    }
    else if (in_out_redirection==1)
    {
     if (out_first==1)
     {
      close(STDOUT_FILENO);
      if (open(buffer[1],O_RDWR|O_CREAT|O_TRUNC,0777)==-1)
      {
       perror("Can not open the file");
       exit(EXIT_SUCCESS);
      }
      close(STDIN_FILENO);
      if (open(buffer[2],O_RDONLY)==-1)
      {
       perror("Can not open file");
       exit(EXIT_SUCCESS);
      }
      if (execvp(myargs[0],myargs)==-1)
      {
        perror("Unable to start");
	exit(EXIT_SUCCESS);
      }
     }
     else
     {
      close(STDIN_FILENO);
      if (open(buffer[1],O_RDONLY)==-1)
      {
        perror("Can not open the file");
	exit(EXIT_SUCCESS);
      }
      close(STDOUT_FILENO);
      if (open(buffer[2],O_RDWR|O_CREAT|O_TRUNC,0777)==-1)
      {
        perror("Can not open file");
	exit(EXIT_SUCCESS);
      }
      if (execvp(myargs[0],myargs)==-1)
      {
       perror("Unable to start");
       exit(EXIT_SUCCESS);
      }
     }
    }
    else
    {
     close(STDIN_FILENO);
     if (open("/dev/null",O_RDWR)==-1)
     {
      perror("Background process could not be started\n");
      exit(EXIT_SUCCESS);
     }
     if (execvp(myargs[0],myargs)==-1)
     {
      perror("Unable to start");
      exit(EXIT_SUCCESS);
     }
    }
   }
   exit(EXIT_SUCCESS);
 }
 else
 {
  waitpid(pid,&status,0);
  if (status>>8==0xff)
  {
   fprintf(stderr,"Unable to execute background process\n");
  }
  return;
 }
}
