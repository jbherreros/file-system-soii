make clean
make
./mi_mkfs disco 100000
./mi_mkdir disco 7 /hola/
./mi_mkdir disco 7 /hola/luis/
./mi_touch disco 7 /hola/jose
./mi_ls disco /hola/ -d
./mi_mkdir disco 7 /pepe/


./mi_mv disco /hola/jose /pepe/
echo "que hay en hola"
./mi_ls disco /hola/ -d
echo "que hay en pepe"
./mi_ls disco /pepe/ -d