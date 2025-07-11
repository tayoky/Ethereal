/**
 * @file userspace/lib/celestial/event.c
 * @brief Event system in Celestial
 * 
 * 
 * @copyright
 * This file is part of the Ethereal Operating System.
 * It is released under the terms of the BSD 3-clause license.
 * Please see the LICENSE file in the main repository for more details.
 * 
 * Copyright (C) 2025 Samuel Stuart
 */

#include <ethereal/celestial.h>
#include <stdlib.h>
#include <errno.h>

/**
 * @brief Subscribe to specific events on the Celestial handler
 * @param win The window to subscribe to events on
 * @param events The events to subscribe to
 * @returns 0 on success
 */
int celestial_subscribe(window_t *win, uint32_t events) {
    if (!win || !events) {
        errno = EINVAL;
        return -1;
    }

    // Build subscribe request
    celestial_req_subscribe_t req = {
        .magic = CELESTIAL_MAGIC,
        .type = CELESTIAL_REQ_SUBSCRIBE,
        .size = sizeof(celestial_req_subscribe_t),
        .events = events,
        .wid = win->wid
    };

    // Send the request
    if (celestial_sendRequest(&req, req.size) < 0) return -1; 

    // Wait for a response
    celestial_resp_ok_t *resp = celestial_getResponse(CELESTIAL_REQ_SUBSCRIBE);
    if (!resp) return -1;

    // Handle error in resp
    CELESTIAL_HANDLE_RESP_ERROR(resp, -1);
    free(resp);

    return 0;
}

/**
 * @brief Unsubscribe from specific events on the celestial handler
 * @param win The window to unsubscribe events on
 * @param events The events to unsubscribe from
 * @returns 0 on success
 */
int celestial_unsubscribe(window_t *win, uint32_t events) {
    if (!win || !events) {
        errno = EINVAL;
        return -1;
    }

    // Build unsubscribe request
    celestial_req_unsubscribe_t req = {
        .magic = CELESTIAL_MAGIC,
        .type = CELESTIAL_REQ_UNSUBSCRIBE,
        .size = sizeof(celestial_req_subscribe_t),
        .events = events,
        .wid = win->wid
    };

    // Send the request
    if (celestial_sendRequest(&req, req.size) < 0) return -1; 

    // Wait for a response
    celestial_resp_ok_t *resp = celestial_getResponse(CELESTIAL_REQ_UNSUBSCRIBE);
    if (!resp) return -1;

    // Handle error in resp
    CELESTIAL_HANDLE_RESP_ERROR(resp, -1);
    free(resp);

    return 0;
}

/**
 * @brief Set a specific event handler in a window
 * @param win The window to set the handler for
 * @param event The event to set the handler for
 * @param handler The handler to set for the event
 * @returns 0 on success, -1 on failure
 */
int celestial_setHandler(window_t *win, uint32_t event, celestial_event_handler_t handler) {
    // Does a handler already exist?
    if (hashmap_has(win->event_handler_map, (void*)(uintptr_t)event)) {
        errno = EEXIST;
        return -1;
    }

    // Set the handler
    hashmap_set(win->event_handler_map, (void*)(uintptr_t)event, handler);
    return 0;
}

/**
 * @brief Handle a receieved event
 * @param event The event that was received
 */
void celestial_handleEvent(void *event) {
    celestial_event_header_t *hdr = (celestial_event_header_t*)event;
    
    // Check if we got a window
    window_t *win = celestial_getWindow(hdr->wid);
    if (!win) { free(event); return; }

    celestial_event_handler_t handler = (celestial_event_handler_t)hashmap_get(win->event_handler_map, (void*)(uintptr_t)hdr->type);
    if (handler) handler(win, hdr->type, event);
    free(event);
}