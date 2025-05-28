//
//  IndexRoute.h
//  Straudio-macOS
//
//  Created by Alexander Olsen on 3/16/25.
//

#ifndef IndexRoute_h
#define IndexRoute_h
#endif /* IndexRoute_h */

#include "src/Util/Logger.h"
#include "src/ReceiverUI/ReceiverUI.hpp"

namespace IndexRoute {
// The request callback used in 'start()'
static int handleRequest(mg_connection* conn, void* cbdata)
{
  mg_printf(conn,
            "HTTP/1.1 200 OK\r\n"
            "Access-Control-Allow-Origin: *\r\n"
            "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
            "Content-Type: text/html\r\n"
            "Content-Length: %u\r\n\r\n",
            RECEIVER_UI_length);
  
  mg_write(conn, RECEIVER_UI, RECEIVER_UI_length);
  return 1;
}

}

