#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BRIGHTNESS_PATH "/sys/class/backlight/acpi_video0/brightness"
#define PROC_FILE "/proc/mouse_brightness"

void change_brightness(int delta) {
    int brightness;
    FILE *file = fopen(BRIGHTNESS_PATH, "r+");
    if (!file) {
        perror("Failed to open brightness file");
        return;
    }
    fscanf(file, "%d", &brightness);
    brightness += delta;

    // Clamp brightness between 5-100
    if (brightness < 5) brightness = 5;
    if (brightness > 100) brightness = 100;

    fseek(file, 0, SEEK_SET);
    fprintf(file, "%d\n", brightness);
    fclose(file);
}

int main() {
    char buffer[10];

    while (1) {
        FILE *proc = fopen(PROC_FILE, "r");
        if (!proc) {
            perror("Failed to open /proc/mouse_brightness");
            return 1;
        }
        fgets(buffer, sizeof(buffer), proc);
        fclose(proc);

        if (strcmp(buffer, "left\n") == 0) {
            printf("Left click detected. Increasing brightness.\n");
            change_brightness(5);
        } else if (strcmp(buffer, "right\n") == 0) {
            printf("Right click detected. Decreasing brightness.\n");
            change_brightness(-5);
        }

        usleep(500000);  // Check every 0.5 seconds
    }

    return 0;
}
