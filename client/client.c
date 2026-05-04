#include "protocol.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
//#include <opencv2/opencv.h>
#include <json-c/json.h>
#include "../include/protocol.h"

// Load image file into memory
char* load_image_file(const char* path, size_t* size) {
    FILE* f = fopen(path, "rb");
    if (!f) {
        printf("Failed to open file: %s\n", path);
        return NULL;
    }

    fseek(f, 0, SEEK_END);
    *size = ftell(f);
    fseek(f, 0, SEEK_SET);

    char* data = (char*)malloc(*size);
    if (!data) {
        fclose(f);
        return NULL;
    }

    fread(data, 1, *size, f);
    fclose(f);

    return data;
}

// Create test polygons configuration
char* create_test_polygons_json(void) {
    struct json_object* root = json_object_new_array();

    // Polygon 1: Inclusion zone (center area)
    struct json_object* poly1 = json_object_new_object();
    json_object_object_add(poly1, "id", json_object_new_string("inclusion_zone"));
    json_object_object_add(poly1, "priority", json_object_new_int(10));
    json_object_object_add(poly1, "is_inclusive", json_object_new_boolean(1));
    json_object_object_add(poly1, "threshold", json_object_new_double(0.3));

    struct json_object* points1 = json_object_new_array();
    struct json_object* point;

    point = json_object_new_object();
    json_object_object_add(point, "x", json_object_new_double(200));
    json_object_object_add(point, "y", json_object_new_double(200));
    json_object_array_add(points1, point);

    point = json_object_new_object();
    json_object_object_add(point, "x", json_object_new_double(600));
    json_object_object_add(point, "y", json_object_new_double(200));
    json_object_array_add(points1, point);

    point = json_object_new_object();
    json_object_object_add(point, "x", json_object_new_double(600));
    json_object_object_add(point, "y", json_object_new_double(600));
    json_object_array_add(points1, point);

    point = json_object_new_object();
    json_object_object_add(point, "x", json_object_new_double(200));
    json_object_object_add(point, "y", json_object_new_double(600));
    json_object_array_add(points1, point);

    json_object_object_add(poly1, "points", points1);

    struct json_object* classes1 = json_object_new_array();
    json_object_array_add(classes1, json_object_new_string("person"));
    json_object_array_add(classes1, json_object_new_string("car"));
    json_object_array_add(classes1, json_object_new_string("dog"));
    json_object_object_add(poly1, "target_classes", classes1);

    json_object_array_add(root, poly1);

    // Polygon 2: Exclusion zone (smaller center)
    struct json_object* poly2 = json_object_new_object();
    json_object_object_add(poly2, "id", json_object_new_string("exclusion_zone"));
    json_object_object_add(poly2, "priority", json_object_new_int(5));
    json_object_object_add(poly2, "is_inclusive", json_object_new_boolean(0));
    json_object_object_add(poly2, "threshold", json_object_new_double(0.5));

    struct json_object* points2 = json_object_new_array();

    point = json_object_new_object();
    json_object_object_add(point, "x", json_object_new_double(350));
    json_object_object_add(point, "y", json_object_new_double(350));
    json_object_array_add(points2, point);

    point = json_object_new_object();
    json_object_object_add(point, "x", json_object_new_double(450));
    json_object_object_add(point, "y", json_object_new_double(350));
    json_object_array_add(points2, point);

    point = json_object_new_object();
    json_object_object_add(point, "x", json_object_new_double(450));
    json_object_object_add(point, "y", json_object_new_double(450));
    json_object_array_add(points2, point);

    point = json_object_new_object();
    json_object_object_add(point, "x", json_object_new_double(350));
    json_object_object_add(point, "y", json_object_new_double(450));
    json_object_array_add(points2, point);

    json_object_object_add(poly2, "points", points2);

    struct json_object* classes2 = json_object_new_array();
    json_object_array_add(classes2, json_object_new_string("person"));
    json_object_object_add(poly2, "target_classes", classes2);

    json_object_array_add(root, poly2);

    const char* json_str = json_object_to_json_string(root);
    char* result = strdup(json_str);
    json_object_put(root);

    return result;
}

