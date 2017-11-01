//
//  tftpServer_0.c
//  
//
//  Created by Daojing Guo on 17/10/31.
//
//

#include <stdio.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "udpstruct.h"

struct RRQ_Packet rrq_packet;
struct client clientlist[4];
int client_total_count = 0;
FILE *fp = NULL;
struct RRQ_Packet
{
    uint16_t opcode;
    char *filename;
    uint8_t endOfFile;
    char mode[512];
    uint8_t endOfFile1;
};

struct DATA_Packet
{
    uint16_t opcode;
    uint16_t blockNo;
    char data[512];
};

struct ACK_Packet
{
    uint16_t opcode;
    uint16_t blockNo;
};

struct ERROR_Packet
{
    uint16_t opcode;
    uint16_t errorCode;
    char errorMessage[512];
    char endOfFile;
};

struct client{
    int clientfd;
    //struct sockaddr addr_in;
    int state;     //0 : inactive    1 : waiting for ack_pkt
    int block_no;
    FILE* fp;
};

int RRQ_request(int listening_fd)
{
    struct sockaddr_in clientaddr;
    int clientfd = 0;
    int index_port ;
    //struct sockaddr_in clientaddr;
    char *buff;
    int len;
    char filename[512];
    //create a new socket for this client, close the old socket.
    //close the socket?
    clientfd = socket(AF_INET, SOCK_DGRAM, 0);
    int optval = 1;
    setsockopt(clientfd, SOL_SOCKET, SO_REUSEADDR, (const void*)&optval, sizeof(int));
    bzero((char *)&clientaddr, sizeof(clientaddr));
    clientaddr.sin_family = AF_INET;
    clientaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    clientaddr.sin_port = htons((unsigned short)portno);
    bind(clientfd, (struct sockaddr *) &clientaddr,sizeof(clientaddr));
    //end of socket creating
    memset(&filename, 0, 512);
    memcpy(&filename, buff + sizeof(uint16_t),512);//get the filename from the buff
    len = strlen(filename);
    filename[len] = '\0';//set the end of filename
    struct hostent *hostp;
    hostp = gethostbyaddr((const char *)&clientaddr.sin_addr.s_addr,
                          sizeof(clientaddr.sin_addr.s_addr), AF_INET);
    char *hostaddrp; // the dotted devimal addrress of client
    hostaddrp = inet_ntoa(clientaddr.sin_addr);
    printf("filename from the client is %s: %s", hostaddrp, filename);
    clientlist[client_total_count].clientfd = clientfd;
    fp = fopen(filename, "rb");
    clientlist[client_total_count].fp = fp;
    clientlist[client_total_count].block_no = 1;
    client_total_count++;
    struct client *client_index == NULL;
    for(j=0;j<client_total_count;j++)
    {
        if(clientfd == clientlist[j].clientfd)
        {
            client_index == &clientlist[j]
        }
    }
    if(client_index != NULL){
        printf("client fd is %d\n",client_index.clientfd);
    }
    FILE *fp_index;
    fp_index = fopen(filename, "rb");
    if(fp_index!=NULL)
    {
        fseek(fp_index,0,SEEK_END);
        int fp_index_size = ftell(fp_index);
        fclose(fp_index);
        
    }
    return clientfd;
}
int main(int argc, char **argv)
{
    int listening_fd; //the one server is listning
    int accept_client_fd; //the one server has create connection with client
    int port = 69;
    if (argc != 3){
        printf("the number of parameters is wrong\n");
    }
    //create soket
    listening_fd = socket(AF_INET, SOCK_DGRAM, 0);
    int optval = 1;
    setsockopt(listening_fd, SOL_SOCKET, SO_REUSEADDR, (const void*)&optval, sizeof(int));
    struct sockaddr_in UDP_addr;//the server address of UDP connection
    bzero((char *)&UDP_addr, sizeof(UDP_addr));
    UDP_addr.sin_family = AF_INET;
    struct hostent* hret;
    hret = gethostbyname(argv[1]);
    memcpy(&UDP_addr.sin_addr.s_addr, hret->h_addr, hret->h_length);
    UDP_addr.sin_port = htons((unsigned short)port);
    bind(listening_fd,(struct sockaddr *)&UDP_addr, sizeof(UDP_addr));
    int process_pid = fork();// in the father process, the fork return the id of new child process, which is not 0; in the child process, the fork return 0 means "i am the child process"
    /*
    printf("pid is %d\n",process_pid);
    if (process_pid == -1){printf("fork error\n");
    }
    if(process_pid != 0){//im in the parent process
    
    
    }//the end of parent process
    if (process_pid == 0){//in the child process
        
    
    }//the end of child process
    */
    fd_set master;
    fd_set read_fds;
    int fdmax;
    FD_SET(listening_fd, &master);
    fdmax = listening_fd;
    int select_index;
    struct sockaddr_in client_addr, index_addr;
    struct hostent *client_info;
    char *client_buffer;
    socklen_t client_len;
    int clientfd = 0;
    int index_r;//the message recv from the client
    int index_port ;
    while (1)
    {
        read_fds = master;
        select_index = select(fdmax+1, &read_fds, NULL,NULL,NULL);
        for (int i=0;i<=fdmax;i++)
        {
            if(FD_ISSET(i, &read_fds)){
                if(i == listening_fd){
                    client_buffer = (char *)malloc(sizeof(struct RRQ_Packet));
                    client_len = sizeof(client_addr);
                    memset(client_buffer,0,516);
                    client_r = recvfrom(listening_fd, client_buffer, sizeof(struct RRQ_Packet), 0, (struct sockaddr *)&client_addr, &client_len);
                    pinrtf("Receive a new client RRQ request\n");
                    //int index_len;
                    //uint16_t ack;
                    char filename[512];
                    unit16_t opcode;
                    memset(&opcode, 0, sizeof(unit16_t));
                    memcpy(&opcode, client_buffer, sizeof(uint16_t));
                    opcode = ntohs(opcode);///opcode 1 is RRQ, opcode 2 is WRQ, opcode 3 is DATA, opcode 4 is ACK, opcode 5 is error
                    printf("the opcode is &s \n", opcode);
                    if (opcode == 1)//RRQ
                    {
                        RRQ_request(i);
                    }//end of opcode == 1
                    FD_SET(clientfd,&master);//add current client fd into the local list
                    if(clientfd>fdmax)
                    {
                        fdmax = clientfd;
                    }
                }//i == sockfd
                else{
                    
                }
            }//i in the list of listen fd
        }//end of for loop for descriptor
    }//end of while loop
}
