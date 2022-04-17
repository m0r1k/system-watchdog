# binary name
TARGET              = system_watchdog
VERSION_NUMBER      = $(shell cut -d' ' -f1 VERSION)
BUILD_NUMBER        = $(BUILD_ID)

# package info
PACKAGE_SECTION     = misc
PACKAGE_PRIORITY    = optional
PACKAGE_NAME        = $(subst _,-,$(TARGET))
PACKAGE_VERSION     = $(VERSION_NUMBER)
PACKAGE_DESCR       = System watchdog kernel module for test labs
PACKAGE_MAINTAINER  = Roman E. Chechnev <interferation3@gmail.com>
PACKAGE_ARCH        = amd64
PACKAGE_DEPENDS     = linux-headers-generic

