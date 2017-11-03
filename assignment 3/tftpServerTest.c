//
//  TFTPserver.c
//
//
//  Created by Daojing Guo on 17/10/30.
//
//

//#include "TFTPserver.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <inttypes.h>
#include <signal.h>
#include <stdint.h>
//#include "udpstruct.h"

#define BUFSIZE 1024
#define MAX_DATA_SIZE 512
#define MAX_FILE_SIZE 52428800
#define MAX_CLIENTS 4
#define ERR1 "FILE NOT FOUND"
#define ERR2 "FILE SIZE EXCEEDED"
/*
struct RRQ_Packet
{
    unsigned int opcode: 2;
    char *filename;
    unsigned int endOfFile: 1;
    char mode[512];
    unsigned int endOfFile1: 1;
};

struct DATA_Packet
{
    unsigned int opcode: 2;
    unsigned int blockNo: 2;
    char data[512];
};

struct ACK_Packet
{
    unsigned int opcode: 2;
    unsigned int blockNo: 2;
};

struct ERROR_Packet
{
    unsigned int opcode: 2;
    unsigned int errorCode: 2;
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

*/
//struct RRQ_Packet RRQPacket;
//struct client clientList[MAX_CLIENTS];
//int clientcount=0;
FILE *fp = NULL;

int sending_packet(int clientfd, struct sockaddr_in clientaddr,socklen_t clientlen ){
    //unsigned char packet_buffer[512] = "";
    char *packet_buffer;
    int index;
    int block_no;
    char sending_buff[516] = {'\0'};
    char received_buff[100];
    int index_timeout= 0;
    while (!feof(fp))
    {
        block_no ++;
        packet_buffer = (char*)malloc(512);
        memset(&packet_buffer, 0, sizeof(packet_buffer));
        index = fread(packet_buffer,512,1,fp);//fread used to read 1 unit of packet with size 512, the return value is the number of unit.
        memset(&sending_buff, 0, sizeof(sending_buff));
        *(int *)sending_buff = htons(3);//this is DATA packet
        *(int *)(sending_buff+2) = htons(block_no);//the wrapping block number
        memcpy(sending_buff+4, (const char *)packet_buffer, 512);//put the packet data into sending buffer
        int result = sendto(clientfd, sending_buff, 516, 0, (struct sockaddr *)&clientaddr, clientlen);
        printf("Sent %d bytes in this packet\n",result);
        printf("waiting for the ACK\n");
        //wait for ACK
        while(1){
            struct timeval tv;  // implementing timeout
            fd_set readfds;
            tv.tv_sec = 1;//timeout is 1 sec
            tv.tv_usec = 0;
            FD_ZERO(&readfds);  // clear the file descriptor list
            FD_SET(clientfd, &readfds);  // add the new server file descriptor to the list
            select(clientfd+1, &readfds, NULL, NULL, &tv);  // the timer is 1 sec
            if (FD_ISSET(clientfd, &readfds)) {  // the ack is recevied before the time out
                recvfrom(clientfd, received_buff,99 , 0, (struct sockaddr *)&clientaddr, &clientlen);
                uint16_t ack_no;  // block number
                memcpy(&ack_no,received_buff, sizeof(uint16_t));
                printf("opcode: %d\n ACK number: %d\n local block number : %d\n", *(received_buff+1), ntohs(ack_no), block_no);
                if( *(received_buff+1) == 4 && ntohs(ack_no)==block_no) {//which means opcode == 4, and ack == block, then break the ACK while loop, continue send next peacket
                    break;
                } else {
                    // if ack != block, then retransmit the packet
                    sendto(clientfd, sending_buff, 516, 0, (struct sockaddr *)&clientaddr, clientlen);//retransmit
                }
            } else {  // time out
                // retransmission
                printf("The timeout count is %d \n",index_timeout);
                index_timeout ++;
                if (index_timeout>10){//after 10 timeout
                    return 2;//timeout
                }
                sendto(clientfd, sending_buff, 516, 0, (struct sockaddr *)&clientaddr, clientlen);//retransmit
            }
        }//end of ACK while loop
    }//end of packet sending while loop
    close(clientfd);
    if(feof(fp)){
        
        return 1; //end of file
    }
    //split the file and get the first packet
    //send the first packet and go in to select(), recorded the length of the first packet
    //while loop below
    //select(timeout)
        //received change in this clientfd, which is ACK, if not, drop; timeout_count = 0
            //compare ack and blocknum
                //if ==, check feof
                    //if end, check the legnth of the first packet.
                        //if == 512, send one more packet with 0 data, blocknum++. if less, break the while loop.
                    //if not end, send next packet, blocknum ++
                //if !=, restransmit the packet, blocknum keep
    
        //timeout reach, check timeout_count
            //if >10, close the socket and clean up, break the while loop
            // if <= 10, blocknum keep, rexmit the packet, timeout_count ++
    //send packet, if blocknum keep, old packet; if blocknum ++, new packet; then begin next while loop；if blocknum > 2 to 16, error, then break;
    //the end of while loop
    //close the child socket and clientfd
    return 0;
}

