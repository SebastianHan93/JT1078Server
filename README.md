#安装依赖

sudo apt-get update
sudo apt install unzip
sudo apt-get install build-essential 

sudo apt-get -y install autoconf automake build-essential \
libass-dev libfreetype6-dev libsdl2-dev libtheora-dev \
libtool libva-dev libvdpau-dev libvorbis-dev libxcb1-dev \
libxcb-shm0-dev libxcb-xfixes0-dev pkg-config texinfo zlib1g-dev
sudo apt-get install yasm

sudo apt-get install libx264-dev
sudo apt-get install libfdk-aac-dev
sudo apt-get install libopus-dev
sudo apt-get install libmp3lame-dev

sudo apt-get install libx264-dev
sudo apt-get install libfdk-aac-dev
sudo apt-get install libopus-dev
sudo apt-get install libmp3lame-dev
sudo apt-get install yasm
sudo apt-get install libvpx-dev
sudo apt-get install libx264-dev

#下载libaom库
git clone https://github.com/mozilla/aom.git
cd path/to/aom
mkdir mybuild
cmake -B./mybuild -H. -DBUILD_SHARED_LIBS=1
cd mybuild
make
若下载速度慢可以用libaom.zip压缩包进行安装
unzip -d ./libaom libaom.zip

#安装ffmpeg
cd ~/ffmpeg_sources && \
wget -O ffmpeg-snapshot.tar.bz2 https://ffmpeg.org/releases/ffmpeg-snapshot.tar.bz2 && \
tar xjvf ffmpeg-snapshot.tar.bz2 && \
cd ffmpeg && \
PATH="$HOME/bin:$PATH" PKG_CONFIG_PATH="/usr/local/ffmpeg/lib/pkgconfig"
 ./configure \
--prefix="/usr/local/ffmpeg" \
--extra-cflags="-I/usr/local/ffmpeg/include" \
--extra-ldflags="-L/usr/local/ffmpeg/lib" \
--extra-libs="-lpthread -lm" \
--bindir="$HOME/bin" \
--disable-static \
--enable-shared \
--enable-gpl \
--enable-libaom \
--enable-libass \
--enable-libfdk-aac \
--enable-libfreetype \
--enable-libmp3lame \
--enable-libopus \
--enable-libvorbis \
--enable-libvpx \
--enable-libx264 \
--enable-libx265 \
--enable-nonfree && \
PATH="$HOME/bin:$PATH" make && \
sudo make install && \
hash -r

#配置可执行文件动态库查找路径
sudo vi /etc/ld.so.conf 加入一行 /usr/local/ffmpeg/lib/ 执行sudo ldconfig
配置ffmpeg 编译include lib查找路径
编辑vim ~/.bashrc
将以下环境变量加入bashrc中
export C_INCLUDE_PATH=$C_INCLUDE_PATH:/usr/local/ffmpeg/include
export CPLUS_INCLUDE_PATH=$CPLUS_INCLUDE_PATH:/usr/local/ffmpeg/include
export LIBRARY_PATH=$LIBRARY_PATH:/usr/local/ffmpeg/lib
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/ffmpeg/lib
测试ffmpeg是否编译成功

#安装依赖
sudo apt install build-essential
sudo apt install libtool
sudo apt install libpcre3 libpcre3-dev
sudo apt install zlib1g-dev
sudo apt-get install openssl libssl-dev

#解压并安装
unzip nginx-http-flv-module-master.zip
tar -zxvf nginx-1.13.10.tar.gz
cd nginx-1.13.10/
./configure --prefix=/usr/local/nginx --add-module=/home/yncw/nginx-http-flv-module-master
make 
sudo make install

修改nginx.conf文件  找到server_name 192.168.0.141;修改ip地址为服务器ip
执行 sudo mv nginx.conf /usr/local/nginx/conf/

#配置hiredis-v库
解压并安装
unzip hiredis-v-master.zip
cd /hiredis-v-master/src
make && sudo make install
添加环境变量
export C_INCLUDE_PATH=$C_INCLUDE_PATH:/usr/local/include
export CPLUS_INCLUDE_PATH=$CPLUS_INCLUDE_PATH:/usr/local/include
export LIBRARY_PATH=$LIBRARY_PATH:/usr/local/lib
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib
配置/etc/ld.so.conf
sudo vim /etc/ld.so.conf 增加hiredis-v库路径 /usr/local/lib
sudo ldconfig

# 安装cmake
sudo apt-get install cmake 
# 安装boost
sudo apt-get install libboost-dev libboost-test-dev
# 三个非必须的依赖库:curl、c-ares DNS、Google Protobuf  
sudo apt-get install libcurl4-openssl-dev libc-ares-dev
sudo apt-get install protobuf-compiler libprotobuf-dev
#编译muduo库和它自带的例子，生成的可执行文件和静态库文件位于build/release-cpp11/lib */

./build.sh -j4

#将muduo头文件和库文件安装到build/release-install-cpp11/lib
./build.sh install 
cat 10352264314-2.bin | pv -L 40k | nc 192.168.0.141 20001
ffplay rtmp://192.168.0.141:20002/live/%E4%BA%91A88888.%E8%93%9D%E8%89%B2.2.0.1