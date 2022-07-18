// ./main/updateSpam.c
// Spins and spams update POST requests to a server repetitively

// ===== Imports ===============================================================

#include <stdio.h>
#include <string.h>
#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_http_client.h"
#include "cJSON.h"

#define TAG "UPDATE SPAM"

// The number of ms to wait between each HTTP POST message
#define TASK_TIME_MS 100

// ===== HTTP Client Functionality =============================================

esp_http_client_handle_t setupHTTPClient() {
    // Create a new HTTP client
    esp_http_client_config_t config = {
        .url = "http://vinsdev.ml:8080/update",  // Server endpoint
        .method = HTTP_METHOD_POST,              // POST connection
    };

    // Start the HTTP session
    esp_http_client_handle_t client = esp_http_client_init(&config);
    // Set the request header to specify JSON content towards the server
    esp_http_client_set_header(client, "Content-Type", "application/json");

    ESP_LOGI(TAG, "HTTP client setup complete.");

    // Return the client object to be used in other functions
    return client;
}

void destroyHTTPClient(esp_http_client_handle_t client) {
    // Close the connection
    esp_http_client_close(client);
    // Clean up any other objects
    esp_http_client_cleanup(client);

    ESP_LOGI(TAG, "HTTP client successfully destroyed.");

    return;
}

// Note: You are responsible for cleaning up your own POST data
void performHTTPPOST(esp_http_client_handle_t client, char *postData) {
    // Set the POST field data
    esp_http_client_set_post_field(client, postData, strlen(postData));

    // Perform the HTTP POST
    esp_err_t error = esp_http_client_perform(client);

    // Check for errors
    if (error != ESP_OK) {
        ESP_LOGE(TAG, "HTTP POST failed: %s", esp_err_to_name(error));
    } else {
        ESP_LOGI(TAG, "Sent HTTP POST request.");
    }

    return;
}

// ===== JSON formatting =======================================================

// Create a JSON string based on a scraped MAC ID
char *createJSON() {
    // Get the ESP's MAC ID
    uint8_t mac[6];
    esp_wifi_get_mac(ESP_IF_WIFI_STA, mac);
    // Convert MAC ID to string
    char macString[18];
    sprintf(macString,                        // Buffer to place data in
            "%02x:%02x:%02x:%02x:%02x:%02x",  // Format string for MAC ID
            mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]  // MAC ID fields
    );
    // Log success of MAC ID conversion
    ESP_LOGI(TAG, "Gained device MAC: %s", macString);

    // Create a JSON object based on this
    cJSON *json = cJSON_CreateObject();
    // Create a new JSON array within this
    cJSON *macArray = cJSON_CreateArray();
    cJSON_AddItemToObject(json, "deviceIDs", macArray);
    // Add the device MAC string into the macArray
    cJSON_AddItemToArray(macArray, cJSON_CreateString(macString));

    // Print this JSON to a character list
    char *jsonString = cJSON_Print(json);

    // Clean up our CJSON object (so as to not lose memory)
    cJSON_Delete(json);

    // Return our created JSON object
    return jsonString;
}

// ===== Main functionality ====================================================

void spamUpdates() {
    // Create and format the JSON data
    char *postData = createJSON();

    // Set up the HTTP client
    esp_http_client_handle_t client = setupHTTPClient();

    // Send the data as an HTTP POST, looping until we can't send anymore
    int i = 30;
    while (i--) {
        // Perform the HTTP POST request
        performHTTPPOST(postData);

        // Delay the loop
        // vTaskDelay(TASK_TIME_MS / portTICK_PERIOD_MS);
    }

    // Clean up the HTTP client
    destroyHTTPClient(client);
    // Free the JSON data
    free(postData);

    return;
}