//#include "geometry.h"
//#include "detector.h"
//#include "polygon_processor.h"
//#include "protocol.h"
//#include <opencv2/opencv.h>

//static int parse_polygons_from_json(const char* json_str, PolygonConfig* polygons, int* num_polygons) {
//  struct json_object* root = json_tokener_parse(json_str);
//  if (!root) return -1;
  
//  if (!json_object_is_type(root, json_type_array)) {
//    json_object_put(root);
//    return -2;
//  }
  
//  int array_len = json_object_array_length(root);
//  *num_polygons = 0;
  
//  for (int i = 0; i < array_len && i < MAX_POLYGONS; ++i) {
//    struct json_object* poly_obj = json_object_array_get_idx(root, i);
//    PolygonConfig* config = &polygons[*num_polygons];
//    memset(config, 0, sizeof(PolygonConfig));
    
//    struct json_object* temp;
    
//    if (json_object_object_get_ex(poly_obj, "id", &temp)) {
//      strncpy(config->id, json_object_get_string(temp), MAX_POLYGON_ID_LEN - 1);
//    }
    
//    if (json_object_object_get_ex(poly_obj, "priority", &temp)) {
//      config->priority = json_object_get_int(temp);
//    }
    
//    if (json_object_object_get_ex(poly_obj, "is_inclusive", &temp)) {
//      config->is_inclusive = json_object_get_boolean(temp);
//    }
    
//    if (json_object_object_get_ex(poly_obj, "threshold", &temp)) {
//      config->iou_threshold = json_object_get_double(temp);
//    }
    
//    // Parse points
//    if (json_object_object_get_ex(poly_obj, "points", &temp)) {
//      int num_points = json_object_array_length(temp);
//      for (int j = 0; j < num_points && j < POLYGON_MAX_POINTS; ++j) {
//        struct json_object* point_obj = json_object_array_get_idx(temp, j);
//        struct json_object* x_obj, * y_obj;
        
//        if (json_object_object_get_ex(point_obj, "x", &x_obj) &&
//            json_object_object_get_ex(point_obj, "y", &y_obj)) {
//          config->polygon.points[j].x = json_object_get_double(x_obj);
//          config->polygon.points[j].y = json_object_get_double(y_obj);
//        }
//      }
//      config->polygon.num_points = num_points;
//    }
    
//    // Parse target classes
//    if (json_object_object_get_ex(poly_obj, "target_classes", &temp)) {
//      int num_classes = json_object_array_length(temp);
//      for (int j = 0; j < num_classes && j < MAX_CLASSES_PER_POLYGON; ++j) {
//        struct json_object* class_obj = json_object_array_get_idx(temp, j);
//        strncpy(config->target_classes[j],
//                json_object_get_string(class_obj),
//                MAX_CLASS_NAME_LEN - 1);
//      }
//      config->num_target_classes = num_classes;
//    }
    
//    (*num_polygons)++;
//  }
  
//  json_object_put(root);
//  return 0;
//}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>
#include <json-c/json.h>

#include "../include/geometry.h"
#include "../include/detector.h"
#include "../include/polygon_processor.h"
#include "../include/protocol.h"

static ObjectDetector* g_detector = NULL;
static volatile int g_server_running = 1;

// Forward declarations
void* handle_client_thread(void* arg);

// Build polygons JSON response
char* build_polygons_json(const PolygonConfig* polygons, int num_polygons) {
    struct json_object* root = json_object_new_array();
    if (!root) return NULL;

    for (int i = 0; i < num_polygons; ++i) {
        struct json_object* poly_obj = json_object_new_object();

        json_object_object_add(poly_obj, "id",
            json_object_new_string(polygons[i].id));
        json_object_object_add(poly_obj, "priority",
            json_object_new_int(polygons[i].priority));
        json_object_object_add(poly_obj, "is_inclusive",
            json_object_new_boolean(polygons[i].is_inclusive));
        json_object_object_add(poly_obj, "threshold",
            json_object_new_double(polygons[i].iou_threshold));

        struct json_object* classes_array = json_object_new_array();
        for (int j = 0; j < polygons[i].num_target_classes; ++j) {
            json_object_array_add(classes_array,
                json_object_new_string(polygons[i].target_classes[j]));
        }
        json_object_object_add(poly_obj, "target_classes", classes_array);

        struct json_object* points_array = json_object_new_array();
        for (size_t j = 0; j < polygons[i].polygon.num_points; ++j) {
            struct json_object* point_obj = json_object_new_object();
            json_object_object_add(point_obj, "x",
                json_object_new_double(polygons[i].polygon.points[j].x));
            json_object_object_add(point_obj, "y",
                json_object_new_double(polygons[i].polygon.points[j].y));
            json_object_array_add(points_array, point_obj);
        }
        json_object_object_add(poly_obj, "points", points_array);

        json_object_array_add(root, poly_obj);
    }

    const char* json_str = json_object_to_json_string(root);
    char* result = strdup(json_str);
    json_object_put(root);

    return result;
}

