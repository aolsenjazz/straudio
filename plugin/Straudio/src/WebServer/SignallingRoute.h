//
//  SignallingRoute.h
//  Straudio-macOS
//
//  Created by Alexander Olsen on 3/16/25.
//
#include "civetweb.h"
#include <cstring>
#include <iostream>
#include <sstream>
#include <memory>
#include <string>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <json.hpp>       // nlohmann/json header
#include "rtc/rtc.hpp"    // libdatachannel v0.22.5

using json = nlohmann::json;

namespace SignallingRoute {

// Global state for this simple example.
// (For production you’ll need a mechanism to manage multiple peer connections.)
static std::shared_ptr<rtc::PeerConnection> peerConnection = nullptr;
static std::mutex sdpMutex;
static std::condition_variable sdpCond;
static std::string localSdpAnswer;
static bool sdpReady = false;

//static int handleRequest(mg_connection* conn, void* cbdata) {
//    return 1;
//}

// Initialize the PeerConnection with default configuration (no ICE servers)
inline void initializePeerConnection() {
   if (!peerConnection) {
       rtc::Configuration config;  // No STUN/TURN servers added.
       peerConnection = std::make_shared<rtc::PeerConnection>(config);

       // Set up the local description callback.
       peerConnection->onLocalDescription([&](rtc::Description description) {
           std::cout << "Local SDP ("
                     << description.typeString()
                     << "): " << std::string(description) << std::endl;
           // If an answer is generated, store it so the waiting HTTP thread can send it back.
           if (description.typeString() == "answer") {
               {
                   std::unique_lock<std::mutex> lock(sdpMutex);
                   localSdpAnswer = std::string(description);
                   sdpReady = true;
               }
               sdpCond.notify_one();
           }
       });

       // Log any locally gathered ICE candidate.
       peerConnection->onLocalCandidate([](rtc::Candidate candidate) {
           std::cout << "Local ICE candidate: " << std::string(candidate) << std::endl;
           // In a full implementation, send the candidate to the remote peer.
       });

       // Log state changes.
       peerConnection->onStateChange([](rtc::PeerConnection::State state) {
           std::cout << "PeerConnection state changed: " << state << std::endl;
       });
   }
}

// Process a signaling message received via HTTP POST.
inline void processSignallingMessage(const std::string &body, mg_connection* conn) {
   try {
       json j = json::parse(body);
       std::string type = j.value("type", "");

       if (type == "offer") {
           // For an offer, the SDP is in the "description" field.
           std::string sdp = j.value("description", "");
           std::cout << "Received SDP offer" << std::endl;
           initializePeerConnection();
           {
               std::unique_lock<std::mutex> lock(sdpMutex);
               sdpReady = false;
               localSdpAnswer.clear();
           }
           // Set the remote description using the offer.
           peerConnection->setRemoteDescription(rtc::Description(sdp, "offer"));
           // Answer is automatically generated via auto-negotiation and the onLocalDescription callback.

           // Wait (up to 5 seconds) for the answer to be generated.
           std::unique_lock<std::mutex> lock(sdpMutex);
           if (!sdpCond.wait_for(lock, std::chrono::seconds(5), [](){ return sdpReady; })) {
               const char* response = "{\"status\":\"error\",\"message\":\"Timeout waiting for answer\"}";
               mg_printf(conn,
                         "HTTP/1.1 500 Internal Server Error\r\n"
                         "Access-Control-Allow-Origin: *\r\n"
                         "Content-Type: application/json\r\n"
                         "Content-Length: %u\r\n\r\n%s",
                         (unsigned int)strlen(response), response);
               return;
           }
           // Answer is ready; send it back as JSON.
           std::ostringstream oss;
           oss << "{\"status\":\"ok\",\"type\":\"answer\",\"description\":\"" << localSdpAnswer << "\"}";
           std::string answerJson = oss.str();
           mg_printf(conn,
                     "HTTP/1.1 200 OK\r\n"
                     "Access-Control-Allow-Origin: *\r\n"
                     "Content-Type: application/json\r\n"
                     "Content-Length: %u\r\n\r\n%s",
                     (unsigned int)answerJson.size(), answerJson.c_str());
       }
       else if (type == "answer") {
           std::string sdp = j.value("description", "");
           std::cout << "Received SDP answer" << std::endl;
           initializePeerConnection();
           peerConnection->setRemoteDescription(rtc::Description(sdp, "answer"));
           const char* response = "{\"status\":\"ok\"}";
           mg_printf(conn,
                     "HTTP/1.1 200 OK\r\n"
                     "Access-Control-Allow-Origin: *\r\n"
                     "Content-Type: application/json\r\n"
                     "Content-Length: %u\r\n\r\n%s",
                     (unsigned int)strlen(response), response);
       }
       else if (type == "candidate") {
           // For ICE candidates, expect "candidate" and "mid" fields.
           std::string candidateStr = j.value("candidate", "");
           std::string mid = j.value("mid", "");
           std::cout << "Received ICE candidate" << std::endl;
           initializePeerConnection();
           peerConnection->addRemoteCandidate(rtc::Candidate(candidateStr, mid));
           const char* response = "{\"status\":\"ok\"}";
           mg_printf(conn,
                     "HTTP/1.1 200 OK\r\n"
                     "Access-Control-Allow-Origin: *\r\n"
                     "Content-Type: application/json\r\n"
                     "Content-Length: %u\r\n\r\n%s",
                     (unsigned int)strlen(response), response);
       }
       else {
           std::cerr << "Unknown signalling type: " << type << std::endl;
           const char* response = "{\"status\":\"error\",\"message\":\"Unknown signalling type\"}";
           mg_printf(conn,
                     "HTTP/1.1 400 Bad Request\r\n"
                     "Access-Control-Allow-Origin: *\r\n"
                     "Content-Type: application/json\r\n"
                     "Content-Length: %u\r\n\r\n%s",
                     (unsigned int)strlen(response), response);
       }
   }
   catch (std::exception &e) {
       std::cerr << "Error parsing signalling message: " << e.what() << std::endl;
       const char* response = "{\"status\":\"error\",\"message\":\"Exception parsing JSON\"}";
       mg_printf(conn,
                 "HTTP/1.1 400 Bad Request\r\n"
                 "Access-Control-Allow-Origin: *\r\n"
                 "Content-Type: application/json\r\n"
                 "Content-Length: %u\r\n\r\n%s",
                 (unsigned int)strlen(response), response);
   }
}


// The HTTP request callback used by CivetWeb.
static int handleRequest(mg_connection* conn, void* cbdata) {
    const mg_request_info* req_info = mg_get_request_info(conn);
    std::string method(req_info->request_method);
  
  rtc::Configuration config;
  auto pc = std::make_shared<rtc::PeerConnection>(config);
    
  if (method == "POST") {
        // Read the POST body (assumes the body is not huge).
        char buffer[4096];
        int ret = mg_read(conn, buffer, sizeof(buffer) - 1);
        if (ret > 0) {
            buffer[ret] = '\0';
            std::string body(buffer);
            std::cout << "POST body received: " << body << std::endl;
           processSignallingMessage(body, conn);
        } else {
            const char* response = "{\"status\":\"error\",\"message\":\"Empty POST body\"}";
            mg_printf(conn,
                      "HTTP/1.1 400 Bad Request\r\n"
                      "Access-Control-Allow-Origin: *\r\n"
                      "Content-Type: application/json\r\n"
                      "Content-Length: %u\r\n\r\n%s",
                      (unsigned int)strlen(response), response);
        }
    }
    else if (method == "OPTIONS") {
        mg_printf(conn,
                  "HTTP/1.1 200 OK\r\n"
                  "Access-Control-Allow-Origin: *\r\n"
                  "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
                  "Content-Length: 0\r\n\r\n");
    }
    else {
        mg_printf(conn,
                  "HTTP/1.1 405 Method Not Allowed\r\n"
                  "Allow: GET, POST, OPTIONS\r\n"
                  "Content-Length: 0\r\n\r\n");
    }
    return 1;
}

}
