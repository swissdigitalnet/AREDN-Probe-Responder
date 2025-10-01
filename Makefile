# OpenWRT Makefile for AREDN Probe Responder

include $(TOPDIR)/rules.mk

PKG_NAME:=aredn-probe-responder
PKG_VERSION:=1.0.1
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk

define Package/aredn-probe-responder
  SECTION:=net
  CATEGORY:=Network
  TITLE:=AREDN Mesh Network Probe Responder
  DEPENDS:=
  URL:=https://github.com/swissdigitalnet/AREDN-Probe-Responder
endef

define Package/aredn-probe-responder/description
  Lightweight UDP echo service for AREDN mesh network monitoring.
  Listens on UDP port 40050 and echoes packets back to sender.
  Compatible with AREDN-Phonebook network monitoring probes.
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Build/Compile
	$(TARGET_CC) $(TARGET_CFLAGS) $(TARGET_LDFLAGS) \
		-o $(PKG_BUILD_DIR)/probe-responder \
		$(PKG_BUILD_DIR)/probe_responder.c
endef

define Package/aredn-probe-responder/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/probe-responder $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/etc/init.d/probe-responder $(1)/etc/init.d/
endef

define Package/aredn-probe-responder/postinst
#!/bin/sh
[ -n "$${IPKG_INSTROOT}" ] || {
	/etc/init.d/probe-responder enable
	/etc/init.d/probe-responder start
}
endef

$(eval $(call BuildPackage,aredn-probe-responder))
