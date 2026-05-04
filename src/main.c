#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

// Forward declaration
void server_run(int port, const char* model_path, const char* config_path);

static volatile int keep_running = 1;

void sigint_handler(int sig) {
    (void)sig;
    printf("\nShutting down server...\n");
    keep_running = 0;
}

int main(int argc, char** argv) {
    int port = 9090;
    const char* model_path = "models/yolov4.weights";
    const char* config_path = "models/yolov4.cfg";
    
    // Parse command line arguments
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--port") == 0 && i + 1 < argc) {
            port = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--model") == 0 && i + 1 < argc) {
            model_path = argv[++i];
        } else if (strcmp(argv[i], "--config") == 0 && i + 1 < argc) {
            config_path = argv[++i];
        } else if (strcmp(argv[i], "--help") == 0) {
            printf("Polygon Detection Server\n");
            printf("Usage: %s [options]\n", argv[0]);
            printf("Options:\n");
            printf("  --port PORT     Server port (default: 9090)\n");
            printf("  --model PATH    Model weights path (default: models/yolov4.weights)\n");
            printf("  --config PATH   Model config path (default: models/yolov4.cfg)\n");
            printf("  --help          Show this help\n");
            return 0;
        }
    }
    
    printf("========================================\n");
    printf("Polygon Detection Server v1.0\n");
    printf("========================================\n");
    printf("Port: %d\n", port);
    printf("Model: %s\n", model_path);
    printf("Config: %s\n", config_path);
    printf("========================================\n\n");
    
    // Setup signal handler
    signal(SIGINT, sigint_handler);
    
    // Run server
    server_run(port, model_path, config_path);
    
    return 0;
}
