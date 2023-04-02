### Image file management

```
# Create disk image from tsv (inside image build artifact root)
scripts/create_sdcard_from_flashlayout.sh flashlayout_st-image-weston/trusted/FlashLayout_sdcard_stm32mp157f-dk2-trusted.tsv`

# Load disk image into sdcard
sudo dd if=FlashLayout_sdcard_stm32mp157f-dk2-trusted.raw of=/dev/sdX bs=8M conv=fdatasync status=progress

# Make one partition within disk image a storage device
fdisk -lu <image file>
sudo losetup -o <block_size * partition_offset> /dev/loop0 <image file>
```

### Distro building

```
repo init -u https://github.com/STMicroelectronics/oe-manifest.git -b refs/tags/openstlinux-5.15-yocto-kirkstone-mp1-v22.11.23
repo sync

DISTRO=openstlinux-eglfs MACHINE=stm32mp15-disco source layers/meta-st/scripts/envsetup.sh

sudo pacman -S chrpath cpio diffstat rpcsvc-proto
```

Patch to a flawed official recipe:

```
project layers/meta-st/meta-st-openstlinux/
diff --git a/recipes-security/optee/optee-stm32mp-addons_3.16.0.bb b/recipes-security/optee/optee-stm32mp-addons_3.16.0.bb
index 4acd6f8..4861895 100644
--- a/recipes-security/optee/optee-stm32mp-addons_3.16.0.bb
+++ b/recipes-security/optee/optee-stm32mp-addons_3.16.0.bb
@@ -24,7 +24,9 @@ EXTRA_OEMAKE += " \
 "
 
 do_compile:prepend() {
+    export SYSROOT="${STAGING_INCDIR}/../.."
     export CFLAGS="${CFLAGS} --sysroot=${STAGING_DIR_HOST}"
+    export LDFLAGS="${LDFLAGS} -L${SYSROOT}/usr/lib -L${SYSROOT}/lib"
     export OPENSSL_MODULES=${STAGING_LIBDIR_NATIVE}/ossl-modules/
 }
```

```
bitbake st-image-core
```

### Distro deploying

```
cd <build_root>/tmp-glibc/deploy/images/stm32mp15-disco/

# Fails randomly so just keep trying
STM32_Programmer_CLI -c port=usb1 -w flashlayout_st-image-core/optee/FlashLayout_sdcard_stm32mp157f-dk2-optee.tsv
```

### Development

```
# Display in GUI dependency graph
bitbake -u taskexp -g <layer/recipe>

# Cleanup everything including cache
bitbake -c cleanall <recipe>

# Force rebuild recipe
bitbake -f <recipe>

# Tip: insert a line in do_install() to copy out the executable

# place <file> into board:/home/root/
# will rename old if exist
sudo ckermit kermit_ttyACM0.cfg -s <file>

# Open serial console through st-link port
# ctrl-A ctrl-X to quit
picocom -b 115200 /dev/ttyACM0
```