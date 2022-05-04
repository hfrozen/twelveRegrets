#include <pthread.h>
#include <sys/wait.h>

#include "CTcx.h"

//#define	_DEBUG_M

#ifdef	_DEBUG_M
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/iofunc.h>
#include <sys/dispatch.h>
#include <sys/neutrino.h>
#include <sys/procfs.h>
#include <sys/stat.h>
#endif

CTcx tcx;

#ifdef	_DEBUG_M
int io_write(resmgr_context_t *ctp, io_write_t *msg, RESMGR_OCB_T *ocb);
static int				dumper_fd;
resmgr_connect_funcs_t	connect_funcs;
resmgr_io_funcs_t		io_funcs;
dispatch_t				*dpp;
resmgr_attr_t			rattr;
dispatch_context_t		*ctp;
iofunc_attr_t			ioattr;
const char* prognameA = "dumphandler";
#endif

int main(int argc, char *argv[])
{
	int aich, bkdis, sdis;
	aich = bkdis = sdis = 0;
	int opt;
	while ((opt = getopt(argc, argv, "aks:")) > 0) {
		switch (opt) {
		case 'a' :	aich = 1;	break;
		case 'k' :	bkdis = 1;	break;
		case 's' :	sdis = 1;	break;
		default :	break;
		}
	}

	tcx.SetOwner((PSZ)"CTcx");
	if (tcx.Run(aich, bkdis, sdis)) {
#ifdef	_DEBUG_M
		dumper_fd = open("/proc/dumper", O_WRONLY);
		dpp = dispatch_create();
		memset(&rattr, 0, sizeof(rattr));
		rattr.msg_max_size = 2048;
		iofunc_func_init(_RESMGR_CONNECT_NFUNCS, &connect_funcs, _RESMGR_IO_NFUNCS, &io_funcs);
		io_funcs.write = io_write;
		iofunc_attr_init(&ioattr, S_IFNAM | 0600, NULL, NULL);
		resmgr_attach(dpp, &rattr, "/tmp/dumper", _FTYPE_DUMPER, _RESMGR_FLAG_BEFORE, &connect_funcs,
						&io_funcs, &ioattr);
		ctp = dispatch_context_alloc(dpp);
		while(1) {
			if ((ctp = dispatch_block(ctp)) == NULL) {
				fprintf(stderr, "%s:  dispatch_block failed: %s\n", prognameA, strerror(errno));
				exit(1);
			}
			dispatch_handler(ctp);
		}
#endif
		sigset_t mask;
		sigemptyset(&mask);
		sigaddset(&mask, SIGINT);

		int sig;
		sigwait(&mask, &sig);
	}
	tcx.Destroy();
	printf("Exit Tcx.\r\n");

	return EXIT_SUCCESS;
}

#ifdef	_DEBUG_M

struct dinfo_s {
	procfs_debuginfo	info;
    char	pathbuffer[PATH_MAX]; /* 1st byte is info.path[0] */
};

int display_process_info(pid_t pid)
{
	char			buf[PATH_MAX + 1];
	int				fd, status;
	struct dinfo_s	dinfo;
	//procfs_greg		reg;

	printf("%s: process %d died\n", prognameA, pid);
	sprintf(buf, "/proc/%d/as", pid);
	if ((fd = open(buf, O_RDONLY|O_NONBLOCK)) == -1)
		return errno;

	status = devctl(fd, DCMD_PROC_MAPDEBUG_BASE, &dinfo, sizeof(dinfo), NULL);
	if (status != EOK) {
		close(fd);
		return status;
	}
	printf("%s: name is %s\n", prognameA, dinfo.info.path);
	close(fd);
	return EOK;
}

int io_write(resmgr_context_t *ctp, io_write_t *msg, RESMGR_OCB_T *ocb)
{
	char	*pstr;
	int		status;

	if ((status = iofunc_write_verify(ctp, msg, ocb, NULL)) != EOK)
		return status;
	if (msg->i.xtype & _IO_XTYPE_MASK != _IO_XTYPE_NONE)
		return ENOSYS;
    if ((int)ctp->msg_max_size < (int)msg->i.nbytes + 1)
    	return ENOSPC; /* not all the message could fit in the message buffer */
    pstr = (char *) (&msg->i) + sizeof(msg->i);
    pstr[msg->i.nbytes] = '\0';
    if (dumper_fd != -1) {
    	/* pass it on to dumper so it can handle it too */
    	if (write(dumper_fd, pstr, strlen(pstr)) == -1) {
    		close(dumper_fd);
    		dumper_fd = -1; /* something wrong, no sense in doing it again later */
    	}
    }
    if ((status = display_process_info(atoi(pstr))) == -1)
    	return status;

    _IO_SET_WRITE_NBYTES(ctp, msg->i.nbytes);
    return EOK;
}
#endif
