set(srcs 
	NVS_Fs.c)

set(incs .
	include)

idf_component_register(SRCS "${srcs}" 
					   INCLUDE_DIRS "${incs}"
					   PRIV_REQUIRES )
