////  hw1_client.c
  //
//

#include "stdio.h"

#include "sys/socket.h"

#include "netinet/in.h"

#include "unistd.h"	//remove the warning from close()

#include "stdlib.h" 	//remove the warning from the exit()

#include "string.h" 	//remove the warning from the memset()

#include "arpa/inet.h"	//remove the warning from inet_pton() as a implicit declaration

#define MAXLINE 1024


#include "errno.h"
//must put calling function at the front of main, otherwise there will be declaration warning in main()
int readline(int fd, char *vptr, size_t maxlen)//fd is the socket descriptor, *vptr is the pointer to the buffer, and the maxlen is the (maximum size of a line +1). And this will return a integer number: -1 means error, other numbers means the number of bytes
{
    int number_bytes = 0;//count the number of bytes we read
    int read_status = 0; //what is the status of our read() from the socket
    char client_buffer_read;//what we read from the socket is stored here.
    
    for (int n = 1; n < maxlen; n++) //the loop of reading each charactor in the line, until it reach the maxsize
    {
    again:
        read_status = recv(fd,&client_buffer_read,1,0);//read one character from the socket
        if ((read_status) > 0)
        {
            vptr[number_bytes] = client_buffer_read;//store this charactor into the message buffer
            number_bytes++;//the count of success bytes increase by 1
            if (client_buffer_read == '\n'){
                break;}//the newline charactor is detected, so break the loop and return the current bytes count
        }	else if (read_status == 0) {//EOF situation, getting zero charactoers return from a call
            client_buffer_read = 0;//to empty the reading storage buffer like fgets() does
            return (number_bytes);
        }	else {
            if (errno == 4){//error == 4 means the error is the EINGTER error from recv(), then read again. EINTER occurs when a process is blocked in a system call.
                goto again;}
            else{return -1;}//if other error, the nreturn a -1
        }
    }
    client_buffer_read = 0;
    return number_bytes;
}
int writen(int fd, char *vptr, size_t n, int t)//writen only write n (as the third argument) bytes to a socket and return the number of bytes of a (-1) on error
{   //n-=1;
    int index = n;
    int loop = t;
    printf("n is %d\n",index);
    char *ptr;//the pointer the the writing buffer
    ptr = vptr;
    int write_status = 0;
    int number_bytes_write = 0;
    //printf("going to write: %s\n",ptr);
    for (int i = 0;i<loop;i++){
    again:
        write_status = write(fd,ptr,1);//send one message with n bytes. The write() will return the number of bytes it sent which may not reach the n we assigned, or -1 for error, or EINTER for process inturrption. !!must not use &ptr, since ptr is already the pointer for me.
        if ((write_status) == -1){
            if ((errno) == 4){goto again;}//its not a error, but a signal interruption
            else{return -1;}//there`s error!
        }else{number_bytes_write += write_status;}//the count of written bytes
        i += write_status;//i become the number of bytes written
        write_status = 0;//refresh for next status
    }
    return number_bytes_write;//minus the "\n", just count the byte sent
    
    
}

//ssize_t readline(int fd, void *vptr, size_t maxlen);
//ssize_t writen(int fd, const void *vptr, size_t n);

int main(int argc,char *argv[])		//argv[0] is file name, argv[1] is the ip address number, argv[2] is the port number,  argc is the number of argv

