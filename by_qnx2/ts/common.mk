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
LDFLAGS+=-lang-c++ -M -Wl,-E -l m

#===== CCFLAGS - add the flags to the C compiler command line. 
CCFLAGS+=-D_AT_TS_

#===== EXTRA_SRCVPATH - a space-separated list of directories to search for source files.
EXTRA_SRCVPATH+= \
	$(PROJECT_ROOT_Component2)  \
	D:/TcmsWork/DuoLine/Qnx2/Base2  \
	D:/TcmsWork/DuoLine/Qnx2/Inform2  \
	D:/TcmsWork/DuoLine/Qnx2/Sheet  \
	D:/TcmsWork/DuoLine/Qnx2/Stage  \
	D:/TcmsWork/Common

#===== LIBS - a space-separated list of library items to be included in the link.
LIBS+=-Bstatic mq -Bdynamic

include $(MKFILES_ROOT)/qmacros.mk
ifndef QNX_INTERNAL
QNX_INTERNAL=$(PROJECT_ROOT)/.qnx_internal.mk
endif
include $(QNX_INTERNAL)

include $(MKFILES_ROOT)/qtargets.mk

OPTIMIZE_TYPE_g=none
OPTIMIZE_TYPE=$(OPTIMIZE_TYPE_$(filter g, $(VARIANTS)))

