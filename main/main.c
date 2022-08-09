// ./main/main.c
// Contains the application entrypoint

#define TAG "MAIN"

// ===== Imports ===============================================================

// ===== App entry =============================================================

// After connecting to WiFi...
void OnConnected(void *params) {
    // Spam a series of updates to the server
    spamUpdates();

    // Delete the task after completion
    vTaskDelete(NULL);
}

// App entry (the first program run)
void app_main(void) {
    // Connect to WiFi
    connectToWiFi();

    ESP_LOGI(TAG, "Control passed back from Wifi to main task.");

    // Create a task after Wifi completion of connection
    xTaskCreate(&OnConnected,             // Task entry function
                "Handle communications",  // Descriptive name for task
                1024 * 10,  // Number of words to allocate for task stack
                NULL,       // Parameters passed to the task
                5,          // Task priority (privileges, etc)
                NULL        // Pass a handler to the created task
    );
}