#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>

#define THINKING 0
#define HUNGRY 1
#define EATING 2

int numOfPhils;
int numOfTimesToEat;

sem_t *forks;
int *state;
int *phils;
int *eatCounts;

sem_t mutex;

void random_sleep() {
    usleep((rand() % 400 + 100) * 1000);  // 0.1 - 0.5 seconds
}

void print_status(int id, const char* status) {
    printf("Philosopher %d is %s ...\n", id, status);
    fflush(stdout);
}

void test(int i) {
    int left = (i + numOfPhils - 1) % numOfPhils;
    int right = (i + 1) % numOfPhils;

    if (state[i] == HUNGRY && state[left] != EATING && state[right] != EATING) {
        state[i] = EATING;
        sem_post(&forks[i]);
    }
}

void pickupFork(int i) {
    sem_wait(&mutex);
    state[i] = HUNGRY;
    print_status(i, "hungry");
    test(i);
    sem_post(&mutex);
    sem_wait(&forks[i]); // Wait until able to eat
}

void putDownFork(int i) {
    sem_wait(&mutex);
    state[i] = THINKING;
    print_status(i, "thinking");
    test((i + numOfPhils - 1) % numOfPhils); // test left neighbor
    test((i + 1) % numOfPhils);             // test right neighbor
    sem_post(&mutex);
}

void *philosopher(void *arg) {
    int id = *((int *)arg);
    int eaten = 0;

    // Random initial state
    int initial = rand() % 2;
    if (initial == THINKING) {
        state[id] = THINKING;
        print_status(id, "thinking");
    } else {
        state[id] = HUNGRY;
        print_status(id, "hungry");
    }

    while (eaten < numOfTimesToEat) {
        if (id % 2 == 0) {
            pickupFork(id);     // even: left then right
        } else {
            pickupFork(id);     // odd: right then left (but still test function uses only own id)
        }

        print_status(id, "eating");
        random_sleep();
        eaten++;
        eatCounts[id]++;

        putDownFork(id);
        random_sleep(); // thinking
    }

    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s num_philosophers num_times_to_eat\n", argv[0]);
        exit(1);
    }

    numOfPhils = atoi(argv[1]);
    numOfTimesToEat = atoi(argv[2]);

    srand(time(NULL));

    pthread_t threads[numOfPhils];
    forks = malloc(numOfPhils * sizeof(sem_t));
    state = malloc(numOfPhils * sizeof(int));
    phils = malloc(numOfPhils * sizeof(int));
    eatCounts = malloc(numOfPhils * sizeof(int));

    sem_init(&mutex, 0, 1);

    for (int i = 0; i < numOfPhils; i++) {
        sem_init(&forks[i], 0, 0);
        state[i] = THINKING;
        phils[i] = i;
        eatCounts[i] = 0;
    }

    for (int i = 0; i < numOfPhils; i++) {
        pthread_create(&threads[i], NULL, philosopher, &phils[i]);
    }

    for (int i = 0; i < numOfPhils; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("\n--- Eating Summary ---\n");
    for (int i = 0; i < numOfPhils; i++) {
        printf("Philosopher %d ate %d times\n", i, eatCounts[i]);
    }

    free(forks);
    free(state);
    free(phils);
    free(eatCounts);

    return 0;
}
