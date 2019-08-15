//#===================================
//#====   Created by rachelive   =====
//#===================================


//#======================
//#====   INCLUDES   ====
//#======================
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include "threadpool.h"

//#======================
//#====   DEFINES   =====
//#======================

#define AINT_PERMITIONS 0
#define PERMITIONS 1
#define END_BUFEER "\r\n"
#define HTTP_VER "HTTP/1.0 "
#define EXIT_ERROR -1
#define TIME_BUF_SIZE 128
#define RFC1123FMT "%a, %d %b %Y %H:%M:%S GMT"
#define HTTP(x) "HTTP/1.0 " x "\r\n"
#define SERVER "Server: webserver/1.0\r\n"
#define DATE "Date: "
#define LOCATION "\r\nLocation: /"
#define CTYPE  "/\r\nContent-Type: text/html\r\n"
#define CTYPE2  "\r\nContent-Type: text/html\r\n"
#define CLEN   "Content-Length: "
#define CLOUSE "\r\nConnection: close\r\n\r\n"
#define SECOUND_PART(a,b,c) "<HTML><HEAD><TITLE>" a "</TITLE></HEAD>\r\n<BODY><H4>" b "</H4>\r\n" c "\r\n</BODY></HTML>\r\n"


//#===========================
//#=====   DECLARATION   =====
//#===========================

int   AnalyzesRequest(void*a);
int   checkForNum(char* string);
int   permitionsCheck(char *path);
void  twoHundred(char* path,int sockfd);
void  seekInFile(char * path,int sockfd);
char* get_mime_type(char *name);
char* tableThree(char* second_word);
char* checkForDirAndFile(char * dir, char * file);
char* fiveHundred(char* path,int sockfd);
char* fourHundred(char* path,int sockfd);
char* fiveHundredOne(char* path,int sockfd);
char* fourHundredFour(char* path,int sockfd);
char* threeHundredTwo(char* path,int sockfd);
char* fourHundredThree(char* path,int sockfd);
char* contentDir(char*  path  , char * table,int sockfd );
char* response_index_html(char * fileContent,char* path,int sockfd);

//#================================
//#=====   ABOUT THE PROGRAM  =====
//#================================



/******
*           - HTTP Server -
* This programming implements HTTP server.
- Constructs an HTTP response based on client's request.
- Sends the response to the client.
* The server handle the connections with the clients.
* In TCP, a server creates a socket for each client it talks to.
* there is always one (main) socket where the server listens to connections and for each client connection request,
* the server opens another socket. In order to enable multithreaded program, the server creates threads(threadPool)
* that handle the connections with the clients.*
*********************************************************************************/




//#================================
//#=====   FUNCTION COMMENTS  =====
//#================================


/*****************main********************
*This method implements network communication connction by the server side.
*specify transport protocol(TCP)
*The socket is an endpoint for communication, to which a name can be bound.
*Like files, each socket is identified by a small integer called its socket descriptor.
*that lets an application read/write data from/to the network.
*SOCK_STREAM: selects the TCP protocol
*The bind function assigns a local protocol address to a socket.
*With PF_INET, the protocol address is the combination of 32-bit IPv4 address and a 16-bit TCP or UDP port number.
*The listen function is called only by a TCP server
*converts an unconnected socket to the listening(passive) socket on which incoming connections from clients can be accepted
*Once a server socket has been established (socket, bind, listen), a server calls accept() to extract the next incoming connection request.
*A call to accept() blocks until a connection request arrives.
*********************************************************************************************/

/******************handle_request**********
* This method  Analyzes the client request
1. Read request from socket
2. Check input: The request first line should contain method, path and protocol
*I expressed the client request by three parts httpPart pathPart protocolPart
*I used the strtok function from library c to get the parts consumed from the client.
*And I checked a specific case in which we received a path from the customer.
*I used the memmove function to move the cerstring one forward.
*stat function provided information for a folder and files.
*After that, it is possible to distinguish between cases to handle customer requests
***********************************************************************************************/

/**********checkForNum********************************************
* The function accepts a string and check if it a number
* (using atoi c libary)
*****************************************************************/

/*******************responses_functions*****************************
*the create a response, we construct it as follow:
*First line (version, status, phrase)\r\n
*Server: webserver/1.0\r\n
*Date: \r\n (more later)
*Location: \r\n (only if status is 302)
*Content-Type: \r\n (more later)
*Content-Length: \r\n
*Last-Modified: \r\n (more later)
*Connection: close\r\n \r\n
*and read from file when needed
*********************************************************************/

//#=====================
//#=====   MAIN  =======
//#=====================




