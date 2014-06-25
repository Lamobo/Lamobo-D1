ifdef CONFIG_VIDEO_RESERVED_MEM_SIZE
   __ZRELADDR	:= $(shell /bin/bash -c 'printf "0x%08x" \
	   			$$[$(CONFIG_RAM_BASE) + $(CONFIG_VIDEO_RESERVED_MEM_SIZE) + 0x8000]')
__PARAMS_PHYS	:= $(shell /bin/bash -c 'printf "0x%08x" \
	   			$$[$(CONFIG_RAM_BASE) + $(CONFIG_VIDEO_RESERVED_MEM_SIZE) + 0x100]')
   zreladdr-y	:= $(__ZRELADDR)
params_phys-y	:= $(__PARAMS_PHYS)
else
   zreladdr-y	:= 0x80008000
params_phys-y	:= 0x80000100
endif
