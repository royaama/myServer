#include "segel.h"
#include "request.h"
#include "Queue.h"
// 
// server.c: A very, very simple web server
//
// To run:
//  ./server <portnum (above 2000)>
//
// Repeatedly handles HTTP requests sent to this port number.
// Most of the work is done within routines written in request.c
//



//Queues declaration
Queue* waiting_requests_queue;
Queue* handling_requests_queue;

//conditon variables declaration
pthread_mutex_t lock1;
pthread_cond_t not_empty;
pthread_mutex_t lock2;
pthread_cond_t not_full;

// HW3: Parse the new arguments too
void getargs(int *port, int argc, char *argv[])
{
    if (argc < 5) {
	fprintf(stderr, "Usage: %s <portnum> <threads> <queue_size> <schedalg>\n", argv[0]);
	exit(1);
    }
    *port = atoi(argv[1]);
}

void drop_head(){
    struct timeval end;
    gettimeofday(&end, NULL);
    Close(waiting_requests_queue->head->data->confd);
    free(waiting_requests_queue->head->data);
    timersub(&end,&waiting_requests_queue->head->data->arrive,&waiting_requests_queue->head->data->dispatch);
    removeFromQueue(waiting_requests_queue, waiting_requests_queue->head);

}

void drop_random(){
    int new_size = 0.5*(waiting_requests_queue->size + 1);
    while(waiting_requests_queue->size > new_size){
    struct timeval end;
    gettimeofday(&end, NULL);
    int index =rand()%(waiting_requests_queue->size);
    Request* current_request = getByRandomIndex(waiting_requests_queue, index);
    timersub(&end,&current_request->arrive,&current_request->dispatch);
    removeByRandomIndex(waiting_requests_queue, index);
    }
}
void add_new_req_to_the_server(struct timeval request_arrival_time,int connfd){
            //adding the new request:
        Request* new_request = (Request*)malloc(sizeof(Request));
        new_request->arrive = request_arrival_time;
        new_request->confd = connfd;
        pthread_mutex_lock(&lock1);
        insertBack(waiting_requests_queue,new_request);
        pthread_cond_signal(&not_empty);
        pthread_mutex_unlock(&lock1);

}


void cleanAllRequestsFromQueue(){
    delete_handling_queue(handling_requests_queue);
    delete_waiting_queue(waiting_requests_queue);
}

void cleanAllThreads(pthread_t* threads, int numberOfThreads){
    for(int i = 0; i< numberOfThreads; i++){
        pthread_join(threads[i], NULL);
    }
    free(threads);
}
//thread routine
void* work_routine(void* new_id){
    int thread_id = *((int*)new_id);
    struct Statistics* stats = (struct Statistics*) malloc(sizeof(*stats));

    stats->thread_count = 0;
    stats->thread_dynamic_count = 0;
    stats->thread_static_count = 0;
    stats->thread_id = thread_id;

    struct timeval start_time;
    while(1){
        pthread_mutex_lock(&lock1);
        //start of critical section1
        while(waiting_requests_queue->size == 0){
            pthread_cond_wait(&not_empty, &lock1);
        }
        //getting the request from the waiting queue
        Request* new_request = getHead(waiting_requests_queue)->data;

        dropHead(waiting_requests_queue);
        //pthread_cond_signal(&not_full);//added
        gettimeofday(&start_time,NULL);
        timersub(&start_time,&new_request->arrive,&new_request->dispatch);
        stats->arrival_time = new_request->arrive;
        stats->dispatch_time = new_request->dispatch;

        //inserting the new request to the handling queue
        node* new_request_node = insertBack(handling_requests_queue, new_request);
       /* gettimeofday(&start_time,NULL);
        timersub(&start_time,&new_request->arrive,&new_request->dispatch);
        stats->arrival_time = new_request->arrive;
        stats->dispatch_time = new_request->dispatch;*/
        //end of critical section1
        pthread_mutex_unlock(&lock1);
        
        requestHandle(new_request->confd, stats);

        Close(new_request->confd);
        free(new_request);

        pthread_mutex_lock(&lock1);
        //start of critical section2
        removeFromQueue(handling_requests_queue, new_request_node);
        pthread_cond_signal(&not_full);
        //end of critical section2
        pthread_mutex_unlock(&lock1);

    }
    free(stats);

}


int main(int argc, char *argv[])
{
    int listenfd, connfd, port, clientlen, numberOfThreads, sizeOfQueue;
    struct sockaddr_in clientaddr;
    char policy[7];
    //get arguments:
    getargs(&port, argc, argv);
    numberOfThreads = atoi(argv[2]);
    sizeOfQueue = atoi(argv[3]);
    strcpy(policy, argv[4]);
    struct timeval request_arrival_time;

    //initiation for condition vars and queues
    waiting_requests_queue = initQueue();
    handling_requests_queue = initQueue();
    int condition_block = !strcmp(policy, "block");
    int condition_drop_tail = !strcmp(policy, "dt");
    int condition_drop_head = !strcmp(policy, "dh");
    int condition_block_flush = !strcmp(policy, "bf");
    int condition_drop_random = !strcmp(policy, "random");


    pthread_cond_init(&not_empty, NULL);
    pthread_cond_init(&not_full, NULL);
    pthread_mutex_init(&lock1, NULL);
    pthread_mutex_init(&lock2, NULL);
    

    // 
    // HW3: Create some threads...
    //
    pthread_t* threads = (pthread_t*) malloc(sizeof(pthread_t)*numberOfThreads);
    for(int i = 0; i < numberOfThreads; i++){
        pthread_create(&threads[i], NULL, work_routine, &i);
    }

    listenfd = Open_listenfd(port);
    while (1) {
        clientlen = sizeof(clientaddr);
        connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *) &clientlen);
        gettimeofday(&request_arrival_time, NULL);

        if((handling_requests_queue->size + waiting_requests_queue->size) >= sizeOfQueue){
            if(condition_block){
                pthread_mutex_lock(&lock2);
                while((handling_requests_queue->size + waiting_requests_queue->size) >= sizeOfQueue){
                    pthread_cond_wait(&not_full, &lock2);
                }
                pthread_mutex_unlock(&lock2);
            } 
            else if(condition_drop_tail){
                Close(connfd);
                continue;
            }
            else if(condition_drop_head){
                if(waiting_requests_queue->size){
                    pthread_mutex_lock(&lock1);
                    drop_head();
                    pthread_mutex_unlock(&lock1);
                    
                } else{
                    Close(connfd);
                    continue;
                }
            }
            else if(condition_block_flush){
                pthread_mutex_lock(&lock2);
                while(handling_requests_queue->size > 0){//waiting_requests_queue->size > 0 &&
                    pthread_cond_wait(&not_empty,&lock2);
                }
                pthread_mutex_unlock(&lock2);
                Close(connfd);
                continue;
            }
            else if(condition_drop_random){
                if(handling_requests_queue->size >= sizeOfQueue){
                    Close(connfd);
                    continue;
                }
                else{
                    pthread_mutex_lock(&lock1);
                    drop_random();
                 //   pthread_cond_signal(&not_full);//added
                    pthread_mutex_unlock(&lock1);
                }
            }
        }
        add_new_req_to_the_server( request_arrival_time,connfd);
    }

    cleanAllRequestsFromQueue();
    cleanAllThreads(threads, numberOfThreads);

}


    


 
