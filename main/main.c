// ./main/main.c
// Contains the application entrypoint

// ===== Imports ===============================================================

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_http_client.h"
#include "meshMacSpam.h"

// ===== App entry =============================================================

// After connecting to WiFi...
void OnConnected(void *params) {
    // Delete the task after completion
    vTaskDelete(NULL);
}

// App entry (the first program run)
void app_main(void) {
    // Connect to WiFi
    connectToWiFi();

    // Create a task after Wifi completion of connection
    xTaskCreate(
        &OnConnected,             // Task entry function
        "Handle communications",  // Descriptive name for task
        1024 * 10,                // Number of words to allocate for task stack
        NULL,                     // Parameters passed to the task
        5,                        // Task priority (privileges, etc)
        NULL                      // Pass a handler to the created task
    );
}