int main(int argc, char **argv)
{
    char* numberRequests=argv[3];
    char* sizePool=argv[2];
    struct sockaddr_in serv_addr;
    struct sockaddr cli_addr;
    int client = sizeof(serv_addr);
    int sockfd;

    if (argc != 4)
    {
        printf("Usage: server <port> <pool-size> <max-number-of-request>\n");
        exit(EXIT_ERROR);
    }

    if(checkForNum(argv[1]) == -1 || checkForNum(sizePool) == -1 || checkForNum(numberRequests) == -1)
    {
        printf("Usage: server <port> <pool-size> <max-number-of-request>\n");
        exit(EXIT_ERROR);
    }
    if(atoi(argv[1]) < 1 || atoi(argv[2]) < 1 || atoi(argv[3])< 1)
    {
        printf("Usage: server <port> <pool-size> <max-number-of-request>\n");
        exit(EXIT_ERROR);
    }
    int newsockfd[atoi(numberRequests)];

    threadpool *t = create_threadpool(atoi(sizePool));
    if(t == NULL)
    {
        printf("Usage: server <port> <pool-size> <max-number-of-request>\n");
        exit(EXIT_ERROR);
    }

    if ((sockfd = socket(AF_INET,SOCK_STREAM,0)) < 0)  //creating the main socket
    {
        perror("socket");
        destroy_threadpool(t);
        exit(EXIT_ERROR);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(atoi(argv[1]));

    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("bind");
        destroy_threadpool(t);
        exit(EXIT_ERROR);
    }

    if(listen(sockfd, 5) < 0)
    {
        perror("listen");
        destroy_threadpool(t);
        close(sockfd);
        exit(EXIT_ERROR);
    }
    int i;
    for(i = 0 ; i < atoi(numberRequests) ; ++i)
    {
        if((newsockfd[i] = accept(sockfd, &cli_addr, (socklen_t *)&client)) < 0)
        {
            perror("accept");
            close(sockfd);
            exit(EXIT_ERROR);
        }
        dispatch(t,AnalyzesRequest,&newsockfd[i]);
    }
	close(sockfd);
    destroy_threadpool(t);
    return 0;
}

//#================================
//#=====   AnalyzesRequest  =======
//#================================
int AnalyzesRequest(void*a){


    char* tokens;
    char* temp;
    char httpPart[1000];
    char pathPart[1000];
    char protocolPart[1000];
    char* httpPart1;
    char* pathPart1;
    char* protocolPart1;
    bzero(httpPart,100);
    bzero(pathPart,100);
    bzero(protocolPart,100);


    char buffer[4000];
    bzero(buffer,4000);

    int sockfd = *(int*)a;

    int size;

    char* fileContent=NULL;



    while(1) {
        if ((size = read(sockfd, buffer, sizeof(buffer))) < 0) //read request
        {
            perror("read Error");
            fiveHundred(pathPart,sockfd);
            return 0;
        }
        if(strchr(buffer,'\r')!=NULL||size==0)
            break;
    }



    char bufDup [strlen(buffer)+1];
    strncpy(bufDup,buffer,strlen(buffer)+1);
    if(strstr(buffer,END_BUFEER)) {
        tokens = strtok(bufDup, "\r\n");

        httpPart1 = strtok(bufDup, " ");
        pathPart1 = strtok(NULL, " ");
        protocolPart1 = strtok(NULL, " ");
        temp = strtok(NULL, " ");


        strncpy(httpPart,httpPart1,strlen(httpPart1));
        strncpy(pathPart,pathPart1,strlen(pathPart1));
        strncpy(protocolPart,protocolPart1,strlen(protocolPart1));

    }


    if(!(strlen(pathPart)==1)&& !(strcmp(pathPart,"/")==0)) {

        memmove(pathPart, pathPart+1, strlen(pathPart));

    }else{
        char privateCase[3]="./";
        strncpy(pathPart,privateCase,strlen(privateCase));
    }


    struct stat fileStat;

    if(strcmp(buffer,"")==0 || httpPart==NULL || pathPart==NULL || protocolPart==NULL || temp!=NULL || tokens==NULL){
        fourHundred(pathPart,sockfd);
        return 0;
    }

    else if(strcmp(httpPart,"GET")!=0)
    {
        fiveHundredOne(pathPart,sockfd);
        return 0;
    }

    else if( (strcmp(protocolPart,"HTTP/1.0")!=0) && (strcmp(protocolPart,"HTTP/1.1")!=0)){
        fourHundred(pathPart,sockfd);
        return 0;
    }

    else if(( stat(pathPart, &fileStat ) != 0)&& (strlen(pathPart) != 0)){ //path request does not exists
        fourHundredFour(pathPart,sockfd);
        return 0;
    }
    else if((fileStat.st_mode & S_IFDIR) && pathPart[strlen(pathPart)-1]!='/'){
        threeHundredTwo(pathPart,sockfd);
        return 0;
    }
    else if((fileStat.st_mode & S_IFDIR) &&  pathPart[strlen(pathPart)-1]=='/'){

        char wholePath [sizeof(pathPart) + strlen("index.html") + 1];
        bzero(wholePath,sizeof(pathPart) + strlen("index.html") + 1);
        strncat(wholePath, pathPart, strlen(pathPart));
        strncat(wholePath, "index.html", strlen("index.html"));

        if (stat(wholePath, &fileStat) >= 0)
        {
            if(permitionsCheck(wholePath)==PERMITIONS) {
                fileContent = checkForDirAndFile(pathPart, "index.html");
                if (fileContent != NULL) {
                    response_index_html(fileContent, pathPart,sockfd);
                    return 0;
                }
                else if(strcmp(fileContent,"ERROR"))
                {
                    fiveHundred(pathPart,sockfd);
                    return 1;
                }
            }
            else{

                fourHundredThree(pathPart,sockfd);
                return 0;
            }
        }
        else if(stat(wholePath, &fileStat) < 0)
        {
            if(permitionsCheck(pathPart)==PERMITIONS) {
                contentDir(pathPart, tableThree(pathPart),sockfd);
                return 0;
            }
            else{

                fourHundredThree(pathPart,sockfd);
                return 0;
            }
        }
    }

    else if(!(fileStat.st_mode & S_IFDIR))///if path is a file
    {

        if(!S_ISREG(fileStat.st_mode))
        {
            fourHundredThree(pathPart,sockfd);
            return 0;
        }
        else if(!(fileStat.st_mode & S_IRUSR) || !(fileStat.st_mode & S_IRGRP) || !(fileStat.st_mode & S_IROTH))
        {
            fourHundredThree(pathPart,sockfd);
            return 0;
        }
        else
        {

            int ans=permitionsCheck(pathPart);
            if(ans==PERMITIONS){

                twoHundred(pathPart,sockfd);
                return 0;
            }
            else {

                fourHundredThree(pathPart,sockfd);
                return 0;
            }
        }
    }

    return 0;
}

