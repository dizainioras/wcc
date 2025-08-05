include $(TOPDIR)/rules.mk

# Name, version and release number
# The name and version of your package are used to define the variable to point to the build directory of your package: $(PKG_BUILD_DIR)
PKG_NAME:=wcc
PKG_VERSION:=1.2
PKG_RELEASE:=2

# Source settings (i.e. where to find the source codes)
# This is a custom variable, used below
SOURCE_DIR:=/e/linsoft/openwrt/wcc

include $(INCLUDE_DIR)/package.mk

# Package definition; instructs on how and where our package will appear in the overall configuration menu ('make menuconfig')
define Package/wcc
  SECTION:=wcc
  CATEGORY:=KTU
  TITLE:=KTU-WCC
  DEPENDS:=+libcurl +openblas +libstdcpp
endef

# Package description; a more verbose description on what our package does
define Package/wcc/description
  A simple "wcc" -application.
endef

# Package preparation instructions; create the build directory and copy the source code. 
# The last command is necessary to ensure our preparation instructions remain compatible with the patching system.
define Build/Prepare
		mkdir -p $(PKG_BUILD_DIR)
		cp -r $(SOURCE_DIR)/* $(PKG_BUILD_DIR)
		$(Build/Patch)
endef

# Package build instructions; invoke the GNU make tool to build our package
define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
               CC="$(TARGET_CC)" \
               CXX="$(TARGET_CXX)" \
           CFLAGS="$(TARGET_CFLAGS)" \
          LDFLAGS="$(TARGET_LDFLAGS)"
endef


define Package/wcc/install
		$(INSTALL_DIR) $(1)/usr/bin
		$(INSTALL_BIN) $(PKG_BUILD_DIR)/wcc $(1)/usr/bin
endef

$(eval $(call BuildPackage,wcc,+libopenblas))
