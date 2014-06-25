#linux/arch/arm/mach-ak39/Makefile
#
# Copyright 2013 Anyka Microelectronics
#
# Licensed under GPLv2

obj-y               :=
obj-m               :=
obj-n               :=
obj-                :=

# Core support for all Anyka SoCs
obj-y			+= cpu.o time.o irq.o clock.o	\
				   l2cache.o devices.o	\
				   ak39-gpio.o gpio.o	\
				   adc.o				\
				   reset.o reboot.o		\


# chip extra interface
obj-$(CONFIG_AK39_PWM_TIMER)		+= pwm_timer.o

# Power Management support
obj-$(CONFIG_AK39_PM)       += pm.o

# Machin support
obj-$(CONFIG_ARCH_SDK3910)	+= mach-sdk3910.o
obj-$(CONFIG_ARCH_AIMER39_AK3916)	+= mach-aimer39_ak3916.o
obj-$(CONFIG_ARCH_AIMER39_AK3918)	+= mach-aimer39_ak3918.o