{
    
	if (argc != 3)
	{
        
		printf("error! Proper usage is: (client file)Name IPAdr Port\n"); 	//check whether we set echo, ip address and port number
    
	}
    
	printf("argv0: %s\n", argv[0]);
    
	printf("argv1: %s\n", argv[1]);
    
	printf("argv2: %s\n", argv[2]);

    
	char client_buffer[MAXLINE];
    
	int IP_port;
    
	sscanf(argv[2], "%d", &IP_port);	//convert the char on the screen(stdin) to integer
    
	printf("the port number: %d\n", IP_port);
 
   
	char sendline[MAXLINE];
    
	char *IP_address;	//char and char * is different, since 127.0.0.1 is char * type
    
	IP_address = argv[1];
    
	printf("the IPv4 address: %s\n", IP_address);


    
    //create a socket
    
	int socket_fd = socket(AF_INET, SOCK_STREAM, 0); 	//choose AF_INET for ipv4, and sock_stream for TCP
    
	if (socket_fd == -1)
	{ 	//if socket return the value -1, that means failure and no socket is created
        
		printf("FAIL: cannot create a socket");
    
	}
    
	printf("socket created, ready for socket setting\n");
    
    
	
	//socket infomation initial setting
    
	struct sockaddr_in socket_address_ipv4; 	//create one sockaddr_in socket type, which is the IPV4 structure
    
	memset(&socket_address_ipv4, 0, sizeof(socket_address_ipv4));	//add 0 to make sure it is an clean space for sockaddr_in
    
	socket_address_ipv4.sin_family = AF_INET;	//this is IPV4 socket reuiqred
    
	socket_address_ipv4.sin_port = htons(IP_port);	//assign the port number, the sockaddr_in reuiqred the port is network byte ordered
    
	
	if ((inet_pton(AF_INET, IP_address, &(socket_address_ipv4.sin_addr))) < 0)
	{	//get IP address from typing in the command line, from char to const char *, we need & to specify the pointer; 
		//& is used for point to the the location of char, because ip_address is already a char * with pointer, so no need to point again.
        
		printf("FAIL: error for the presentation format");
    
	}	//socket_address_ipv4.sin_addr.s_addr = inet_pton("127.0.0.1") is no longer used, 
		//since inet_pton is not supporting ipv6;assign the IP address, using 127.0.0.1 to connect the local.
    
		printf("socket setting finished, ready for connecting\n");
    
    

	//connect()
    
	int connection_index = connect(socket_fd, (struct sockaddr*)&socket_address_ipv4, sizeof(socket_address_ipv4));
		//socket_fd is the decription of this socket, the third argument is the lenghth of this socket which is the size of what we defined. 
		//must use (struct sockaddr*)& to turn the sockadr_in into 'const struct sockaddr*'
    
	if ((connection_index) == -1)
	{
        
		printf("FAIL: cannot connect the socket\n");
    
	}
    
	printf("socket connected, ready for msg sending\n");
 
   
	
    
	//send
	printf("msg to be sent to server: \n");
    memset(&sendline,0,sizeof(sendline));
/*
	fgets(sendline, MAXLINE, stdin);	//get what we type in the command line, the first argument is the buffer for message reading, 
		//the second argument is the number of message, the third is the source of this message, which is the command line here.
        
	printf("\n");
	if ((send(socket_fd, sendline, strlen(sendline), 0)) < 0)
	{	//socket_fd is the descriptor of the socket, sendline is the buffer, MAXLINE is the lenth of the buffer, 0 is the flags
        
		printf("FAIL: error when sending message\n ");
	}
    memset(&client_buffer, 0, sizeof(client_buffer));
    
	int msg_recv = recv(socket_fd, client_buffer, sizeof(client_buffer), 0);        
	client_buffer[msg_recv] = 0;	
		//the msg_recv is the number of tuples that client receives, so it can determine the terminating point in the buffer
        
	printf("the msg echoed back from the server is: ");

        
	fputs(client_buffer, stdout);	//put the message in the buffer to the screen
    
	if ((msg_recv) == -1)
	{	//when the return value is -1, means it cannot receive message
        
		printf("FAIL: cannot receive the msg from the server");
        
    }
*/
    //while (fgets(sendline, MAXLINE, stdin); //!= NULL)
    //read file
    /*FILE *file_read = fopen("test.txt","r");//open a file
    fgets(sendline,10,file_read);//read the file to the buffer
    printf("%s\n",sendline);
    fclose(file_read);*/
    //typing
    while(1){
        printf("The Maximum lenth of line is 10, which will include 8 bytes of data + 1 byte of ");
    fgets(sendline,1024,stdin);//the actual size is data+"\n"+"\0", so the lenth of data is 8
    //writen
        int index_w = writen(socket_fd, sendline, strlen(sendline),10);
    printf("the return value of writen is: %d\n",index_w-1);//minus the "\n"
    if (index_w == -1){//-1 means error!
        printf("error! message sending failed because the return value of writen is -1\n");}
    else{printf("the client sent %d bytes data\n",index_w-1);}//how many bytes we send back,//minus the "\n"
    //readline
    int index_r = readline(socket_fd,client_buffer,10);
    printf("the return value of readline is: %d\n",index_r);
    if (index_r == -1){//-1 means error!
        printf("error! message recving failed because the return value of readline is -1\n");}
    else{printf("the client received %d bytes data from server\n",index_r);}//how many bytes we send back
    printf("the data from the server is: ");
	fputs(client_buffer, stdout);
        printf("\n");}
	//}
   /* while(1){
        memset(&sendline,0,sizeof(sendline));//clean the writting buffer
        fgets(sendline,10,stdin);
        if (strcmp(sendline,"d")==0){
            close(socket_fd);
            printf("socket closed\n");
        }	//close the socket}
        
    }*/
    //fgets(sendline,10,stdin);
    close(socket_fd);
	printf("echoed msg received, socket closed");
	exit(0);

}

