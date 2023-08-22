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
    #include<stdint.h>
    #define PORT 8080
     
    uint8_t seq=0x0000;
    uint8_t src_addr[1024];
    uint8_t dest_adrr=0x80;
    uint8_t flag=0x00;
    uint8_t srcaddr_i=0x00;
    uint8_t ind_src;
     
    struct packet{
    	uint8_t seq_number;
    	uint8_t src_number;
    	uint8_t type;
    	char data[1024];
    	uint8_t r;
        uint8_t flag;
        uint8_t checksum;
        uint8_t dest;
        int len;
    };
     
     

     
     
    sem_t wrt;
    sem_t rc;
    sem_t cl;
    int readcount=0;
    char chat[1024]={"NULL"};
    struct sockaddr_in clientaddr[100];
    int newsock[100];
    int sock=0;
    char* client_details[100];
     
    char* client_username[100];
     
    int no_of_cl=0;
    bool exit_status=false;
    char usr[1024]={ 0 };
    bool av=true;
    uint8_t cal_checksum(struct packet pack)
    {
        uint8_t final_check_sum=0;
        uint8_t data=0;
        for(int i=0;i<strlen(pack.data);i++)
        {
            data+=(uint8_t)pack.data[i];
        }
        final_check_sum=pack.flag+pack.len+pack.seq_number+pack.src_number+pack.type+data;
        final_check_sum=0xff-final_check_sum;

        return final_check_sum;
    }
    uint8_t check_sum_valid(struct packet pack)
     {
        uint8_t init_checksum;
        init_checksum=pack.checksum;
        uint8_t final_check_sum=0;
        uint8_t data=0;
        for(int i=0;i<strlen(pack.data);i++)
        {
            data+=(uint8_t)pack.data[i];
        }
        final_check_sum=pack.flag+pack.len+pack.seq_number+pack.src_number+pack.type+data;
        final_check_sum=0xff-final_check_sum;
        if(final_check_sum==init_checksum)return 1;
        else return 0;
     }
     
    void funcS(struct packet pack)
    {
    	uint8_t pack_src=pack.src_number;
    	uint8_t type=pack.type;
    	uint8_t pack_seq=pack.seq_number;
    	int len=strlen(pack.data);
    	len=len+7;
    	if(check_sum_valid(pack))pack.flag=pack.flag & (0b11111110);
        else pack.flag=pack.flag & (0b11111111);

    	printf("//////////////////////\n");
    	printf("// PACKET RECEIVED  //\n");
    	printf("// SRC		%x\n",pack_src);
    	printf("// DEST 	0x80\n");
    	printf("// LEN 		%x\n",len);
    	printf("// Flag		0x%x\n",pack.flag);
    	printf("// TYPE 	%x\n",type);
    	printf("// SEQ 		%x\n",pack_seq);
    	printf("// CHKSUM 	%x\n",pack.checksum);
    	printf("// Unused	0x00\n");
    	printf("// Data		%s\n",pack.data);
    	printf("//////////////////////\n");
    	return;
    }
     
    void funcC(struct packet pack)
    {
    	int type=pack.type;
    	uint8_t pack_seq=pack.seq_number;
    	int pack_src=pack.src_number;
        int len=strlen(pack.data);
    	len=len+7;
    	if(check_sum_valid(pack))pack.flag=pack.flag & (0b11111110);
        else pack.flag=pack.flag & (0b11111111);

    	printf("//////////////////////\n");
    	printf("// PACKET RECEIVED  //\n");
    	printf("// SRC		0x%x\n",pack_src);
    	printf("// DEST 	%x\n",ind_src);
    	printf("// LEN 		%x\n",len);
    	printf("// Flag		0x%x\n",pack.flag);
    	printf("// TYPE 	%x\n",type);
    	printf("// SEQ 		%x\n",pack_seq);
    	printf("// CHKSUM 	%x\n",pack.checksum);
    	printf("// Unused	0x00\n");
    	printf("// Data		%s\n",pack.data);
    	printf("//////////////////////\n");
    	return;
    }
     
     
     
     
    void* handle_client_read(void* param)
    {
    	
    	pthread_exit(0);
    	    	   
    }
    void* handle_client_send(void* param)
    {
    	 while(1){
    	    	   	printf("Enter your next message: ");
    	    	   	char command[1024]={ 0 };
    	    	   	gets(command);
    				
    				struct packet pack;
    				pack.seq_number=seq;
    				seq+=1;
    				pack.src_number=ind_src;
    	    	   	if(strcmp(command,"LIST")==0)
    				{
    					strcpy(pack.data,"LIST");
    					pack.type=0;
    					send(sock,&pack,sizeof(pack),0);
    				}
    				else if(command[0]=='M' && command[1]=='E' && command[2]=='S' && command[3]=='G')
    				{
    					pack.type=2;
    					bzero(pack.data,1024);
    					strcpy(pack.data,chat);
    					strcat(pack.data,"?");
    					char data_to_be_sent[1024]={ 0 };
    					strcpy(data_to_be_sent,command+5);
    					strcat(pack.data,data_to_be_sent);
    					pack.r=0;
    					send(sock,&pack,sizeof(pack),0);
    					printf("Message sent to %s\n",chat);
    				}
    	    	   	else if(strcmp(command,"EXIT")==0)
    	    	   	{
    					bzero(pack.data,1024);
    					strcpy(pack.data,chat);
    					av=true;
    					printf("Connection  closed with %s\n",chat);
    					strcpy(chat,"NULL");
    					pack.type=7;
    	    	   		send(sock,&pack,sizeof(pack),0);
    	    	   		
    	    	   	}
    	    	   	else
    	    	   	{
    					pack.type=1;
    					pack.r=0;
                        pack.flag=pack.flag | (0b00100000);
                        pack.flag=pack.flag & (0b11101110);
                       
    					bzero(pack.data,1024);
    					strcpy(pack.data,command);
    					strcpy(chat,command+5);
                        uint8_t checksum=cal_checksum(pack);
                        pack.checksum=checksum;
    					send(sock,(void*)&pack,sizeof(pack),0);
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
    	char sin_user[1024]={ 0 };
    	int a=5;
    	int k=0;
    	
    	while(1)
    	{
    		if(buffer[a]=='\0')break;
    		single_usr[k]=buffer[a];
    		sin_user[k]=buffer[a];
    		k+=1;
    		a+=1;
    	}
            
            
    		
    		srcaddr_i+=1;
    		src_addr[i]=srcaddr_i;
    		client_username[i]=sin_user;
    		client_details[i]=single_usr;
            printf("%s\n",client_username[i]);
    		char srcaddr_to_be_sent[1024]={ 0 };
    		sprintf(srcaddr_to_be_sent, "%02X",srcaddr_i);
     
    		strcat(client_details[i],"@");
    		strcat(client_details[i],"0x");
    		strcat(client_details[i],srcaddr_to_be_sent);
     
    		send(newsock[i],srcaddr_to_be_sent,strlen(srcaddr_to_be_sent),0);
           
    	    while(1){
    				char mesg[1024];
    				struct packet pack;
    				bzero(pack.data,1024);
    				int vr1=recv(newsock[i],(void*)&pack,sizeof(pack),0);
    				
     
    				
    				
    				if(pack.r==1)
    				{
    					
    					char tuser[1024]={ 0 };
    					
    					int a=0;
    					int k=5;
    					char conn[1024]={"CONN "};
    					
    					while(1)
    					{
    						if(pack.data[k]==':')break;
    						tuser[a]=pack.data[k];
    						k+=1;
    						a+=1;
    					}
    					strcat(conn,tuser);
    					strcat(conn,":");
    					while(1)
    					{
    						if(pack.data[k]=='\0')
    						{
    							if(pack.data[k-1]=='Y')
    							{
    								strcat(conn,"Y");
    							}
    							else
    							{
    								strcat(conn,"N");
    							}
    							break;
    						}
    						k+=1;
    					}
     
    					bzero(pack.data,1024);
    					
    					strcpy(pack.data,conn);
    					pack.type=1;
    					pack.r=1;
    					int y=0;
    					for(y=0;y<no_of_cl;y++)
    					{
    						if(strcmp(client_username[y],tuser)==0)
    						{
    							send(newsock[y],(void*)&pack,sizeof(pack),0);
    							break;
    						}
    					}
    				}
    				else if(pack.type==7)
    				{
    					int y=0;
    					for(y=0;y<no_of_cl;y++)
    					{
    						if(strcmp(client_username[y],pack.data)==0)
    						{
    							bzero(pack.data,1024);
    							strcpy(pack.data,"EXIT");
    							send(newsock[y],(void*)&pack,sizeof(pack),0);
    							break;
    						}
    					}
    				}
    				else if(pack.type==0)
    				{
    					char userslist[1024]={ 0 };
    				
    					funcS(pack);
    					int y=0;
    					for(y=0;y<no_of_cl;y++)
    					{
    						strcat(userslist,client_details[y]);
    						strcat(userslist,"\n");
    					}
    					
    					pack.seq_number=seq;
    					seq+=1;
    					pack.src_number=0x80;
    					strcpy(pack.data,userslist);
    					send(newsock[i],(void*)&pack,sizeof(pack),0);
    					continue;
    				}
    				else if(pack.type==2)
    				{
    					funcS(pack);
     
    					char tuser[1024]={ 0 };
    					int k=0;
    					int a=0;
    					while(1)
    					{
    						if(pack.data[k]=='?')break;
    						tuser[a]=pack.data[k];
    						k+=1;
    						a+=1;
    					}
    					k+=1;
    					a=0;
    					char data_to_be_sent[1024]={ 0 };
    					while(1)
    					{
    						if(pack.data[k]=='\0')break;
    						data_to_be_sent[a]=pack.data[k];
    						k+=1;
    						a+=1;
    					}
    					k+=1;
    					int y=0;
    					for(y=0;y<no_of_cl;y++)
    					{
    						if(strcmp(client_username[y],tuser)==0)
    						{
    							strcpy(pack.data,data_to_be_sent);
    							pack.r=0;
    							pack.type=2;
    							send(newsock[y],(void*)&pack,sizeof(pack),0);
    							break;
    						}
    					}
    				}
    				else if(pack.type==1)
    				{
    					funcS(pack);
     
    					char conn[1024]={ 0 };
    					strcat(conn,"CONN ");
    					strcat(conn,sin_user);
    					char tuser[1024]={ 0 };
    					strcpy(tuser,pack.data+5);
    					int y=0;
    					
    					pack.r=0;
     
    					
    					for(y=0;y<no_of_cl;y++)
    					{
    						if(strcmp(client_username[y],tuser)==0)
    						{
    							bzero(pack.data,1024);
    							strcpy(pack.data,conn);
    							send(newsock[y],(void*)&pack,sizeof(pack),0);
    							break;
    						}
    					}
     
     
    					continue;
    					
    				}
     
     
    				else if(mesg[0]=='E' && mesg[1]=='X' && mesg[2]=='I' && mesg[3]=='T')
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
    	    	   
    	    	   
    	    	   
    	    	   
    	    	   char greetings[1024]={"HELO "};
    	    	   strcat(greetings,usr);
    	    	   send(sock,greetings,strlen(greetings),0);
    	    	   printf("200 OK\n");
    	    	   
    			   char buffer[1024]={ 0 };
    	    	   int vr=recv(sock,buffer,1024,0);
    			   ind_src=strtoul(buffer,NULL,8);
    	    	   
    			   
    	    	   pthread_attr_t attr;
    	    	   pthread_attr_init(&attr);
     
    	    	   
    	    	   pthread_t cltid_send;
    	    	   pthread_attr_init(&attr);
    	    	   pthread_create(&cltid_send,&attr,handle_client_send,NULL);
    	    	   
    	    	   while(1){
     
    					struct packet pack;
    					bzero(pack.data,1024);
    					char buffer[1024]={ 0 };
    					int vr=recv(sock,&pack,sizeof(pack),0);	   		
    					int j=0;
    					
     
    					
    					
    					if(pack.type==0)
    					{
    						funcC(pack);
    						continue;
    					}
    					else if(pack.type==2)
    					{
    						funcC(pack);
    						continue;
    					}
    					else if(pack.r==1)
    					{
    						funcC(pack);
    						char tuser[1024]={ 0 };
    						int a=0;
    						int k=5;
    						while(1)
    						{
    							if(pack.data[k]==':')break;
    							tuser[a]=pack.data[k];
    							a+=1;
    							k+=1;
    						}
    						while(1)
    						{
    							if(pack.data[k]=='\0')
    							{
    								if(pack.data[k-1]=='Y')
    								{
    									av=false;
    									printf("Connected to %s\n",chat);
    									
    								}
    								else{

    									printf("Connection to %s dropped\n",chat);
    									strcpy(chat,"NULL");
    									
    									
    								}
    								break;
    							}
    							k+=1;
     
    						}
    						
    						continue;
    					}
     
    					else if(pack.type==7)
    					{
    						funcC(pack);
     
    						if(strcmp(pack.data,"EXIT")==0)
    						{
    							printf("Connection closed with %s\n",chat);
    							av=true;
    							strcpy(chat,"NULL");
    						}
    					}
    					else if(pack.type==1)
    					{
    						funcC(pack);
    						
    						char ruser[1024]={ 0 };
    						strcpy(ruser,pack.data+5);
     
    						if(av)
    						{
    							av=false;
    							struct packet send_pack;
    							send_pack.seq_number=seq;
    							seq+=1;
    							send_pack.src_number=ind_src;
    							char conn[1024]={ 0 };
    							
    							strcpy(chat,ruser);
    							strcat(conn,"CONN ");
    							strcat(conn,ruser);
    							strcat(conn,":");
    							strcat(conn,"Y");
    							bzero(send_pack.data,1024);
    							strcpy(send_pack.data,conn);
                                send_pack.flag=send_pack.flag & (0b11001110);
    							send_pack.type=9;
    							send_pack.r=1;

                                uint8_t finalchecksum=cal_checksum(send_pack);
                                send_pack.checksum=finalchecksum;
    							send(sock,(void*)&send_pack,sizeof(send_pack),0);
    							
    							printf("Connected to %s\n",ruser);
    							continue;
    						}
    						else{
    							
    							struct packet send_pack;
    							send_pack.seq_number=seq;
    							seq+=1;
    							send_pack.src_number=ind_src;
    							char conn[1024]={ 0 };
    							send_pack.flag=send_pack.flag | (0b00010000);
    							strcat(conn,"CONN ");
    							strcat(conn,ruser);
    							strcat(conn,":");
    							strcat(conn,"N");
    							bzero(send_pack.data,1024);
    							strcpy(send_pack.data,conn);
    							send_pack.type=9;							
    							send_pack.r=1;
                                uint8_t finalchecksum=cal_checksum(send_pack);
                                send_pack.checksum=finalchecksum;
    							send(sock,&send_pack,sizeof(send_pack),0);
    							printf("Connection to %s\n dropped\n",ruser);
    							continue;
    						}
    					}
     
     
    					else if(buffer[0]=='M' && buffer[1]=='E' && buffer[2]=='S' && buffer[3]=='G')
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
    	    	  
    	    	   
    		   
    		   close(client_s);
            }
    	return 0;	
    }
     
     
     
    /*
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
     
     
     
     
    			// else if(buffer[0]=='R')
    		// {
    			
    		// 	// arr[0]=1;
    		// 	// arr[2]=(uint8_t)buffer[1];
    		// 	// arr[1]=(uint8_t)buffer[2];
     
    		// 	char data[1024]={ 0 };
    		// 	int a=0;
    		// 	int k=7;
     
    		// 	char tuser[1024]={ 0 };
    		// 	while(1)
    		// 	{
    		// 		if(buffer[k]=='@')break;
    		// 		tuser[a]=buffer[k];
    		// 		k+=1;
    		// 		a+=1;
    		// 	}
    			
    		// 	k=0;
     
    		// 	while(1)
    		// 	{
    		// 		if(buffer[k]=='\0')
    		// 		{
    		// 			if(buffer[k-1]=='Y')
    		// 			{
    		// 				strcat(data,"Connected to ");
    		// 				strcat(data,tuser);
    		// 				//funcC(arr,data);
    		// 				break;
    		// 			}
    		// 			else{
    		// 				strcat(data,"Connection to ");
    		// 				strcat(data,tuser);
    		// 				strcat(data," dropped");
    		// 				//funcC(arr,data);
    		// 			}
    		// 		}
    		// 		k+=1;
    		// 	}
     
    		// }
    */