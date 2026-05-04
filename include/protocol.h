#ifndef POLYGON_DETECTOR_PROTOCOL_H_
#define POLYGON_DETECTOR_PROTOCOL_H_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <sys/socket.h>

#define MAX_IMAGE_SIZE 10 * 1024 * 1024
#define PROTOCOL_MAGIC 0xABCD1234
#define PROTOCOL_VERSION 1
#define HEADER_SIZE sizeof(MessageHeader)

typedef enum {
  MSG_REQUEST = 1,
  MSG_RESPONSE = 2,
  MSG_ERROR = 3
} MessageType;

typedef struct {
  uint32_t magic;
  uint32_t version;
  uint32_t type;
  uint32_t payload_size;
  uint32_t checksum;
} MessageHeader;

typedef struct {
  uint8_t* data;
  size_t size;
} MessagePayload;

// Protocol functions
MessagePayload* protocol_create_request(const char* image_data, size_t image_size,
                                        const char* polygons_json);
MessagePayload* protocol_create_response(const char* result_image_data, size_t image_size,
                                         const char* objects_json, bool success,
                                         const char* error);
MessagePayload* protocol_create_error(const char* error_msg);
int protocol_parse_request(const uint8_t* data, size_t size,
                           uint8_t** image_data, size_t* image_size,
                           char** polygons_json);
int protocol_parse_response(const uint8_t* data, size_t size,
                            uint8_t** result_image, size_t* image_size,
                            char** objects_json, bool* success, char** error);
void protocol_free_payload(MessagePayload* payload);
uint32_t protocol_checksum(const uint8_t* data, size_t size);
MessageHeader* protocol_read_header(int sockfd);
int protocol_write_message(int sockfd, const MessagePayload* payload);

#endif  // POLYGON_DETECTOR_PROTOCOL_H_
