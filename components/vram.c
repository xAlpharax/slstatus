#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "../slstatus.h"
#include "../util.h"

// Function to run a command and capture its output
char* runCommand(const char* command) {
    char buffer[128];
    char* result = NULL;
    FILE* pipe = popen(command, "r");

    if (!pipe) {
        perror("popen");
        exit(1);
    }

    while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
        if (result == NULL) {
            result = strdup(buffer);
        } else {
            char* old_result = result;
            result = (char*)malloc(strlen(old_result) + strlen(buffer) + 1);
            strcpy(result, old_result);
            strcat(result, buffer);
            free(old_result);
        }
    }

    pclose(pipe);
    return result;
}

// Funtion to gather used VRAM information (MiB)
float GetUsedVRAM() {
    char* command_output;
    float vram_used = -1.0;

    // Check if an NVIDIA card is present
    command_output = runCommand("nvidia-smi --query-gpu=memory.used --format=csv,noheader,nounits"); // Returns MiB
    if (command_output != NULL) {
        sscanf(command_output, "%f", &vram_used);
        free(command_output);
    } else {
        // If no NVIDIA card is found, check for an AMD Radeon card
        // todo intel: "intel-gpu-top -b -s 1 -o - | grep 'used:'"
        command_output = runCommand("cat /sys/class/drm/card0/device/mem_info_vram_used"); // Returns Bytes
        if (command_output != NULL) {
            sscanf(command_output, "%f", &vram_used);
            vram_used = vram_used / 1024 / 1024; // Bytes --> MiB
            free(command_output);
        }
    }

    return vram_used;
}

// Funtion to gather total VRAM information (MiB)
float GetTotalVRAM() {
    char* command_output;
    float vram_total = -1.0;

    // Check if an NVIDIA card is present
    command_output = runCommand("nvidia-smi --query-gpu=memory.total --format=csv,noheader,nounits");// Returns MiB
    if (command_output != NULL) {
        sscanf(command_output, "%f", &vram_total);
        free(command_output);
    } else {
        // If no NVIDIA card is found, check for an AMD Radeon card
        // todo intel: "intel-gpu-top -b -s 1 -o - | grep 'total:'"
        command_output = runCommand("cat /sys/class/drm/card0/device/mem_info_vram_total"); // Returns Bytes
        if (command_output != NULL) {
            sscanf(command_output, "%f", &vram_total);
            vram_total = vram_total / 1024 / 1024; // Bytes --> MiB
            free(command_output);
        }
    }

    return vram_total;
}

/// ACTUAL FUNCTIONS TO CALL IN CONFIG.H

const char *
vram_perc(const char *unused) {
    float vram_used = GetUsedVRAM();
    float vram_total = GetTotalVRAM();

    if (vram_used >= 0.0 && vram_total >= 0.0) {
        int vram_percentage = (int) vram_used / vram_total * 100;
        return bprintf("%d", vram_percentage);
    } else {
        return NULL;
    }
}

const char *
vram_used(const char *unused) {
    int vram_used = (int) GetUsedVRAM();

    if (vram_used >= 0.0) {
        return fmt_human(vram_used * 1024, 1024);
    } else {
        return NULL;
    }
}

const char *
vram_total(const char *unused) {
    int vram_total = (int) GetTotalVRAM();

    if (vram_total >= 0.0) {
        return fmt_human(vram_total * 1024, 1024);
    } else {
        return NULL;
    }
}
