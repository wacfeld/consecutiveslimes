// copied from c/slimechunks.c, modified to find slime chunks in the same area and announce them

#include <stdio.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#define __STDC_FORMAT_MACROS
#include <inttypes.h>

uint64_t worldposseeds[12];
int threshold = 8;
int freakout = 9;
int counts[12];
int xoff, zoff;

uint64_t calculateXZSeed(uint32_t x, uint32_t z) {
    return (x * x * 0x4c1906) + (x * 0x5ac0db) + (z * z) * 0x4307a7 + (z * 0x5f24f);
}

// 2 RNG functions below from https://stackoverflow.com/questions/33010010/how-to-generate-random-64-bit-unsigned-integer-in-c#33021408
uint64_t rand_uint64_slow(void) {
  uint64_t r = 0;
  for (int i=0; i<64; i++) {
    r = r*2 + rand()%2;
  }
  return r;
}

uint64_t rand_uint64(void) {
  uint64_t r = 0;
  for (int i=0; i<64; i += 15 /*30*/) {
    r = r*((uint64_t)RAND_MAX + 1) + rand();
  }
  return r;
}

struct seedfinder_struct {
    uint64_t worldPositionSeed;
    uint64_t seedsOffset;
    uint64_t seedsRequested;
    uint64_t index;
}args;

void *runSeedfinder(void *args) {
    struct seedfinder_struct *arguments = (struct seedfinder_struct *)args;
    uint64_t worldPositionSeed = arguments -> worldPositionSeed;
    uint64_t seedsOffset = arguments -> seedsOffset;
    uint64_t seedsRequested = arguments -> seedsRequested;
    uint64_t index = arguments -> index;
    seedsRequested += seedsOffset;
    uint64_t seedsCounted = seedsOffset;
    uint64_t seedsFound = 0;
    struct timeval stop, start;
    gettimeofday(&start, 0);

    int counter;
    uint64_t seed;

    /* Optimizations start here */
    //warning: i am bad at programming - wac
    //printf("hi");
    while (seedsCounted < seedsRequested) {
        counter = 0;
        //seed = rand_uint64() >> 32; // this is slow... and stupid.
        seed = seedsCounted;
        for(int i = 0; i < 9; i++)
        {
                if((((((((seed + worldposseeds[i] ^ 0x3ad8025f) ^ 0x5DEECE66D) & 0xffffffffffff) * 0x5DEECE66D + 0xB) & 0xffffffffffff) >> 17) % 10) == 0)
                    counter++;
                else break;
        }

        counts[counter]++;
        if(counter >= threshold)
            printf("%d %"PRIu64" %d,%d\n", counter, seed, xoff, zoff);
        if(counter >= freakout)
            printf("WOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO\n\n\n\n\n");
        seedsCounted++;
    }
    gettimeofday(&stop, NULL);
    uint64_t microsTime = (stop.tv_sec - start.tv_sec) * 1000000 + stop.tv_usec - start.tv_usec;
    /* Optimizations end (but don't have to) here */
    printf("Thread %lu finished: %lu million seeds, %lu million matches and took %lu milliseconds\n", index, (seedsCounted - seedsOffset) / 1000000, seedsFound / 1000000, microsTime / 1000);
    //for(int i = 0; i < 12; i++)
    //    printf("found %d seeds with %d consecutive\n", counts[i], i);
    pthread_exit(NULL);
}

int main() {
    //for(int i = 0; i < 9; i++)
    //{
    //    worldposseeds[i] = calculateXZSeed(i, 0);
    //    printf("%"PRIu64"\n", worldposseeds[i]);
    //}
    // im bad at indexing so here's my solution

    uint64_t seedsRequested;
    uint64_t threads;
    printf("Enter an amount of Seeds to check\nEnter in billions: ");
    scanf("%lu", &seedsRequested);
    seedsRequested *= 1000000000;
    printf("Enter the amount of Threads to use: ");
    scanf("%lu", &threads);
    printf("Enter threshold to announce (7 or 8 recommended): ");
    scanf("%d", &threshold);
    printf("Enter threshold to go crazy over (9 recommended): ");
    scanf("%d", &freakout);
    printf("x: ");
    scanf("%d", &xoff);
    printf("z: ");
    scanf("%d", &zoff);
    printf("\n");
    if (threads < 2) {
        printf("More threads...\n");
        return (-1);
    }

    worldposseeds[0] = calculateXZSeed(0+xoff,0+zoff);
    worldposseeds[1] = calculateXZSeed(0+xoff,1+zoff);
    worldposseeds[2] = calculateXZSeed(0+xoff,2+zoff);
    worldposseeds[3] = calculateXZSeed(1+xoff,0+zoff);
    worldposseeds[4] = calculateXZSeed(1+xoff,1+zoff);
    worldposseeds[5] = calculateXZSeed(1+xoff,2+zoff);
    worldposseeds[6] = calculateXZSeed(2+xoff,0+zoff);
    worldposseeds[7] = calculateXZSeed(2+xoff,1+zoff);
    worldposseeds[8] = calculateXZSeed(2+xoff,2+zoff);
    uint64_t worldPositionSeed = calculateXZSeed(420, 69);
    uint64_t seedsPerThread = seedsRequested / threads;

    pthread_t tlist[threads];
    pthread_t ct;
    for (uint64_t threadIndex = 1; threadIndex < threads; threadIndex++) {
        struct seedfinder_struct* args = (struct seedfinder_struct *)malloc(sizeof(struct seedfinder_struct));
        args->worldPositionSeed = worldPositionSeed;
        args->seedsOffset = threadIndex * seedsPerThread;
        args->seedsRequested = seedsPerThread;
        args->index = threadIndex;
        pthread_create(&ct, NULL, runSeedfinder, (void *)args);
        tlist[threadIndex] = ct;
    }

    if (threads > 1) {
        struct seedfinder_struct* args = (struct seedfinder_struct *)malloc(sizeof(struct seedfinder_struct));
        args->worldPositionSeed = worldPositionSeed;
        args->seedsOffset = 0;
        args->seedsRequested = seedsPerThread;
        args->index = 0;
        runSeedfinder((void*)args);
    }

    for (uint64_t i = 0; i < threads; i++) {
        pthread_join(tlist[i], NULL);
    }
}
