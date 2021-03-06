/*  S H A R ED    M E M O R Y - the creator */


/* What is Shared Memory under QNX?
 * In QNX Neutrino, every process has its own, private,
 * virtual address space. It can not, even if it wanted to,
 * read from or write to the memory of another, different
 * process. For communication between processes, the most
 * common means is Message Passing (see the examples regarding
 * message passing and resource managers). To exchange a large
 * amount of data very fast, or to easily share data between
 * multiple processes, you can use shared memory. This is a
 * memory area that can be accessed by several processes, given
 * they have opened the shared memory object and mapped it into
 * their virtual address space. You can specify if you want to
 * all your processes to read, write or both regarding the
 * shared memory object. The processor's MMU is programmed
 * accordingly by the kernel.

 * Accessing shared memory 
 * The access of a shared memory object has a Unix-style
 * "file system" part - you actually create/open the shared
 * memory object with a filename that will usually be located
 * under /dev/shmem. Then, you use a function called mmap()
 * to map the shared memory into your virtual address space.
 * You receive a pointer to it, and usually you then use a
 * pre-defined structure to access the contents of the shared
 * memory object via this pointer.
 * 
 * Synchronisation
 * To synchronise the access to the shared memory, in this
 * example we will use a mutex, as discussed in the "mutex"
 * example. To make access to the mutex possible for all
 * participating process, we locate the mutex itself in shared
 * memory. Of course you can use any other means of 
 * synchronization, too. You can use for example message passing
 * to let another process (which is blocked in receive) know
 * that it now can access the shared memory.
 *
 * What this program does:
 * A shared memory object is created, a mutex is locked,
 * some text is written into the shared memory. Then the program
 * goes to sleep() for a while, you can start the second program,
 * "shmemuser". This program will try to aquire the
 * mutex and will not be able to do so, as long as the first
 * program, "shmemcreator" has the mutex locked. As
 * soon as the mutex is unlocked, the second program locks it
 * and reads out the shared memory. The creator program will
 * then remove the shared memory object.
 *  
 * More information:
 * See the System Architecture Guide, The QNX Neutrino
 * Microkernel, QNX Neutrino IPC, Shared Memory. 
 * You will find it in the Help Viewer.
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
 * the shared memory object. The header file is part of 
 * the this project, but also will be referenced by the
 * "example_shmem_user" project. */
#include "shmem.h"
#include <pthread.h>


/* We print the program's name on the console, so in case you
 * only have one console, you can distinguish between outputs
 * of shmemcreator and shmemuser. */
char        *progname = "shmemcreator";

int main(int argc, char *argv[]) {
	char    *text = "Text by shmemcreator.c";
	int     fd;
	shmem_t *ptr; // shmem_t is defined in shmem.h !
	pthread_mutexattr_t myattr;

	/* Create the shared memory object */
	fd = shm_open("/myshmemobject", O_RDWR | O_CREAT | O_EXCL, S_IRWXU);

	/* Check for errors - if  the shmem object already exists,
	 * you can simply delete with using the 'rm' utility or do 
	 * it from the Target Filesystem Navigator. It can happen
	 * that the object still exists from a previous run that
	 * you cancelled before the object could be removed. */ 
	if (fd == -1) {
		printf("%s: error creating the shared memory object. %s.\n",
				progname,  strerror(errno));
		exit(EXIT_FAILURE);
	}

	/* Set the size of the shared memory object */
	ftruncate(fd, sizeof(shmem_t));

	/* Get a pointer to the shared memory, map it into
	 * our address space */
	ptr = mmap(0, sizeof(shmem_t), PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);

	/* Initialize the mutex - see the "mutex" example for
	 * more info on mutexes. In this case we use a special flag
	 * to make the mutex shared between processes. */
	pthread_mutexattr_init(&myattr);
	pthread_mutexattr_setpshared(&myattr, PTHREAD_PROCESS_SHARED);
	pthread_mutex_init(&ptr->myshmemmutex, &myattr);

	/* We want to access (alter) the shared memory. To prevent
	 * other processes from accessing it meanwhile, we first
	 * lock a mutex. This assumes of course that the other
	 * processes honour the mutex aswell. */    
	pthread_mutex_lock(&ptr->myshmemmutex);

	strcpy(ptr->text, text); /* write to the shared memory */

	printf("%s: Shared memory created and semaphore initialized to 0.\n"
			"%s: Wrote text '%s' to shared memory.\n"
			"%s: Sleeping for 20 seconds.  While this program is sleeping\n"
			"%s: run 'example_shmem_user'.\n",
			progname, progname, ptr->text, progname, progname);

	/* Program now goes to sleep. This gives you time to start
	 * "example_shmem_user" which will try to access the
	 * shared memory object by first locking the mutex and then
	 * reading the string. */
	sleep(20);

	printf("%s: Woke up.  Now unlocking the mutex.\n", progname);

	/* We unlock the mutex now, because we are done with our
	 * access to the shared memory (normally this should be
	 * done directly after accessing the shared memory).
	 * The second program, "example_shmem_user" can now
	 * aquire the mutex and then access the shared memory. */
	pthread_mutex_unlock(&ptr->myshmemmutex);

	close(fd); // Closing the file descriptor
	munmap(ptr, sizeof(shmem_t)); // removing the mapping

	/* Delete the shared memory object. The function will
	 * return immediately and the entry under /dev/shmem will
	 * disappear, but the actual deleting may be delayed as long
	 * as another process still references the object. */
	shm_unlink("/myshmemobject"); 

	return(EXIT_SUCCESS);
}

