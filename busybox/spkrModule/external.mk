SPKRMODULE_MODULE_VERSION = 1.0
SPKRMODULE_SITE = ../proyecto/SE-Altavoz/kernel
SPKRMODULE_SITE_METHOD = local
SPKRMODULE_LICENSE = GPLv2

define KERNEL_MODULE_BUILD_CMDS
        $(MAKE) -C '$(@D)' LINUX_DIR='$(LINUX_DIR)' CC='$(TARGET_CC)' LD='$(TARGET_LD)' modules
endef

$(eval $(kernel-module))
$(eval $(generic-package))
