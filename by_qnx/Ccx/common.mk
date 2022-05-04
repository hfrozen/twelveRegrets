# This is an automatically generated record.
# The area between QNX Internal Start and QNX Internal End is controlled by
# the QNX IDE properties.

ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

#===== USEFILE - the file containing the usage message for the application. 
USEFILE=

# Next lines are for C++ projects only

EXTRA_SUFFIXES+=cxx cpp

#===== LDFLAGS - add the flags to the linker command line.
LDFLAGS+=-lang-c++ -lsocket

VFLAG_g=-gstabs+

#===== EXTRA_INCVPATH - a space-separated list of directories to search for include files.
EXTRA_INCVPATH+= \
	D:/Tcmsx/Inform  \
	D:/Tcmsx/Mfb/20101130/library  \
	C:/QNX641/target/qnx6/usr/include/z17/INCLUDE/COM  \
	C:/QNX641/target/qnx6/usr/include/z17/INCLUDE/NATIVE

#===== EXTRA_LIBVPATH - a space-separated list of directories to search for library files.
EXTRA_LIBVPATH+=D:/Tcmsx/Mfb/20101130/library

#===== EXTRA_SRCVPATH - a space-separated list of directories to search for source files.
EXTRA_SRCVPATH+=$(PROJECT_ROOT_Cmn) $(PROJECT_ROOT_Device)

#===== LIBS - a space-separated list of library items to be included in the link.
LIBS+=-Bstatic mfb mq -Bdynamic

#===== CCFLAGS - add the flags to the C compiler command line. 
CCFLAGS+=-D_CCCX

include $(MKFILES_ROOT)/qmacros.mk

#===== EXTRA_OBJS - object files and libraries contributed additionaly on link/archive step
EXTRA_OBJS+= \
	C:/QNX641/target/qnx6/z17/ppcbe/usr/lib/mdis/libmdis_api.so  \
	C:/QNX641/target/qnx6/z17/ppcbe/usr/lib/mdis/libusr_oss.so  \
	C:/QNX641/target/qnx6/ppcbe/lib/libm.so.2

ifndef QNX_INTERNAL
QNX_INTERNAL=$(PROJECT_ROOT)/.qnx_internal.mk
endif
include $(QNX_INTERNAL)

include $(MKFILES_ROOT)/qtargets.mk

OPTIMIZE_TYPE_g=none
OPTIMIZE_TYPE=$(OPTIMIZE_TYPE_$(filter g, $(VARIANTS)))

