// WebSocketManager.h
#pragma once
#include <civetweb.h>
#include <set>

class WebSocketManager {
public:
    static void handleWebsocket(mg_connection* conn, int ev, void* ev_data) {
        if(ev == MG_EV_WS_OPEN) {
            s_connections.insert(conn);
        }
        else if(ev == MG_EV_WS_MSG) {
            // Handle message
        }
        else if(ev == MG_EV_CLOSE) {
            s_connections.erase(conn);
        }
    }
    
private:
    static std::set<mg_connection*> s_connections;
};
