#include "../include/protocol.h"
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <zlib.h>

uint32_t protocol_checksum(const uint8_t* data, size_t size) {
    return crc32(0, data, size);
}

static MessagePayload* protocol_create_payload(uint32_t type, const uint8_t* payload_data,
                                                size_t payload_size) {
    MessagePayload* payload = (MessagePayload*)malloc(sizeof(MessagePayload));
    if (!payload) return NULL;

    size_t total_size = HEADER_SIZE + payload_size;
    uint8_t* buffer = (uint8_t*)calloc(1, total_size);
    if (!buffer) {
        free(payload);
        return NULL;
    }

    MessageHeader* header = (MessageHeader*)buffer;
    header->magic = htonl(PROTOCOL_MAGIC);
    header->version = htonl(PROTOCOL_VERSION);
    header->type = htonl(type);
    header->payload_size = htonl(payload_size);

    if (payload_size > 0 && payload_data) {
        header->checksum = htonl(protocol_checksum(payload_data, payload_size));
        memcpy(buffer + HEADER_SIZE, payload_data, payload_size);
    } else {
        header->checksum = 0;
    }

    payload->data = buffer;
    payload->size = total_size;

    return payload;
}

MessagePayload* protocol_create_request(const char* image_data, size_t image_size,
                                        const char* polygons_json) {
    if (!image_data || !polygons_json) return NULL;

    size_t json_len = strlen(polygons_json);
    size_t total_payload = 4 + image_size + 4 + json_len;

    uint8_t* payload_data = (uint8_t*)malloc(total_payload);
    if (!payload_data) return NULL;

    uint8_t* ptr = payload_data;
    uint32_t net_val;

    net_val = htonl((uint32_t)image_size);
    memcpy(ptr, &net_val, 4);
    ptr += 4;

    memcpy(ptr, image_data, image_size);
    ptr += image_size;

    net_val = htonl((uint32_t)json_len);
    memcpy(ptr, &net_val, 4);
    ptr += 4;

    memcpy(ptr, polygons_json, json_len);

    MessagePayload* result = protocol_create_payload(MSG_REQUEST, payload_data, total_payload);
    free(payload_data);

    return result;
}

MessagePayload* protocol_create_response(const char* result_image_data, size_t image_size,
                                         const char* objects_json, bool success,
                                         const char* error) {
    size_t json_len = objects_json ? strlen(objects_json) : 0;
    size_t error_len = error ? strlen(error) : 0;
    size_t total_payload = 1 + 4 + image_size + 2 + error_len + 4 + json_len;

    uint8_t* payload_data = (uint8_t*)malloc(total_payload);
    if (!payload_data) return NULL;

    uint8_t* ptr = payload_data;
    uint32_t net_val;

    *ptr++ = success ? 1 : 0;

    net_val = htonl((uint32_t)image_size);
    memcpy(ptr, &net_val, 4);
    ptr += 4;

    if (image_size > 0 && result_image_data) {
        memcpy(ptr, result_image_data, image_size);
        ptr += image_size;
    }

    uint16_t net_val16 = htons((uint16_t)error_len);
    memcpy(ptr, &net_val16, 2);
    ptr += 2;

    if (error_len > 0 && error) {
        memcpy(ptr, error, error_len);
        ptr += error_len;
    }

    net_val = htonl((uint32_t)json_len);
    memcpy(ptr, &net_val, 4);
    ptr += 4;

    if (json_len > 0 && objects_json) {
        memcpy(ptr, objects_json, json_len);
    }

    MessagePayload* result = protocol_create_payload(MSG_RESPONSE, payload_data, total_payload);
    free(payload_data);

    return result;
}

MessagePayload* protocol_create_error(const char* error_msg) {
    return protocol_create_response(NULL, 0, NULL, false, error_msg);
}

int protocol_parse_request(const uint8_t* data, size_t size,
                           uint8_t** image_data, size_t* image_size,
                           char** polygons_json) {
    if (size < 8) return -1;

    const uint8_t* ptr = data;
    uint32_t net_val;

    memcpy(&net_val, ptr, 4);
    *image_size = ntohl(net_val);
    ptr += 4;

    if (*image_size > MAX_IMAGE_SIZE) return -2;

    *image_data = (uint8_t*)malloc(*image_size);
    if (!*image_data) return -3;
    memcpy(*image_data, ptr, *image_size);
    ptr += *image_size;

    memcpy(&net_val, ptr, 4);
    size_t json_len = ntohl(net_val);
    ptr += 4;

    *polygons_json = (char*)malloc(json_len + 1);
    if (!*polygons_json) {
        free(*image_data);
        return -4;
    }
    memcpy(*polygons_json, ptr, json_len);
    (*polygons_json)[json_len] = '\0';

    return 0;
}

int protocol_parse_response(const uint8_t* data, size_t size,
                            uint8_t** result_image, size_t* image_size,
                            char** objects_json, bool* success, char** error) {
    if (size < 1) return -1;

    const uint8_t* ptr = data;

    *success = (*ptr++ == 1);

    uint32_t net_val;
    memcpy(&net_val, ptr, 4);
    *image_size = ntohl(net_val);
    ptr += 4;

    if (*image_size > 0) {
        *result_image = (uint8_t*)malloc(*image_size);
        if (!*result_image) return -2;
        memcpy(*result_image, ptr, *image_size);
        ptr += *image_size;
    } else {
        *result_image = NULL;
    }

    uint16_t net_val16;
    memcpy(&net_val16, ptr, 2);
    size_t error_len = ntohs(net_val16);
    ptr += 2;

    if (error_len > 0) {
        *error = (char*)malloc(error_len + 1);
        if (!*error) {
            if (*result_image) free(*result_image);
            return -3;
        }
        memcpy(*error, ptr, error_len);
        (*error)[error_len] = '\0';
        ptr += error_len;
    } else {
        *error = NULL;
    }

    memcpy(&net_val, ptr, 4);
    size_t json_len = ntohl(net_val);
    ptr += 4;

    if (json_len > 0) {
        *objects_json = (char*)malloc(json_len + 1);
        if (!*objects_json) {
            if (*result_image) free(*result_image);
            if (*error) free(*error);
            return -4;
        }
        memcpy(*objects_json, ptr, json_len);
        (*objects_json)[json_len] = '\0';
    } else {
        *objects_json = NULL;
    }

    return 0;
}

void protocol_free_payload(MessagePayload* payload) {
    if (payload) {
        if (payload->data) free(payload->data);
        free(payload);
    }
}

MessageHeader* protocol_read_header(int sockfd) {
    MessageHeader* header = (MessageHeader*)malloc(sizeof(MessageHeader));
    if (!header) return NULL;

    ssize_t bytes_read = recv(sockfd, header, HEADER_SIZE, MSG_WAITALL);
    if (bytes_read != HEADER_SIZE) {
        free(header);
        return NULL;
    }

    header->magic = ntohl(header->magic);
    header->version = ntohl(header->version);
    header->type = ntohl(header->type);
    header->payload_size = ntohl(header->payload_size);
    header->checksum = ntohl(header->checksum);

    if (header->magic != PROTOCOL_MAGIC) {
        free(header);
        return NULL;
    }

    return header;
}

int protocol_write_message(int sockfd, const MessagePayload* payload) {
    if (!payload || !payload->data) return -1;

    ssize_t bytes_written = send(sockfd, payload->data, payload->size, MSG_NOSIGNAL);
    if (bytes_written != (ssize_t)payload->size) {
        return -2;
    }

    return 0;
}