//#================================
//#=====   checkForNum  ===========
//#================================
int checkForNum(char* string)
{

    if((string==NULL) || (strcmp(string,"")==0) || (strcmp(string," ")==0))
        return EXIT_ERROR;

    for (int i = 0; i < strlen(string); i++)
    {
        if (string[i] < 48 || string[i] > 57)
            return -1;
    }

    int num = atoi( string );

    if (num == 0 && string[0] != '0')
        return EXIT_ERROR;
    if(num<0 || num>65535)
        return EXIT_ERROR;
    else
        return num;
}



//#================================
//#=====   fourHundred ===========
//#================================
char* fourHundred(char* path ,int sockfd){
    time_t now;
    char currectTime[TIME_BUF_SIZE];
    now = time(NULL);
    strftime(currectTime, sizeof(currectTime), RFC1123FMT, gmtime(&now));




    char orignalTextSize[100];int size_response=0;
    sprintf(orignalTextSize,"%d" ,(int)strlen(SECOUND_PART("400 Bad Request","400 Bad Request","Bad Request.")));
    size_response += strlen(HTTP("400 Bad Request"));
    size_response += strlen(SERVER);
    size_response += strlen(DATE);
    size_response += strlen(currectTime);
    size_response += strlen(CTYPE2);
    size_response += strlen(CLEN);
    size_response += strlen(CLOUSE);
    size_response += strlen(orignalTextSize);
    size_response += strlen(SECOUND_PART("400 Bad Request","400 Bad Request","Bad Request."));
    size_response +=1000;
    char str [size_response];
    bzero(str,size_response);
    strcat(str,HTTP("400 Bad Request"));
    strcat(str,SERVER);
    strcat(str,DATE);
    strcat(str,currectTime);
    strcat(str,CTYPE2);
    strcat(str,CLEN);
    strcat(str,orignalTextSize);
    strcat(str,CLOUSE);
    strcat(str,SECOUND_PART("400 Bad Request","400 Bad Request","Bad Request."));




    str[size_response]='\0';
    int n = 0;
    int sum_w = 0;

    while (1) {
        n = write(sockfd, str, strlen((str)));
        sum_w += n;

        if (sum_w == strlen(str)) { break; }
        if (n < 0) {
            perror("ERROR writing to socket");

            return NULL;
        }
    }
    close(sockfd);
    return "SUCCESS";
}

