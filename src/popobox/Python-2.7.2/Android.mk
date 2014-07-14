LOCAL_PATH := $(call my-dir)

include $(LOCAL_PATH)/../Common.mk

pymodule_sources := \
	Modules/config.c \
	Modules/getpath.c \
	Modules/main.c \
	Modules/gcmodule.c \
	Modules/threadmodule.c \
	Modules/signalmodule.c \
	Modules/posixmodule.c \
	Modules/errnomodule.c \
	Modules/pwdmodule.c \
	Modules/_sre.c \
	Modules/_codecsmodule.c \
	Modules/_weakref.c \
	Modules/zipimport.c \
	Modules/symtablemodule.c \
	Modules/xxsubtype.c \
	Modules/getbuildinfo.c

pyparser_sources := \
	Parser/acceler.c \
	Parser/grammar1.c \
	Parser/listnode.c \
	Parser/node.c \
	Parser/parser.c \
	Parser/parsetok.c \
	Parser/bitset.c \
	Parser/metagrammar.c \
	Parser/firstsets.c \
	Parser/grammar.c \
	Parser/pgen.c \
	Parser/myreadline.c \
	Parser/tokenizer.c

pypython_sources := \
	Python/_warnings.c \
	Python/Python-ast.c \
	Python/asdl.c \
	Python/ast.c \
	Python/bltinmodule.c \
	Python/ceval.c \
	Python/compile.c \
	Python/codecs.c \
	Python/errors.c \
	Python/frozen.c \
	Python/frozenmain.c \
	Python/future.c \
	Python/getargs.c \
	Python/getcompiler.c \
	Python/getcopyright.c \
	Python/getplatform.c \
	Python/getversion.c \
	Python/graminit.c \
	Python/import.c \
	Python/importdl.c \
	Python/marshal.c \
	Python/modsupport.c \
	Python/mystrtoul.c \
	Python/mysnprintf.c \
	Python/peephole.c \
	Python/pyarena.c \
	Python/pyctype.c \
	Python/pyfpe.c \
	Python/pymath.c \
	Python/pystate.c \
	Python/pythonrun.c \
	Python/structmember.c \
	Python/symtable.c \
	Python/sysmodule.c \
	Python/traceback.c \
	Python/getopt.c \
	Python/pystrcmp.c \
	Python/pystrtod.c \
	Python/dtoa.c \
	Python/formatter_unicode.c \
	Python/formatter_string.c \
	Python/dynload_shlib.c \
	Python/thread.c

pyobject_sources := \
	Objects/abstract.c \
	Objects/boolobject.c \
	Objects/bufferobject.c \
	Objects/bytes_methods.c \
	Objects/bytearrayobject.c \
	Objects/capsule.c \
	Objects/cellobject.c \
	Objects/classobject.c \
	Objects/cobject.c \
	Objects/codeobject.c \
	Objects/complexobject.c \
	Objects/descrobject.c \
	Objects/enumobject.c \
	Objects/exceptions.c \
	Objects/genobject.c \
	Objects/fileobject.c \
	Objects/floatobject.c \
	Objects/frameobject.c \
	Objects/funcobject.c \
	Objects/intobject.c \
	Objects/iterobject.c \
	Objects/listobject.c \
	Objects/longobject.c \
	Objects/dictobject.c \
	Objects/memoryobject.c \
	Objects/methodobject.c \
	Objects/moduleobject.c \
	Objects/object.c \
	Objects/obmalloc.c \
	Objects/rangeobject.c \
	Objects/setobject.c \
	Objects/sliceobject.c \
	Objects/stringobject.c \
	Objects/structseq.c \
	Objects/tupleobject.c \
	Objects/typeobject.c \
	Objects/weakrefobject.c \
	Objects/unicodeobject.c \
	Objects/unicodectype.c

python_sources := \
	$(pymodule_sources) \
	$(pyobject_sources) \
	$(pyparser_sources) \
	$(pypython_sources) \
	utils_for_android.c

