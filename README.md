# How to deploy Warble project?

Run `vagrant up`. The Vagrantfile will automatically start a virtual machine and set up the environments. If the environment is not set up properly. The following commands can be used to install the packages manually. Run `vagrant ssh` to connect to the VM. 


===============

Install grpc:
```
cd ~
sudo apt-get update
sudo apt-get install build-essential autoconf libtool pkg-config
sudo apt-get install cmake
sudo apt-get install libgflags-dev
sudo apt-get install clang-5.0 libc++-dev

git clone https://github.com/grpc/grpc
cd grpc
git submodule update --init

make
sudo make install
```

================

Install protobuf:
```
cd ~
git clone https://github.com/protocolbuffers/protobuf.git
cd protobuf
git submodule update --init --recursive
./autogen.sh
./configure
make
make check
sudo make install
sudo ldconfig
```

================

Install gtest:
```
cd ~
sudo apt-get install libgtest-dev
sudo apt-get install cmake
cd /usr/src/gtest
sudo cmake CMakeLists.txt
sudo make
sudo cp *.a /usr/lib
```

================

Install glog:
```
sudo apt-get install libgoogle-glog-dev
```

================

Install gflag:
```
sudo apt-get install libgflags-dev
```




# How to build and run services?

## protos:
```
cd protos
make
```

## kvstore:
```
cd kvstore/utils
make  //run unittest: ./concurrent_hash_map_unittest

cd kvstore
make
./kvstore_server
```

The persistence mode can be activated by using "--store file" flag. The kvstore server stores the data to the given file. If the file already exists when the server starts, the server first load data from previously-stored file and then store new data to the same file.

```
./kvstore_server --store data
```

## func
```
cd func
make //run unittest: ./warble_funcs_unittest
./func_server
```

The implementation of "functions" are "ephemeral". No data is stored within func layer.

## command line tool:
```
cd command_line_tool
make
```


# How to use:
```
cd command_line_tool
```

## Hook all functions:
```
./command_line_tool --hookall
```

## Unhook all functions:
```
./command_line_tool --unhookall
```

## Register User:
To register a user with username Adam. 
```
./command_line_tool --registeruser Adam
```

## Follow a User:
To follow a user with username Bob, logging as Adam.
```
./command_line_tool --user Adam --follow Bob
```

## Read User Profile:
To read user's self profile, logging as Adam. 
 ```
./command_line_tool --user Adam --profile
```

## Post a Warble:
To post a Warble with content "HelloWorld", logging as Adam. 
```
./command_line_tool --user Adam --warble HelloWorld
```

## Reply a Warble:
To reply a Warble with warble id "wid_12345" logging as Adam. The new warble's content is "HelloReply". 
```
./command_line_tool --user Adam --warble HelloReply --reply wid_12345
```

## Read Warble thread:
To read a Warble thread starting at warble id "wid_12345", logging as Adam. 
```
./command_line_tool --user Adam --read wid_12345
```
