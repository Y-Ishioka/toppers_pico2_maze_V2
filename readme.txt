◆TOPPERSライセンスについて
当プログラムは一部，TOPPERSライセンス下のソースを流用・改造して作成されています．
機器に組み込んで利用する場合にはTOPPERSライセンスを確認してください．
https://www.toppers.jp/license.html

◆免責
プログラムを用いることで生じるいかなる損害に関して，当プログラムの著作者，
流用元ソースの著作権者・管理者は一切の責任を負わないものとします．


◆ビルドメモ

----------------------------------------
drwxr-xr-x 16 ubuntu ubuntu 4096 Aug 19 07:20 asp/
drwxrwxr-x  2 ubuntu ubuntu 4096 Sep  4 19:58 build/
drwxrwxr-x  3 ubuntu ubuntu 4096 Sep  4 20:01 build-sdk/
drwxrwxr-x  2 ubuntu ubuntu 4096 Sep  4 20:02 pico-sdk/
drwxrwxr-x  4 ubuntu ubuntu 4096 Sep  2 21:03 src/
----------------------------------------

----------
git clone -b 2.2.0 https://github.com/raspberrypi/pico-sdk.git
cd build-sdk
export PICO_SDK_PATH=../pico-sdk
cmake -DPICO_PLATFORM=rp2350 -DPICO_BOARD=pico2 ../pico-sdk
make picotoolBuild
----------

----------
cd ../build
make
----------

----------------------------------------
-rwxrwxr-x 1 ubuntu ubuntu 1312012 Sep  4 20:38 asp*
-rwxrwxr-x 1 ubuntu ubuntu 1312012 Sep  4 20:38 asp.elf*
-rwxrwxr-x 1 ubuntu ubuntu 1177500 Sep  4 20:38 asp.srec*
-rw-rw-r-- 1 ubuntu ubuntu   29394 Sep  4 20:38 asp.syms
-rw-rw-r-- 1 ubuntu ubuntu  785408 Sep  4 20:38 asp.uf2
----------------------------------------


◆ラズパイPico2用TOPPERS/ASP 基本ビルド メモ
----------
wget https://www.toppers.jp/download.cgi/asp-1.9.3.tar.gz
wget https://www.toppers.jp/download.cgi/asp_arch_arm_m33_gcc-1.9.3.tar.gz
wget https://www.toppers.jp/download.cgi/cfg-1.9.6.tar.gz
----------
tar xzf asp-1.9.3.tar.gz
tar xzf asp_arch_arm_m33_gcc-1.9.3.tar.gz
tar xzf cfg-1.9.6.tar.gz
----------

``` cfg/toppers/text.hpp
 54 #include "toppers/text_line.hpp"
 55 #include "toppers/misc.hpp"
 56 #include <boost/next_prior.hpp>  /* ※ 追加 */
 57 #include <boost/utility.hpp>
 58 #include <boost/iterator/iterator_facade.hpp>
```

``` cfg/toppers/cpp.hpp
 44 #include "toppers/codeset.hpp"
 45 #include "toppers/diagnostics.hpp"
 46 #include <boost/next_prior.hpp>  /* ※ 追加 */
 47 #include <boost/utility.hpp>
 48 #include <boost/filesystem/path.hpp>
```

----------
cd cfg
mv configure configure.org
tr -d '\r' < configure.org > configure
chmod u+x configure
----------
make OPTIONS=-std=c++11
----------
mkdir -p ../asp/cfg/cfg
cp ./cfg/cfg ../asp/cfg/cfg/cfg
----------

``` asp/target/raspberrypi_pico2_gcc/target_serial.h
 64 #define TX1_PINPOS         0
 65 #define RX1_PINPOS         1
 66 #define TX2_PINPOS         8
 67 #define RX2_PINPOS         9
```

``` asp/target/raspberrypi_pico2_gcc/Makefile.target
 70 #
 71 #  シリアルポートの定義
 72 #
 73 COPTS := $(COPTS) -DSIO_PORTID=1
```

----------
cd ..
mkdir build
cd build
----------
../asp/configure -T raspberrypi_pico2_gcc -dROM
----------
make
----------


