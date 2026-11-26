# Standalone build of the core as its own kernel module (vc-mipi-core.ko, module name
# vc_mipi_core): core and sensor module tables, exports the core functions for a separate
# camera driver. The platform builds (Raspberry Pi, NXP, NVIDIA) compile the sources with
# their own Makefiles.
obj-m := vc-mipi-core.o
vc-mipi-core-objs := vc_mipi_core.o vc_mipi_modules.o
ccflags-y += -DVC_CORE_AS_MODULE
