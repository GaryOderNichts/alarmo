#-------------------------------------------------------------------------------
.SUFFIXES:
#-------------------------------------------------------------------------------

TOPDIR ?= $(CURDIR)

#---------------------------------------------------------------------------------
# the prefix on the compiler executables
#---------------------------------------------------------------------------------
PREFIX		:=	arm-none-eabi-
export CC	:=	$(PREFIX)gcc
export CXX	:=	$(PREFIX)g++
export AS	:=	$(PREFIX)as
export AR	:=	$(PREFIX)gcc-ar
export OBJCOPY	:=	$(PREFIX)objcopy
export STRIP	:=	$(PREFIX)strip
export RANLIB	:=	$(PREFIX)gcc-ranlib

#---------------------------------------------------------------------------------
# allow seeing compiler command lines with make V=1 (similar to autotools' silent)
#---------------------------------------------------------------------------------
ifeq ($(V),1)
    SILENTMSG := @true
    SILENTCMD :=
else
    SILENTMSG := @echo
    SILENTCMD := @
endif

#-------------------------------------------------------------------------------
# TARGET is the name of the output
# BUILD is the directory where object files & intermediate files will be placed
# SOURCES is a list of directories containing source code
# INCLUDES is a list of directories containing header files
#-------------------------------------------------------------------------------
TARGET		:=	$(notdir $(CURDIR))
BUILD		:=	build
SOURCES		:=	source source/libalarmo
INCLUDES	:=	source
DATA		:=	data

# Add required driver source files
SOURCE_FILES	:=	STM32CubeH7/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_adc_ex.c \
			STM32CubeH7/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_adc.c \
			STM32CubeH7/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_adc.c \
			STM32CubeH7/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_cortex.c \
			STM32CubeH7/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dma_ex.c \
			STM32CubeH7/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dma.c \
			STM32CubeH7/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_dma.c \
			STM32CubeH7/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_gpio.c \
			STM32CubeH7/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_gpio.c \
			STM32CubeH7/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_mdma.c \
			STM32CubeH7/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_mdma.c \
			STM32CubeH7/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_pwr_ex.c \
			STM32CubeH7/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_pwr.c \
			STM32CubeH7/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_pwr.c \
			STM32CubeH7/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_rcc_ex.c \
			STM32CubeH7/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_rcc.c \
			STM32CubeH7/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_rcc.c \
			STM32CubeH7/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_sram.c \
			STM32CubeH7/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_fmc.c \
			STM32CubeH7/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_tim_ex.c \
			STM32CubeH7/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_tim.c \
			STM32CubeH7/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_tim.c \
			STM32CubeH7/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal.c

# Add required driver includes
INCLUDES	+=	STM32CubeH7/Drivers/CMSIS/Include \
			STM32CubeH7/Drivers/CMSIS/Device/ST/STM32H7xx/Include \
			STM32CubeH7/Drivers/STM32H7xx_HAL_Driver/Inc

#-------------------------------------------------------------------------------
# options for code generation
#-------------------------------------------------------------------------------
MACHDEP =	-mthumb -march=armv7e-m -mtune=cortex-m7 -mfpu=fpv5-d16 -mfloat-abi=hard -DSTM32H730xx

CFLAGS	:=	$(MACHDEP) $(INCLUDE) -O0

CXXFLAGS	:= $(CFLAGS)

CFLAGS	+=	

ASFLAGS	:=	$(MACHDEP)

LDFLAGS	=	$(MACHDEP) -specs=nano.specs \
		-Wl,-Map,$(notdir $*.map) \
		-Wl,-T $(TOPDIR)/link.ld

LIBS	:=	-lc -lm -lnosys

ifeq ($(DEBUG), 1)
	CFLAGS += -g
	ASFLAGS += -g
	LDFLAGS += -g
endif

#-------------------------------------------------------------------------------
# list of directories containing libraries, this must be the top level
# containing include and lib
#-------------------------------------------------------------------------------
LIBDIRS	:=


#-------------------------------------------------------------------------------
# no real need to edit anything past this point unless you need to add additional
# rules for different file extensions
#-------------------------------------------------------------------------------
ifneq ($(BUILD),$(notdir $(CURDIR)))
#-------------------------------------------------------------------------------

export OUTPUT	:=	$(CURDIR)/$(TARGET)
export TOPDIR	:=	$(CURDIR)

