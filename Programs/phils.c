#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>

int numOfPhils; // number of phils, passed in as argument[1]
int numOfTimesToEat; // number of times to eat each, passed in as argument[2]
sem_t *forks;
int *state;
int *phils;
sem_t startSignal; // used to delay eating until all initial states are printed

void pickupFork(int i) { // waits to grab forks for philospher (denotes when philospher is hungry)
    state[i] = 1;
    printf("Philosopher %d is hungry ...\n", i);

    if (i % 2 == 0) {
        sem_wait(&forks[i]); // left
        sem_wait(&forks[(i + 1) % numOfPhils]); // right
    } else {
        sem_wait(&forks[(i + 1) % numOfPhils]); // right
        sem_wait(&forks[i]); // left
    }

    state[i] = 2;
    printf("Philosopher %d is eating ...\n", i);
}

void putDownFork(int i) { // puts forks back down (denotes when philospher is thinking)
    state[i] = 0;
    printf("Philosopher %d is thinking ...\n", i);

    sem_post(&forks[i]);
    sem_post(&forks[(i + 1) % numOfPhils]);
}

void *philosopher(void *arg) { // must be a pointer when working with threading // determines first action of a philospher when thread is created
    int id = *((int *)arg);

    // Wait until all initial states are printed
    sem_wait(&startSignal);

    for (int i = 0; i < numOfTimesToEat; i++) {
        pickupFork(id);
        usleep((rand() % 400 + 100) * 1000); // Simulate eating
        putDownFork(id);
        usleep((rand() % 400 + 100) * 1000); // Simulate thinking
    }

    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s numOfPhilosophers numOfTimesToEat\n", argv[0]);
        return 1;
    }

    numOfPhils = atoi(argv[1]);
    numOfTimesToEat = atoi(argv[2]);

    srand(time(NULL));
     
    // thread usage
    pthread_t threads[numOfPhils];

    // memory allocation for forks, state, and philosphers
    forks = malloc(numOfPhils * sizeof(sem_t));
    state = malloc(numOfPhils * sizeof(int));
    phils = malloc(numOfPhils * sizeof(int));
    sem_init(&startSignal, 0, 0);

    // Initialize semaphores and philosopher data
    for (int i = 0; i < numOfPhils; i++) {
        sem_init(&forks[i], 0, 1);
        phils[i] = i;
    }

    // Set and print initial states
    for (int i = 0; i < numOfPhils; i++) {
        state[i] = rand() % 2;
        if (state[i] == 0)
            printf("Philosopher %d is thinking ...\n", i);
        else
            printf("Philosopher %d is hungry ...\n", i);
    }

    // Launch threads
    for (int i = 0; i < numOfPhils; i++) {
        pthread_create(&threads[i], NULL, philosopher, &phils[i]);
    }

    // Allow threads to proceed only after initial states are all printed
    for (int i = 0; i < numOfPhils; i++) {
        sem_post(&startSignal);
    }

    // Wait for all threads to finish
    for (int i = 0; i < numOfPhils; i++) {
        pthread_join(threads[i], NULL);
    }

    free(forks);
    free(state);
    free(phils);
    return 0;
}
