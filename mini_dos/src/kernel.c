#include <stdint.h>
#include <stddef.h>

#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_MEMORY ((uint16_t*)0xB8000)
#define COLOR 0x0F

static size_t cursor_x = 0;
static size_t cursor_y = 0;

static inline void outb(uint16_t port, uint8_t value)
{
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline uint8_t inb(uint16_t port)
{
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static void vga_put_entry(char c, uint8_t color, size_t x, size_t y)
{
    const size_t index = y * VGA_WIDTH + x;
    VGA_MEMORY[index] = ((uint16_t)color << 8) | (uint8_t)c;
}

static void scroll_if_needed(void)
{
    if (cursor_y < VGA_HEIGHT) return;

    for (size_t y = 1; y < VGA_HEIGHT; ++y)
    {
        for (size_t x = 0; x < VGA_WIDTH; ++x)
        {
            VGA_MEMORY[(y - 1) * VGA_WIDTH + x] = VGA_MEMORY[y * VGA_WIDTH + x];
        }
    }

    for (size_t x = 0; x < VGA_WIDTH; ++x)
    {
        vga_put_entry(' ', COLOR, x, VGA_HEIGHT - 1);
    }

    cursor_y = VGA_HEIGHT - 1;
}

static void terminal_clear(void)
{
    for (size_t y = 0; y < VGA_HEIGHT; ++y)
    {
        for (size_t x = 0; x < VGA_WIDTH; ++x)
        {
            vga_put_entry(' ', COLOR, x, y);
        }
    }
    cursor_x = 0;
    cursor_y = 0;
}

static void terminal_putchar(char c)
{
    if (c == '\n')
    {
        cursor_x = 0;
        ++cursor_y;
        scroll_if_needed();
        return;
    }

    vga_put_entry(c, COLOR, cursor_x, cursor_y);
    ++cursor_x;

    if (cursor_x >= VGA_WIDTH)
    {
        cursor_x = 0;
        ++cursor_y;
        scroll_if_needed();
    }
}

static void terminal_writestring(const char* str)
{
    for (size_t i = 0; str[i] != '\0'; ++i)
    {
        terminal_putchar(str[i]);
    }
}

static int str_equals(const char* a, const char* b)
{
    while (*a != '\0' && *b != '\0')
    {
        if (*a != *b) return 0;
        ++a;
        ++b;
    }
    return *a == '\0' && *b == '\0';
}

static char scancode_to_ascii(uint8_t sc)
{
    static const char map[128] = {
        [0x02] = '1', [0x03] = '2', [0x04] = '3', [0x05] = '4', [0x06] = '5',
        [0x07] = '6', [0x08] = '7', [0x09] = '8', [0x0A] = '9', [0x0B] = '0',
        [0x0C] = '-', [0x0D] = '=',
        [0x10] = 'q', [0x11] = 'w', [0x12] = 'e', [0x13] = 'r', [0x14] = 't',
        [0x15] = 'y', [0x16] = 'u', [0x17] = 'i', [0x18] = 'o', [0x19] = 'p',
        [0x1A] = '[', [0x1B] = ']',
        [0x1E] = 'a', [0x1F] = 's', [0x20] = 'd', [0x21] = 'f', [0x22] = 'g',
        [0x23] = 'h', [0x24] = 'j', [0x25] = 'k', [0x26] = 'l', [0x27] = ';',
        [0x28] = '\'', [0x29] = '`',
        [0x2B] = '\\',
        [0x2C] = 'z', [0x2D] = 'x', [0x2E] = 'c', [0x2F] = 'v', [0x30] = 'b',
        [0x31] = 'n', [0x32] = 'm', [0x33] = ',', [0x34] = '.', [0x35] = '/',
        [0x39] = ' '
    };

    return map[sc];
}

static char keyboard_getchar(void)
{
    while (1)
    {
        if ((inb(0x64) & 1) == 0) continue;

        uint8_t scancode = inb(0x60);
        if (scancode & 0x80) continue;

        if (scancode == 0x1C) return '\n';
        if (scancode == 0x0E) return '\b';

        char c = scancode_to_ascii(scancode);
        if (c != 0) return c;
    }
}

static void print_prompt(void)
{
    terminal_writestring("GirOS> ");
}

static void execute_command(const char* cmd)
{
    if (str_equals(cmd, "help"))
    {
        terminal_writestring("Commands: help, clear, ver, echo <text>\n");
    }
    else if (str_equals(cmd, "clear"))
    {
        terminal_clear();
    }
    else if (str_equals(cmd, "ver"))
    {
        terminal_writestring("GirOS mini DOS v0.1\n");
    }
    else if (cmd[0] == 'e' && cmd[1] == 'c' && cmd[2] == 'h' && cmd[3] == 'o' && cmd[4] == ' ')
    {
        terminal_writestring(cmd + 5);
        terminal_putchar('\n');
    }
    else if (cmd[0] != '\0')
    {
        terminal_writestring("Unknown command. Type 'help'.\n");
    }
}

void kernel_main(void)
{
    terminal_clear();
    terminal_writestring("Welcome to GirOS mini DOS\n");
    terminal_writestring("Type 'help' for available commands.\n\n");

    char input[128];

    while (1)
    {
        print_prompt();
        size_t len = 0;

        while (1)
        {
            char c = keyboard_getchar();

            if (c == '\n')
            {
                terminal_putchar('\n');
                input[len] = '\0';
                execute_command(input);
                break;
            }

            if (c == '\b')
            {
                if (len > 0)
                {
                    --len;
                    if (cursor_x == 0)
                    {
                        cursor_x = VGA_WIDTH - 1;
                        if (cursor_y > 0) --cursor_y;
                    }
                    else
                    {
                        --cursor_x;
                    }
                    vga_put_entry(' ', COLOR, cursor_x, cursor_y);
                }
                continue;
            }

            if (len + 1 < sizeof(input))
            {
                input[len++] = c;
                terminal_putchar(c);
            }
        }
    }
}