//#================================
//#=====   fourHundredTwo  ========
//#================================
char* threeHundredTwo(char* path,int sockfd){
    time_t now;
    char currectTime[TIME_BUF_SIZE];
    now = time(NULL);
    strftime(currectTime, sizeof(currectTime), RFC1123FMT, gmtime(&now));


    char orignalTextSize[100]; int size_response=0;
    sprintf(orignalTextSize,"%d" ,(int)strlen(SECOUND_PART("302 Found","302 Found","Directories must end with a slash.")));
    size_response += strlen(HTTP("302 Found"));
    size_response += strlen(SERVER);
    size_response += strlen(DATE);
    size_response += strlen(currectTime);
    size_response += strlen(LOCATION);
    size_response += strlen(path);
    size_response += strlen(CTYPE);
    size_response += strlen(CLEN);
    size_response += strlen(CLOUSE);
    size_response += strlen(orignalTextSize);
    size_response += strlen(SECOUND_PART("302 Found","302 Found","Directories must end with a slash."));
    size_response +=1000;
    char str [size_response+1];
    bzero(str,size_response+1);
    strcat(str,HTTP("302 Found"));
    strcat(str,SERVER);
    strcat(str,DATE);
    strcat(str,currectTime);
    strcat(str,LOCATION);
    strcat(str,path);
    strcat(str,CTYPE);
    strcat(str,CLEN);
    strcat(str,orignalTextSize);
    strcat(str,CLOUSE);
    strcat(str,SECOUND_PART("302 Found","302 Found","Directories must end with a slash."));




    str[size_response]='\0';
    int n = 0;
    int sum_w = 0;

    while (1) {
        n = write(sockfd, str, strlen((str)));
        sum_w += n;

        if (sum_w == strlen(str)) { break; }
        if (n < 0) {
            perror("ERROR writing to socket");

            return NULL;
        }
    }
    close(sockfd);
    return "SUCCESS";
}

//#================================
//#=====   threeHundredThree  =====
//#================================
char* fourHundredThree(char* path,int sockfd){
    time_t now;
    char currectTime[TIME_BUF_SIZE];
    now = time(NULL);
    strftime(currectTime, sizeof(currectTime), RFC1123FMT, gmtime(&now));


    char orignalTextSize[100]; int size_response=0;
    sprintf(orignalTextSize,"%d" ,(int)strlen(SECOUND_PART("403 Forbidden","403 Forbidden","Access denied.")));
    size_response += strlen(HTTP("403 Forbidden"));
    size_response += strlen(SERVER);
    size_response += strlen(DATE);
    size_response += strlen(currectTime);
    size_response += strlen(CTYPE2);
    size_response += strlen(CLEN);
    size_response += strlen(orignalTextSize);
    size_response += strlen(CLOUSE);
    size_response += strlen(SECOUND_PART("403 Forbidden","403 Forbidden","Access denied."));
    size_response +=1000;
    char str[size_response+1];
    bzero(str,size_response+1);
    strcat(str,HTTP("403 Forbidden"));
    strcat(str,SERVER);
    strcat(str,DATE);
    strcat(str,currectTime);
    strcat(str,CTYPE2);
    strcat(str,CLEN);
    strcat(str,orignalTextSize);
    strcat(str,CLOUSE);
    strcat(str,SECOUND_PART("403 Forbidden","403 Forbidden","Access denied."));




    str[size_response]='\0';
    int n = 0;
    int sum_w = 0;

    while (1) {
        n = write(sockfd, str, strlen((str)));
        sum_w += n;

        if (sum_w == strlen(str)) { break; }
        if (n < 0) {
            perror("ERROR writing to socket");

            return NULL;
        }
    }
    close(sockfd);
    return "SUCCESS";
}

//#================================
//#=====   fourHundredFour  =======
//#================================
char* fourHundredFour(char* path,int sockfd) {
    time_t now;
    char currectTime[TIME_BUF_SIZE];
    now = time(NULL);
    strftime(currectTime, sizeof(currectTime), RFC1123FMT, gmtime(&now));


    char orignalTextSize[100];
    int size_response = 0;
    sprintf(orignalTextSize, "%d", (int) strlen(SECOUND_PART("404 Not Found", "404 Not Found", "File not found.")));
    size_response += strlen(HTTP("404 Not Found"));
    size_response += strlen(SERVER);
    size_response += strlen(DATE);
    size_response += strlen(currectTime);
    size_response += strlen(CTYPE2);
    size_response += strlen(CLEN);
    size_response += strlen(CLOUSE);
    size_response += strlen(orignalTextSize);
    size_response += strlen(SECOUND_PART("404 Not Found", "404 Not Found", "File not found."));
    size_response +=1000;

    char str [size_response + 10];
    bzero(str, size_response + 10);
    strcat(str, HTTP("404 Not Found"));
    strcat(str, SERVER);
    strcat(str, DATE);
    strcat(str, currectTime);
    strcat(str, CTYPE2);
    strcat(str, CLEN);
    strcat(str, orignalTextSize);
    strcat(str, CLOUSE);

    strcat(str,SECOUND_PART("404 Not Found","404 Not Found","File not found."));

    str[size_response] = '\0';

    int n = 0;
    int sum_w = 0;

    while (1) {
        n = write(sockfd, str, strlen((str)));
        sum_w += n;

        if (sum_w == strlen(str)) { break; }
        if (n < 0) {
            perror("ERROR writing to socket");

            return NULL;
        }
    }
    close(sockfd);
    return "SUCCESS";
}

