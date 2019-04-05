#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <pthread.h>


#define	QLEN			5
#define	BUFSIZE			4096
#define MAXGROUPSIZE            32

struct clientInfo {
       int status;
       int socket;
       char name[100];
       int score;
       int answered;
};
struct questionInfo{
    char question[2048];
    char answer[1024];
    int qsize;
}; 
struct groupInfo {
       int status;
       int  adminSocket;
       char quizTopic[100];
       char groupName[100];
       struct clientInfo clients[1100];
       int  maxSize;
       int  currSize;
       struct questionInfo Quiz[128];
       int Q_num;
};

struct groupInfo GROUPS[32];

pthread_t trd[32];

fd_set		afds;
int			nfds;
int			msock;

void GetOpenGroups(int ssock);
int CreateGroup(int ssock, char *buf);
int JoinGroup(int ssock, char *buf);
void EndGroup(int index);
void *groupQuiz(void *var);
void *QUIZ(void *var);

int passivesock( char *service, char *protocol, int qlen, int *rport );

/*
**	The server ... 
*/
int main( int argc, char *argv[] )
{
	char			buf[BUFSIZE];
	char			*service;
	struct sockaddr_in	fsin;
	int			ssock;
	fd_set		rfds;
	int			alen;
	int			fd;
	
	int			rport = 0;
	int			cc;
	  
	switch (argc) 
	{
		case	1:
			rport = 1;
			break;
		case	2:
			service = argv[1];
			break;
		default:
			fprintf( stderr, "usage: server [port]\n" );
			exit(-1);
	}

	msock = passivesock( service, "tcp", QLEN, &rport );
	if (rport)
	{
		printf( "server: port %d\n", rport );
		fflush( stdout );
	}
    
        for(int i=0;i<32;i++)
           GROUPS[i].status=0;

	nfds = msock+1;
        
	FD_ZERO(&afds);
	FD_SET( msock, &afds );
	for (;;)
	{
		memcpy((char *)&rfds, (char *)&afds, sizeof(rfds));
		if (select(nfds, &rfds, (fd_set *)0, (fd_set *)0,
				(struct timeval *)0) < 0)
		{
			fprintf( stderr, "server select: %s\n", strerror(errno) );
			exit(-1);
		}

		if (FD_ISSET( msock, &rfds)) 
		{
			int	ssock;
			alen = sizeof(fsin);
			ssock = accept( msock, (struct sockaddr *)&fsin, &alen );
			if (ssock < 0)
			{
				fprintf( stderr, "accept: %s\n", strerror(errno) );
				exit(-1);
			}
			FD_SET( ssock, &afds );
            GetOpenGroups(ssock); //OPENGROUPS
			if ( ssock+1 > nfds )
				nfds = ssock+1;
		}
		for ( fd = 0; fd < nfds; fd++ )
		{
			if (fd != msock && FD_ISSET(fd, &rfds))
			{
				if ( (cc = read( fd, buf, BUFSIZE )) <= 0 )
				{
					printf( "The client has gone.\n" );
                    fflush(stdout);
					(void) close(fd);
					FD_CLR( fd, &afds );
					if ( nfds == fd+1 )
						nfds--;
				}
				else
				{
					buf[cc] = '\0';
					printf( "The client says: %s\n", buf );
                    fflush(stdout);
                    //////////CASE 1: CREATE GROUP//////////////
                    if(buf[0]=='G' && buf[1]=='R' && buf[2]=='O' && buf[3]=='U' && buf[4]=='P')
                    {
                        int asd = CreateGroup(fd, buf);
                        printf("%d\n",asd); //TEMP
                        if( asd == 0)
                         write( fd, "BAD\r\n", 4);
                         else write(fd,"SENDQUIZ\r\n",10);
                    }
                    /////////CASE 2: GET OPEN GROUPS///////////
                    else if(buf[0]=='G' && buf[1]=='E' && buf[2]=='T' && buf[3]=='O' && buf[4]=='P')
                       GetOpenGroups(fd); 
                       
                    /////////CASE 3: JOIN TO GROUP////////////
                    else if(buf[0]=='J' && buf[1]=='O' && buf[2]=='I' && buf[3]=='N')
                    {
                        int temp = JoinGroup(fd,buf);
                        if(temp==-1) write( fd, "BAD\r\n", 5);
                            else if(temp==0) write( fd, "NOGROUP\r\n", 9);
                            else if(temp==1) write( fd, "FULL\r\n", 6);
                            else if(temp==2) write( fd, "OK\r\n", 4);
                    } 
                    else {write( fd, "BAD\r\n", 4);} 
				}
			}

		}
	}
}


