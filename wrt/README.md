# Quick installation guide

Assume fully functional OpenWRT development environment is installed into folder

/tmp/openwrt

1. Upadate OpenWRT environment

cd /tmp/openwrt/source

./scripts/feeds update -a

./scripts/feeds install -a

2. Get the newest version of the software:
git clone https://github.com/dizainioras/wcc.git

Place packet description makefile (./wcc-pkg/Makefile) into folder: /tmp/openwrt/mypackages/wcc

Copy source code files (./wcc-src/*) into: /tmp/openwrt/wcc

Edit file /tmp/openwrt/mypackages/wcc/Makefile
and provide correct path for source code:

SOURCE_DIR:=/tmp/openwrt/wcc

3. Update OpenWRT development environment.

cd /tmp/openwrt/source
./scripts/feeds update mypackages
./scripts/feeds install -a -p mypackages

4. Enable wcc software package generation

cd /tmp/openwrt/source
make menuconfig

Check followting options:
KTU/wcc - ON <*>
Libraries/openblas - ON <*>

5. Make software package:

make -j1 V=sc package/wcc/clean
make -j1 V=sc package/wcc/compile

6. Check the results (assume mipsel_24kc architecture is used on target router)

OpewnWRT software package should be loacated:
openwrt/source/bin/packages/mipsel_24kc/mypackages/wcc_1.2-2_mipsel_24kc.ipk

Executable should be located:
openwrt/source/build_dir/target-mipsel_24kc_musl/wcc-1.2/wcc 