python_cflags := \
	-DNDEBUG \
	-DPYTHONPATH='"/system/lib/python2.7"' \
	-DPy_BUILD_CORE \
	-DVERSION='"2.7"' \
	-DENABLE_IPV6 \
    -DHAVE_STATVFS \
    -DHAVE_SYS_STATVFS_H \
    -D_FILE_OFFSET_BITS=64 \
    -D_LARGEFILE64_SOURCE=1 \
    -D__LARGE64_FILES=1 \
    -DHAVE_LARGEFILE_SUPPORT=1

#
# libpython.so
#
include $(CLEAR_VARS)

LOCAL_MODULE           := libpython
LOCAL_MODULE_TAGS      := optional eng
LOCAL_MODULE_PATH      := $(TARGET_OUT_SHARED_LIBRARIES)
LOCAL_SRC_FILES        := $(python_sources)
LOCAL_CFLAGS           += $(python_cflags)
LOCAL_C_INCLUDES       += $(LOCAL_PATH) $(LOCAL_PATH)/Include
LOCAL_SHARED_LIBRARIES += libdl
POPOBOX_TARGETS        += $(LOCAL_MODULE)

include $(BUILD_SHARED_LIBRARY)

#
# python
#
include $(CLEAR_VARS)

LOCAL_MODULE           := python
LOCAL_MODULE_TAGS      := optional eng
LOCAL_MODULE_PATH      := $(TARGET_OUT_EXECUTABLES)
LOCAL_SRC_FILES        := Modules/python.c
LOCAL_CFLAGS           += $(python_cflags)
LOCAL_C_INCLUDES       += $(LOCAL_PATH) $(LOCAL_PATH)/Include
LOCAL_SHARED_LIBRARIES += libpython libdl
POPOBOX_TARGETS        += $(LOCAL_MODULE)

include $(BUILD_EXECUTABLE)

#
# build python module
#
build-python-module = \
	$(eval include $(CLEAR_VARS)) \
	$(eval LOCAL_MODULE           := $1) \
	$(eval LOCAL_MODULE_TAGS      := optional eng) \
	$(eval LOCAL_MODULE_PATH      := $(TARGET_OUT_SHARED_LIBRARIES)/python2.7/lib-dynload) \
	$(eval LOCAL_SRC_FILES        := $(addprefix Modules/,$2)) \
	$(eval LOCAL_C_INCLUDES       += $(LOCAL_PATH) $(LOCAL_PATH)/Include $(LOCAL_PATH)/Modules $3) \
	$(eval LOCAL_CFLAGS           += $4) \
	$(eval LOCAL_STATIC_LIBRARIES += $6) \
	$(eval LOCAL_SHARED_LIBRARIES += libdl libpython $5) \
	$(eval POPOBOX_TARGETS        += $(LOCAL_MODULE)) \
	$(eval include $(BUILD_SHARED_LIBRARY))