void GetOpenGroups(int ssock){
    char buf[BUFSIZE];
    strcpy(buf,"OPENGROUPS");
                        for(int i=0;i<32;i++)
                           {    
                            if(GROUPS[i].status==1)
                            {
                               char msg[500];
                               memset(msg,0,500);
                              sprintf(msg, "|%s|%s|%d|%d",GROUPS[i].quizTopic,GROUPS[i].groupName,GROUPS[i].maxSize,GROUPS[i].currSize); 
                              strcat(buf,msg);
                            }
                           }
                        strcat(buf,"\r\n");
                        write(ssock,buf,strlen(buf));
}


int CreateGroup(int ssock, char *buf){
    char         *argmnts[BUFSIZE];
    int          n=0;
    int          g_num;
    for(int i=0;i<32;i++)
        if(GROUPS[i].status==0)
            {
                g_num=i;
                break;
            }
    argmnts[n] = strtok(buf,"|");
                 while(argmnts[n]!=NULL)
                     {   n++;
                         argmnts[n]=strtok(NULL,"|"); 
                     }
   if(n!=4)
       return 0;
   else{
         GROUPS[g_num].status=0;
         GROUPS[g_num].adminSocket = ssock;
         strcpy(GROUPS[g_num].quizTopic,argmnts[1]);
         strcpy(GROUPS[g_num].groupName,argmnts[2]);
         if((GROUPS[g_num].maxSize=atoi(argmnts[3])) == 0)
                 return 0;
         GROUPS[g_num].currSize=0;
         for(int i=0;i<1100;i++)
             GROUPS[g_num].clients[i].status = 0;
             FD_CLR( ssock, &afds );
					if ( nfds == ssock+1 )
						nfds--;
             pthread_create(&trd[g_num],NULL,groupQuiz,(void*)(long)g_num);
       }
    return 1;
}


int JoinGroup(int ssock, char *buf){
    char         *argmnts[BUFSIZE];
    int          n=0;
    int          g_num=-1;

    argmnts[n] = strtok(buf,"|");
                 while(argmnts[n]!=NULL)
                     {   n++;
                         argmnts[n]=strtok(NULL,"|"); 
                     }
   if(n!=3)
       return -1;
   else{
        for(int i=0;i<32;i++)
         if(GROUPS[i].status==1 && strcmp(GROUPS[i].groupName, argmnts[1])==0)
            {
                g_num=i;
                break;
            }
         if(g_num==-1) return 0;
         if(GROUPS[g_num].maxSize==GROUPS[g_num].currSize) return 1;
         
         for(int i=0;i < 1100;i++)
             if(GROUPS[g_num].clients[i].status == 0)
             {
                 GROUPS[g_num].clients[i].status = 1;
                 GROUPS[g_num].clients[i].socket = ssock;
                 memset(GROUPS[g_num].clients[i].name,0,100);
                 strcpy(GROUPS[g_num].clients[i].name,argmnts[2]);
                 GROUPS[g_num].clients[i].name[strlen(argmnts[2])-1]='\0';
                 GROUPS[g_num].clients[i].name[strlen(argmnts[2])-2]='\0';
                 GROUPS[g_num].clients[i].score=0;
                 GROUPS[g_num].clients[i].answered=0;
                 GROUPS[g_num].currSize++;
                 FD_CLR( ssock, &afds );
					if ( nfds == ssock+1 )
						nfds--;
                break;
             }
       }
    if(GROUPS[g_num].maxSize==GROUPS[g_num].currSize){
        GROUPS[g_num].status=1;
        pthread_cancel(trd[g_num]);
        pthread_create(&trd[g_num],NULL,QUIZ,(void*)(long)g_num);
                   printf("QUIZ started!\n");
                   fflush(stdout);
                    FD_SET(GROUPS[g_num].adminSocket,&afds);
                    if ( GROUPS[g_num].adminSocket+1 > nfds )
				      nfds = GROUPS[g_num].adminSocket+1;
    }
    return 2;
}


