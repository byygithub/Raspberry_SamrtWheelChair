################################################################################
# 自动生成的文件。不要编辑！
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include Drivers/STM32F4xx_HAL_Driver/Src/subdir.mk
-include Core/Startup/subdir.mk
-include Core/Src/subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

OPTIONAL_TOOL_DEPS := \
$(wildcard ../makefile.defs) \
$(wildcard ../makefile.init) \
$(wildcard ../makefile.targets) \


<<<<<<< HEAD
BUILD_ARTIFACT_NAME := WheelchairControl-gitee
=======
BUILD_ARTIFACT_NAME := WheelchairControl
>>>>>>> 786f1618a70ce4136384469ca2ae15a95eecd699
BUILD_ARTIFACT_EXTENSION := elf
BUILD_ARTIFACT_PREFIX :=
BUILD_ARTIFACT := $(BUILD_ARTIFACT_PREFIX)$(BUILD_ARTIFACT_NAME)$(if $(BUILD_ARTIFACT_EXTENSION),.$(BUILD_ARTIFACT_EXTENSION),)

# Add inputs and outputs from these tool invocations to the build variables 
EXECUTABLES += \
<<<<<<< HEAD
WheelchairControl-gitee.elf \

MAP_FILES += \
WheelchairControl-gitee.map \
=======
WheelchairControl.elf \

MAP_FILES += \
WheelchairControl.map \
>>>>>>> 786f1618a70ce4136384469ca2ae15a95eecd699

SIZE_OUTPUT += \
default.size.stdout \

OBJDUMP_LIST += \
<<<<<<< HEAD
WheelchairControl-gitee.list \
=======
WheelchairControl.list \
>>>>>>> 786f1618a70ce4136384469ca2ae15a95eecd699


# 所有目标
all: main-build

# 主构建目标
<<<<<<< HEAD
main-build: WheelchairControl-gitee.elf secondary-outputs

# 工具调用
WheelchairControl-gitee.elf WheelchairControl-gitee.map: $(OBJS) $(USER_OBJS) C:\StudyHub\竞赛资料\大创资料\new-gitee\Voice-Activated-Smart-Wheelchair\src\stm32\WheelchairControl\STM32F407VGTX_FLASH.ld makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-gcc -o "WheelchairControl-gitee.elf" @"objects.list" $(USER_OBJS) $(LIBS) -mcpu=cortex-m4 -T"C:\StudyHub\竞赛资料\大创资料\new-gitee\Voice-Activated-Smart-Wheelchair\src\stm32\WheelchairControl\STM32F407VGTX_FLASH.ld" --specs=nosys.specs -Wl,-Map="WheelchairControl-gitee.map" -Wl,--gc-sections -static --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -Wl,--start-group -lc -lm -Wl,--end-group
=======
main-build: WheelchairControl.elf secondary-outputs

# 工具调用
WheelchairControl.elf WheelchairControl.map: $(OBJS) $(USER_OBJS) C:\StudyHub\竞赛资料\大创资料\new-gitee\Voice-Activated-Smart-Wheelchair\src\stm32\WheelchairControl\STM32F407VGTX_FLASH.ld makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-gcc -o "WheelchairControl.elf" @"objects.list" $(USER_OBJS) $(LIBS) -mcpu=cortex-m4 -T"C:\StudyHub\竞赛资料\大创资料\new-gitee\Voice-Activated-Smart-Wheelchair\src\stm32\WheelchairControl\STM32F407VGTX_FLASH.ld" --specs=nosys.specs -Wl,-Map="WheelchairControl.map" -Wl,--gc-sections -static --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -Wl,--start-group -lc -lm -Wl,--end-group
>>>>>>> 786f1618a70ce4136384469ca2ae15a95eecd699
	@echo '已结束构建目标： $@'
	@echo ' '

default.size.stdout: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-size  $(EXECUTABLES)
	@echo '已结束构建： $@'
	@echo ' '

<<<<<<< HEAD
WheelchairControl-gitee.list: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-objdump -h -S $(EXECUTABLES) > "WheelchairControl-gitee.list"
=======
WheelchairControl.list: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-objdump -h -S $(EXECUTABLES) > "WheelchairControl.list"
>>>>>>> 786f1618a70ce4136384469ca2ae15a95eecd699
	@echo '已结束构建： $@'
	@echo ' '

# 其他目标
clean:
<<<<<<< HEAD
	-$(RM) WheelchairControl-gitee.elf WheelchairControl-gitee.list WheelchairControl-gitee.map default.size.stdout
=======
	-$(RM) WheelchairControl.elf WheelchairControl.list WheelchairControl.map default.size.stdout
>>>>>>> 786f1618a70ce4136384469ca2ae15a95eecd699
	-@echo ' '

secondary-outputs: $(SIZE_OUTPUT) $(OBJDUMP_LIST)

fail-specified-linker-script-missing:
	@echo 'Error: Cannot find the specified linker script. Check the linker settings in the build configuration.'
	@exit 2

warn-no-linker-script-specified:
	@echo 'Warning: No linker script specified. Check the linker settings in the build configuration.'

.PHONY: all clean dependents main-build fail-specified-linker-script-missing warn-no-linker-script-specified

-include ../makefile.targets