//#================================
//#=====   fiveHundred  ===========
//#================================
char* fiveHundred(char* path,int sockfd) {
    time_t now;
    char currectTime[TIME_BUF_SIZE];
    now = time(NULL);
    strftime(currectTime, sizeof(currectTime), RFC1123FMT, gmtime(&now));


    char orignalTextSize[100];
    int size_response = 0;
    sprintf(orignalTextSize,"%d" ,(int)strlen(SECOUND_PART("500 Internal Server Error","500 Internal Server Error","Some server side error.")));
    size_response += strlen(HTTP("500 Internal Server Error"));
    size_response += strlen(SERVER);
    size_response += strlen(DATE);
    size_response += strlen(currectTime);
    size_response += strlen(CTYPE2);
    size_response += strlen(CLEN);
    size_response += strlen(CLOUSE);
    size_response += strlen(orignalTextSize);
    size_response += strlen(SECOUND_PART("500 Internal Server Error","500 Internal Server Error","Some server side error."));
    size_response +=1000;
    char str [size_response + 1];
    bzero(str,size_response+1);
    strcat(str, HTTP("500 Internal Server Error"));
    strcat(str, SERVER);
    strcat(str, DATE);
    strcat(str, currectTime);
    strcat(str, CTYPE2);
    strcat(str, CLEN);
    strcat(str, orignalTextSize);
    strcat(str, CLOUSE);
    strcat(str,SECOUND_PART("500 Internal Server Error","500 Internal Server Error","Some server side error."));


    str[size_response]='\0';
    int n = 0;
    int sum_w = 0;

    while (1) {
        n = write(sockfd, str, strlen((str)));
        sum_w += n;

        if (sum_w == strlen(str)) { break; }
        if (n < 0) {
            perror("ERROR writing to socket");

            return NULL;
        }
    }
    close(sockfd);
    return "SUCCESS";
}
//#================================
//#=====   fiveHundredOne  ========
//#================================
char* fiveHundredOne(char* path,int sockfd) {
    time_t now;
    char currectTime[TIME_BUF_SIZE];
    now = time(NULL);
    strftime(currectTime, sizeof(currectTime), RFC1123FMT, gmtime(&now));


    char orignalTextSize[100];
    int size_response = 0;
    sprintf(orignalTextSize,"%d" ,(int)strlen(SECOUND_PART("501 Not supported","501 Not supported","Method is not supported.")));
    size_response += strlen(HTTP("501 Not supported"));
    size_response += strlen(SERVER);
    size_response += strlen(DATE);
    size_response += strlen(currectTime);
    size_response += strlen(CTYPE2);
    size_response += strlen(CLEN);
    size_response += strlen(CLOUSE);
    size_response += strlen(orignalTextSize);
    size_response += strlen(SECOUND_PART("501 Not supported","501 Not supported","Method is not supported."));
    size_response +=1000;
    char str [size_response + 1];
    bzero(str,size_response+1);
    strcat(str, HTTP("501 Not supported"));
    strcat(str, SERVER);
    strcat(str, DATE);
    strcat(str, currectTime);
    strcat(str, CTYPE2);
    strcat(str, CLEN);
    strcat(str, orignalTextSize);
    strcat(str, CLOUSE);
    strcat(str,SECOUND_PART("501 Not supported","501 Not supported","Method is not supported."));

    str[size_response] = '\0';

    int n = 0;
    int sum_w = 0;

    while (1) {
        n = write(sockfd, str, strlen((str)));
        sum_w += n;

        if (sum_w == strlen(str)) { break; }
        if (n < 0) {
            perror("ERROR writing to socket");

            return NULL;
        }
    }
    close(sockfd);
    return "SUCCESS";
}
//#================================
//#=====   checkForDirAndFile  ====
//#================================
char* checkForDirAndFile(char * dir, char * file) {
    struct dirent **listOfNames;
    struct stat sb;

    int n;

    char *wholePath = (char *) malloc(sizeof(dir) + sizeof(file) + 1);
    if(wholePath==NULL)
    {
        printf("malloc Error");
        return "ERROR";
    }
    bzero(wholePath,(int)sizeof(wholePath));
    strncat(wholePath, dir,strlen(dir));
    strncat(wholePath, file,strlen(file));
    if(stat(wholePath, &sb)!=0){
        perror("stat Error\n");
        return "ERROR";
    };


    char *buffer = (char *) malloc((long long) sb.st_size +1);
    if(buffer==NULL)
    {
        printf("malloc Error\n");
        return "ERROR";
    }
    bzero(buffer, (int)sizeof(buffer));


    n = scandir(dir, &listOfNames, NULL, alphasort);
    if (n == -1) {
        perror("scandir");
        return "ERROR";
    }
    for (int i = 0; i < n; i++) {

        if (strcmp(listOfNames[i]->d_name, file) == 0) {



            if (stat(wholePath, &sb) == -1) {
                perror("stat");
                exit(EXIT_FAILURE);
            }

            int nbytes=0;
            FILE *f;


            f = fopen(wholePath, "rb");
            while ((nbytes = fread(buffer, 1,1024, f)) > 0);
            fclose(f);
            while (n--) {

                free(listOfNames[n]);
            }

            free(wholePath);
            free(listOfNames);
            buffer[sb.st_size]='\0';

            return buffer;

        }
    }

    while (n--) {

        free(listOfNames[n]);
    }

    free(wholePath);
    free(listOfNames);
    return NULL;
}
//#================================
//#=====   response_index_html  ===
//#================================
char * response_index_html(char * fileContent,char* path,int sockfd){
    int size_response=0;

    time_t now;
    now = time(NULL);
    char currectTime[TIME_BUF_SIZE];
    char time_modifed[128];

    strftime(currectTime, sizeof(currectTime), RFC1123FMT, gmtime(&now));

    struct stat info;
    stat(path,&info);


    strftime(time_modifed,sizeof(time_modifed),RFC1123FMT,gmtime(&info.st_mtime));

    char orignalTextSize[100];
    sprintf(orignalTextSize,"%d",(int)strlen(fileContent));

    size_response += strlen(HTTP("200 OK"));
    size_response += strlen(SERVER);
    size_response += strlen(DATE);
    size_response += strlen(currectTime);
    size_response += strlen(CTYPE2);
    size_response += strlen(CLEN);
    size_response += strlen(orignalTextSize);
    size_response += strlen("\r\n");
    size_response += strlen("Last-Modified: ");
    size_response += strlen(time_modifed);
    size_response += strlen(CLOUSE);
    size_response += strlen(fileContent);
    size_response +=1000;

    char str [size_response];
    bzero(str,size_response);

    strcat(str,HTTP_VER);
    strcat(str,"200 OK\r\n");
    strcat(str,"Server: webserver/1.0\r\n");
    strcat(str,"Date: ");
    strcat(str, currectTime);
    strcat(str,"\r\n");
    strcat(str,"Content-Type: ");
    strcat(str,"text/html");
    strcat(str, "\r\nContent-Length: ");
    strcat(str,orignalTextSize);
    strcat(str, "Last-Modified: ");
    strcat(str,time_modifed);
    strcat(str ,"\r\nConnection: close\r\n\r\n");


    strcat(str, fileContent);



    str[size_response]='\0';
    int n = 0;
    int sum_w = 0;

    while (1) {
        n = write(sockfd, str, strlen((str)));
        sum_w += n;

        if (sum_w == strlen(str)) { break; }
        if (n < 0) {
            perror("ERROR writing to socket");

            return NULL;
        }
    }
    close(sockfd);
    free(fileContent);
    return "SUCCESS";
}
//#================================
//#=====   tableThree  ============
//#================================
char* tableThree(char* second_word){
    struct dirent **listOfNames;
    struct stat info;
    int size_response=0;
    char orignalTextSize[100];

    size_response+=strlen("<HTML>\r\n<HEAD><TITLE>Index of /");
    size_response+=strlen(second_word);
    size_response+=strlen("</TITLE></HEAD>\r\n");
    size_response+=strlen("<BODY>\r\n<H4>Index of /");
    size_response+=strlen(second_word);
    size_response+=strlen("</H4>\r\n");
    size_response+=strlen("<table CELLSPACING=8>\r\n");
    size_response+=strlen("<tr><th>");
    size_response+=strlen("Name");
    size_response+=strlen("</th><th>");
    size_response+=strlen("Last Modified");
    size_response+=strlen("</th><th>");
    size_response+=strlen("Size");
    size_response+=strlen("</th></tr>\r\n");
    size_response+=strlen("</table>\r\n<HR>\r\n<ADDRESS>webserver/1.0</ADDRESS>\r\n</BODY></HTML>");
    size_response+=1000;

    int n = scandir(second_word, &listOfNames, NULL, alphasort);
    if (n == -1) {
        perror("scandir");
        return NULL;
    }
    char * treeTable = (char*)malloc(n*500 +size_response);
    if(treeTable==NULL)
    {
        printf("malloc Error");
        return NULL;
    }
    bzero(treeTable,500*n+size_response);
    strcat(treeTable,"<HTML>\r\n<HEAD><TITLE>Index of /");
    strcat(treeTable,second_word);
    strcat(treeTable,"</TITLE></HEAD>\r\n");
    strcat(treeTable,"<BODY>\r\n<H4>Index of /");
    strcat(treeTable,second_word);
    strcat(treeTable,"</H4>\r\n");
    strcat(treeTable,"<table CELLSPACING=8>\r\n");
    strcat(treeTable,"<tr><th>Name</th><th>Last Modified</th><th>Size</th></tr>\r\n");
    for(int i =0 ; i< n; i++){
        strcat(treeTable,"<tr><td><A HREF=\"");
        strcat(treeTable,listOfNames[i]->d_name);
        strcat(treeTable,"\">");
        strcat(treeTable,listOfNames[i]->d_name);
        strcat(treeTable,"</A></td><td>");
        char nameList[strlen(second_word)+strlen(listOfNames[i]->d_name)+1];
        bzero(nameList,strlen(second_word)+strlen(listOfNames[i]->d_name)+1);
        strcat(nameList,second_word);
        strcat(nameList, listOfNames[i]->d_name);

        char time_modifed[128];
        time_t now;
        now=time(NULL);
        strftime(time_modifed,sizeof(time_modifed),RFC1123FMT,gmtime(&now));
        strcat(treeTable,time_modifed);
        strcat(treeTable,"</td><td>");
        stat(nameList, &info);
        if(S_ISREG(info.st_mode))
        {
            sprintf(orignalTextSize,"%lld" ,(long long) info.st_size);
            strcat(treeTable,orignalTextSize);
        }
        strcat(treeTable,"</td></tr>");
        if(n!=0)
        {
            strcat(treeTable,"\r\n");
        }
    }
    strcat(treeTable,"</table>\r\n<HR>\r\n<ADDRESS>webserver/1.0</ADDRESS>\r\n</BODY></HTML>");

    while (n--) {
        printf("%s\n", listOfNames[n]->d_name);
        free(listOfNames[n]);
    }


    free(listOfNames);
    return treeTable;

}
//#================================
//#=====   contentDir  ===========
//#================================
char * contentDir(char*  path  , char * table,int sockfd ){
    if(table==NULL)
    {
        fiveHundred(path,sockfd);
        return NULL;
    }
    int size_response=0;

    time_t now;
    char currectTime[TIME_BUF_SIZE];
    now = time(NULL);
    strftime(currectTime, sizeof(currectTime), RFC1123FMT, gmtime(&now));



    struct stat info;
    stat(path,&info);
    char time_modifed[128];

    strftime(time_modifed,sizeof(time_modifed),RFC1123FMT,gmtime(&info.st_mtime));






    char orignalTextSize[100];
    sprintf(orignalTextSize,"%d\r\n",(int)strlen(table));

    size_response += strlen(HTTP("200 OK"));
    size_response += strlen(SERVER);
    size_response += strlen(DATE);
    size_response += strlen(currectTime);
    size_response += strlen(CTYPE2);
    size_response += strlen(CLEN);
    size_response += strlen(orignalTextSize);
    size_response += strlen("Last-Modified: ");
    size_response += strlen(time_modifed);
    size_response += strlen(CLOUSE);
    size_response += strlen(table);


    char str [size_response +3];
    bzero(str,size_response +3);

    strcat(str,(HTTP("200 OK")));
    strcat(str,SERVER);
    strcat(str,DATE);
    strcat(str, currectTime);
    strcat(str,CTYPE2);
    strcat(str,CLEN);
    strcat(str,orignalTextSize);
    strcat(str, "Last-Modified: ");
    strcat(str,time_modifed);
    strcat(str ,CLOUSE);
    strcat(str ,table);


    int n=0;
    int sum_w=0;


    while (1) {
        n = write(sockfd, str, strlen(str));
        sum_w += n;
        if (sum_w == strlen(str)) { break; }
        if (n < 0) {
            perror("ERROR writing to socket");

            return NULL;
        }
    }

    close(sockfd);

    free(table);
    return "SUCCESS";
}