void EndGroup(int index){
    for(int i=0;i<1100;i++)
              if(GROUPS[index].clients[i].status==1)
              {
                  FD_SET(GROUPS[index].clients[i].socket,&afds);
                  GROUPS[index].clients[i].status=0;
              }
                  GROUPS[index].status=0;
                  
    char msg[120];
    strcpy(msg,"ENDGROUP|");
    strcat(msg,GROUPS[index].groupName);
    strcat(msg,"\r\n");
    for(int fd; fd<nfds;fd++)
        if (fd != msock && FD_ISSET(fd, &afds))
            write(fd,msg,strlen(msg));
}


void *groupQuiz(void *var){
const int x = (long) var;
char         buf[BUFSIZE];
int          cc;
//////////QUIZ_TEXT_///////////
char fromAdmin[BUFSIZE*64];
char question[BUFSIZE];
char answer[BUFSIZE];
int quizsize;
/////////////////////////////
//READING QUIZ///
while(1){
                    char temp[2];
                    read(GROUPS[x].adminSocket,temp,5);
                    memset(buf,0,BUFSIZE);
                    read(GROUPS[x].adminSocket,temp,1);
                    temp[1]='\0';
                    strcpy(buf,temp);
                    while(1){
                        read(GROUPS[x].adminSocket,temp,1);
                        temp[1]='\0';
                        if(temp[0]=='|')
                            break;
                        strcat(buf,temp);
                    }
                    quizsize = atoi(buf);
               read(GROUPS[x].adminSocket,fromAdmin,quizsize);
               write(GROUPS[x].adminSocket,"OK\r\n",4);
               printf("Quiz was received(%d).\n",quizsize);
               fflush(stdout);
               break;
}

printf("%s",fromAdmin);
fflush(stdout);

          //////READING QUESTION AND ANSWER////////
          int oldoffset = 0;
          int endoffset = quizsize;
          int q_number = 0;
          while(oldoffset!=endoffset)/////QUIZ START//////
          {
           memset(question,0,BUFSIZE/2);
           memset(answer,0,BUFSIZE/2);

          for(int i=oldoffset;i<BUFSIZE;i++)
              if(fromAdmin[i]=='\n' && fromAdmin[i+1]=='\n')
              {
                  for(int j=oldoffset,k=0;j<i;j++,k++)
                      question[k]=fromAdmin[j];
                      oldoffset = i+2;
                      break;
              }
              
          for(int i=oldoffset;i<BUFSIZE;i++)
              if(fromAdmin[i]=='\n' && fromAdmin[i+1]=='\n')
              {
                  for(int k=0,j=oldoffset;j<i;j++,k++)
                     answer[k] = fromAdmin[j];
                     oldoffset = i+2;
                     break;
              }
           printf("[DONE] %d | %d\n",oldoffset,endoffset);
               fflush(stdout);
          ////////////////////////////////////////
          strcpy(GROUPS[x].Quiz[q_number].question,question);
          strcpy(GROUPS[x].Quiz[q_number].answer,answer);
          GROUPS[x].Quiz[q_number].qsize = strlen(question);
          q_number++;
          }    
          GROUPS[x].Q_num = q_number;
           for(int i=0;i<GROUPS[x].Q_num;i++)
             {  printf("%s %s\n",GROUPS[x].Quiz[i].question,GROUPS[x].Quiz[i].answer);
               fflush(stdout);
             } 
               printf("Q_NUM: %d\n",GROUPS[x].Q_num );
               fflush(stdout);
               
               GROUPS[x].status=1;
               printf("[DONE]\n");
               fflush(stdout);
/////////////////////////////////////////
fd_set		rfdsG;
fd_set		afdsG;
int			fd;
int			nfdsG;
////////////////////////////////////////LEAVE_WAIT/////////////////////////////////
while(1){ 
   FD_ZERO(&afdsG);
   FD_SET(GROUPS[x].adminSocket,&afdsG);
   nfdsG=GROUPS[x].adminSocket+1;
   for(int i=0;i<1100;i++)
              if(GROUPS[x].clients[i].status==1)
              {
                  FD_SET(GROUPS[x].clients[i].socket, &afdsG);
                  if ( GROUPS[x].clients[i].socket+1 > nfdsG )
                        nfdsG = GROUPS[x].clients[i].socket+1;
              }
              
if (select(nfdsG, &afdsG, (fd_set *)0, (fd_set *)0,(struct timeval *)0) < 0)
		{
			fprintf( stderr, "server select: %s\n", strerror(errno));
			exit(-1);
		}    
        for(int i=0;i<1100; i++)
            if (GROUPS[x].clients[i].status==1 && FD_ISSET(GROUPS[x].clients[i].socket, &afdsG))
			{
				if ( (cc = read( GROUPS[x].clients[i].socket, buf, BUFSIZE )) <= 0 )
				{
					printf( "The client %s has gone.\n",GROUPS[x].clients[i].name);
                    fflush(stdout);
					(void) close(GROUPS[x].clients[i].socket);
                    GROUPS[x].currSize--;
                    GROUPS[x].clients[i].status=0; //CLR
				}
                else
				{
                    buf[cc] = '\0';
                    printf( "The client %s says: %s\n", GROUPS[x].clients[i].name, buf );
                    fflush(stdout);
                    if(strcmp(buf,"LEAVE\r\n")==0)
                    {
                        write(GROUPS[x].clients[i].socket,"OK\r\n",4);
                        GROUPS[x].clients[i].status=0; //CLR
                        GROUPS[x].currSize--;
                        FD_SET(GROUPS[x].clients[i].socket,&afds);
                        printf( "The client %s has gone.\n",GROUPS[x].clients[i].name);
                        fflush(stdout);
                    }
                }
            }
            if(FD_ISSET(GROUPS[x].adminSocket,&afdsG))
            {
                cc = read(GROUPS[x].adminSocket,buf,BUFSIZE);
                buf[cc] = '\0';
                printf( "The Admin of %s says(%d): %s\n", GROUPS[x].groupName,cc, buf );
                fflush(stdout);
                if(buf[0]=='C' && buf[1]=='A' && buf[2]=='N')
                {
                    write(GROUPS[x].adminSocket,"OK\r\n",4);
                    FD_SET(GROUPS[x].adminSocket,&afds);
                    if ( GROUPS[x].adminSocket+1 > nfds )
				      nfds = GROUPS[x].adminSocket+1;
                      EndGroup(x);
                      break;
                }
                else write(GROUPS[x].adminSocket,"BAD\r\n",5);
            }
}
///////////////////////////////////////////////////////////////////////////////////
                  pthread_exit(NULL);
}


