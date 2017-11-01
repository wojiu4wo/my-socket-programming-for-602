#include "stdio.h"
#include "sys/socket.h"
#include "netinet/in.h"
#include "unistd.h"//remove the warning from close()
#include "stdlib.h" //remove the warning from the exit()
#include "string.h" //remove the warning from the memset()
#include "arpa/inet.h"//remove the warning from inet_pton() as a implicit declaration
#define MAXLINE 1024
int main(int argc,char *argv[])//argv[0] is file name, argv[1] is the ipaddress number, argv[2] is the port number,  argc is the number of argv
{
    if (argc != 3){
        printf("FAIL: client file name and client host name required\n"); //check whether we set echo, ip address and port number
    }
    printf("argv0:%s\n",argv[0]);
    printf("argv1:%s\n",argv[1]);
    printf("argv2:%s\n",argv[2]);
    char client_buffer[MAXLINE];
    int IP_port;
    sscanf(argv[2],"%d",&IP_port);//convert the char on the screen (stdin) to integer
    printf("IP_port is %d\n",IP_port);
    char sendline[MAXLINE];
    char *IP_address;//char and char * is different, since 127.0.0.1 is char * type
    IP_address = argv[1];
    printf("IP_ADDRESS is %s\n",IP_address);
    
    //create a socket
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0); //choose AF_INET for ipv4, and sock_stream for TCP
    if (socket_fd == -1){ //if socket return the value -1, that means failure and no socket is created
        printf("FAIL: cannot create a socket");
    }
    printf("created a socket, ready to socket setting\n");
    
    //socket infomation initial setting
    struct sockaddr_in socket_address_ipv4; //create one sockaddr_in socket type, which is the IPV4 structure
    memset(&socket_address_ipv4,0,sizeof(socket_address_ipv4));//add 0 to make sure it is an clean space for sockaddr_in
    socket_address_ipv4.sin_family = AF_INET;//this is IPV4 socket reuiqred
    socket_address_ipv4.sin_port = htons(IP_port);//assign the port number, the sockaddr_in reuiqred the port is network byte ordered
    if ((inet_pton(AF_INET,IP_address,&(socket_address_ipv4.sin_addr)))<0){//get IP address from typing in the command line, from char to const char *, we need & to specify the pointer; & is used for point to the the location of char, because ip_address is already a char * with pointer, so no need to point again.
        printf("FAIL: error for the presentation format");
    }//socket_address_ipv4.sin_addr.s_addr = inet_pton("127.0.0.1") is no longer used, since inet_pton is not supporting ipv6;assign the IP address, using 127.0.0.1 to connect the local.
    printf("finish socket setting, ready to connect\n");
    
    //connect()
    int connection_index = connect(socket_fd,(struct sockaddr*)&socket_address_ipv4,sizeof(socket_address_ipv4));//socket_fd is the decription of this socket, the third argument is the lenghth of this socket which is the size of what we defined. Must use (struct sockaddr*)& to turn the sockadr_in into 'const struct sockaddr*'
    if ((connection_index) == -1){
        printf("FAIL: cannot connect the socket\n");
    }
    printf("ready to send msg, please type\n");
    memset(&sendline,0,sizeof(sendline));
    //send
    while(1){
        printf("what i send to the server: ");
    fgets(sendline,MAXLINE,stdin);//get what we type in the command line, the first argument is the buffer for message reading, the second argument is the number of message, the third is the source of this message, which is the command line here.
        printf("\n");
    if((send(socket_fd,sendline,strlen(sendline),0))<0){//socket_fd is the descriptor of the socket, sendline is the buffer, MAXLINE is the lenth of the buffer, 0 is the flags
        printf("FAIL: error when sending message\n ");
    }
    memset(&client_buffer,0,sizeof(client_buffer));
    int msg_recv = recv(socket_fd,client_buffer, sizeof(client_buffer),0 ); //
        client_buffer[msg_recv] = 0;//the msg_recv is the numberof tuples that client receives, so it can determine the terminating point in the buffer
        printf("the echo message from server is :");
        fputs(client_buffer,stdout);//put the message in the buffer to the screen
    if ((msg_recv) == -1){ //when the return value is -1, means it cannot receive message
        printf("FAIL: cannot receive the msg from the server");
        }
    
    }
    close(socket_fd);//close the socket
    printf("receive, closed the socket");//

    
    
    
    exit(0);
    
    
    
    
    
    
}
