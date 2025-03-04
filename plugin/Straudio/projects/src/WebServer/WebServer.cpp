// WebServer.cpp
#include "WebServer.h"
#include "ResourceUtils.h"
#include "NetworkUtils.h"
#include <filesystem>
#include <iostream>
#include <netdb.h>
#include <unistd.h>

WebServer::WebServer() : mCtx(nullptr), preferredPort(8080), currentPort(-1) {}

WebServer::~WebServer() {
    stop();
}

bool WebServer::start(const std::string& webRoot) {
    // Try to use the preferred port first
    int port = findAvailablePort(preferredPort);
    
    if (port == -1) {
        std::cerr << "No available ports found. Could not start the server." << std::endl;
        return false;
    }

    currentPort = port;  // Store the actual port used
    
    const char* options[] = {
        "document_root", webRoot.c_str(),
        "listening_ports", std::to_string(port).c_str(),
        "num_threads", "4",
        "enable_directory_listing", "no",
        NULL
    };
    
    mg_init_library(0);
    mCtx = mg_start(nullptr, nullptr, options);
    return mCtx != nullptr;
}

void WebServer::stop() {
    if (mCtx) {
        mg_stop(mCtx);
        mCtx = nullptr;
        currentPort = -1;  // Reset the port when stopping
    }
}

void WebServer::addRoute(const std::string& uri, RequestHandler handler) {
    mRoutes[uri] = handler;
}

int WebServer::handleRequest(mg_connection* conn, void* cbdata) {
    WebServer* self = static_cast<WebServer*>(cbdata);
    const mg_request_info* req = mg_get_request_info(conn);
    
    auto it = self->mRoutes.find(req->local_uri);
    if (it != self->mRoutes.end()) {
        std::string response = it->second(req->local_uri);
        mg_printf(conn, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n%s", response.c_str());
        return 1;
    }
    
    // Let CivetWeb handle static files
    return 0;
}

int WebServer::findAvailablePort(int startPort) {
    struct sockaddr_in sa;
    int sockfd;
    for (int port = startPort; port < 65535; ++port) {
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd == -1) {
            continue;
        }
        
        sa.sin_family = AF_INET;
        sa.sin_port = htons(port);
        sa.sin_addr.s_addr = INADDR_ANY;

        if (bind(sockfd, (struct sockaddr*)&sa, sizeof(sa)) == 0) {
            close(sockfd);
            return port;
        }
        
        close(sockfd); // Port is already in use
    }
    return -1; // No available port found
}

int WebServer::getPort() const {
    return currentPort;  // Return the current port the server is using
}

std::string WebServer::getFullUrl() {
  return "http://" + NetworkUtils::GetPrimaryLocalIPAddress() + ":" + std::to_string(getPort());
}
