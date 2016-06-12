#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <getopt.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>

#define BLOCK_SIZE 1024





pthread_mutex_t myMutex = PTHREAD_MUTEX_INITIALIZER;
//struct that will be passed in to the threads as argument
struct arg_struct{
    int tI;		//each thread has it's own index number
    char* host;	//host for each port
    int portNum;	//each port
    FILE * file;		//file pointer to write to file
    int inc;		//implementation detail
};

//function that will be passed into threads
void* client(void* n){
		//variables to 
	    struct arg_struct *args = n;	//argument to function
		int client_fd;					//client socket
		int rec;						//data received
		char rec_buf[BLOCK_SIZE];		//buffer that receives data
		struct sockaddr_in serv_addr;	//sock structs
		
		FILE* toWrite = args->file;		//file pointer from argument to write to
		char send_buf[128];				//string buffer to request 
	    sprintf(send_buf, "%d", args->tI);		//turns int into string for chunk request from server
	    
  		
	    while(1){
	   
	   //create socket
  		 if((client_fd = socket(AF_INET, SOCK_STREAM, 0))<0){
    		printf("\n Could not create socket \n");
  	  	return;
  		}
  		
   		 serv_addr.sin_family = AF_INET;
  		 serv_addr.sin_port = htons(args->portNum);
   		 serv_addr.sin_addr.s_addr = inet_addr(args->host);
    	//connect to socket
   		if(connect(client_fd, (struct sockaddr *)&serv_addr,sizeof(serv_addr))<0){
    	printf("Could not connect \n");
    	return;
    	}
    	//send server the chunk you want
        send(client_fd, send_buf, 128, 0);
        //receive the chunk
    	rec  = recv(client_fd, rec_buf, sizeof(rec_buf), 0);
        //check if received no chunk == end of file
	    if(rec<=0){
	    	break;
	    }
	    //check if last chunk, if so then write only that size of chunk
	    if(rec<1024){
	    	
	    	char rec_buf_2[rec];
	    	strncpy (rec_buf_2,rec_buf, rec);
	    	fseek(args->file, args->tI * BLOCK_SIZE, SEEK_SET);
	    	fwrite(rec_buf_2, rec, 1,args->file);
	    	//printf("%s",rec_buf_2); IGNORE
	    	break;
	    	
	    	
	    }
	    //create lock for exclusive access to file
	    pthread_mutex_lock(&myMutex);
	    //printf("TI = %d\n", args->tI); IGNORE
	    fseek(args->file, args->tI * BLOCK_SIZE, SEEK_SET);
	    fwrite(rec_buf, BLOCK_SIZE, 1,args->file);
	    pthread_mutex_unlock(&myMutex);
	    //increment so same chunk is not downloaded
	    args->tI = args->tI + args->inc;
	    
	    //printf("send _ buf %s\n", send_buf); IGNORE
	    
    	//change int to string
    	sprintf(send_buf, "%d", args->tI);
    	//close client so can be opened again
    	 close(client_fd);
		}
    
    
   
    
}


int main(int argc, char *argv[]){
	//make sure a port and host is provided
	if (argc < 3) {
		printf("Need to supply a host and port!\n");
		exit(0);
    }
   //calculate number of ports
    int numOf = (argc-1)/2;	
    struct arg_struct args[numOf]; //make number of argument structs
    pthread_t threads[numOf];
    //create file to write to
    FILE *f;
    f = fopen("output.txt", "w");
    //assign hosts to struct array
    int i = 0;
    int j = 0;
    for(i=1; i<=numOf*2; i=i+2){
    	args[j].host = argv[i];
    	
    	j++;
    }
    j=0;
    //assign portNum to struct array
    for(i=2; i<=(numOf*2)+1; i=i+2){
    	args[j].portNum = atoi(argv[i]);
    	j++;
    }
    //assign increment and file to struct array and create threads
    for(i = 0; i<numOf; i++){
    	args[i].tI = i;
    	args[i].file = f;
    	args[i].inc = numOf;
    	pthread_create(&threads[i], NULL, &client, (void *)&args[i]);	
    	
    	
    }
    
   
    //join threads to make sure completion
    for(i=0; i<numOf; i++){
    	pthread_join(threads[i], NULL);
    }
    	
    //close file
    fclose(f);	
    	
    	
    
   
  
   
   //end program
    return 0;
}