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
#define number_of_cycle 5        // act like a Thread (# of customer)
#define payementBooth 2          // # of customer that can contain in payment room
#define INFINITY 9999

pthread_mutex_t mutex, payMutex;
sem_t service_array[number_of_serviceBooth]; //decide which one is currently occupied
sem_t sAvailablePayBooth;                    //# currently kotojon payment nite parbe
sem_t qLock, pLock;

queue<void *> paymentQ;   // before entering into paymentroom for payment
queue<void *> departureQ; // before entering into serviceroom for departure
int paymentComplete = 0;

int readCount=0, writeCount=0;
pthread_mutex_t rcs,wcs; //mutex
sem_t s_rd,s_wrt;

void *consume_service(void *arg)
{
    printf("%s Enter into Service Room\n", (char *)arg);
    for (int i = 0; i < number_of_serviceBooth; i++)
    {
        int serviceMan = i + 1;
        sem_wait(&service_array[i]); ///decrease
        printf("%s started taking service from serviceman %d\n", (char *)arg, serviceMan);
        sleep(1);
        printf("%s finished taking service from serviceman %d\n", (char *)arg, serviceMan);
        sem_post(&service_array[i]); ///increase
    }
    printf("%s Left Service Room\n", (char *)arg);

    pthread_mutex_lock(&mutex);
    paymentQ.push((char *)arg); //leaving serviceroom,cycle now is in the queue for entering into paymentRoom
    pthread_mutex_unlock(&mutex);
    sem_post(&qLock); //increase,Binary SP--initialized by 0
    // printf("Q UPPED\n");
}

void *paymentProcess(void *arg)
{
    while (1)
    {
        //case 1:jokhon last 1 ta item quesu theke ber korbo + duijon qLock_Down call...porertar jnno deadLock
        //case 2: at same time last two threa mutex down call..for one paymentComplete=5 , for anothre one paymentComplete=6
        pthread_mutex_lock(&payMutex);
        paymentComplete = paymentComplete + 1;
        // printf("paymentComplete: %d\n", paymentComplete);
        if (paymentComplete > number_of_cycle)
        {
            // printf("ALL PAYMENT COMPLETE\n");
            pthread_mutex_unlock(&payMutex);
            break;
        }
        pthread_mutex_unlock(&payMutex);

        sem_wait(&qLock); //decrease

        sem_wait(&sAvailablePayBooth); ///decrease
        pthread_mutex_lock(&mutex);

        void *poppedCycle = paymentQ.front(); //Empty kina check kora lagbe
        paymentQ.pop();
        printf("%s started paying the service bill\n", (char *)poppedCycle);
        sleep(2);
        printf("%s finished paying the service bill\n", (char *)poppedCycle);

        pthread_mutex_unlock(&mutex);
        printf("%s Left Payment room\n", (char *)poppedCycle);
        sem_post(&sAvailablePayBooth); ///increase,
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
            printf("Failed\n");
        }
    }

    res = pthread_mutex_init(&mutex, NULL);
    if (res != 0)
    {
        printf("Failed\n");
    }

    res = sem_init(&sAvailablePayBooth, 0, payementBooth);
    if (res != 0)
    {
        printf("Failed\n");
    }
    res = sem_init(&qLock, 0, 0);
    if (res != 0)
    {
        printf("Failed\n");
    }

    res = pthread_mutex_init(&payMutex, NULL);
    if (res != 0)
    {
        printf("Failed\n");
    }

    //Reader-writter code er jnno nicher gula
    res = pthread_mutex_init(&rcs, NULL);
    if (res != 0){
        printf("Failed\n");
    }
    
    res = pthread_mutex_init(&wcs, NULL);
    if (res != 0){
        printf("Failed\n");
    }

    res = sem_init(&s_rd, 0, 1);
    if (res != 0){
        printf("Failed\n");
    }

    res = sem_init(&s_wrt, 0, 1);
    if (res != 0){
        printf("Failed\n");
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
            printf("Consumer_cycle_thread creation failed\n");
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
            printf("Payment_thread creation failed\n");
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
        printf("Failed\n");
    }
    res = sem_destroy(service_array);
    if (res != 0)
    {
        printf("Failed\n");
    }
    res = sem_destroy(&sAvailablePayBooth);
    if (res != 0)
    {
        printf("Failed\n");
    }
    res = sem_destroy(&qLock);
    if (res != 0)
    {
        printf("Failed\n");
    }

    res = pthread_mutex_destroy(&payMutex);
    if (res != 0)
    {
        printf("Failed\n");
    }

    return 0;
}
