# nfs-ganesha build required
SHELL=/bin/bash
PREFIX=/usr
export

CPUNR=4

CMAKE_ARGS=-DCMAKE_BUILD_TYPE=Debug

GANESHA_ARGS=-DDISTNAME_HAS_GIT_DATA=OFF -DUSE_GUI_ADMIN_TOOLS=OFF -DUSE_9P=OFF -DUSE_GSS=OFF \
        -DUSE_FSAL_NULL=OFF -DUSE_FSAL_XFS=OFF -DUSE_FSAL_GLUSTER=OFF -DUSE_FSAL_LUSTRE=OFF -DUSE_FSAL_GPFS=OFF \
        -DUSE_FSAL_CEPH=OFF -DUSE_FSAL_RGW=OFF -DALLOCATOR="tcmalloc" -DCEPHFS_POSIX_ACL=ON

all:
	@echo "make genbuild - Generate a build environment"
	@echo "make builddeb - Generate a deb package"
	@echo "make clean - Get rid of scratch and byte files"

dobuild:
	mkdir -p $(CURDIR)/build
	pushd $(CURDIR)/build; \
	cmake $(CMAKE_ARGS) $(GANESHA_ARGS) -DFSAL_DESTINATION=/usr/lib/x86_64-linux-gnu/ganesha ../src/; \
	popd

builddeb:
