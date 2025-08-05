Assume fully functional openwrt development environment is installed into folder
/tmp/openwrt

1. Upadate openwrt environment

cd /tmp/openwrt/source
./scripts/feeds update -a
./scripts/feeds install -a

2. git clone:

Packet makefile place into folder: /tmp/openwrt/mypackages/wcc

Place source folder into: /tmp/openwrt/wcc

Edit file /tmp/openwrt/mypackages/wcc/Makefile
SOURCE_DIR:=/tmp/openwrt/wcc

3. Update openwert env.

cd /tmp/openwrt/source
./scripts/feeds update mypackages
./scripts/feeds install -a -p mypackages

4. Enable 

cd /tmp/openwrt/source
make menuconfig

Check:
KTU/wcc - ON <*>
Libraries/openblas - ON <*>

5. Make software:

make -j1 V=sc package/wcc/clean
make -j1 V=sc package/wcc/compile

6. Results

openwrt/source/bin/packages/mipsel_24kc/mypackages/wcc_1.2-2_mipsel_24kc.ipk
openwrt/source/build_dir/target-mipsel_24kc_musl/wcc-1.2/wcc 


