#Makefile
##include $(TOPDIR)/rules.mk

# Nameand release number of this package
##PKG_NAME:=dabai
##PKG_RELEASE:=1

# This specifies the directory where we're going to build the program.
# The root build directory, $(BUILD_DIR), is by default the build_mipsel
# directory in your OpenWrt SDK directory
##PKG_BUILD_DIR:= $(BUILD_DIR)/$(PKG_NAME)

##include $(INCLUDE_DIR)/package.mk

# Specify package information for this program.
# The variables defined here should be self explanatory.
# If youare running Kamikaze, delete the DESCRIPTION
# variable below and uncomment the Kamikaze define
# directivefor the description below
##define Package/dabai
##    SECTION:=utils
##    CATEGORY:=Utilities
##    TITLE:=DaibaiTest -- Wireless Charge module
##endef


# Specifywhat needs to be done to prepare for building the package.
# In ourcase, we need to copy the source files to the build directory.
# This isNOT the default.  The default uses thePKG_SOURCE_URL and the
#PKG_SOURCE which is not defined here to download the source from the web.
# Inorder to just build a simple program that we have just written, it is
# mucheasier to do it this way.
##define Build/Prepare
##    mkdir -p $(PKG_BUILD_DIR)
##    $(CP) ./src/* $(PKG_BUILD_DIR)/
##endef

# We donot need to define Build/Configure or Build/Compile directives
# Thedefaults are appropriate for compiling a simple program such as this one
# Specifywhere and how to install the program. Since we only have one file,
# thehello executable, install it by copying it to the /bin directory on
# therouter. The $(1) variable represents the root directory on the router running
#OpenWrt. The $(INSTALL_DIR) variable contains a command to prepare the install
#directory if it does not already exist. Likewise $(INSTALL_BIN) contains the
# commandto copy the binary file from its current location (in our case the build
#directory) to the install directory.
##define Package/dabai/install
##    $(INSTALL_DIR) $(1)/bin
##    $(INSTALL_BIN) $(PKG_BUILD_DIR)/dabai $(1)/bin/
##endef

# This line executes the necessary commands to compile our program.
# The above define directives specify all the information needed, but this
# line calls BuildPackage which in turn actually uses this information to
# build apackage.
##$(eval $(call BuildPackage,dabai))

#
# Copyright (C) 2011-2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=dabai
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define Package/dabai
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=Dabai Test for Wireless Charge Module
  # MAINTAINER:=Jo-Philipp Wich <xm@subsignal.org>
endef

define Package/dabai/description
 This package contains wireless charge utility which
 use bluetooth spp function.
endef

define Build/Prepare
	$(INSTALL_DIR) $(PKG_BUILD_DIR)
	$(INSTALL_DATA) ./src/uart.h $(PKG_BUILD_DIR)/
	$(INSTALL_DATA) ./src/uart.c $(PKG_BUILD_DIR)/
	$(INSTALL_DATA) ./src/dabai.c $(PKG_BUILD_DIR)/
endef

define Build/Compile
	$(TARGET_CC) $(TARGET_CFLAGS) -Wall \
		-o $(PKG_BUILD_DIR)/dabai $(PKG_BUILD_DIR)/dabai.c $(PKG_BUILD_DIR)/uart.c
endef

define Package/dabai/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/dabai $(1)/usr/bin/
	$(CP) ./files/* $(1)
endef

$(eval $(call BuildPackage,dabai))
