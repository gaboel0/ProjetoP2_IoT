# Executando um projeto no emulador

Na pasta raiz do projeto:

```bash
qemu-system-xtensa -nographic -machine esp32 -serial mon:stdio -drive file=.pio/build/esp32doit-devkit-v1/qemu_flash.bin,if=mtd,format=raw,id=flash
```