//#================================
//#=====   twoHundred  ============
//#================================
void twoHundred(char* path,int sockfd) {

    time_t now;
    char currectTime[TIME_BUF_SIZE];
    now = time(NULL);
    strftime(currectTime, sizeof(currectTime), RFC1123FMT, gmtime(&now));



    struct stat info;
    stat(path,&info);
    char time_modifed[128];

    strftime(time_modifed,sizeof(time_modifed),RFC1123FMT,gmtime(&info.st_mtime));
    char *type = get_mime_type(path);

    char orignalTextSize[100];
    sprintf(orignalTextSize, "%lld", (long long) info.st_size);

    int size_response=0;


    size_response+= strlen(HTTP("200 OK"));
    size_response += strlen(SERVER);
    size_response += strlen(DATE);
    size_response += strlen(currectTime);
    size_response += strlen(CTYPE2);
    size_response += strlen(CLEN);
    size_response += strlen(orignalTextSize);
    size_response += strlen("\r\n");
    size_response += strlen("Last-Modified: ");
    size_response += strlen(time_modifed);
    size_response += strlen(CLOUSE);
    size_response += 1000;


    unsigned char  str [size_response];
    bzero(str, size_response);

    strcat((char*)str, HTTP_VER);
    strcat((char*)str, "200 OK\r\n");
    strcat((char*)str, "Server: webserver/1.0\r\n");
    strcat((char*)str, "Date: ");
    strcat((char*)str, currectTime);
    if(type){
        strcat((char*)str, "\r\n");
        strcat((char*)str, "Content-Type: ");
        strcat((char*)str, type);}
    strcat((char*)str, "\r\nContent-Length: ");
    strcat((char*)str, orignalTextSize);
    strcat((char*)str, "\r\nLast-Modified: ");
    strcat((char*)str, time_modifed);
    strcat((char*)str, "\r\nConnection: close\r\n\r\n");

    str[size_response] = '\0';

    int n = 0;
    int sum_w = 0;

    while (1) {
        n = write(sockfd, str, (int)strlen((const char*)str));
        sum_w += n;

        if (sum_w == (int)strlen((const char*)str)) { break; }
        if (n < 0) {
            perror("ERROR writing to socket");

            return;
        }
    }

    seekInFile(path,sockfd);


}

