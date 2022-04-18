# shellcheck disable=SC2034
SKIPUNZIP=1

FLAVOR=zygisk

enforce_install_from_magisk_app() {
  if $BOOTMODE; then
    ui_print "- Installing from Magisk app"
  else
    ui_print "*********************************************************"
    ui_print "! Install from recovery is NOT supported"
    ui_print "! Recovery sucks"
    ui_print "! Please install from Magisk app"
    abort "*********************************************************"
  fi
}

check_magisk_version() {
  ui_print "- Magisk version: $MAGISK_VER_CODE"
  if [ "$MAGISK_VER_CODE" -lt 24000 ]; then
    ui_print "*********************************************************"
    ui_print "! Please install Magisk v24.0+ (24000+)"
    abort    "*********************************************************"
  fi
}

VERSION=$(grep_prop version "${TMPDIR}/module.prop")
ui_print "- Module version ${VERSION}"

# Extract verify.sh
ui_print "- Extracting verify.sh"
unzip -o "$ZIPFILE" 'verify.sh' -d "$TMPDIR" >&2
if [ ! -f "$TMPDIR/verify.sh" ]; then
  ui_print "*********************************************************"
  ui_print "! Unable to extract verify.sh!"
  ui_print "! This zip may be corrupted, please try downloading again"
  abort    "*********************************************************"
fi
. "$TMPDIR/verify.sh"

extract "$ZIPFILE" 'customize.sh' "$TMPDIR"
extract "$ZIPFILE" 'verify.sh' "$TMPDIR"

check_magisk_version
enforce_install_from_magisk_app

# Check architecture
if [ "$ARCH" != "arm" ] && [ "$ARCH" != "arm64" ] && [ "$ARCH" != "x86" ] && [ "$ARCH" != "x64" ]; then
  abort "! Unsupported platform: $ARCH"
else
  ui_print "- Device platform: $ARCH"
fi

if [ "$API" -lt 27 ]; then
  abort "! Only support SDK 27+ devices"
fi

extract "$ZIPFILE" 'module.prop'        "$MODPATH"
extract "$ZIPFILE" 'service.sh'         "$MODPATH"
extract "$ZIPFILE" 'uninstall.sh'       "$MODPATH"

ui_print "- Extracting zygisk libraries"

if [ "$FLAVOR" == "zygisk" ]; then
  mkdir -p "$MODPATH/zygisk"
  if [ "$ARCH" = "arm" ] || [ "$ARCH" = "arm64" ]; then
    extract "$ZIPFILE" "lib/armeabi-v7a/libjussigad.so" "$MODPATH/zygisk" true
    mv "$MODPATH/zygisk/libjussigad.so" "$MODPATH/zygisk/armeabi-v7a.so"

    if [ "$IS64BIT" = true ]; then
      extract "$ZIPFILE" "lib/arm64-v8a/libjussigad.so" "$MODPATH/zygisk" true
      mv "$MODPATH/zygisk/libjussigad.so" "$MODPATH/zygisk/arm64-v8a.so"
    fi
  fi

  if [ "$ARCH" = "x86" ] || [ "$ARCH" = "x64" ]; then
    extract "$ZIPFILE" "lib/x86_64/libjussigad.so" "$MODPATH/zygisk" true
    mv "$MODPATH/zygisk/libjussigad.so" "$MODPATH/zygisk/x86_64.so"

    if [ "$IS64BIT" = true ]; then
      extract "$ZIPFILE" "lib/x86/libjussigad.so" "$MODPATH/zygisk" true
      mv "$MODPATH/zygisk/libjussigad.so" "$MODPATH/zygisk/x86.so"
    fi
  fi
fi


ui_print "- Extracting system libraries"
mkdir "$MODPATH/system"
mkdir "$MODPATH/system/lib"
mkdir "$MODPATH/system/lib64"
if [ "$ARCH" = "arm" ] || [ "$ARCH" = "arm64" ]; then
  ui_print "- Extracting arm libraries"
  extract "$ZIPFILE" "system/lib/libminitool.so" "$MODPATH/system/lib" true
#  extract "$ZIPFILE" "system/lib/libminitool.config.so" "$MODPATH/system/lib" true

  if [ "$IS64BIT" = true ]; then
    ui_print "- Extracting arm64 libraries"
    extract "$ZIPFILE" "system/lib64/libminitool.so" "$MODPATH/system/lib64" true
#    extract "$ZIPFILE" "system/lib64/libminitool.config.so" "$MODPATH/system/lib64" true
  fi
fi

extract "$ZIPFILE" "system/etc/minitool.prop" "$MODPATH/system/etc" true

set_perm_recursive "$MODPATH" 0 0 0755 0644
