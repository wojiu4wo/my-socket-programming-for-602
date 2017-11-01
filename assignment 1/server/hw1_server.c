//
//  hw1_server.c
//  
//
//  Created by Daojing Guo on 17/9/18.
//
//

#include "stdio.h"
#include "sys/socket.h"
#include "netinet/in.h"
#include "unistd.h"//remove the warning from close()
#include "stdlib.h" //remove the warning from the exit()
#include "string.h" //remove the warning from the memset()
#include "arpa/inet.h"//remove the warning from inet_pton() as a implicit declaration
#include "sys/types.h"
#define MAXLINE 1024
int main(int argc,char *argv[])//argv[0] is echos, argv[1] is the port
{
    
    if (argc!=2){
        printf("FAIL: SERVER file name and port required\n");
    }
    
    int IP_port;
    sscanf(argv[1],"%d",&IP_port);//convert the char on the screen (stdin) to integer
    printf("IP_port is %d\n",IP_port);
    //socket()
    
    int listen_channel = socket(AF_INET, SOCK_STREAM, 0); //choose AF_INET for ipv4, and sock_stream for TCP,
    if (listen_channel == -1){ //if socket return the value -1, that means failure and no socket is created
        printf("FAIL: cannot create a socket");
    }
    printf("socket is created\n");
    
    //socket infomation initial setting
    struct sockaddr_in socket_address_ipv4; //create one sockaddr_in socket type, which is the IPV4 structure
    memset(&socket_address_ipv4,0,sizeof(socket_address_ipv4));//clean up the current sockaddr_in and leave enough space with 0
    socket_address_ipv4.sin_family = AF_INET;//this is IPV4 socket reuiqred
    socket_address_ipv4.sin_port = htons(IP_port);//assign the port number, the sockaddr_in reuiqred the port is network byte ordered; server only listen to the port, and all local ip address are available
    socket_address_ipv4.sin_addr.s_addr = htonl(INADDR_ANY);//s_addr is the presentation of ipaddress, there are 3 types need to be specify; INADDR_ANY represent ipaddress as 0.0.0.0, which means all the local server ip address is available for client to connect.
    printf("socket setting is initialed");
    
    //bind()
    int bind_index = bind(listen_channel,(struct sockaddr*)&socket_address_ipv4,sizeof(socket_address_ipv4));//socket_fd is the decription of this socket, the third argument is the lenghth of this socket which is the size of what we defined.
    if (bind_index == -1){
        printf("FAIL: cannot bind the port");
    }
    printf("server bind to the socket\n");
    
    //listen()
    if((listen(listen_channel,5))<0){
        printf("FAIL: cannot listen to the socket");
        }//listen_channel is the socket going to listen, 5 means the total number of socket can be held by server
    printf("server is listening to the socket\n");
    
    //accept() and echo back the message
    struct sockaddr_in client_channel_info;
    memset(&client_channel_info,0,sizeof(client_channel_info));
    socklen_t client_channel_len = sizeof(client_channel_info);//the last element in accept() is the socklen_t type of data, aka unsigned int *.
    
    char client_buffer[MAXLINE];//this is used for msg store
    char message[] = {"hello, this is server\n"};
    
    while(1){
        memset(&client_buffer,0,sizeof(client_buffer));
        int accpet_socket_fd = accept(listen_channel,(struct sockaddr*)&client_channel_info,&client_channel_len);//the last arguement is flag. the first three arguments are same as connect()
        if ((accpet_socket_fd) == -1){
            printf("FAIL: cannot accept the socket");
        }
        printf("accept one client socket\n");
        if((recv(accpet_socket_fd,client_buffer,sizeof(client_buffer),0))<0){
            printf("FAIL:cannot recv the message from the client");
        }//recv() share the same first three argument as bind accept does, but what it listen is the one is accepted by server from the parent socket descriptor; The last argument is the flag.
        printf("server received one message from client: ");
        printf("%s\n",client_buffer);
        //send(accpet_socket_fd,message,sizeof(message),0);
        if ((send(accpet_socket_fd,client_buffer,sizeof(client_buffer),0))<0){
            printf("FAIL: cannot send the message");
        }
        printf("server send the echo message back to client\n");
        close(accpet_socket_fd);//close the current socket connecting to client and communication, which is the child
        printf("the child socket is closed\n");
    }
    close(listen_channel);//close the listening socket, which is the parent
    printf("the parent socket is closed\n");

    
    
    
    exit(0);
    
    
    
    
    
    
}