int main(int argc, char** argv) {
    const char* server_ip = "127.0.0.1";
    int port = 9090;
    const char* image_path = "test.jpg";

    // Parse command line arguments
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--server") == 0 && i + 1 < argc) {
            server_ip = argv[++i];
        } else if (strcmp(argv[i], "--port") == 0 && i + 1 < argc) {
            port = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--image") == 0 && i + 1 < argc) {
            image_path = argv[++i];
        } else if (strcmp(argv[i], "--help") == 0) {
            printf("Polygon Detection Client\n");
            printf("Usage: %s [options]\n", argv[0]);
            printf("Options:\n");
            printf("  --server IP     Server IP address (default: 127.0.0.1)\n");
            printf("  --port PORT     Server port (default: 9090)\n");
            printf("  --image PATH    Path to image file (default: test.jpg)\n");
            printf("  --help          Show this help\n");
            return 0;
        }
    }

    printf("========================================\n");
    printf("Polygon Detection Client v1.0\n");
    printf("========================================\n");
    printf("Server: %s:%d\n", server_ip, port);
    printf("Image: %s\n", image_path);
    printf("========================================\n\n");

    // Load image
    size_t image_size;
    char* image_data = load_image_file(image_path, &image_size);
    if (!image_data) {
        printf("Error: Cannot load image file '%s'\n", image_path);
        return 1;
    }
    printf("Loaded image: %zu bytes\n", image_size);

    // Create polygons configuration
    char* polygons_json = create_test_polygons_json();
    printf("Polygons config: %s\n", polygons_json);

    // Connect to server
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        free(image_data);
        free(polygons_json);
        return 1;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    if (inet_pton(AF_INET, server_ip, &addr.sin_addr) <= 0) {
        perror("inet_pton");
        close(sockfd);
        free(image_data);
        free(polygons_json);
        return 1;
    }

    if (connect(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("connect");
        close(sockfd);
        free(image_data);
        free(polygons_json);
        return 1;
    }
    printf("Connected to server\n");

    // Create request
    MessagePayload* request = protocol_create_request(image_data, image_size, polygons_json);
    if (!request) {
        printf("Error: Failed to create request\n");
        close(sockfd);
        free(image_data);
        free(polygons_json);
        return 1;
    }

    // Send request
    if (protocol_write_message(sockfd, request) < 0) {
        printf("Error: Failed to send request\n");
        protocol_free_payload(request);
        close(sockfd);
        free(image_data);
        free(polygons_json);
        return 1;
    }
    printf("Request sent\n");

    // Read response header
    MessageHeader* header = protocol_read_header(sockfd);
    if (!header) {
        printf("Error: Failed to read response header\n");
        protocol_free_payload(request);
        close(sockfd);
        free(image_data);
        free(polygons_json);
        return 1;
    }

    // Read response payload
    size_t payload_size = header->payload_size;
    uint8_t* payload = (uint8_t*)malloc(payload_size);
    if (!payload) {
        free(header);
        protocol_free_payload(request);
        close(sockfd);
        free(image_data);
        free(polygons_json);
        return 1;
    }

    ssize_t bytes_read = recv(sockfd, payload, payload_size, MSG_WAITALL);
    if (bytes_read != (ssize_t)payload_size) {
        printf("Error: Failed to read payload\n");
        free(payload);
        free(header);
        protocol_free_payload(request);
        close(sockfd);
        free(image_data);
        free(polygons_json);
        return 1;
    }

    // Parse response
    uint8_t* result_image = NULL;
    size_t result_image_size = 0;
    char* objects_json = NULL;
    bool success = false;
    char* error = NULL;

    if (protocol_parse_response(payload, payload_size, &result_image, &result_image_size,
                                 &objects_json, &success, &error) != 0) {
        printf("Error: Failed to parse response\n");
    } else if (!success) {
        printf("Server error: %s\n", error ? error : "Unknown error");
    } else {
        printf("\n========================================\n");
        printf("Detection Results\n");
        printf("========================================\n");
        printf("Success: Yes\n");
        printf("Result image size: %zu bytes\n", result_image_size);
        printf("Objects JSON: %s\n", objects_json ? objects_json : "[]");
        printf("========================================\n");

        // Parse and display objects
        if (objects_json) {
            struct json_object* root = json_tokener_parse(objects_json);
            if (root && json_object_is_type(root, json_type_array)) {
                int num_objects = json_object_array_length(root);
                printf("\nDetected objects: %d\n", num_objects);
                for (int i = 0; i < num_objects; ++i) {
                    struct json_object* obj = json_object_array_get_idx(root, i);
                    struct json_object* temp;

                    if (json_object_object_get_ex(obj, "id", &temp)) {
                        printf("  %d: ID: %s\n", i + 1, json_object_get_string(temp));
                    }
                }
                json_object_put(root);
            }
        }
    }

    // Cleanup
    if (result_image) free(result_image);
    if (objects_json) free(objects_json);
    if (error) free(error);
    free(payload);
    free(header);
    protocol_free_payload(request);
    close(sockfd);
    free(image_data);
    free(polygons_json);

    printf("\nClient finished\n");
    return 0;
}