export VPATH	:=	$(foreach dir,$(SOURCES),$(CURDIR)/$(dir)) \
			$(foreach dir,$(DATA),$(CURDIR)/$(dir)) \
			$(CURDIR)/STM32CubeH7/Drivers/STM32H7xx_HAL_Driver/Src

export DEPSDIR	:=	$(CURDIR)/$(BUILD)

CFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
CPPFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
SFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s)))
BINFILES	:=	$(foreach dir,$(DATA),$(notdir $(wildcard $(dir)/*.*)))

CFILES		+=	$(foreach f,$(HAL_SOURCE),$(notdir $(f)))
CFILES		+=	$(foreach f,$(SOURCE_FILES),$(notdir $(f)))

#-------------------------------------------------------------------------------
# use CXX for linking C++ projects, CC for standard C
#-------------------------------------------------------------------------------
ifeq ($(strip $(CPPFILES)),)
#-------------------------------------------------------------------------------
	export LD	:=	$(CC)
#-------------------------------------------------------------------------------
else
#-------------------------------------------------------------------------------
	export LD	:=	$(CXX)
#-------------------------------------------------------------------------------
endif
#-------------------------------------------------------------------------------

export OFILES_SRC	:=	$(CPPFILES:.cpp=.o) $(CFILES:.c=.o) $(SFILES:.s=.o)
export OFILES 		:=	$(OFILES_SRC)
export HFILES_BIN	:=	$(addsuffix .h,$(subst .,_,$(BINFILES)))

export INCLUDE	:=	$(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) \
			$(foreach dir,$(LIBDIRS),-I$(dir)/include) \
			-I$(CURDIR)/$(BUILD)

export LIBPATHS	:=	$(foreach dir,$(LIBDIRS),-L$(dir)/lib)

.PHONY: $(BUILD) clean all

#-------------------------------------------------------------------------------
all: $(BUILD)

$(BUILD):
	@[ -d $@ ] || mkdir -p $@
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile

#-------------------------------------------------------------------------------
clean:
	@echo clean ...
	@rm -fr $(BUILD) $(TARGET).elf $(TARGET).bin a.bin

#-------------------------------------------------------------------------------
else
.PHONY:	all

DEPENDS	:=	$(OFILES:.o=.d)

#-------------------------------------------------------------------------------
# main targets
#-------------------------------------------------------------------------------
all	: $(TOPDIR)/a.bin

$(TOPDIR)/a.bin: $(OUTPUT).bin
	@echo encrypting ... $(notdir $@)
	$(SILENTCMD)python3 $(TOPDIR)/ciphtool.py encrypt $< $@

$(OUTPUT).bin: $(OUTPUT).elf
	@echo creating ... $(notdir $@)
	$(SILENTCMD)$(OBJCOPY) -O binary $< $@

$(OUTPUT).elf: $(OFILES)
	@echo linking ... $(notdir $@)
	$(SILENTCMD)$(LD) $(LDFLAGS) $(OFILES) $(LIBPATHS) $(LIBS) -o $@ $(ERROR_FILTER)

$(OFILES_SRC) : $(HFILES_BIN)

#---------------------------------------------------------------------------------
%.o: %.cpp
	$(SILENTMSG) $(notdir $<)
	$(SILENTCMD)$(CXX) -MMD -MP -MF $(DEPSDIR)/$*.d $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@ $(ERROR_FILTER)

#---------------------------------------------------------------------------------
%.o: %.c
	$(SILENTMSG) $(notdir $<)
	$(SILENTCMD)$(CC) -MMD -MP -MF $(DEPSDIR)/$*.d $(CPPFLAGS) $(CFLAGS) -c $< -o $@ $(ERROR_FILTER)

#---------------------------------------------------------------------------------
%.o: %.s
	$(SILENTMSG) $(notdir $<)
	$(SILENTCMD)$(CC) -MMD -MP -MF $(DEPSDIR)/$*.d -x assembler-with-cpp $(CPPFLAGS) $(ASFLAGS) -c $< -o $@ $(ERROR_FILTER)

#---------------------------------------------------------------------------------

#-------------------------------------------------------------------------------
%_png.h :	%.png
#-------------------------------------------------------------------------------
	@echo $(notdir $<)
	$(SILENTCMD)python3 $(TOPDIR)/imgconv.py $< > $@

-include $(DEPENDS)

#-------------------------------------------------------------------------------
endif
#-------------------------------------------------------------------------------
