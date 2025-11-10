/**
 * server_resolver.h
 * Central place to resolve backend hostnames (mDNS) and build API URLs.
 */

#ifndef SERVER_RESOLVER_H
#define SERVER_RESOLVER_H

#include <Arduino.h>
#include <WiFi.h>
#include "config.h"

class ServerResolver {
public:
    ServerResolver();

    /**
     * Resolve SERVER_HOSTNAME (if provided) to an IP and cache base URLs.
     * Returns true when hostname lookup succeeds, false when falling back to static IP.
     */
    bool resolve();

    /**
     * Build a full API URL (base + path). Path should start with '/'.
     */
    String buildApiUrl(const String& path) const;

    /**
     * Get the cached base URL (http://host:port/api).
     */
    const String& baseUrl() const;

    /**
     * Host string to be used by MQTT (hostname when resolved, else fallback IP).
     */
    const String& mqttHost() const;

    /**
     * Whether the hostname was successfully resolved this session.
     */
    bool resolvedViaMdns() const;

private:
    String _baseUrl;
    String _mqttHost;
    bool _resolved;

    String buildBaseUrlForHost(const String& host) const;
};

extern ServerResolver serverResolver;

#endif // SERVER_RESOLVER_H
