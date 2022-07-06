// ./main/wifiConnect.c
// Handles WiFi connection to network SSID and password, as well as IP
//  generation and all that fun stuff

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

#define TAG "COMMS"

// Semaphore for passing control after WiFi and IP initialization
xSemaphoreHandle connectionSemaphore;

// ===== WiFi connection success event handler =================================

static void event_handler(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    // If we get the go-ahead to start WiFi connectivity, start it
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
        esp_wifi_connect();

    // If we get an IP, we can return control back to the main loop
    if (event_id == IP_EVENT_STA_GOT_IP)
        xSemaphoreGive(connectionSemaphore);
}

// ===== Wifi initialization ===================================================

// Initialize and error-check all WiFi connections
void initializeWiFi() {
    // Initialize non-volatile storage partitions
    ESP_ERROR_CHECK(nvs_flash_init());
    // Initialize underlying TCP/IP stack behind HTTP structure
    ESP_ERROR_CHECK(esp_netif_init());
    // Create the default event loop for future task creation
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    // Create the default WiFi station
    esp_netif_create_default_wifi_sta();

    // Initialize a WiFi connection with default configurations
    wifi_init_config_t wifi_init_config = WIFI_INIT_CONFIG_DEFAULT();
    // Allocate resources for WiFi tasks and start WiFi task in background
    ESP_ERROR_CHECK(esp_wifi_init(&wifi_init_config));
    // Register a WiFi event to the event loop (for getting WiFi information)
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, event_handler, NULL));
    // Register an IP event to the event loop (for getting IP)
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, event_handler, NULL));
    // Store WiFi information in system RAM
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));

    // WiFi configuration parameters
    wifi_config_t wifi_config = {
        .sta = {
            // Data passed in from idf.py menuconfig parameters
            .ssid = CONFIG_WIFI_SSID,
            .password = CONFIG_WIFI_PASSWORD,
        },
    };
    // Set the operating WiFi mode to require a router to connect to
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config);

    // Start WiFi according to previously set configurations
    ESP_ERROR_CHECK(esp_wifi_start());
}

void connectToWiFi() {
    // If the semaphore isn't initialized, initialize it
    if (!connectionSemaphore)
        connectionSemaphore = xSemaphoreCreateBinary();

    // Handle initialization of WiFi quickly
    initializeWiFi();

    // Wait for IP to be initialized before returning
    xSemaphoreTake(connectionSemaphore, portMAX_DELAY);
}