#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

/* 
 * count is number of readings since start
 * avg_usage indicates average usage of CPU
 * data shows most recent 3600 readings (1 hour reading)
 * max shows highest reading so far
 * total is sum of readings
 */

int count = 0;
double data[3600];
double avg_usage = 0.0;
double max = 0.0;
double total = 0.0;

extern pthread_mutex_t lock;
extern pthread_mutex_t lock2;
int quit = 0;

void* get_data(){  
    /* 
     * init indicates whether this is first reading
     * prev is previous CPU idle time
     * curr is current CPU idle time
     */
    int init = 1;
    double prev = 0.0;
    double curr = 0.0;
    
    /* continue to print out result every 1 sec */
    while(1){
        pthread_mutex_lock(&lock2); 
        if(quit == 1){
            pthread_mutex_unlock(&lock2); 
			break;
        }
        pthread_mutex_unlock(&lock2); 
        
        /* only update prev if curr exists */
        if(init == 0) prev = curr;        
        
        /* read from proc/stat */ 
        char* filename = "/proc/stat";
        char fileline[100];
        FILE* fp = fopen(filename,"r");
        
        /* read the first line of proc/stat */
        fgets(fileline,100,fp);

        /* get the 4th number in the line */
        const char delim[2] = " ";
        char* token = NULL;
        token = strtok(fileline,delim);

        for(int i = 0; i < 4; i++){    
            token = strtok(NULL,delim);
        }   
        
        /* convert token to double */
        curr = atof(token);
        
        /* if this is the first time reading, continue */
        if (init == 1){
            init = 0;
            sleep(1);
            continue;
        }
        
        pthread_mutex_lock(&lock); 
        /* if prev data exists, calculate average usage of CPU */
        avg_usage = 100 - ((curr - prev) / 4);
        
        /* record the maximum usage */
        if(avg_usage > max)
            max = avg_usage;
        
        /* print avg_usage, 0, or 100 to console */
        if(avg_usage < 0){
            printf("%lf%%\n",0.0);
        }else if(avg_usage > 100){
            printf("%lf%%\n",100.0);
        }else{
            printf("%lf%%\n",avg_usage);
            
            /* if more than an hour passed, remove in FIFO order */
            data[count % 3600] = avg_usage;           
            if (count >= 3600){
                total -= data[count % 3600];
            }
            total += avg_usage;
            count++;
        }
        pthread_mutex_unlock(&lock); 
    
        /* sleep for 1 sec and repeat */
        fclose(fp);
        sleep(1);
    } 
    return NULL;
}

void* quit_program() {
    char str[50];
    
    while(strcmp(str,"q") != 0){
        printf("Press 'q' to end : \n");
        scanf("%s",str);        
    }   
    pthread_mutex_lock(&lock2); 
    quit = 1;
    pthread_mutex_unlock(&lock2); 
    return NULL;    
}