//#================================
//#=====   seekInFile  ============
//#================================
void seekInFile(char * path,int sockfd){
    size_t size = 1024;
    unsigned char buffer[size];
    FILE * f = fopen (path, "rb");
    fseek (f, 0, SEEK_SET);
    int nbytes;
    while((nbytes = fread (buffer,1,size, f)) > 0){
        write(sockfd,buffer,nbytes);}
    fclose(f);
}

//#================================
//#=====   get_mime_type  =========
//#================================
char *get_mime_type(char *name) {
    char *ext = strrchr(name, '.');
    if (!ext) return NULL;
    if (strcmp(ext, ".html") == 0 || strcmp(ext, ".htm") == 0) return "text/html";
    if (strcmp(ext, ".jpg") == 0 || strcmp(ext, ".jpeg") == 0) return "image/jpeg";
    if (strcmp(ext, ".gif") == 0) return "image/gif";
    if (strcmp(ext, ".png") == 0) return "image/png";
    if (strcmp(ext, ".css") == 0) return "text/css";
    if (strcmp(ext, ".au") == 0) return "audio/basic";
    if (strcmp(ext, ".wav") == 0) return "audio/wav";
    if (strcmp(ext, ".avi") == 0) return "video/x-msvideo";
    if (strcmp(ext, ".avi") == 0) return "video/x-msvideo";
    if (strcmp(ext, ".mpeg") == 0 || strcmp(ext, ".mpg") == 0) return "video/mpeg";
    if (strcmp(ext, ".mp3") == 0) return "audio/mpeg";
    return NULL;
}

