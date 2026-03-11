# GirOS mini DOS

Минимальный учебный проект ОС в стиле DOS:

- **Bootloader**: GRUB + Multiboot заголовок (`src/boot.asm`)
- **Kernel**: 32-битный freestanding kernel (`src/kernel.c`)
- **Shell**: встроенная командная строка `GirOS>` с командами `help`, `clear`, `ver`, `echo`

## Сборка ISO

```bash
cd mini_dos
./build_iso.sh
```

На выходе создается файл:

- `mini_dos/GirOS.iso`

## Запуск в эмуляторе (пример)

```bash
qemu-system-i386 -cdrom GirOS.iso
```

## Запуск на реальном железе

> ⚠️ Делай это только на тестовой машине или с полной резервной копией данных.

### 1) Подготовить ISO

```bash
cd mini_dos
./build_iso.sh
```

### 2) Записать ISO на USB-флешку

На Linux/macOS:

```bash
# Узнай устройство флешки (например /dev/sdb)
lsblk

# ВНИМАНИЕ: команда полностью перезапишет флешку
sudo dd if=GirOS.iso of=/dev/sdX bs=4M status=progress oflag=sync
sync
```

На Windows:

- Используй **Rufus** или **balenaEtcher**.
- Выбери `GirOS.iso` и USB-накопитель, затем запиши образ.

### 3) Настроить BIOS/UEFI

- Отключи **Secure Boot** (если включен).
- В приоритете загрузки поставь USB.
- Если есть выбор режима, предпочти **Legacy/CSM** (GRUB ISO тут BIOS-ориентированный).

### 4) Загрузка

- Перезагрузи ПК с USB.
- В меню GRUB выбери `GirOS`.

Если загрузка не идет:

- попробуй другой USB-порт (лучше USB 2.0);
- перепиши флешку заново;
- проверь, что в BIOS разрешена загрузка в Legacy/CSM режиме.
