#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>

// Global constant variables
int BUFFER_SIZE; // Size of the shared buffer
int NUM_MASTERS;
int NUM_WORKERS;
int NUM_ITEMS; // Number of items collectively produced by ALL producers

// Initialize shared buffer
int *buffer;
int produce_count = 0; // Number of items produced
int produce_idx = 0;   // Points to where to produce/put new item
int consume_idx = 0;   // Points to where to consume/get new item

// Synchronization variables
sem_t mutex;
sem_t empty;
sem_t full;

// Producer confirmation message
void print_produced(int num, int master)
{

  printf("Produced %d by master %d\n", num, master);
}

// Consumer confirmation message
void print_consumed(int num, int worker)
{

  printf("Consumed %d by worker %d\n", num, worker);
}

// Producer thread
void *producer_thread(void *data)
{
  int thread_id = *((int *)data);

  while (produce_count < NUM_ITEMS)
  {

    // TODO: Implement the wait condition when the buffer is full
    sem_wait(&empty);
    sem_wait(&mutex); // Lock

    buffer[produce_idx] = produce_count;
    print_produced(produce_count, thread_id);
    produce_idx = (produce_idx + 1) % BUFFER_SIZE;
    produce_count++;

    // TODO: Signal the consumers that buffer is not empty
    sem_post(&mutex); // Unlock
    sem_post(&full);
  }

  return 0;
}

// write function to be run by worker threads
// ensure that the workers call the function print_consumed when they consume an item
void *consumer_thread(void *data)
{
  int thread_id = *((int *)data);

  for (int i = 0; i < (NUM_ITEMS) / NUM_WORKERS; i++)
  {

    // TODO: Implement the wait condition when the buffer is empty
    sem_wait(&full);
    sem_wait(&mutex); // Lock

    int item = buffer[consume_idx];
    print_consumed(item, thread_id);
    consume_idx = (consume_idx + 1) % BUFFER_SIZE;

    // TODO: Signal the consumers that buffer is not full
    sem_post(&mutex); // Unlock
    sem_post(&empty);
  }

  return 0;
}

int main(int argc, char *argv[])
{
  int *master_thread_id, *worker_thread_id;
  pthread_t *master_thread, *worker_thread;

  int i;

  if (argc < 5)
  {
    printf("./master-worker #total_items #max_buf_size #NUM_WORKERS #masters e.g. ./exe 10000 1000 4 3\n");
    exit(1);
  }
  else
  {
    NUM_MASTERS = atoi(argv[4]);
    NUM_WORKERS = atoi(argv[3]);
    NUM_ITEMS = atoi(argv[1]);
    BUFFER_SIZE = atoi(argv[2]);
  }

  buffer = (int *)malloc(sizeof(int) * BUFFER_SIZE);

  // Initialize semaphores
  sem_init(&mutex, 0, 1);
  sem_init(&full, 0, 0);
  sem_init(&empty, 0, BUFFER_SIZE);

  // create master producer threads
  master_thread_id = (int *)malloc(sizeof(int) * NUM_MASTERS);
  master_thread = (pthread_t *)malloc(sizeof(pthread_t) * NUM_MASTERS);
  for (i = 0; i < NUM_MASTERS; i++)
    master_thread_id[i] = i;

  for (i = 0; i < NUM_MASTERS; i++)
    pthread_create(&master_thread[i], NULL, producer_thread, (void *)&master_thread_id[i]);

  // create worker consumer threads
  worker_thread_id = (int *)malloc(sizeof(int) * NUM_WORKERS);
  worker_thread = (pthread_t *)malloc(sizeof(pthread_t) * NUM_WORKERS);
  for (i = 0; i < NUM_WORKERS; i++)
    worker_thread_id[i] = i;

  for (i = 0; i < NUM_WORKERS; i++)
    pthread_create(&worker_thread[i], NULL, consumer_thread, (void *)&worker_thread_id[i]);

  // wait for all master threads to complete
  for (i = 0; i < NUM_MASTERS; i++)
  {
    pthread_join(master_thread[i], NULL);
    printf("master %d joined\n", i);
  }

  // wait for all worker threads to complete
  for (i = 0; i < NUM_WORKERS; i++)
  {
    pthread_join(worker_thread[i], NULL);
    printf("worker %d joined\n", i);
  }

  /*----Deallocating Buffers---------------------*/
  free(buffer);
  free(master_thread_id);
  free(master_thread);
  free(worker_thread_id);
  free(worker_thread);

  return 0;
}
