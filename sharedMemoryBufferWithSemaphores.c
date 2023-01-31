#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h> /* For O_* constants */
#include <sys/wait.h>

#define SHMSIZE 128
#define SHM_R 0400
#define SHM_W 0200

const char *semName1 = "my_sema1";
const char *semName2 = "my_sema2";

int main(int argc, char **argv)
{
	// Create a new POSIX semaphore or open an existing semaphore
	sem_t *sem_id1 = sem_open(semName1, O_CREAT, O_RDWR,10);
	sem_t *sem_id2 = sem_open(semName2, O_CREAT, O_RDWR, 0);
	int i, status; 

	struct shm_struct {
		int buffer[10];
		int producer_counter;
		int consumer_counter;
	};

	volatile struct shm_struct *shmp = NULL;
	char *addr = NULL;
	pid_t pid;
	int var1 = 0, var2 = 0, shmid = -1;
	struct shmid_ds *shm_buf;


	/* allocate a chunk of shared memory */
	shmid = shmget(IPC_PRIVATE, SHMSIZE, IPC_CREAT | SHM_R | SHM_W);
	shmp = (struct shm_struct *) shmat(shmid, addr, 0);
	shmp->producer_counter = 0;
	shmp->consumer_counter = 0;

	pid = fork();
	if (pid != 0) {
		/* here's the parent, acting as producer */
		while (var1 < 100) {
			if (shmp->producer_counter > 9) {
				shmp->producer_counter = 0;
			}
			// write to shmem */
			// Lock the semaphore
			sem_wait(sem_id1);
			var1++;
			//while (shmp->empty == 10); /* busy wait until the buffer is empty */
			printf("Sending %d\n", var1); fflush(stdout);
			shmp->buffer[shmp->producer_counter] = var1;
			//usleep(rand()%400000+100000);
			shmp->producer_counter++;
            // Unlock the semaphore
			sem_post(sem_id2);
		}
		shmdt(addr);
		shmctl(shmid, IPC_RMID, shm_buf);

        sem_close(sem_id1); // close a semaphore
		sem_close(sem_id2); // close a semaphore
		wait(&status);
		sem_unlink(semName1); // remove a named semaphore
		sem_unlink(semName2); // remove a named semaphore

	} else {
		/* here's the child, acting as consumer */
		while (var2 < 100) {
			if (shmp->consumer_counter > 9) {
				shmp->consumer_counter = 0;
			}
			/* read from shmem */
			//while (shmp->empty == 0); /* busy wait until there is something */
			// Lock the semaphore
			sem_wait(sem_id2);
			// usleep(rand()%400000+100000);
			var2 = shmp->buffer[shmp->consumer_counter];
			shmp->consumer_counter++;
			printf("Received %d\n", var2); fflush(stdout);
            // Unlock the semaphore
			sem_post(sem_id1);
		}
		shmdt(addr);
		shmctl(shmid, IPC_RMID, shm_buf);

        sem_close(sem_id1); // close a semaphore
		sem_close(sem_id2); // close a semaphore
	}
}