int main(int argc, char **argv)
{
    int sockfd;
    int clientfd;
    int port;
    struct sockaddr_in serveraddr;
    int optval;
    struct sockaddr_in clientaddr;
    uint16_t opcode, error_opcode, error_type;
    char *buff;
    char index_buffer[100];
    char sending_buff[516] = {'\0'};
    char error_buff[516] = {'\0'};
    int count;
    socklen_t clientlen;
    //char error_message1[512] = {'\0'};
    //char error_message2[512] = {'\0'};
    int index_len;
    int parent_received;//the length from recv from
    char filename[512];
    struct hostent* hret;
    
    port = atoi(argv[2]);//the server is going to bind this port
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);//the server`s fd
    if (sockfd<0){printf("Socket initialization fail! \n");}
    printf("socket initialization success\n");
    //optval = 1;
    //setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR,(const void *)&optval , sizeof(int));
    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    hret = gethostbyname(argv[1]);
    memcpy(&serveraddr.sin_addr.s_addr, hret->h_addr,hret->h_length);
    serveraddr.sin_port = htons((unsigned short)port);
    //bind
    bind(sockfd, (struct sockaddr *) &serveraddr,sizeof(serveraddr));
    while(1){
        printf("The server is wating from new coming request. \n");
        //buff= (char *)malloc(sizeof(struct RRQ_Packet));
        clientlen = sizeof(clientaddr);
        memset(buff, 0, 516);//512+2+2
        parent_received = recvfrom(sockfd, index_buffer, 99, 0, (struct sockaddr *)&clientaddr,&clientlen);//the server sockfd only receive the RRQ packet or WRQ packet
        printf("\n Got a client request");
        count++;
        //check file existed or not
        memset(&filename, 0, 512);
        memcpy(&filename,index_buffer+2, 512);
        filename[strlen(filename)] = '\0';
        FILE *fp;
        printf("The file name is %s \n", filename);
        fp = fopen (filename, "rb");
        if (fp == NULL)
        {
            memset(&error_buff, 0, sizeof(error_buff));
            *(int *)(error_buff) = htons(5);//opcode 5 means error message
            *(int *)(error_buff+2) = htons(1);//file not found
            char error_message1[20] = "FILE NOT FOUND ! \n ";
            index_len = strlen(error_message1);
            strncpy(error_buff+4, error_message1, index_len);//add error message in the buffer
            index_len = 4+index_len;
            sending_buff[index_len] = '\0';
            sendto(sockfd, error_buff, index_len, 0, (struct sockaddr *)&clientaddr, clientlen);
        }
        else // file exists
        {
            int process_pid = fork();// in the father process, the fork return the id of new child process, which is not 0; in the child process, the fork return 0 means "i am the child process"
        
        
            if (process_pid == -1){printf("fork error\n");}
            if(process_pid != 0){//im in the parent process
                printf("this is the parent process, and I will close the child 1 connected socket, which will continue run in child 1 process.\n");
            //continue;//continue to waiting for next recvfrom()
            }
            if (process_pid == 0){//in the child process, get a new connection
                //close current socket, since it is belonged to parent process
                //close(sockfd);
                //opcode = ntohs(*(unsigned int *)buff);
                //get the clientaddr and other information
                //create new socket, have a new clientfd
                printf("create  a new socket\n");
                //int portno=0;
                //struct sockaddr_in clientaddr;
                clientfd = socket(AF_INET, SOCK_DGRAM, 0);//a new clientfd
                int optval = 1;
                setsockopt(clientfd, SOL_SOCKET, SO_REUSEADDR,(const void *)&optval , sizeof(int));
                bzero((char *) &clientaddr, sizeof(clientaddr));
                clientaddr.sin_family = AF_INET;
                clientaddr.sin_addr.s_addr = htonl(INADDR_ANY);
                clientaddr.sin_port = htons((unsigned short)port);
                bind(clientfd, (struct sockaddr *) &clientaddr,sizeof(clientaddr));
                printf(" Bind successfully completed of client \n");
                //get the filename from the parent received RRQ request, get the clientaddr
                opcode = htons(3);
                memset(&sending_buff, 0, sizeof(sending_buff));
                memcpy(sending_buff, (const char *)&opcode, sizeof(uint16_t));
                /*
                int file_size;
                
                FILE *fp_index;
                fp_index = fopen(filename, "rb");
                fseek(fp_index, 0, SEEK_END);//the first argument is the pointer of this file, the second argument is the pointer-moving offset, the third argument is the position that start moving. SEEK_END: set the pointer to the end of file and move (add) offset (offset can be negative here) position; SEEK_SET: set the pointer to the start of file and move(add) offset.
                file_size = ftell(fp_index);
                printf("The file size is %d \n", file_size);
                fclose(fp_index);
                if (file_size > 33554432)
                {
                    memset(&error_buff, 0, sizeof(error_buff));
                    *(int *)(error_buff) = htons(5);//opcode 5 means error message
                    *(int *)(error_buff+2) = htnos(4);//Illegal TFTP operation
                    error_message2[] = "FILE REACH MAXIMUM SIZE \n ";
                    index_len = strlen(error_message2);
                    strncpy(error_buff+4, error_message2, index_len);//add error message in the buffer
                    index_len = 4+index_len;
                    sending_buff[index_len] = '\0';
                    sendto(sockfd, error_buff, index_len, 0, (struct sockaddr *)&clientaddr, &clientlen);
                    printf("Sent the error message about meximum file size \n ");
                    break;
                }
                 */
                //file exit and OK, sending file
                int final_result = sending_packet(clientfd, clientaddr, clientlen);
                if (final_result == 1){//end of file
                    printf("Succesfully send the file\n");
                }
                else if (final_result == 2){//10 timeout
                    printf("reached the timeout count as 10\n");
                }
                printf("the end of whole process \n");
                
                //if filename not existed, error. if filename existed, start sending packet.
                //initial blocknum == 1
                //sending_packet(clientaddr, clientfd, blocknum)
                return -1;
            }//end of child process
            fclose(fp);

        }//end of file existed situation
        
    }//end of while loop
    exit(0);//end of fork loop
    //return;
}
