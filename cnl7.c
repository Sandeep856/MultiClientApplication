#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<sys/types.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<string.h>
#include<semaphore.h>
#include<pthread.h>
#include<stdbool.h>
#include<time.h>

#define PORT 8080

sem_t wrt;
sem_t rc;
sem_t cl;
int readcount=0;
struct sockaddr_in clientaddr[100];
int newsock[100];
int sock=0;
char* client_details[100];
int no_of_cl=0;
bool exit_status=false;
void* handle_client_read(void* param)
{
	while(1){
		
		char buffer[1024]={ 0 };
		int vr=recv(sock,buffer,1024,0);	   		
		int j=0;
		
		if(buffer[0]=='M' && buffer[1]=='E' && buffer[2]=='S' && buffer[3]=='G')
		{
			char user[1024]={ 0 };
			
			int i=0;
			int ind=5;
			while(1)
			{
				if(buffer[ind]==' ')break;
				user[i]=buffer[ind];
				i+=1;
				ind+=1;
			}
			
			ind+=1;
			i=0;
			
			char data[1024]={ 0 };
			
			while(1)
			{
				if(buffer[ind]=='\0')break;
				data[i]=buffer[ind];
				i+=1;
				ind+=1;
			}
			time_t t;  
    			time(&t);

			printf("%s %s : %s\n",ctime(&t),user,data);
		}
		
		else if(buffer[0]=='E' && buffer[1]=='X' && buffer[2]=='I' && buffer[3]=='T')
		{
			printf("Client Exiting...\n");
			break;
		}
		else
		{
			printf("%s\n",buffer);
		}
		
		
	}
	pthread_exit(0);
	    	   
}
void* handle_client_send(void* param)
{
	 while(1){
	    	   	sleep(1);
	    	   	printf("Enter your next message: ");
	    	   	char command[1024]={ 0 };
	    	   	gets(command);
	    	   	
	    	   	if(strcmp(command,"EXIT")==0)
	    	   	{
	    	   		send(sock,"EXIT",strlen("EXIT"),0);
	    	   		//printf("Client Exiting...\n");
	    	   		break;
	    	   	}
	    	   	else
	    	   	{
	    	   		send(sock,command,strlen(command),0);
	    	   	}
	    	   	
	    	   }  
	
	
	pthread_exit(0);
	
}
void* runner(void* param)
{
	int* res=(int*)param;
	int i=res[0];
	free(res);
        printf("Server accepted the Client %d | %s:%d!\n\n", i+1, inet_ntoa(clientaddr[i].sin_addr), ntohs(clientaddr[i].sin_port));
        
        
        char buffer[1024]={ 0 };
	int vr=recv(newsock[i],buffer,1024,0);
	printf("%s\n",buffer);
	
        char single_usr[1024]={ 0 };
        int a=5;
        int k=0;
        
        while(1)
        {
        	if(buffer[a]=='\0')break;
        	single_usr[k]=buffer[a];
        	k+=1;
        	a+=1;
        }
        
        client_details[i]=single_usr;
       
	while(1){
	         char mesg[1024]={ 0 };
	         int vr1=recv(newsock[i],mesg,1024,0);
		    if(mesg[0]=='M' && mesg[1]=='E' && mesg[2]=='S' && mesg[3]=='G')
			{
				a=5;
				k=0;
				
				char chkuser[1024]={ 0 };
				while(1)
				{
					if(mesg[a]==' ')break;
					chkuser[k]=mesg[a];
					k+=1;
					a+=1;
				}
			        
			        char data[1024]={ 0 };
			        
			        a+=1;
			        k=0;
			        while(1)
			        {
			        	if(mesg[a]=='\0')break;
			        	
			        	data[k]=mesg[a];
			        	a+=1;
			        	k+=1;
			        }
			        bool check=false;
			        
			        int y=0;
			        
				for(y=0;y<no_of_cl;y++)
				{
					if(strcmp(client_details[y],chkuser)==0)
					{
						char talk[1024]={"MESG "};
						strcat(talk,single_usr);
						strcat(talk," ");
						strcat(talk,data);
						strcat(talk,"\n");
						send(newsock[y],talk,strlen(talk),0);
						check=true;
						break;
					}
				}
				if(!check)
				{
					send(newsock[i],"Error 401 User not present",strlen("Error 401 User not present"),0);
					break;
				}
				char filecontent[1024]={ 0 };
				
				strcat(filecontent,single_usr);
				strcat(filecontent," : ");
				strcat(filecontent,data);
				strcat(filecontent,"\n");
				
				
				char dir[1024]="./server/clientid1_clientid2.txt";
				
				sem_wait(&wrt);
					FILE* fd;
					fd=fopen(dir,"a");
					if(fd==NULL)
					{
						printf("FILE NOT FOUND\n");
					}
					else
					{
						fprintf(fd,"%s",filecontent);
					}
					fclose(fd);
				sem_post(&wrt);
				
				
				
				
				printf("MESSAGE SENT TO %s\n",chkuser);
			
				
			}
			
			
			if(mesg[0]=='E' && mesg[1]=='X' && mesg[2]=='I' && mesg[3]=='T')
			{
				int y=0;
				for(y=0;y<no_of_cl;y++)
				{
					
					char talk[1024]={"EXIT"};
					send(newsock[y],talk,strlen(talk),0);
					
					
				}
				
				break;
			}
			
			
	}	
	
	
	pthread_exit(0);	
}
int main(int argc,char* argv[])
{
	
	const char* ip_a=argv[1];
	int pn=atoi(argv[2]);
	const char* mrn=argv[3];
	if(strcmp(mrn,"s")==0)
	{
		struct sockaddr_in sa;
		int sockid;
		int addrlen=sizeof(sa);
		
		sockid=socket(AF_INET,SOCK_STREAM,0);
		sa.sin_family=AF_INET;
		sa.sin_port=htons(pn);
		sa.sin_addr.s_addr=inet_addr(ip_a);
		int n=3;
		if(bind(sockid,(struct 	sockaddr*)&sa,sizeof(sa))<0)
		{
			perror("bind failed\n");
			exit(EXIT_FAILURE);
		}
		pthread_t tid[100];
	        pthread_attr_t attr;
		sem_init(&wrt,0,1);
		sem_init(&rc,0,1);
		if(listen(sockid,0)<0)
			{
				perror("listen failed");
				exit(EXIT_FAILURE);
			}
	        pthread_attr_init(&attr);
	        int i=0;
			 while(1){
				
				
					if((newsock[i]=accept(sockid,(struct sockaddr*)&clientaddr[i],(socklen_t*)&addrlen))<0)
					{
						perror("accept failed\n");
						exit(EXIT_FAILURE);
					}
					no_of_cl+=1;
					int* res=(int*)malloc(sizeof(int));
					res[0]=i;
					
					pthread_create(&tid[i],&attr,runner,(void*)res);
			                i+=1;
				
			       }
		sem_destroy(&wrt);
		sem_destroy(&rc);
		
	}
	if(strcmp(mrn,"c")==0)
	{
		   int client_s;
		   struct sockaddr_in serv_addr,client_addr;
		 
		    
		   if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
			printf("\n Socket creation error \n");
			return -1;
		  }
		 
		   serv_addr.sin_family = AF_INET;
		   serv_addr.sin_port = htons(pn);
		   serv_addr.sin_addr.s_addr=inet_addr(ip_a);
		    
		   char usr[1024]={ 0 };
		   printf("Enter your username: ");
		   gets(usr);
		   if ((client_s
			 = connect(sock, (struct sockaddr*)&serv_addr,
				   sizeof(serv_addr)))
			< 0) {
			printf("\nConnection Failed \n");
			return -1;
		   }
	           socklen_t clientaddr_len = sizeof(client_addr);
	           memset(&client_addr, 0, clientaddr_len);
	           if(getsockname(sock, (struct sockaddr *)&client_addr, &clientaddr_len))
	           perror("getsockname error");
	           printf("%s:%d is connected to ", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
	           printf("%s:%d\n", inet_ntoa(serv_addr.sin_addr), ntohs(serv_addr.sin_port));
	  
	    	   
	    	   int n=2;
	    	   
	    	   
	    	   
	    	   pthread_t cltid_read;
	    	   pthread_attr_t attr;
	    	   pthread_attr_init(&attr);
	    	   pthread_create(&cltid_read,&attr,handle_client_read,NULL);
	    	   
	    	   pthread_t cltid_send;
	    	   pthread_attr_init(&attr);
	    	   pthread_create(&cltid_send,&attr,handle_client_send,NULL);
	    	   
	    	   char greetings[1024]={"HELO "};
	    	   strcat(greetings,usr);
	    	   send(sock,greetings,strlen(greetings),0);
	    	   printf("200 OK\n");
	    	   
	    	   
	    	   
	    	   pthread_join(cltid_read,NULL);
                   
	    	  
	    	   
		   
		   close(client_s);
        }
	return 0;	
}

