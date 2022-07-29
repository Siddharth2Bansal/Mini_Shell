#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include<sys/wait.h>

#define MAXCWDLENGTH 512
#define MAXCMDLENGTH 1024
#define MAXWORDS 64
#define DELIM " "

char* Location();
char* ReadInput();
char** Parser(char *);
void getEnvFunc(char**);
void setEnvFunc(char**);
void cdFunc(char **);
void savetoHistory(char*, char*);
void showHistory(char*);
void EmptyHistory(char*);
int redirectionChecker(char**);


int main()
{    
    //stores the path to the directory containing the shell file.
    char* originalcwd = Location();

    while(1)
    {
        //stores the current directory's path to a pointer.
        char* cwd = Location();
        printf("%s> ",cwd);
        char* input = ReadInput();

        if(input[0]=='\0')
        {
            continue;
            free(cwd);
            free(input);
        }

        char** parsedinput = Parser(input);

        if(!(strcmp(parsedinput[0],"getenv")))
        {
            getEnvFunc(parsedinput);
            savetoHistory(input,originalcwd);
            free(cwd);
            free(input);
            free(parsedinput);
            continue;
        }

        if(!(strcmp(parsedinput[0],"setenv")))
        {
            setEnvFunc(parsedinput);
            savetoHistory(input,originalcwd);
            free(cwd);
            free(input);
            free(parsedinput);
            continue;
        }

        if(!(strcmp(parsedinput[0],"cd")))
        {
            cdFunc(parsedinput);
            savetoHistory(input,originalcwd);
            free(cwd);
            free(input);
            free(parsedinput);
            continue;
        }

        if(!(strcmp(parsedinput[0],"history")))
        {
            savetoHistory(input, originalcwd);
            showHistory(originalcwd);
            free(cwd);
            free(input);
            free(parsedinput);
            continue;
        }

        if(!(strcmp(parsedinput[0], "exit")))
        {
            EmptyHistory(originalcwd);
            break;
        }
        
        int id= fork();

        if(id==0)
        {   
            if(redirectionChecker(parsedinput) == -1)
            {
                printf("Error processing the  Redirection.\n");
            }
            execvp(parsedinput[0], parsedinput);
            printf("Error processing request, please make sure the command was valid.\n");
        }else if(id < 0)
        {
            printf("error forking.\n");
        }else
        {
            wait(NULL);
            free(cwd);
            free(input);
            free(parsedinput);
            
        }

    }

    free(originalcwd);

    return 0;
}


// absolute path of current directory
char* Location()
{
    char* buf;
    buf=(char*)malloc(MAXCWDLENGTH*sizeof(char));
    getcwd(buf, MAXCWDLENGTH);
    return buf;
}


// read input line
char* ReadInput()
{
    char* buf;
    buf=(char*)malloc(MAXCMDLENGTH*sizeof(char));
    int c;
    int position=0;
    
    while(1)
    {
        c= getchar();
        if(c=='\n')
        {
            buf[position] = '\0';
            return buf;
        }else
        {
            buf[position] = c;
            position++;
        }
    }
}

//function for getenv
void getEnvFunc(char** args)
{
    if(getenv(args[1]) == NULL)
    {
        printf("No Such Environmental Variable Exists.\n");
    }else
    {
        printf("%s\n",getenv(args[1]));
    }
    return;
}

//function for setenv
void setEnvFunc(char** args)
{
    
    {
        setenv(args[1], args[3], 1);
    }
}

//parses the input 
char** Parser(char * str)
{
    char * temp;
    temp=(char*)malloc(MAXCMDLENGTH*sizeof(char));
    int i=0;
    do
    {
        temp[i] = str[i];
        i++;
    } while (str[i] != '\0');
    
    char* token = strtok(temp, DELIM);
    char ** buf;
    buf = (char**)malloc(sizeof(char*)*MAXWORDS);
    buf[0]= token;
    i=1;
    while (token != NULL)
    {           
        token = strtok(NULL, DELIM);
        buf[i]=token;
        i++;
    }
    return buf;
}


void cdFunc(char ** args)
{
    if(args[1]!=NULL)
    {
        if ( chdir(args[1]) != 0 )
        {
            perror("ERROR CHANGING DIRECTORY ");
        }
    }else
    {
        chdir(getenv("HOME"));
    }
    return;
}

void savetoHistory(char* inp, char* oriloc)
{
    char* currentlocation = Location();
    chdir(oriloc);
    FILE * fp;
    fp= fopen("/tmp/history.txt", "a");
    fprintf(fp,"%s\n",inp);
    fclose(fp);
    chdir(currentlocation);
    free(currentlocation);
    return ;
}

void showHistory(char* oriloc)
{
    char* currentlocation = Location();
    chdir(oriloc);
    FILE * fp;
    fp= fopen("/tmp/history.txt", "r");
    
    
    int c = fgetc(fp);
    while (c != EOF)
    {
        printf ("%c", c);
        c = fgetc(fp);
    }


    fclose(fp);
    chdir(currentlocation);
    free(currentlocation);
    return ;
}

void EmptyHistory(char* oriloc)
{
    char* currentlocation = Location();
    chdir(oriloc);
    FILE * fp;
    fp= fopen("/tmp/history.txt", "w");
    fclose(fp);
    chdir(currentlocation);
    free(currentlocation);
}

int redirectionChecker(char** args)
{
    int i=0;
    while(args[i] != NULL)
    {
        if (!(strcmp(args[i], "<")))
        {
            if(freopen(args[i+1], "r", stdin) == NULL)
            {
                return -1;
            }
        }else if(!(strcmp(args[i], ">")))
        {
            if(freopen(args[i+1], "w", stdout) == NULL)
            {
                return -1;
            }
        }else if(!(strcmp(args[i], ">>" )))
        {
            if(freopen(args[i+1], "a", stdout) == NULL)
            {
                return -1;
            }
        }
        i++;
    }

    return 0;

}