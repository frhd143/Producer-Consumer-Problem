# Producer-Consumer using Shared Memory and Semaphores
This is a simple C program that implements the Producer-Consumer problem using shared memory and semaphores.

## Overview
The program creates two semaphores, one for the producer and one for the consumer. The producer writes to the shared memory buffer and signals the consumer using the first semaphore. The consumer reads from the shared memory buffer and signals the producer using the second semaphore.