// Parse polygons from JSON
int parse_polygons_from_json(const char* json_str, PolygonConfig* polygons, int* num_polygons) {
    struct json_object* root = json_tokener_parse(json_str);
    if (!root) return -1;

    if (!json_object_is_type(root, json_type_array)) {
        json_object_put(root);
        return -2;
    }

    int array_len = json_object_array_length(root);
    *num_polygons = 0;

    for (int i = 0; i < array_len && i < MAX_POLYGONS; ++i) {
        struct json_object* poly_obj = json_object_array_get_idx(root, i);
        PolygonConfig* config = &polygons[*num_polygons];
        memset(config, 0, sizeof(PolygonConfig));

        struct json_object* temp;

        if (json_object_object_get_ex(poly_obj, "id", &temp)) {
            strncpy(config->id, json_object_get_string(temp), MAX_POLYGON_ID_LEN - 1);
        }

        if (json_object_object_get_ex(poly_obj, "priority", &temp)) {
            config->priority = json_object_get_int(temp);
        }

        if (json_object_object_get_ex(poly_obj, "is_inclusive", &temp)) {
            config->is_inclusive = json_object_get_boolean(temp);
        }

        if (json_object_object_get_ex(poly_obj, "threshold", &temp)) {
            config->iou_threshold = json_object_get_double(temp);
        }

        // Parse points
        if (json_object_object_get_ex(poly_obj, "points", &temp)) {
            int num_points = json_object_array_length(temp);
            for (int j = 0; j < num_points && j < POLYGON_MAX_POINTS; ++j) {
                struct json_object* point_obj = json_object_array_get_idx(temp, j);
                struct json_object* x_obj, * y_obj;

                if (json_object_object_get_ex(point_obj, "x", &x_obj) &&
                    json_object_object_get_ex(point_obj, "y", &y_obj)) {
                    config->polygon.points[j].x = json_object_get_double(x_obj);
                    config->polygon.points[j].y = json_object_get_double(y_obj);
                }
            }
            config->polygon.num_points = num_points;
        }

        // Parse target classes
        if (json_object_object_get_ex(poly_obj, "target_classes", &temp)) {
            int num_classes = json_object_array_length(temp);
            for (int j = 0; j < num_classes && j < MAX_CLASSES_PER_POLYGON; ++j) {
                struct json_object* class_obj = json_object_array_get_idx(temp, j);
                strncpy(config->target_classes[j],
                        json_object_get_string(class_obj),
                        MAX_CLASS_NAME_LEN - 1);
            }
            config->num_target_classes = num_classes;
        }

        (*num_polygons)++;
    }

    json_object_put(root);
    return 0;
}

