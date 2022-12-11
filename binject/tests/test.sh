set -e

cp /bin/ls ./bin/test
./bin/binject ./bin/test hello.bin '.injected' 0x800000 0
./bin/test | grep 'hello world!' > /dev/null