// ./main/meshMacSpam.h
// Headerfile for all other code files and code splitting simplification

#pragma once

// ===== MAC Macros ============================================================

// MAC address length standardization - 6 bytes
#define MAC_ADDR_LEN (6U)

// Compare two MAC addresses for equality
#define MAC_ADDR_EQUAL(a, b) (memcmp(a, b, MAC_ADDR_LEN) == 0)

// ===== Typedefs ==============================================================

// Mesh event handler callback for root/non-root node
typedef void(mesh_raw_recv_cb_t)(mesh_addr_t *from, mesh_data_t *data);

// ===== Imports ===============================================================

/**
 * @brief Simplifies connection to WiFi server and returns once completed
 *
 */
void connectToWiFi();

/**
 * @brief Spams HTTP POSTs to /update/ on the external server with MAC ID
 *
 */
void spamUpdates();