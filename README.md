## step1
- pointcloud, image 
- tcp bridge

## step2
- protobuf

## step3
- visualization by opengl

## step4
- /tmp/omega unix domain socket (by protobuf)


## generate proto cpp

    ~/.conan/data/protoc_installer/3.9.1/bincrafters/stable/package/c0c1ef10e3d0ded44179e28b669d6aed0277ca6a/bin/protoc --proto_path=../proto/ --cpp_out=. ../proto/*.proto