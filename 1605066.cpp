#include <iostream>
#include <cstdio>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#include <cstring>
#include <time.h>
#include <queue>

using namespace std;

#define number_of_serviceBooth 3 // array be array of BinarySemaphore/mutex (# of customer that can contain in Service room)
#define number_of_cycle 10        // act like a Thread (# of customer)
#define payementBooth 2          // # of customer that can contain in payment room
#define INFINITY 9999

pthread_mutex_t mutex, payMutex, rdMutex;
sem_t service_array[number_of_serviceBooth]; //decide which one is currently occupied
sem_t sAvailablePayBooth;                    //# currently kotojon payment nite parbe
sem_t qLock;

queue<void *> paymentQ;   // before entering into paymentroom for payment
queue<void *> departureQ; // before entering into serviceroom for departure
int paymentComplete = 0;

int readCount = 0, writeCount = 0;
pthread_mutex_t rcs, wcs; //mutex
sem_t s_rd, s_wrt;

//act like a reader
void *consume_service(void *arg)
{

    //readerCode Start
    //shobai jen s_rd te wait na kore tai er age ekta lock nibo
    pthread_mutex_lock(&rdMutex);
    sem_wait(&s_rd);          //decrease
    // printf("%s Enter into Service Room\n", (char *)arg); fflush(stdout);
    pthread_mutex_lock(&rcs); //lock rcs
    readCount++;
    if (readCount == 1){
        sem_wait(&s_wrt); //decrease, initiazlized by 1
    }
    pthread_mutex_unlock(&rcs); //unlock rcs
    //sem_post(&s_rd);            //increase--->later need to decide correct position
    //readerCode End
    //Now nicher code -->read Resource

    for (int i = 0; i < number_of_serviceBooth; i++){
        int serviceMan = i + 1;
        sem_wait(&service_array[i]); ///decrease
        printf("%s started taking service from serviceman %d\n", (char *)arg, serviceMan); fflush(stdout);
        sleep(rand()%3);
        printf("%s finished taking service from serviceman %d\n", (char *)arg, serviceMan); fflush(stdout);
        sem_post(&service_array[i]); ///increase
        
        //readerCode Start
        pthread_mutex_lock(&rcs);
        if (serviceMan==1)
        { 
            sem_post(&s_rd); //increase
            pthread_mutex_unlock(&rdMutex);  //আগে এটা s_rd এর নীচেছিল
        }
        pthread_mutex_unlock(&rcs);
    }
    // printf("%s Left Service Room\n", (char *)arg); fflush(stdout);

    pthread_mutex_lock(&mutex);
    paymentQ.push((char *)arg); //leaving serviceroom,cycle now is in the queue for entering into paymentRoom
    pthread_mutex_unlock(&mutex);
    // printf("Q UPPED\n");

    //readerCode Start
        pthread_mutex_lock(&rcs); //lock rcs
        readCount--;
        if (readCount == 0)
        {
            sem_post(&s_wrt); //increase
        }
        pthread_mutex_unlock(&rcs); //unlock rcs
    sem_post(&qLock); //increase,Binary SP--initialized by 0
}

//act like a writer
void *paymentProcess(void *arg){
    while (1){
        //case 1:jokhon last 1 ta item quesu theke ber korbo + duijon qLock_Down call...porertar jnno deadLock
        //case 2: at same time last two threa mutex down call..for one paymentComplete=5 , for anothre one paymentComplete=6
        pthread_mutex_lock(&payMutex);
        paymentComplete = paymentComplete + 1;
        // printf("paymentComplete: %d\n", paymentComplete);
        if (paymentComplete > number_of_cycle){
            // printf("ALL PAYMENT COMPLETE\n");
            pthread_mutex_unlock(&payMutex);
            break;
        }
        pthread_mutex_unlock(&payMutex);
        sem_wait(&qLock); //decrease,initialized by 0

        //WriterCode Start
        //Now nicher code -->Write Resource
        

        sem_wait(&sAvailablePayBooth); ///decrease
        
        pthread_mutex_lock(&mutex);

        void *poppedCycle = paymentQ.front(); //Empty kina check kora lagbe
        paymentQ.pop();
        printf("%s started paying the service bill\n", (char *)poppedCycle); fflush(stdout);
        sleep(rand()%3);
        pthread_mutex_unlock(&mutex);
        // printf("%s Left Payment room\n", (char *)poppedCycle); fflush(stdout);
        // printf("%s finished paying the service bill\n", (char *)poppedCycle); fflush(stdout); //--> now in 127
        sem_post(&sAvailablePayBooth); ///increase,
        
        pthread_mutex_lock(&wcs); //lock wcs
        writeCount = writeCount + 1;
        // printf("COUNT UP---:%d\n",writeCount);
        if (writeCount == 1){
            sem_wait(&s_rd); //decrease
        }
        pthread_mutex_unlock(&wcs); //unlock wcs
        
        sem_wait(&s_wrt);           //decrease
        //WriterCode End
        printf("%s finished paying the service bill\n", (char *)poppedCycle); fflush(stdout); // from--> 114 
        sleep(rand()%2);
        // printf("%s has departed\n", (char *)poppedCycle); fflush(stdout);  //--> now in 136
        sem_post(&s_wrt);         //increase
        
        
        
        //WriterCode Start
        pthread_mutex_lock(&wcs); //lock wcs
        printf("%s has departed\n", (char *)poppedCycle); fflush(stdout);  // from--> 129
        writeCount = writeCount-1;
        // printf("COUNT DOWN---:%d\n",writeCount);
        if (writeCount == 0){
            sem_post(&s_rd); //increase
        }
        pthread_mutex_unlock(&wcs); //unlock wcs
        //WriterCode End   
    }
}