//#================================
//#=====   permitionsCheck  =======
//#================================
int permitionsCheck(char *path)
{
    if(strcmp(path,"") == 0) //if root
        return PERMITIONS;

    char *token;
    char *str;
    char checkCurrect[strlen(path)];
    struct stat info;
    str = strdup(path);

    stat(str, &info);

    if(!(info.st_mode & S_IFDIR)) //is file , will check permitions
        if(!(info.st_mode & S_IRUSR) || !(info.st_mode & S_IRGRP) || !(info.st_mode & S_IROTH))
        {
            free(str);
            str = NULL;
            return AINT_PERMITIONS;
        }

    token = strtok(str, "/");
    if(token == NULL)
    {
        free(str);
        str = NULL;
        return PERMITIONS;
    }
    strcpy(checkCurrect,token);

    while( token != NULL )
    {
        stat(checkCurrect, &info);
        if(info.st_mode & S_IFDIR)
            if (!(info.st_mode & S_IXUSR) || !(info.st_mode & S_IXGRP) || !(info.st_mode & S_IXOTH)) {
                free(str);
                str = NULL;
                return AINT_PERMITIONS;
            }

        else if(!(info.st_mode & S_IRUSR) || !(info.st_mode & S_IRGRP) || !(info.st_mode & S_IROTH)) \
            {
            free(str);
            str = NULL;
            return AINT_PERMITIONS;
        }
        token = strtok(NULL, "/");
        if(token != NULL)
        {
            strcat(checkCurrect,"/");
            strcat(checkCurrect,token);
        }
    }
    free(str);
    str = NULL;
    return PERMITIONS;
}
