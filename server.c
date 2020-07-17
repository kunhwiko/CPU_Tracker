/* 
This code primarily comes from 
http://www.prasannatech.net/2008/07/socket-programming-tutorial.html
and
http://www.binarii.com/files/papers/c_sockets.txt
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>

extern int count;
extern int quit;
extern double data[3600];
extern double avg_usage;
extern double max;
extern double total;
extern void* get_data();
extern void* quit_program();

pthread_mutex_t lock;
pthread_mutex_t lock2;

void* start_server(void* PORT_NUMBER)
{
    int port_num = *(int*) PORT_NUMBER;
    
    // structs to represent the server and client
    struct sockaddr_in server_addr,client_addr;    
      
    int sock; // socket descriptor

    // 1. socket: creates a socket descriptor that you later use to make other system calls
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Socket");
        exit(1);
    }

    int temp;
    if (setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&temp,sizeof(int)) == -1) {
        perror("Setsockopt");
        exit(1);
    }

    // configure the server
    server_addr.sin_port = htons(port_num); // specify port number
    server_addr.sin_family = AF_INET;         
    server_addr.sin_addr.s_addr = INADDR_ANY; 
    bzero(&(server_addr.sin_zero),8); 
      
    // 2. bind: use the socket and associate it with the port number
    if (bind(sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1) {
        perror("Unable to bind");
        exit(1);
    }

    // 3. listen: indicates that we want to listen to the port to which we bound; second arg is number of allowed connections
    if (listen(sock, 1) == -1) {
        perror("Listen");
        exit(1);
    }
          
    // once you get here, the server is set up and about to start listening
    printf("\nServer configured to listen on port %d\n", port_num);
    fflush(stdout);
    
    while(1) { // keep looping and accept additional incoming connections
        
        // 4. accept: wait here until we get a connection on that port
        int sin_size = sizeof(struct sockaddr_in);
        int fd = accept(sock, (struct sockaddr *)&client_addr,(socklen_t *)&sin_size);
        if (fd != -1) {
            printf("Server got a connection from (%s, %d)\n", inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port));

            // buffer to read data into
            char request[1024];

            // 5. recv: read incoming message (request) into buffer
            int bytes_received = recv(fd,request,1024,0);
            // null-terminate the string
            request[bytes_received] = '\0';
            // print it to standard out
            printf("REQUEST:\n%s\n", request);

            pthread_mutex_lock(&lock);
            double avg = 0.0;
            if(count >= 3600){
                avg = total / 3600;
            }else{
                avg = total / count;
            }

            // this is the message that we'll send back
            char* response = (char*)malloc(100 * sizeof(char));
            sprintf(response, "HTTP/1.1 200 OK\nContent-Type: text/html\n\n\
            <html>\
            <body>\
                <br>Input Threshold : \n</br>\
                <td colspan='4'><input id='display' name='display'></input></td>\
                <button id='submit'>Check!</button>\
                <p>\
                <br><span id = 'reading'>Latest Reading : %lf%%\n</span></br>\
                <br>Max Reading : %lf%%\n</br>\
                <br>Average Reading : %lf%%\n</br> \
                </p>\
                <script>\
                var value = 100;\
                var init = JSON.parse(localStorage.getItem('initial'));\
                if (init == 1 && JSON.parse(localStorage.getItem('val')) < %lf){\
                    document.getElementById('reading').style.color = 'red';\
                    alert('Threshold exceeded');\
                }\
                document.getElementById('submit').onclick = function(){\
                    value = document.getElementById('display').value;\
                    localStorage.setItem('initial',JSON.stringify(1));\
                    localStorage.setItem('val',JSON.stringify(value));\
                } \
                </script>\
            </body>\
            </html>", avg_usage,max,avg,avg_usage);
            pthread_mutex_unlock(&lock); 
          

            // 6. send: send the outgoing message (response) over the socket
            // note that the second argument is a char*, and the third is the number of chars 
            send(fd, response, strlen(response), 0);

            free(response);

            // 7. close: close the connection
            pthread_mutex_lock(&lock2); 
            if(quit == 1){
                close(fd);
                close(sock);
                printf("Server shutting down\n");
                pthread_mutex_unlock(&lock2); 
                exit(1);
            }else{
                close(fd);
                printf("Server closed connection\n");
                pthread_mutex_unlock(&lock2); 
            }      
        }
    }  
    return NULL;
} 


int main(int argc, char *argv[])
{
    int port_number = 3000; 
    return 0;
}