void init_semaphore()
{
    int res;
    for (int i = 0; i < number_of_serviceBooth; i++)
    {
        res = sem_init(&service_array[i], 0, 1);
        if (res != 0)
        {
            printf("Failed\n"); fflush(stdout);
        }
    }

    res = pthread_mutex_init(&mutex, NULL);
    if (res != 0)
    {
        printf("Failed\n"); fflush(stdout);
    }

    res = sem_init(&sAvailablePayBooth, 0, payementBooth);
    if (res != 0)
    {
        printf("Failed\n"); fflush(stdout);
    }
    res = sem_init(&qLock, 0, 0);
    if (res != 0)
    {
        printf("Failed\n"); fflush(stdout);
    }

    res = pthread_mutex_init(&payMutex, NULL);
    if (res != 0)
    {
        printf("Failed\n"); fflush(stdout);
    }

    //Reader-writter code er jnno nicher gula
    res = pthread_mutex_init(&rcs, NULL);
    if (res != 0)
    {
        printf("Failed\n"); fflush(stdout);
    }

    res = pthread_mutex_init(&wcs, NULL);
    if (res != 0)
    {
        printf("Failed\n"); fflush(stdout);
    }
    
    res = pthread_mutex_init(&rdMutex, NULL);
    if (res != 0)
    {
        printf("Failed\n"); fflush(stdout);
    }

    res = sem_init(&s_rd, 0, 1);
    if (res != 0)
    {
        printf("Failed\n"); fflush(stdout);
    }

    res = sem_init(&s_wrt, 0, 1);
    if (res != 0)
    {
        printf("Failed\n"); fflush(stdout);
    }
}

int main()
{

    int res;
    pthread_t consumer_cycle_thread[number_of_cycle];
    pthread_t payment_thread[payementBooth];
    init_semaphore();

    //creating thread for all the cycles
    for (int i = 0; i < number_of_cycle; i++)
    {
        char *id = new char[3];
        strcpy(id, to_string(i + 1).c_str());

        res = pthread_create(&consumer_cycle_thread[i], NULL, consume_service, (void *)id);

        if (res != 0)
        {
            printf("Consumer_cycle_thread creation failed\n"); fflush(stdout);
        }
    }
    //creating thread for paymentRoom
    for (int i = 0; i < payementBooth; i++)
    {
        char *id = new char[3];
        strcpy(id, to_string(i + 1).c_str());
        res = pthread_create(&payment_thread[i], NULL, paymentProcess, (void *)id);
        if (res != 0)
        {
            printf("Payment_thread creation failed\n"); fflush(stdout);
        }
    }
    //---------Thread Joining---------
    for (int i = 0; i < number_of_cycle; i++)
    {
        void *result;
        pthread_join(consumer_cycle_thread[i], &result);
        // printf("%s",(char*)result);
    }
    for (int i = 0; i < payementBooth; i++)
    {
        void *result;
        pthread_join(payment_thread[i], &result);
    }

    res = pthread_mutex_destroy(&mutex);
    if (res != 0)
    {
        printf("Failed\n"); fflush(stdout);
    }
    res = sem_destroy(service_array);
    if (res != 0)
    {
        printf("Failed\n"); fflush(stdout);
    }
    res = sem_destroy(&sAvailablePayBooth);
    if (res != 0)
    {
        printf("Failed\n"); fflush(stdout);
    }
    res = sem_destroy(&qLock);
    if (res != 0)
    {
        printf("Failed\n"); fflush(stdout);
    }

    res = pthread_mutex_destroy(&payMutex);
    if (res != 0)
    {
        printf("Failed\n"); fflush(stdout);
    }

    return 0;
}
