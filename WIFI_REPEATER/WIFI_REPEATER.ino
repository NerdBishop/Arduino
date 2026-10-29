#include <WiFi.h>
#include <cstring>
#include "esp_netif.h"
#include "sdkconfig.h"

#if !defined(CONFIG_LWIP_IP_FORWARD) || !CONFIG_LWIP_IP_FORWARD
#error "Use an ESP32 package with CONFIG_LWIP_IP_FORWARD enabled."
#endif
#if !defined(CONFIG_LWIP_IPV4_NAPT) || !CONFIG_LWIP_IPV4_NAPT
#error "Use an ESP32 package with CONFIG_LWIP_IPV4_NAPT enabled."
#endif

// CHANGE THESE FOUR SETTINGS.
const char* HOME_SSID = "Home_WiFi_Name";
const char* HOME_PASS = "WiFi_Password";
const char* AP_SSID = "Your_New_Wifi_Name";
const char* AP_PASS = "WiFi_Password";

// DONT TOUCH ANTHING FROM HERE ON
const uint32_t CONNECT_TIMEOUT_MS = 30000;
const uint32_t CHECK_INTERVAL_MS = 5000;
const IPAddress AP_MASK(255, 255, 255, 0);
const IPAddress AP_CANDIDATES[] = {
  IPAddress(192,168,4,1), IPAddress(192,168,50,1),
  IPAddress(172,31,250,1), IPAddress(10,250,250,1)
};
IPAddress apIP = AP_CANDIDATES[0];
IPAddress savedIP, savedMask, savedGateway, savedDNS;
bool enabled = false, apStarted = false, natEnabled = false;
bool configured = false, wasConnected = false;
uint32_t lastAttempt = 0, lastCheck = 0;

bool checkError(esp_err_t err, const char* operation) {
  if (err == ESP_OK) return true;
  Serial.printf("ERROR: %s: %s\n", operation, esp_err_to_name(err));
  return false;
}

bool validSettings() {
  if (!strlen(HOME_SSID) || strlen(HOME_SSID) > 32 ||
      !strlen(AP_SSID) || strlen(AP_SSID) > 32) {
    Serial.println("ERROR: Wi-Fi names must contain 1-32 bytes.");
    return false;
  }
  if (strlen(AP_PASS) < 8 || strlen(AP_PASS) > 63) {
    Serial.println("ERROR: AP_PASS must contain 8-63 characters.");
    return false;
  }
  return true;
}

// Explicit byte order avoids depending on IPAddress's internal representation.
uint32_t ipNumber(IPAddress ip) {
  return (uint32_t(ip[0]) << 24) | (uint32_t(ip[1]) << 16) |
         (uint32_t(ip[2]) << 8) | uint32_t(ip[3]);
}
bool overlaps(IPAddress candidate) {
  uint32_t mask = ipNumber(AP_MASK) & ipNumber(WiFi.subnetMask());
  return (ipNumber(candidate) & mask) == (ipNumber(WiFi.localIP()) & mask);
}
bool chooseAPAddress() {
  if (!overlaps(apIP)) return true;
  for (const IPAddress& candidate : AP_CANDIDATES) {
    if (!overlaps(candidate)) {
      apIP = candidate;
      Serial.print("Subnet conflict: changing AP address to ");
      Serial.println(apIP);
      return true;
    }
  }
  Serial.println("ERROR: All AP candidates overlap home Wi-Fi.");
  Serial.println("Add a non-overlapping private /24 address to AP_CANDIDATES.");
  return false;
}
bool disableNAT() {
  if (!natEnabled) return true;
  esp_netif_t* ap = esp_netif_get_handle_from_ifkey("WIFI_AP_DEF");
  if (!ap) { Serial.println("ERROR: AP interface missing."); return false; }
  if (!checkError(esp_netif_napt_disable(ap), "Disable NAPT")) return false;
  natEnabled = false;
  return true;
}
bool startAP() {
  if (!WiFi.softAPConfig(apIP, apIP, AP_MASK)) {
    Serial.println("ERROR: AP IP configuration failed."); return false;
  }
  if (!WiFi.softAP(AP_SSID, AP_PASS)) {
    Serial.println("ERROR: Could not start access point."); return false;
  }
  apStarted = true;
  Serial.printf("Access point: %s\n", AP_SSID);
  Serial.print("AP address: "); Serial.println(WiFi.softAPIP());
  return true;
}

