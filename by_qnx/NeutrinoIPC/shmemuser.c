
/*  S H A R E D    M E M O R Y - the user program */

/*
 *  shmemuser.c
 *
 *  This module demonstrates shared memory and semaphores
 *  by opening some shared memory, waiting on a semaphore (whose
 *  semaphore structure is in the shared memory) and then printing out
 *  what it finds in the shared memory.
 *
 *  This one is meant to be run in tandem with shmemcreator.c.
 *
 *  Run it as: shmemuser
 */

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>

/* shmem.h contains the structure that is overlayed on
 * the shared memory object. */
#include "shmem.h"


/* We print the program's name on the console, so in case you
 * only have one console, you can distinguish between outputs
 * of shmem_creator and shmem_user. */
char        *progname = "shmemuser";

int main(int argc, char *argv[]) {
	int     fd;
	shmem_t *ptr;

	/* Open the shared memory object 
	 * We check if the open succeeds - if you have not run
	 * the example_shmem_creator program, the shared memory
	 * object will not be present. */
	fd = shm_open("/myshmemobject", O_RDWR, S_IRWXU);
	if (fd == -1) {
		printf("%s: error opening the shared memory object: %s\n",
				progname,  strerror(errno));
		exit(EXIT_FAILURE);
	}

	/* Get a pointer to the shared memory object */
	ptr = mmap(0, sizeof(shmem_t), 
			PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);

	/* Trying to obtain exclusive access to the
	 * shared memory region by locking the mutex. */

	/* With the 'pidin' utility or from the System Information
	 * Perspective, you will see that this thread will wait
	 * for the Mutex. */    
	printf("%s: Waiting on the mutex.  Run 'pidin'.  I should be MUTEX blocked.\n", progname);

	/* We now try to lock the mutex in the shared memory.
	 * This mutex is used to ensure exclusive access to the
	 * shared memory, so we use it as a means of synchronisation. */

	pthread_mutex_lock(&ptr->myshmemmutex);

	/* There are other means of synchronisation, for example
	 * read/write locks (multiple readers while no writers, or
	 * no readers while one writer) which may be useful for you
	 * for shared memory object - or anything else - synch.
	 * See the System Architecture Guide, The QNX Neutrino
	 * Microkernel, Synchronization services. */

	printf("%s: Got the mutex, now accessing shared memory\n",progname);    
	printf("%s: The shared memory contains '%s'\n", progname, ptr->text);

	/* Of course we need to unlock the mutex, to allow
	 * other programs to access the shared memory region */    
	pthread_mutex_unlock(&ptr->myshmemmutex);

	/* We close the fd and unmap the shared memory from our
	 * virtual address space. */
	close(fd);
	munmap(ptr, sizeof(shmem_t));

	return (EXIT_SUCCESS);
}