void *QUIZ(void *var){
const int x = (long) var;
char         buf[BUFSIZE];
int          cc;
for(int i=0;i<GROUPS[x].Q_num;i++)
{
///////////SENDING QUESTIONS//////////////////////
char msg[BUFSIZE];
char buffer[10];
snprintf(buffer,10,"%d",GROUPS[x].Quiz[i].qsize);
memset(msg,0,BUFSIZE);
strcpy(msg,"QUES|");
strcat(msg,buffer);
strcat(msg,"|");
strcat(msg,GROUPS[x].Quiz[i].question);

char ans[BUFSIZE];
memset(ans,0,BUFSIZE);
strcpy(ans,"ANS|");
strcat(ans,GROUPS[x].Quiz[i].answer);
strcat(ans,"\r\n");

          for(int i=0;i<1100;i++)
              if(GROUPS[x].clients[i].status==1)
              {
                  write(GROUPS[x].clients[i].socket, msg, strlen(msg));
                  GROUPS[x].clients[i].answered=0;
              }
//////////////////////////////////////////////////////////////////
/////////////////////////////RECEIVING ANSWERS///////////////////
////////////////////////////////////////////////////////////////
fd_set		rfdsG;
fd_set		afdsG;
int			fd;
int			nfdsG;
int first_answered=0;
int answered=0;
char Wname[100];
memset(Wname,0,100);
strcpy(Wname,"WIN|");
   
        
        while(answered!=GROUPS[x].currSize){
   FD_ZERO(&afdsG);
   FD_SET(GROUPS[x].adminSocket,&afdsG);
   nfdsG=0;
   for(int i=0;i<1100;i++)
              if(GROUPS[x].clients[i].status==1)
              {
                  FD_SET(GROUPS[x].clients[i].socket, &afdsG);
                  if ( GROUPS[x].clients[i].socket+1 > nfdsG )
                        nfdsG = GROUPS[x].clients[i].socket+1;
              }
if (select(nfdsG, &afdsG, (fd_set *)0, (fd_set *)0,(struct timeval *)0) < 0)
		{
			fprintf( stderr, "server select: %s\n", strerror(errno));
			exit(-1);
		}
            for(int i=0;i<1100; i++)
            if (GROUPS[x].clients[i].status==1 && FD_ISSET(GROUPS[x].clients[i].socket, &afdsG) && GROUPS[x].clients[i].answered==0)
			{
				if ( (cc = read( GROUPS[x].clients[i].socket, buf, BUFSIZE )) <= 0 )
				{
					printf( "The client %s has gone.\n",GROUPS[x].clients[i].name);
                    fflush(stdout);
					(void) close(GROUPS[x].clients[i].socket);
                    GROUPS[x].currSize--;
                    GROUPS[x].clients[i].status=0; //CLR
				}
                else
				{
                    buf[cc] = '\0';
                    printf( "The client %s says: %s\n", GROUPS[x].clients[i].name, buf );
                    fflush(stdout);
                    if(strcmp(buf,"LEAVE\r\n")==0)
                    {
                        write(GROUPS[x].clients[i].socket,"OK\r\n",4);
                        GROUPS[x].clients[i].status=0; //CLR
                        GROUPS[x].currSize--;
                        FD_SET(GROUPS[x].clients[i].socket,&afds);
                        printf( "The client %s has gone.\n",GROUPS[x].clients[i].name);
                        fflush(stdout);
                    } else if(strcmp(ans,buf)==0){
                           if(first_answered==0)
                           {
                               GROUPS[x].clients[i].score+=2;
                               strcat(Wname,GROUPS[x].clients[i].name);
                               first_answered=1;
                           }
                           else GROUPS[x].clients[i].score++;
                    }
                     else if(strcmp("NOANS\r\n",buf)==0){}
                     else GROUPS[x].clients[i].score--;
                     GROUPS[x].clients[i].answered=1;
                }
                answered++;
            }
        }
        printf("%s|\n",Wname);
        fflush(stdout);
         for(int i=0;i<1100;i++)
              if(GROUPS[x].clients[i].status==1)
                  write(GROUPS[x].clients[i].socket, Wname, strlen(Wname));
}
       char result[2048];
       memset(result,0,2048);
       strcpy(result,"RESULT");
       for(int i=0;i<1100;i++)
           if(GROUPS[x].clients[i].status==1)
           {
               strcat(result,"|");
               strcat(result,GROUPS[x].clients[i].name);
               strcat(result,"|");
               char buffer[10];
               snprintf(buffer,10,"%d",GROUPS[x].clients[i].score);
               strcat(result,buffer);
           }
        for(int i=0;i<1100;i++)
            if(GROUPS[x].clients[i].status==1)
                write(GROUPS[x].clients[i].socket,result,strlen(result));
                
     pthread_exit(NULL);
}