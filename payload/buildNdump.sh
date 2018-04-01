rm -f egghunter.o egghunter.bin egghunter.rop loaderstub.o loaderstub.elf loaderstub.bin payload.bin loader.bin

cd loader
make
cp loader.bin ..
cd ..

make ropFormat
make payloadBuilder

powerpc-eabi-as egghunter.s -o egghunter.o
powerpc-eabi-objdump -D egghunter.o
powerpc-eabi-objcopy -Obinary egghunter.o egghunter.bin
./ropFormat egghunter.bin egghunter.rop

powerpc-eabi-gcc -m32 -Wall -W -O2 -ffreestanding -mno-eabi -mno-sdata -mcpu=750 -c loaderstub.s -o loaderstub.o
powerpc-eabi-ld -T loaderstub.lds loaderstub.o -o loaderstub.elf
powerpc-eabi-objcopy -Obinary loaderstub.elf loaderstub.bin
./payloadBuilder payload.bin PONY loaderstub.bin loader.bin boot.elf

rm -f ../flash-exploit/egghunter.rop
cp egghunter.rop ../flash-exploit/egghunter.rop

rm -f ../server/payload.bin
cp payload.bin ../server/