bool configureRouter() {
  configured = false;
  esp_netif_t* sta = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
  if (!sta) { Serial.println("ERROR: Station interface missing."); return false; }
  esp_netif_dns_info_t dns = {};
  if (!checkError(esp_netif_get_dns_info(sta, ESP_NETIF_DNS_MAIN, &dns),
                  "Read upstream DNS")) return false;
  // No silent public DNS substitution; some networks block external DNS.
  if (dns.ip.type != ESP_IPADDR_TYPE_V4 || dns.ip.u_addr.ip4.addr == 0) {
    Serial.println("ERROR: No upstream IPv4 DNS server. Will retry."); return false;
  }
  if (!disableNAT() || !chooseAPAddress()) return false;
  // Reconnection/network changes briefly restart the AP so DHCP clients can
  // obtain fresh addresses and DNS. Manually fixed client IPs need updating.
  if (apStarted) {
    Serial.println("Refreshing AP/DHCP; clients will briefly disconnect.");
    if (!WiFi.softAPdisconnect(false)) {
      Serial.println("ERROR: Could not stop access point."); return false;
    }
    apStarted = false;
  }
  if (!startAP()) return false;
  esp_netif_t* ap = esp_netif_get_handle_from_ifkey("WIFI_AP_DEF");
  if (!ap) { Serial.println("ERROR: AP interface missing."); return false; }
  esp_err_t err = esp_netif_dhcps_stop(ap);
  if (err != ESP_OK && err != ESP_ERR_ESP_NETIF_DHCP_ALREADY_STOPPED) {
    checkError(err, "Stop DHCP server"); return false;
  }
  uint8_t offerDNS = 1;
  bool dnsOK = checkError(esp_netif_set_dns_info(ap, ESP_NETIF_DNS_MAIN, &dns),
                          "Set AP DNS");
  bool offerOK = checkError(esp_netif_dhcps_option(
      ap, ESP_NETIF_OP_SET, ESP_NETIF_DOMAIN_NAME_SERVER,
      &offerDNS, sizeof(offerDNS)), "Enable DHCP DNS offer");
  // Restore DHCP even if a preceding DNS operation failed.
  bool dhcpOK = checkError(esp_netif_dhcps_start(ap), "Start DHCP server");
  if (!dnsOK || !offerOK || !dhcpOK) return false;
  if (!checkError(esp_netif_set_default_netif(sta), "Set upstream route")) return false;
  if (!checkError(esp_netif_napt_enable(ap), "Enable NAPT")) return false;
  natEnabled = true;
  if (WiFi.status() != WL_CONNECTED) { disableNAT(); return false; }
  savedIP = WiFi.localIP(); savedMask = WiFi.subnetMask();
  savedGateway = WiFi.gatewayIP(); savedDNS = WiFi.dnsIP(0);
  configured = true;
  Serial.println("Router configured: upstream connected, DHCP/DNS and NAPT enabled.");
  Serial.println("Internet/cloud availability still depends on the home network.");
  Serial.print("Upstream IP: "); Serial.println(savedIP);
  Serial.print("Upstream DNS: "); Serial.println(savedDNS);
  Serial.printf("Connect your Growatt/device to: %s\n", AP_SSID);
  return true;
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  if (!validSettings()) return;
  if (!WiFi.mode(WIFI_AP_STA)) {
    Serial.println("ERROR: Could not enable AP + station mode."); return;
  }
  WiFi.setAutoReconnect(true);
  startAP(); // Retried in loop if it fails; no internet until upstream connects.
  Serial.printf("Connecting to home Wi-Fi: %s\n", HOME_SSID);
  WiFi.begin(HOME_SSID, HOME_PASS);
  lastAttempt = millis();
  enabled = true;
}

void loop() {
  if (!enabled) { delay(100); return; }
  uint32_t now = millis();
  bool connected = WiFi.status() == WL_CONNECTED;
  if (!connected) {
    if (wasConnected) {
      Serial.println("Home Wi-Fi disconnected. AP has no upstream connection.");
      configured = false;
      disableNAT();
      lastAttempt = now;
    }
    wasConnected = false;
    // No blocking wait. Allow automatic recovery, then retry every 30 seconds.
    if (uint32_t(now - lastAttempt) >= CONNECT_TIMEOUT_MS) {
      lastAttempt = now;
      Serial.println("Still offline: retrying. Check 2.4 GHz Wi-Fi and credentials.");
      WiFi.disconnect(false, false);
      WiFi.begin(HOME_SSID, HOME_PASS);
    }
    if (!apStarted && uint32_t(now - lastCheck) >= CHECK_INTERVAL_MS) {
      lastCheck = now;
      startAP();
    }
  } else {
    if (!wasConnected) {
      configured = false;
      lastCheck = now - CHECK_INTERVAL_MS;
    }
    wasConnected = true;
    if (uint32_t(now - lastCheck) >= CHECK_INTERVAL_MS) {
      lastCheck = now;
      bool changed = WiFi.localIP() != savedIP || WiFi.subnetMask() != savedMask ||
                     WiFi.gatewayIP() != savedGateway || WiFi.dnsIP(0) != savedDNS;
      if (!configured || changed) configureRouter();
    }
  }
  delay(50);
}