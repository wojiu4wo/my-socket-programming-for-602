////  hw1_server.c
  //
//  Created by Daojing Guo on 17/9/18.
//
//

#include "stdio.h"
#include "sys/socket.h"
#include "netinet/in.h"
#include "unistd.h"	//remove the warning from close()
#include "stdlib.h" 	//remove the warning from exit()
#include "string.h" 	//remove the warning from memset()
#include "arpa/inet.h"	//remove the warning from inet_pton() as an implicit declaration
#include "sys/types.h"
#include "errno.h"
#define MAXLINE 1024
int writen(int fd, char *vptr, size_t n)//writen only write n (as the third argument) bytes to a socket and return the number of bytes of a (-1) on error
{
    char *ptr;//the pointer the the writing buffer
    ptr = vptr;
    int write_status = 0;
    int number_bytes_write = 0;
    printf("going to write: %s\n",ptr);
    for (int i = 0;i<n;i++){
    again:
        write_status = write(fd,ptr,n);//send one message with n bytes. The write() will return the number of bytes it sent which may not reach the n we assigned, or -1 for error, or EINTER for process inturrption. !!must not use &ptr, since ptr is already the pointer for me.
        if ((write_status) == -1){
            if ((errno) == 4){goto again;}//its not a error, but a signal interruption
            else{return -1;}//there`s error!
        }else{number_bytes_write += write_status;}//the count of written bytes
        i += write_status;//i become the number of bytes written
        write_status = 0;//refresh for next status
    }
    return number_bytes_write;
}

int main(int argc, char *argv[])		//argv[0] is echos, argv[1] is the port
{
	if (argc != 2){
        printf("error! Proper usage is: Name Port\n");
    }
    int IP_port;
	sscanf(argv[1], "%d", &IP_port);	//convert the char on the screen(stdin) to integer
    printf("the port number is: %d\n", IP_port);
    //socket()
    int listen_channel = socket(AF_INET, SOCK_STREAM, 0);	//choose AF_INET for IPv4, SOCK_STREAM for TCP
    if (listen_channel == -1)
	{	//if the socket return the value -1, that means failure in socket creating        
		printf("error! socket creating failed");
    }
    printf("socket created\n");
    //initial setting of socket infomation
	struct sockaddr_in socket_address_ipv4;		//create one sockaddr_in socket type, which is the IPv4 structure
    memset(&socket_address_ipv4, 0, sizeof(socket_address_ipv4));	//clean up current sockaddr_in and leave enough space with 0
    socket_address_ipv4.sin_family = AF_INET;	//this is IPv4 socket
    socket_address_ipv4.sin_port = htons(IP_port);	//assign the port number, the sockaddr_in reuiqre the port be network byte ordered //server only listen to this port, and all the local ip addresses are available
    socket_address_ipv4.sin_addr.s_addr = htonl(INADDR_ANY);	//s_addr is the presentation of ip address, there are 3 types to be specified //INADDR_ANY represent ip address as 0.0.0.0, which means all the local server ip addresses are available for client to connect with
    printf("socket setting initialized\n");
    //bind()
    int bind_index = bind(listen_channel, (struct sockaddr*)&socket_address_ipv4, sizeof(socket_address_ipv4));
		//socket_fd is the decriptor of this socket, the 3rd argument is the lenghth of this socket, which is the size we defined
    if (bind_index == -1)
	{
		printf("error! binding failed\n");
	}
    printf("server bound to the socket\n");
    //listen()
	if ( (listen(listen_channel,5)) < 0)
	{
		printf("error! listen to the socket failed");
	}	//listen_channel is the channel the socket is about to listen, 5 is the total number of sockets can be held by the server
    printf("server is listening to the socket\n");
    //accept() and echo back the message
	struct sockaddr_in client_channel_info;
    memset(&client_channel_info, 0, sizeof(client_channel_info));
    socklen_t client_channel_len = sizeof(client_channel_info);
    char client_buffer[MAXLINE];	//this is used for msg storing
    int child_number = 0;
    int status; //the status of parent processes
    char index_parent[1024]; //the index for parent process exit
	while(1)//for the fork()
	{
        child_number++;
        memset(&client_buffer, 0, sizeof(client_buffer));
        int accpet_socket_fd = accept(listen_channel, (struct sockaddr*)&client_channel_info, &client_channel_len);
		//the last argument in accept() is of type socklen_t(unsigned int *)
        if ((accpet_socket_fd) == -1)
            {printf("error! socket not accepted");
            }
        int process_pid = fork();// in the father process, the fork return the id of new child process, which is not 0; in the child process, the fork return 0 means "i am the child process"
        printf("pid is %d\n",process_pid);
        if (process_pid == -1){printf("fork error\n");
            }
        if(process_pid != 0){//im in the parent process
            printf("this is the parent process, and I will close the child 1 connected socket, which will continue run in child 1 process.\n");
            //close(listen_channel);
            /*while(1){
                memset(index_parent,0,sizeof(index_parent));
                status = recv(accpet_socket_fd,index_parent,sizeof(index_parent),0);
                if (status == 0){
                printf("parent process close\n");
                    //kill(process_pid,SIGUSR1);
                    close(accpet_socket_fd);
                    exit(0);
                }
                else{break;}
            }*/
            /*
            index_parent =waitpid(-1,NULL,WNOHANG);
            printf("my wait pid is : %d\n",process_pid);
            if(index_parent== -1){;//hold the parent process untill all child process die
                printf("parent process close\n");
                close(accpet_socket_fd);		//close the listening socket, which is the parent
                break;}*/
            
        }
        if (process_pid == 0){//in the child process
            printf("This is the child process: %d\n",child_number);
            printf("child %d : socket from client accepted, and one new child socket is created\n",child_number);
            while(1){//the loop is for waiting the child socket close, when it receive the tcp fin
            printf("child %d : the server is ready to receive msg\n",child_number);
                //recv
            int index_r = recv(accpet_socket_fd, client_buffer, sizeof(client_buffer), 0);//recv() share the same first three arguments as bind() and accept() does, but what it listens to is the one that
        //is accepted by the server from the parent socket descriptor, the last argument is the flag
            printf("child %d : received %d\n",child_number,index_r);
            if (index_r < 0){
                printf("child %d : error! message from the client not received\n",child_number);}
            //child close
            else if(index_r == 0){//means the server receive TCP FIN packet from client, and the data size is 0
                close(accpet_socket_fd);	//close current socket, which is the child"
                printf("child %d : child socket closed\n",child_number);
                exit(0);//when child process exit, it will jump back to the parent process, because of the wait() in the parent process. !!cannot use break here, since the child process is still running in the fork() while loop, stucking in the "listening" part.
            }
            else{
                printf("child %d : msg received! the msg is: \n",child_number);
                printf("%s\n", client_buffer);
            }
                char writing_buffer[index_r];
                memcpy(writing_buffer,client_buffer,index_r+1);
            //if ((send(accpet_socket_fd, client_buffer, sizeof(client_buffer), 0)) < 0)
            int index_w = writen(accpet_socket_fd,writing_buffer,index_r);
            printf("child %d : the return value of writen is: %d\n",child_number,index_w);
            if (index_w == -1){//-1 means error!
                printf("child %d : error! message sending failed because the return value of writen is -1\n",child_number);}
            else{printf("child %d : the server echo back %d bytes data\n",child_number,index_w);}//how many bytes we send back
                break;}//end of client transmission
            return -1;//exit the child process
        }//end of child process
        
    }//end of fork loop
    exit(0);
}