$(call build-python-module,_bisect,_bisectmodule.c)
$(call build-python-module,_codecs_cn,cjkcodecs/_codecs_cn.c)
$(call build-python-module,_codecs_hk,cjkcodecs/_codecs_hk.c)
$(call build-python-module,_codecs_iso2022,cjkcodecs/_codecs_iso2022.c)
$(call build-python-module,_codecs_jp,cjkcodecs/_codecs_jp.c)
$(call build-python-module,_codecs_kr,cjkcodecs/_codecs_kr.c)
$(call build-python-module,_codecs_tw,cjkcodecs/_codecs_tw.c)
$(call build-python-module,_codecs,_codecsmodule.c)
$(call build-python-module,_collections,_collectionsmodule.c)
$(call build-python-module,_csv,_csv.c)
$(call build-python-module,_ctypes,$(addprefix _ctypes/,callbacks.c callproc.c cfield.c _ctypes.c _ctypes_test.c malloc_closure.c stgdict.c),$(LOCAL_PATH)/../libffi-3.0.11/include,,,libffi_static)
$(call build-python-module,_ctypes_test,_ctypes/_ctypes_test.c,$(LOCAL_PATH)/../libffi-3.0.11/include)
$(call build-python-module,_curses,_cursesmodule.c,$(LOCAL_PATH)/../ncurses-5.9/include,,libncurses)
$(call build-python-module,_curses_panel,_curses_panel.c,$(LOCAL_PATH)/../ncurses-5.9/include $(LOCAL_PATH)/../ncurses-5.9/panel,,libpanel libncurses)
$(call build-python-module,_elementtree,_elementtree.c,$(LOCAL_PATH)/Modules/expat,-DHAVE_EXPAT_CONFIG_H -DUSE_PYEXPAT_CAPI)
$(call build-python-module,_functools,_functoolsmodule.c)
$(call build-python-module,_hashlib,_hashopenssl.c,external/openssl/include,,libssl libcrypto)
$(call build-python-module,_heapq,_heapqmodule.c)
$(call build-python-module,_hotshot,_hotshot.c)
$(call build-python-module,_io,_io/bufferedio.c _io/bytesio.c _io/fileio.c _io/iobase.c _io/_iomodule.c _io/stringio.c _io/textio.c,$(LOCAL_PATH)/Modules/_io)
$(call build-python-module,_json,_json.c)
$(call build-python-module,_locale,_localemodule.c ../utils_for_android.c)
$(call build-python-module,_lsprof,_lsprof.c rotatingtree.c)
$(call build-python-module,_md5,md5module.c md5.c)
$(call build-python-module,_multibytecodec,cjkcodecs/multibytecodec.c)
$(call build-python-module,_multiprocessing,_multiprocessing/multiprocessing.c _multiprocessing/socket_connection.c,Modules/_multiprocessing,-DPOSIX_SEMAPHORES_NOT_ENABLED)
$(call build-python-module,_random,_randommodule.c)
$(call build-python-module,_sha,shamodule.c)
$(call build-python-module,_sha256,sha256module.c)
$(call build-python-module,_sha512,sha512module.c)
$(call build-python-module,_socket,socketmodule.c ../utils_for_android.c)
$(call build-python-module,_sqlite3,_sqlite/cache.c _sqlite/connection.c _sqlite/cursor.c _sqlite/microprotocols.c _sqlite/module.c _sqlite/prepare_protocol.c _sqlite/row.c _sqlite/statement.c _sqlite/util.c,$(LOCAL_PATH)/Modules/_sqlite external/sqlite/dist,-DMODULE_NAME=\"sqlite3\" -DSQLITE_OMIT_LOAD_EXTENSION=1,libsqlite)
$(call build-python-module,_sre,_sre.c)
$(call build-python-module,_ssl,_ssl.c,external/openssl/include external/openssl/include/openssl,,libssl libcrypto)
$(call build-python-module,_struct,_struct.c)
$(call build-python-module,_symtable,symtablemodule.c)
$(call build-python-module,_testcapi,_testcapimodule.c)
$(call build-python-module,_weakref,_weakref.c)
#$(call build-python-module,al,al.c,,,libaudio)
$(call build-python-module,array,arraymodule.c)
$(call build-python-module,audioop,audioop.c)
$(call build-python-module,binascii,binascii.c)
#$(call build-python-module,cd,cdmodule.c,,,libcdaudio libds libmediad)
#$(call build-python-module,cl,clmodule.c,,,libcl libawareaudio)
$(call build-python-module,cmath,cmathmodule.c _math.c,,,libm)
$(call build-python-module,cPickle,cPickle.c)
$(call build-python-module,crypt,cryptmodule.c ../utils_for_android.c)
$(call build-python-module,cStringIO,cStringIO.c)
$(call build-python-module,datetime,datetimemodule.c timemodule.c)
#$(call build-python-module,dbm,dbmmodule.c)
$(call build-python-module,dl,dlmodule.c)
$(call build-python-module,errno,errnomodule.c)
$(call build-python-module,fcntl,fcntlmodule.c)
#$(call build-python-module,fm,fmmodule.c,,,libfm libgl)
$(call build-python-module,future_builtins,future_builtins.c)
$(call build-python-module,grp,grpmodule.c ../utils_for_android.c)
$(call build-python-module,imageop,imageop.c)
#$(call build-python-module,imgfile,imgfile.c,,,libimage libgutil libgl libm)
$(call build-python-module,itertools,itertoolsmodule.c)
$(call build-python-module,linuxaudiodev,linuxaudiodev.c)
$(call build-python-module,math,mathmodule.c _math.c,,,libm)
$(call build-python-module,mmap,mmapmodule.c)
#$(call build-python-module,nis,nismodule.c,,,libnsl)
$(call build-python-module,operator,operator.c)
#$(call build-python-module,ossaudiodev,ossaudiodev.c)
$(call build-python-module,parser,parsermodule.c)
$(call build-python-module,posix,posixmodule.c,,-DHAVE_STATVFS -DHAVE_SYS_STATVFS_H)
$(call build-python-module,pwd,pwdmodule.c)
$(call build-python-module,pyexpat,expat/xmlparse.c expat/xmlrole.c expat/xmltok.c pyexpat.c,$(LOCAL_PATH)/Modules/expat,-DHAVE_EXPAT_CONFIG_H -DUSE_PYEXPAT_CAPI)
#$(call build-python-module,readline,readline.c,,,libreadline)
$(call build-python-module,resource,resource.c)
$(call build-python-module,select,selectmodule.c)
#$(call build-python-module,sgi,sgimodule.c)
$(call build-python-module,signal,signalmodule.c)
$(call build-python-module,spwd,spwdmodule.c ../utils_for_android.c,,-DHAVE_GETSPNAM=1 -DHAVE_GETSPENT=1)
$(call build-python-module,strop,stropmodule.c)
#$(call build-python-module,sv,svmodule.c yuvconvert.c,,,libsvideo libXext libX11)
$(call build-python-module,syslog,syslogmodule.c)
$(call build-python-module,termios,termios.c ../utils_for_android.c)
$(call build-python-module,thread,threadmodule.c)
$(call build-python-module,time,timemodule.c,,,libm)
$(call build-python-module,timing,timingmodule.c)
$(call build-python-module,unicodedata,unicodedata.c)
$(call build-python-module,xx,xxmodule.c)
$(call build-python-module,xxsubtype,xxsubtype.c)
$(call build-python-module,zipimport,zipimport.c)
$(call build-python-module,zlib,zlibmodule.c,external/zlib,,libz)
## laichao.zhou added for compiling bz2 module --2014-06-03
$(call build-python-module,bz2,bz2module.c,external/bzip2,,,libbz)

#
# copy scripts
#
$(call build-copy-files,$(LOCAL_PATH)/Lib/*.py,$(TARGET_OUT_SHARED_LIBRARIES)/python2.7)
#$(call build-copy-files,$(LOCAL_PATH)/Lib/*.pyc,$(TARGET_OUT_SHARED_LIBRARIES)/python2.7)
#$(call build-copy-files,$(LOCAL_PATH)/Lib/*.pyo,$(TARGET_OUT_SHARED_LIBRARIES)/python1.7)
$(call build-copy-files,$(LOCAL_PATH)/Lib/wsgiref.egg-info,$(TARGET_OUT_SHARED_LIBRARIES)/python2.7)
$(call build-copy-dir,$(LOCAL_PATH)/Lib/bsddb,$(TARGET_OUT_SHARED_LIBRARIES)/python2.7)
$(call build-copy-dir,$(LOCAL_PATH)/Lib/compiler,$(TARGET_OUT_SHARED_LIBRARIES)/python2.7)
#$(call build-copy-dir,$(LOCAL_PATH)/Lib/config,$(TARGET_OUT_SHARED_LIBRARIES)/python2.7)
$(call build-copy-dir,$(LOCAL_PATH)/Lib/ctypes,$(TARGET_OUT_SHARED_LIBRARIES)/python2.7)
$(call build-copy-dir,$(LOCAL_PATH)/Lib/curses,$(TARGET_OUT_SHARED_LIBRARIES)/python2.7)
$(call build-copy-dir,$(LOCAL_PATH)/Lib/distutils,$(TARGET_OUT_SHARED_LIBRARIES)/python2.7)
$(call build-copy-dir,$(LOCAL_PATH)/Lib/email,$(TARGET_OUT_SHARED_LIBRARIES)/python2.7)
$(call build-copy-dir,$(LOCAL_PATH)/Lib/encodings,$(TARGET_OUT_SHARED_LIBRARIES)/python2.7)
$(call build-copy-dir,$(LOCAL_PATH)/Lib/hotshot,$(TARGET_OUT_SHARED_LIBRARIES)/python2.7)
$(call build-copy-dir,$(LOCAL_PATH)/Lib/idlelib,$(TARGET_OUT_SHARED_LIBRARIES)/python2.7)
$(call build-copy-dir,$(LOCAL_PATH)/Lib/importlib,$(TARGET_OUT_SHARED_LIBRARIES)/python2.7)
$(call build-copy-dir,$(LOCAL_PATH)/Lib/json,$(TARGET_OUT_SHARED_LIBRARIES)/python2.7)
$(call build-copy-dir,$(LOCAL_PATH)/Lib/lib2to3,$(TARGET_OUT_SHARED_LIBRARIES)/python2.7)
#$(call build-copy-dir,$(LOCAL_PATH)/Lib/lib-dynload,$(TARGET_OUT_SHARED_LIBRARIES)/python2.7)
$(call build-copy-dir,$(LOCAL_PATH)/Lib/lib-tk,$(TARGET_OUT_SHARED_LIBRARIES)/python2.7)
$(call build-copy-dir,$(LOCAL_PATH)/Lib/logging,$(TARGET_OUT_SHARED_LIBRARIES)/python2.7)
$(call build-copy-dir,$(LOCAL_PATH)/Lib/multiprocessing,$(TARGET_OUT_SHARED_LIBRARIES)/python2.7)
$(call build-copy-dir,$(LOCAL_PATH)/Lib/plat-linux2,$(TARGET_OUT_SHARED_LIBRARIES)/python2.7)
$(call build-copy-dir,$(LOCAL_PATH)/Lib/pydoc_data,$(TARGET_OUT_SHARED_LIBRARIES)/python2.7)
$(call build-copy-dir,$(LOCAL_PATH)/Lib/site-packages,$(TARGET_OUT_SHARED_LIBRARIES)/python2.7)
$(call build-copy-dir,$(LOCAL_PATH)/Lib/sqlite3,$(TARGET_OUT_SHARED_LIBRARIES)/python2.7)
$(call build-copy-dir,$(LOCAL_PATH)/Lib/test,$(TARGET_OUT_SHARED_LIBRARIES)/python2.7)
$(call build-copy-dir,$(LOCAL_PATH)/Lib/unittest,$(TARGET_OUT_SHARED_LIBRARIES)/python2.7)
$(call build-copy-dir,$(LOCAL_PATH)/Lib/wsgiref,$(TARGET_OUT_SHARED_LIBRARIES)/python2.7)
$(call build-copy-dir,$(LOCAL_PATH)/Lib/xml,$(TARGET_OUT_SHARED_LIBRARIES)/python2.7)

#
# only for python testing.
#
#$(call build-copy-files,$(LOCAL_PATH)/Modules/config.c,$(TARGET_OUT_SHARED_LIBRARIES)/python2.7/config)
#$(call build-copy-files,$(LOCAL_PATH)/Modules/python.o,$(TARGET_OUT_SHARED_LIBRARIES)/python2.7/config)
#$(call build-copy-files,$(LOCAL_PATH)/Modules/config.c.in,$(TARGET_OUT_SHARED_LIBRARIES)/python2.7/config)
$(call build-copy-files,$(LOCAL_PATH)/Makefile,$(TARGET_OUT_SHARED_LIBRARIES)/python2.7/config)
#$(call build-copy-files,$(LOCAL_PATH)/Modules/Setup,$(TARGET_OUT_SHARED_LIBRARIES)/python2.7/config)
#$(call build-copy-files,$(LOCAL_PATH)/Modules/Setup.local,$(TARGET_OUT_SHARED_LIBRARIES)/python2.7/config)
#$(call build-copy-files,$(LOCAL_PATH)/Modules/Setup.config,$(TARGET_OUT_SHARED_LIBRARIES)/python2.7/config)
#$(call build-copy-files,$(LOCAL_PATH)/Modules/makesetup,$(TARGET_OUT_SHARED_LIBRARIES)/python2.7/config)
#$(call build-copy-files,$(LOCAL_PATH)/install-sh,$(TARGET_OUT_SHARED_LIBRARIES)/python2.7/config)

$(call build-copy-files,$(LOCAL_PATH)/pyconfig.h,$(TARGET_OUT_SHARED_LIBRARIES)/../include/python2.7)