/*
#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<sys/types.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<string.h>
#include<semaphore.h>
#include<pthread.h>
#include<stdbool.h>

#define PORT 8080

sem_t wrt;
sem_t rc;
sem_t cl;
int readcount=0;
struct sockaddr_in clientaddr[100];
int newsock[100];
int sock=0;
char* details[100];
int no_of_cl=0;
char userd[1024]={ 0 };
void* handle(void* param)
{
	int* res=(int*)param;
	free(res);
	while(1){
		
		char buffer[1024]={ 0 };
		int vr=recv(sock,buffer,1024,0);
		    //send	   		
		int j=0;
		if(buffer[0]=='C' && buffer[1]=='U' && buffer[2]=='R' && buffer[3]=='R'){
			 j=5;
	   		while(buffer[j]!='\0')
	   		{
				char usl[1024]={ 0 };
				
				int m=0;
				while(1)
				{
					if(buffer[j]=='\n')break;
					usl[m]=buffer[j];
					m+=1;
					j+=1;
				}
				j+=1;
				if(strcmp(usl,userd)!=0)
				{
					printf("%s\n",usl);
				}
	   		}
		}
		else if(buffer[0]=='T' && buffer[1]=='A' && buffer[2]=='L' && buffer[3]=='K')
		{
			char data[1024]={ 0 };
			
			strcpy(data,buffer+5);
			
			printf("%s\n",data);	
		}
		else
		{
			printf("%s\n",buffer);
		}
		
		
	}
	
}
void* runner(void* param)
{
	int* res=(int*)param;
	int i=res[0];
	free(res);
        printf("Server accepted the Client %d | %s:%d!\n\n", i+1, inet_ntoa(clientaddr[i].sin_addr), ntohs(clientaddr[i].sin_port));
        char buffer[1024]={ 0 };
	int vr=recv(newsock[i],buffer,1024,0);
	printf("%s from client %d\n",buffer,i+1);
	send(newsock[i],"MESG What is your username?",strlen("MESG What is your username?"),0);
        char usr[1024]={ 0 };
        char mesg[1024]={ 0 };
	int vr1=recv(newsock[i],mesg,1024,0);
	
        if(mesg[0]=='M' && mesg[1]=='E' && mesg[2]=='S' && mesg[3]=='G')
		{
			int a=5;
			int k=0;
			
			while(1)
			{
				if(mesg[a]=='\0')break;
				usr[k]=mesg[a];
				a+=1;
				k+=1;
			}
		
			strcat(usr,"@");
			strcat(usr,inet_ntoa(clientaddr[i].sin_addr));
			strcat(usr,":");
			char port[1024]={ 0 };
			sprintf(port,"%d",ntohs(clientaddr[i].sin_port));
			strcat(usr,port);
			details[i]=usr;
	                strcat(usr,"\n");
			
			
			printf("Connected Client details are: %s\n",usr);
			
			char dir[1024]="./server/user.txt";
			
			sem_wait(&wrt);
				FILE* fd;
				fd=fopen(dir,"a");
				if(fd==NULL)
				{
					printf("FILE NOT FOUND\n");
				}
				if(fd==NULL)
				{
					printf("FILE NOT FOUND");
				}
				else
				{
					fprintf(fd,"%s",usr);
				}
				fclose(fd);
			sem_post(&wrt);
		}	
	while(1){
		
		bzero(buffer,1024);
		
		bzero(mesg,1024);
		int vr1=recv(newsock[i],mesg,1024,0);
		
		if(mesg[0]=='M' && mesg[1]=='E' && mesg[2]=='S' && mesg[3]=='G')
		{
			int a=5;
			char chkuser[1024]={ 0 };
			int k=0;
			
			while(1)
			{
				if(mesg[a]=='\0')break;
				chkuser[k]=mesg[a];
				k+=1;
				a+=1;
			}
			int y=0;
			 
			for(y=0;y<no_of_cl;y++)
			{
				if(strcmp(details[y],chkuser)==0)
				{
					char talk[1024]={"TALK "};
					//strcat(talk,chkuser);
					strcat(talk,"HELLO");
					send(newsock[y],talk,strlen(talk),0);
					break;
				}
			}
			
			
			sem_wait(&rc);
			readcount+=1;
			if(readcount==1)
			sem_wait(&wrt);
			sem_post(&rc);
			
			FILE* fd;
			char dir[1024]="./server/user.txt";
			fd=fopen(dir,"r");
			if(fd==NULL)
			{
				printf("Unable to open the file:Error\n");
			}
				
			char line[100];
			int flag=0;
			while(fgets(line,sizeof(line),fd))
			{
				if(strcmp(line,chkuser)==0)
				{
					send(newsock[i],"201 Available",strlen("201 Available"),0);
					flag=1;
					break;
				}
				else continue;
			}
			
			if(flag==0)
			{
				send(newsock[i],"404 not found",strlen("404 not found"),0);
			}
			
			fclose(fd);
			
			sem_wait(&rc);
			readcount-=1;
			if(readcount==0)sem_post(&wrt);
			sem_post(&rc);
			
		}
		if(mesg[0]=='E' && mesg[1]=='X' && mesg[2]=='I' && mesg[3]=='T')
		{
		    
		    char dir[1024]="./server/user.txt";
			
		    char line[1024];	
		    FILE *fd,*temp;
		    
		    fd=fopen(dir,"r");
		    sem_wait(&wrt);
		    if(fd==NULL)
		    {
		    	printf("Unable to open the user.txt file:Error\n");
		    }
		    
		    temp=fopen("temp.txt","w");
		    
		    if(temp==NULL)
		    {
		    	printf("Unable to open the temp.txt file:Error\n");
		    }
		    
		    while(fgets(line,sizeof(line),fd))
		    {
		    	if(strcmp(line,usr)!=0)
		    	{
		    		fputs(line,temp);
		    	}
		    }
		    
		    fclose(fd);
		    
		    fclose(temp);
		    
		    remove(dir);
		    
		    rename("temp.txt",dir);
		    
		    printf("User details deleted Successfully\n");
		    
		    sem_post(&wrt);
   
		}
		
		if(mesg[0]=='L' && mesg[1]=='I' && mesg[2]=='S' && mesg[3]=='T')
		{
			FILE* fd;
			
			char dir[1024]="./server/user.txt";
			
			fd=fopen(dir,"r");
			
			char users[1024]={ 0 };
			int j=0;
			fseek(fd,0,SEEK_END);
			int res=ftell(fd);
			fseek(fd,0,SEEK_SET);
			while(1)
			{
				users[j]=fgetc(fd);
				j+=1;
				if(j==res)break;
			}
			char data[1024]={"CURR "};
			strcat(data,users);
			send(newsock[i],data,strlen(data),0);
			fclose(fd);
		}
		
	}
	
	pthread_exit(0);	
}
int main(int argc,char* argv[])
{
	
	const char* ip_a=argv[1];
	int pn=atoi(argv[2]);
	const char* mrn=argv[3];
	if(strcmp(mrn,"s")==0)
	{
		struct sockaddr_in sa;
		int sockid;
		int addrlen=sizeof(sa);
		
		sockid=socket(AF_INET,SOCK_STREAM,0);
		sa.sin_family=AF_INET;
		sa.sin_port=htons(pn);
		sa.sin_addr.s_addr=inet_addr(ip_a);
		int n=3;
		if(bind(sockid,(struct 	sockaddr*)&sa,sizeof(sa))<0)
		{
			perror("bind failed\n");
			exit(EXIT_FAILURE);
		}
		pthread_t tid[100];
	        pthread_attr_t attr;
		sem_init(&wrt,0,1);
		sem_init(&rc,0,1);
		if(listen(sockid,0)<0)
			{
				perror("listen failed");
				exit(EXIT_FAILURE);
			}
	        pthread_attr_init(&attr);
	        int i=0;
			 while(1){
				
				
					if((newsock[i]=accept(sockid,(struct sockaddr*)&clientaddr[i],(socklen_t*)&addrlen))<0)
					{
						perror("accept failed\n");
						exit(EXIT_FAILURE);
					}
					no_of_cl+=1;
					int* res=(int*)malloc(sizeof(int));
					res[0]=i;
					
					pthread_create(&tid[i],&attr,runner,(void*)res);
			                i+=1;
				
			       }
		sem_destroy(&wrt);
		sem_destroy(&rc);
		
	}
	if(strcmp(mrn,"c")==0)
	{
		   int client_s;
		   struct sockaddr_in serv_addr,client_addr;
		 
		    
		   if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
			printf("\n Socket creation error \n");
			return -1;
		  }
		 
		   serv_addr.sin_family = AF_INET;
		   serv_addr.sin_port = htons(pn);
		   serv_addr.sin_addr.s_addr=inet_addr(ip_a);
		    
		 
		   if ((client_s
			 = connect(sock, (struct sockaddr*)&serv_addr,
				   sizeof(serv_addr)))
			< 0) {
			printf("\nConnection Failed \n");
			return -1;
		   }
	           socklen_t clientaddr_len = sizeof(client_addr);
	           memset(&client_addr, 0, clientaddr_len);
	           if(getsockname(sock, (struct sockaddr *)&client_addr, &clientaddr_len))
	           perror("getsockname error");
	           printf("%s:%d is connected to ", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
	           printf("%s:%d\n", inet_ntoa(serv_addr.sin_addr), ntohs(serv_addr.sin_port));
	  
	    	   
	    	   int n=2;
	    	   
	    	   
	    	   int*res=malloc(sizeof(int));
	    	   res[0]=1;
	    	   pthread_t tid;
	    	   pthread_attr_t attr;
	    	   pthread_attr_init(&attr);
	    	   pthread_create(&tid,&attr,handle,(void*)res);
	    	   
	    	   
	    	   send(sock,"HELO",strlen("HELO"),0);
    	   	   char buffer[1024]={ 0 };
    	   	   int vr=recv(sock,buffer,1024,0);
    	       	   printf("Enter your username: ");
    	   	   char usr[1024]={ 0 };
    	   	   char data[1024]={"MESG "};
    	   	   gets(usr);
    	   	   strcat(data,usr);
    	   	   send(sock,data,strlen(data),0);
	    	   sem_init(&cl,0,1);
	    	   
	    	   strcat(userd,usr);
	    	   strcat(userd,"@");
	    	   strcat(userd,inet_ntoa(client_addr.sin_addr));
	    	   strcat(userd,":");
	    	   char port[100];
	    	   sprintf(port,"%d",ntohs(client_addr.sin_port));
	    	   strcat(userd,port);
	    	   printf("%s\n",userd);
	    	   while(1){
	    	   //flag check recv
	    	   	sleep(1);
	    	   	printf("Enter your next message: ");
	    	   	char command[1024]={ 0 };
	    	   	gets(command);
	    	   	
	    	   	if(strcmp(command,"EXIT")==0)
	    	   	{
	    	   		send(sock,"EXIT",strlen("EXIT"),0);
	    	   		printf("Client Exiting...\n");
	    	   		break;
	    	   	}
	    	   	
	    	   	else if(strcmp(command,"LIST")==0)
	    	   	{
	    	   		char users[1024]={ 0 };
	    	   		
	    	   		//Receive the list of clients that are active currently now.
	    	   		send(sock,"LIST",strlen("LIST"),0);
	    	   		
	    	   		
	    	   	}
	    	   	else
	    	   	{
	    	   		char data[1024]={"MESG "};
	    	   		strcat(data,command);
	    	   		strcat(data,"\n");
	    	   		send(sock,data,strlen(data),0);
	    
	    	   		//bzero(data,1024);
	    	   	}
	    	   	
	    	   }   
		   //pthread_join(tid,NULL);
		   close(client_s);
        }
	return 0;	
}

*/