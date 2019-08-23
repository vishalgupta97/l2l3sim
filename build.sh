
if [ $# -eq 0 ]
  then
    echo "./build.sh config"
    exit 1
fi

cp inc/${1}.config inc/config.h
mkdir -p bin
rm -f bin/champsim
make clean
make
mv bin/l2l3sim bin/l2l3sim-${1}