/**
 * server_resolver.cpp
 * Resolves backend hostname via mDNS/DNS and builds API URLs.
 */

#include <cstring>
#include "server_resolver.h"

ServerResolver serverResolver;

ServerResolver::ServerResolver()
    : _baseUrl(buildBaseUrlForHost(String(SERVER_IP))),
      _mqttHost(String(MQTT_BROKER)),
      _resolved(false) {}

String ServerResolver::buildBaseUrlForHost(const String& host) const {
    String url = "http://" + host + ":" + String(SERVER_PORT) + SERVER_API_PATH;
    return url;
}

bool ServerResolver::resolve() {
    _resolved = false;
    _baseUrl = buildBaseUrlForHost(String(SERVER_IP));
    _mqttHost = String(MQTT_BROKER);

#ifdef SERVER_HOSTNAME
    IPAddress ip;
    Serial.print("[NET] Resolving host ");
    Serial.print(SERVER_HOSTNAME);
    Serial.println(" ...");

    if (WiFi.hostByName(SERVER_HOSTNAME, ip) == 1) {
        _resolved = true;
        String ipStr = ip.toString();
        _baseUrl = buildBaseUrlForHost(ipStr);
        _mqttHost = String(SERVER_HOSTNAME);
        Serial.print("[NET] Host resolved: ");
        Serial.print(SERVER_HOSTNAME);
        Serial.print(" -> ");
        Serial.println(ipStr);
    } else {
        Serial.println("[NET] Hostname lookup failed, using fallback IP");
    }
#else
    Serial.println("[NET] SERVER_HOSTNAME not set, using fallback IP only");
#endif

    Serial.print("[NET] API base URL: ");
    Serial.println(_baseUrl);
    return _resolved;
}

String ServerResolver::buildApiUrl(const String& path) const {
    if (path.startsWith("/")) {
        return _baseUrl + path;
    }
    return _baseUrl + "/" + path;
}

const String& ServerResolver::baseUrl() const {
    return _baseUrl;
}

const String& ServerResolver::mqttHost() const {
    return _mqttHost;
}

bool ServerResolver::resolvedViaMdns() const {
    return _resolved;
}
