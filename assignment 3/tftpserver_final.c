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
#include <arpa/inet.h>
#include <sys/stat.h>

#define BUFFER_LENGTH 100


int timeout_count = 0;

int create_new_socket(char *argv[]){
    struct addrinfo addr_index, *server_info,*p;
    int clientfd;
    socklen_t addr_len;
    int a,b,c;
    int rv;
    char z[5] = "";
    //generating the random port number
    a = 1025;
    b = rand()%64511;
    c = a+b;
    sprintf(z,"%d",c);//random port
    printf("New port number for the client is: %s\n", z);
    if ((rv = getaddrinfo((const char *)argv[1], (const char *)z , &addr_index, &server_info)) != 0) {
        return 1;
    }
    
    for(p = server_info; p != NULL; p = p->ai_next)//go through all potential socket
    {
        if ((clientfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
        {
            printf("cannot create socket for client\n");
            continue;
        }
        if (bind(clientfd, p->ai_addr, p->ai_addrlen) == -1)
        {
            close(clientfd);
            printf("connet bind socket for client\n");
            continue;
        }
        break;
    }
    return clientfd;
}

int main(int argc, char *argv[]) {
    struct addrinfo addr_index, *server_info, *p;
    int rv, sockfd, clientfd;
    int received_bytes;
    char *sending_buffer, *error_buffer;
    char received_buffer[BUFFER_LENGTH];
    struct sockaddr_storage client_addr;
    socklen_t addr_len;
    uint16_t opcode, opcode_FileError, blockno, errorcode_FileError;
    uint8_t split;
    split = htons(0);
    int count = 0;

    if (argc != 3) {the second argument is the ipaddress, the third argument is the port number
        printf("argument number wrong\n");
    }
    memset(&addr_index, 0, sizeof addr_index);
    //creating the server socket to bind
    addr_index.ai_family = AF_UNSPEC;
    addr_index.ai_socktype = SOCK_DGRAM;//UDP is sock_dgram,
    if ((rv = getaddrinfo((const char *)argv[1], (const char *)argv[2], &addr_index, &server_info)) != 0) {
        return 0;
    }
    for(p = server_info; p != NULL; p = p->ai_next) {//go through all potential socket
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {//create the socket
            printf("socket cannot create\n");
            continue;
        }
        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {//bind the socket as server socket fd
            close(sockfd);
            printf("socket cannot bind\n");
            continue;
        }
        break;
    }
    while(1) {//The loop for server main process, waiting for client connection and transfer
      printf("The server is waiting for the client request\n");
      addr_len = sizeof (client_addr);
      if ((received_bytes = recvfrom(sockfd, received_buffer, BUFFER_LENGTH-1 , 0, (struct sockaddr *)&client_addr, &addr_len)) == -1) {//Get the message from client, RRQ or ACK packet
          printf("recvfrom error !\n");
          exit(0);
      }
      if (received_bytes > 4){//This is RRQ packet, if equal 4 means ACK packet
          count++;//count the wrapped around sending packet
          //count = count++;//The block no count
          FILE *fp ;//open a new file stream
          fp = fopen ( received_buffer+2, "rb");
          error_buffer = malloc(200);//This is the buffer for sending the error message
          char errmsg[14] = "FILE NOT FOUND";
          if ( fp == NULL ) {// File not found Error, send the error message to client
              printf("Error!! File not found\n");
              printf("The opcode is 5\n");
              opcode_FileError = htons(5);//the opcode for error message is 5
              errorcode_FileError = htons(1);//the type of error is 1
              memcpy(error_buffer, (const char *)&opcode_FileError, sizeof(uint16_t));//the first two byte of message is opcode representindg the error message
              memcpy(error_buffer+sizeof(uint16_t), (const char *)&errorcode_FileError, sizeof(uint16_t));//the second two bytes of error message is the type of the message
              memcpy(error_buffer+(2*sizeof(uint16_t)), (const char *)errmsg, sizeof(errmsg));//the 3rd two bytes is the error message content, representing what is the error
              memcpy(error_buffer+(2*sizeof(uint16_t))+sizeof(errmsg), (const char *)&split, sizeof(uint8_t));
              sendto(sockfd, error_buffer, 19, 0, (struct sockaddr *)&client_addr, p->ai_addrlen)//send the error message to the client
              printf("The server has sent FILE NOT FOUND message\n");
          }
          else
          {// file exists situation, then we are going to fork the child process
              pid_t pid = fork();
              if (pid == 0) {  // this is the child process
                  printf("This is child process\n");
                  //find another port number for future communication
                  clientfd = create_new_socket(argv);//create a random port number socket
                  opcode =htons(3);//the good packet will have opcode as 3, means data
                  sending_buffer = malloc(516);//the buffer of sending message is 2 bytes of opcode, 2 bytes of block number, and 512 bytes of data, so is 516 bytes.
                  memcpy(sending_buffer, (const char *)&opcode, sizeof(uint16_t));// add the opcode in to the sending message
                  int i,j;
                  int d = 0;
                  j = 0;
                  int local_blockno = 0;

                  // the while loop for spliting the file in packet of 512 bytes and send each byte
                  while( !feof(fp))//keep sending until it is the end of file
                  {
                      unsigned char msg[512] = "";//the data of the packet will be 512 bytes
                      local_blockno++;//block number
                      for( i=0; i<=511; i++)// go through the file, and add 512 bytes of data of the file into our sending message
                      {
                          d = fgetc(fp);//get the data of the current file pointer
                          if ( d == EOF)//the end of file
                          {
                              break;
                          }
                          else//put what we read into the packet
                          {
                              msg[i] =(unsigned char) d;
                          }
                      }
                      blockno = htons(local_blockno);//get the block num
                      //buffer initialization
                      memcpy(sending_buffer+sizeof(uint16_t), (const char *)&blockno, sizeof(uint16_t));//add the block number into the sending message, which is wrapped around for different packet
                      memcpy(sending_buffer+(2*sizeof(uint16_t)), (const unsigned char *)msg, sizeof(msg));//add the data of the file, which is 512 long
                      sendto(clientfd, sending_buffer, i+4, 0, (struct sockaddr *)&client_addr, p->ai_addrlen)//sent to the client
                      printf(" waiting for ACK message from clients\n");
                      while(1)
                      {//The ACK while loop
                          struct timeval tv;  // implementing timeout with select()
                          fd_set readfds;
                          tv.tv_sec = 1;//The time of timeout, 1 sec
                          tv.tv_usec = 0;
                          FD_ZERO(&readfds);//empty the listening list of fd
                          FD_SET(clientfd, &readfds);//add the clientfd in to the reading fd
                          select(clientfd+1, &readfds, NULL, NULL, &tv);  // listen to the timeout, if the listneing fd has new message comming, then it pass the timeout.
                          if (FD_ISSET(clientfd, &readfds))
                          {  //pass time out as normal!
                              recvfrom(clientfd, received_buffer, BUFFER_LENGTH-1 , 0, (struct sockaddr *)&client_addr, &addr_len);//the potential ACK message from the client
                              uint16_t received_blockno;  // block number received
                              memcpy(&received_blockno,received_buffer+2, sizeof(uint16_t));//get the ack number from the received message, it has to match the local block number
                              printf("opcode is %d\nblock number is %d\naACK number is %d\n", *(received_buffer+1), ntohs(received_blockno), local_blockno);
                              if(*(received_buffer+1)==4)//if this is the ACK message. break
                              {
                                  break;
                              }
                          }
                          else
                          {  // time out, retransmission is required
                              timeout_count++;//calculate how much timeout we have waitied
                              printf("The count of timeout is %d\n",timeout_count);
                              if (timeout_count>10)//if the server wait for 10 timeouts, then will notice the client has left
                              {
                                  printf("The timeout count is over 10 times, the server close the file stream and child socket.\n");
                                  printf("The server is waiting for next client\n");
                                  exit(0);
                              }
                              sendto(clientfd, sending_buffer, i+4, 0, (struct sockaddr *)&client_addr, p->ai_addrlen);//retransmission
                          }
                      }
                  }//end of ack while loop
                  close(clientfd);  //
                  if(feof(fp)){
                      printf("tranfser complete \n");
                  }

              }//end of child process
              fclose(fp);
          }  // done with the file manipulation
      }  // done with the transmission
    }  // end of while loop
    return 0;
}