void* handle_client_thread(void* arg) {
    int client_fd = *(int*)arg;
    free(arg);

    printf("[Client] Reading header...\n");
    MessageHeader* header = protocol_read_header(client_fd);
    if (!header) {
        printf("[Client] Failed to read header\n");
        close(client_fd);
        return NULL;
    }

    size_t payload_size = header->payload_size;
    printf("[Client] Payload size: %zu\n", payload_size);

    uint8_t* payload = (uint8_t*)malloc(payload_size);
    if (!payload) {
        free(header);
        close(client_fd);
        return NULL;
    }

    ssize_t bytes_read = recv(client_fd, payload, payload_size, MSG_WAITALL);
    if (bytes_read != (ssize_t)payload_size) {
        printf("[Client] Failed to read payload: %zd vs %zu\n", bytes_read, payload_size);
        free(header);
        free(payload);
        close(client_fd);
        return NULL;
    }

    uint8_t* image_data = NULL;
    size_t image_size = 0;
    char* polygons_json = NULL;

    int parse_result = protocol_parse_request(payload, payload_size,
                                              &image_data, &image_size,
                                              &polygons_json);

    free(payload);

    if (parse_result != 0) {
        printf("[Client] Failed to parse request: %d\n", parse_result);
        MessagePayload* error_resp = protocol_create_error("Failed to parse request");
        protocol_write_message(client_fd, error_resp);
        protocol_free_payload(error_resp);
        free(header);
        close(client_fd);
        return NULL;
    }

    printf("[Client] Received image: %zu bytes\n", image_size);
    printf("[Client] Polygons JSON: %.100s...\n", polygons_json);

    // Parse polygons
    PolygonConfig polygons[MAX_POLYGONS];
    int num_polygons = 0;

    if (parse_polygons_from_json(polygons_json, polygons, &num_polygons) != 0) {
        printf("[Client] Failed to parse polygons JSON\n");
        MessagePayload* error_resp = protocol_create_error("Failed to parse polygons JSON");
        protocol_write_message(client_fd, error_resp);
        protocol_free_payload(error_resp);
        free(image_data);
        free(polygons_json);
        free(header);
        close(client_fd);
        return NULL;
    }

    free(polygons_json);
    printf("[Client] Parsed %d polygons\n", num_polygons);

    // Configure processor
    PolygonProcessor* processor = processor_create();
    for (int i = 0; i < num_polygons; ++i) {
        processor_add_polygon(processor, &polygons[i]);
    }

    // Run detection (using stub detector)
    printf("[Client] Running detection...\n");
    DetectionArray detections = detector_detect(g_detector, image_data);
    printf("[Client] Detected %zu objects\n", detections.num_detections);

    // Process with polygons
    ProcessedResult processed = processor_process_detections(processor, &detections);
    printf("[Client] After polygon filtering: %zu objects included\n", processed.num_objects);

    // Build response JSON
    char* response_json = build_polygons_json(polygons, num_polygons);

    // Create response
    MessagePayload* response = protocol_create_response(
        NULL, 0,
        response_json ? response_json : "[]",
        true, NULL);

    // Send response
    protocol_write_message(client_fd, response);
    printf("[Client] Response sent\n");

    // Cleanup
    free(response_json);
    protocol_free_payload(response);
    free(image_data);
    processor_destroy(processor);
    free(header);
    close(client_fd);

    return NULL;
}

void server_run(int port, const char* model_path, const char* config_path) {
    int server_fd;
    struct sockaddr_in address;
    int opt = 1;

    printf("[Server] Initializing detector with model: %s\n", model_path);

    // Initialize detector (stub version)
    g_detector = detector_create(model_path, config_path, 0.5f, 0.4f);
    if (!g_detector) {
        fprintf(stderr, "Failed to create detector\n");
        return;
    }

    // Create socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        return;
    }

    // Set socket options
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt");
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    // Bind
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("bind");
        close(server_fd);
        return;
    }

    // Listen
    if (listen(server_fd, 10) < 0) {
        perror("listen");
        close(server_fd);
        return;
    }

    printf("[Server] ========================================\n");
    printf("[Server] Polygon Detection Server Started\n");
    printf("[Server] Listening on port %d\n", port);
    printf("[Server] ========================================\n");

    while (g_server_running) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);

        int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
        if (client_fd < 0) {
            if (g_server_running) {
                perror("accept");
            }
            continue;
        }

        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));
        printf("[Server] New connection from %s:%d\n", client_ip, ntohs(client_addr.sin_port));

        // Create thread for client
        pthread_t thread;
        int* client_fd_ptr = malloc(sizeof(int));
        *client_fd_ptr = client_fd;

        if (pthread_create(&thread, NULL, handle_client_thread, client_fd_ptr) != 0) {
            perror("pthread_create");
            free(client_fd_ptr);
            close(client_fd);
        } else {
            pthread_detach(thread);
        }
    }

    close(server_fd);
    detector_destroy(g_detector);
    printf("[Server] Shutdown complete\n");
}
