/**
 * server_resolver.cpp
 * Resolves backend hostname via mDNS/DNS and builds API URLs.
 */

#include <cstring>
#include "server_resolver.h"

ServerResolver serverResolver;

static const char* PREF_NAMESPACE = "servercfg";
static const char* PREF_KEY_LAST_IP = "last_ip";

ServerResolver::ServerResolver()
    : _baseUrl(buildBaseUrlForHost(String(SERVER_HOSTNAME))),
      _mqttHost(String(MQTT_BROKER)),
      _resolved(false) {}

String ServerResolver::buildBaseUrlForHost(const String& host) const {
    String url = "http://" + host + ":" + String(SERVER_PORT) + SERVER_API_PATH;
    return url;
}

bool ServerResolver::resolve() {
    _resolved = false;
    bool endpointReady = false;

    const char* hostCandidate =
#ifdef SERVER_HOSTNAME
        SERVER_HOSTNAME;
#else
        "";
#endif
    String hostname = String(hostCandidate);
    if (hostname.isEmpty()) {
        hostname = String(MQTT_BROKER);
    }
    if (hostname.isEmpty()) {
        Serial.println("[NET] No hostname configured for backend");
        return false;
    }

    _baseUrl = buildBaseUrlForHost(hostname);
    _mqttHost = hostname;

    IPAddress ip;
    Serial.print("[NET] Resolving host ");
    Serial.print(hostname);
    Serial.println(" ...");

    if (WiFi.hostByName(hostname.c_str(), ip) == 1) {
        _resolved = true;
        String ipStr = ip.toString();
        _baseUrl = buildBaseUrlForHost(ipStr);
        _mqttHost = String(hostname);
        Serial.print("[NET] Host resolved: ");
        Serial.print(hostname);
        Serial.print(" -> ");
        Serial.println(ipStr);
        storeLastKnownIp(ipStr);
        endpointReady = true;
    } else {
        Serial.println("[NET] Hostname lookup failed");
    }

    if (!_resolved) {
        String cached = loadLastKnownIp();
        if (!cached.isEmpty()) {
            Serial.print("[NET] Using cached backend IP: ");
            Serial.println(cached);
            _baseUrl = buildBaseUrlForHost(cached);
            _mqttHost = cached;
            endpointReady = true;
        } else {
            Serial.println("[NET] No cached backend IP available");
        }
    }

    Serial.print("[NET] API base URL: ");
    Serial.println(_baseUrl);
    return endpointReady;
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

bool ServerResolver::ensurePrefs() {
    if (_prefsReady) {
        return true;
    }
    _prefsReady = _prefs.begin(PREF_NAMESPACE, false);
    if (!_prefsReady) {
        Serial.println("[NET] Failed to open preferences for resolver cache");
    }
    return _prefsReady;
}

void ServerResolver::storeLastKnownIp(const String& ip) {
    if (!ensurePrefs()) {
        return;
    }
    _prefs.putString(PREF_KEY_LAST_IP, ip);
}

String ServerResolver::loadLastKnownIp() {
    if (!ensurePrefs()) {
        return String();
    }
    return _prefs.getString(PREF_KEY_LAST_IP, "");
}
