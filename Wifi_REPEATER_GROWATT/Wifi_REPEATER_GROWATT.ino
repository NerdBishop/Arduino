#include <WiFi.h>
#include "esp_netif.h"

const char* HOME_SSID = "Ananse_2.4G";
const char* HOME_PASS = "xxyyzz";

const char* AP_SSID = "NerdBishop";
const char* AP_PASS = "xxyyzz";

IPAddress apIP(192, 168, 4, 1);
IPAddress gateway(192, 168, 4, 1);
IPAddress subnet(255, 255, 255, 0);
void WiFiEvent(WiFiEvent_t event, WiFiEventInfo_t info);

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  WiFi.onEvent(WiFiEvent);

  WiFi.mode(WIFI_AP_STA);

  // ============================
  // Connect to home Wi-Fi
  // ============================
  Serial.print("Connecting to ");
  Serial.println(HOME_SSID);

  WiFi.begin(HOME_SSID, HOME_PASS);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("Connected to home Wi-Fi");
  Serial.print("Upstream IP: ");
  Serial.println(WiFi.localIP());

  Serial.print("Upstream gateway: ");
  Serial.println(WiFi.gatewayIP());

  Serial.print("Upstream DNS: ");
  Serial.println(WiFi.dnsIP(0));

  // ============================
  // Start SoftAP
  // ============================
  WiFi.softAPConfig(apIP, gateway, subnet);

  if (!WiFi.softAP(AP_SSID, AP_PASS)) {
    Serial.println("Failed to start SoftAP");
    return;
  }

  Serial.println();
  Serial.println("ESP32 Access Point started");
  Serial.print("AP SSID: ");
  Serial.println(AP_SSID);
  Serial.print("AP IP: ");
  Serial.println(WiFi.softAPIP());

  // ============================
  // Get network interfaces
  // ============================
  esp_netif_t* ap_netif =
      esp_netif_get_handle_from_ifkey("WIFI_AP_DEF");

  esp_netif_t* sta_netif =
      esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");

  if (!ap_netif || !sta_netif) {
    Serial.println("ERROR: Network interface not found");
    return;
  }

  // ============================
  // Get upstream DNS
  // ============================
  esp_netif_dns_info_t dns;

  esp_err_t err =
      esp_netif_get_dns_info(
          sta_netif,
          ESP_NETIF_DNS_MAIN,
          &dns
      );

  if (err != ESP_OK) {
    Serial.print("Failed getting upstream DNS: ");
    Serial.println(esp_err_to_name(err));
    return;
  }

  // ============================
  // Reconfigure AP DHCP server
  // ============================
  err = esp_netif_dhcps_stop(ap_netif);

  if (err != ESP_OK &&
      err != ESP_ERR_ESP_NETIF_DHCP_ALREADY_STOPPED) {
    Serial.print("DHCP stop failed: ");
    Serial.println(esp_err_to_name(err));
  }

  // Tell DHCP server to OFFER DNS
  uint8_t dns_offer = 1;

  err = esp_netif_dhcps_option(
      ap_netif,
      ESP_NETIF_OP_SET,
      ESP_NETIF_DOMAIN_NAME_SERVER,
      &dns_offer,
      sizeof(dns_offer)
  );

  if (err == ESP_OK) {
    Serial.println("DHCP DNS option enabled");
  } else {
    Serial.print("DHCP DNS option failed: ");
    Serial.println(esp_err_to_name(err));
  }

  // Give AP the same DNS server as upstream
  err = esp_netif_set_dns_info(
      ap_netif,
      ESP_NETIF_DNS_MAIN,
      &dns
  );

  if (err == ESP_OK) {
    Serial.println("AP DNS configured");
  } else {
    Serial.print("AP DNS configuration failed: ");
    Serial.println(esp_err_to_name(err));
  }

  // Restart DHCP
  err = esp_netif_dhcps_start(ap_netif);

  if (err == ESP_OK) {
    Serial.println("DHCP restarted");
  } else {
    Serial.print("DHCP restart: ");
    Serial.println(esp_err_to_name(err));
  }

  // ============================
  // Make STA default route
  // ============================
  esp_netif_set_default_netif(sta_netif);

  // ============================
  // Enable NAT
  // ============================
  err = esp_netif_napt_enable(ap_netif);

  if (err == ESP_OK) {
    Serial.println("NAPT enabled successfully");
  } else {
    Serial.print("NAPT failed: ");
    Serial.println(esp_err_to_name(err));
  }

  Serial.println();
  Serial.println("==========================");
  Serial.println("Router ready");
  Serial.print("Connect Growatt to: ");
  Serial.println(AP_SSID);
  Serial.println("==========================");
}

void loop() {

  // Reconnect ESP32 upstream if home Wi-Fi drops
  if (WiFi.status() != WL_CONNECTED) {

    Serial.println("Upstream Wi-Fi lost");

    WiFi.disconnect();
    WiFi.begin(HOME_SSID, HOME_PASS);

    unsigned long start = millis();

    while (
      WiFi.status() != WL_CONNECTED &&
      millis() - start < 15000
    ) {
      delay(500);
      Serial.print(".");
    }

    Serial.println();

    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("Upstream reconnected");
      Serial.print("IP: ");
      Serial.println(WiFi.localIP());
    }
  }

  delay(5000);
}

void WiFiEvent(WiFiEvent_t event, WiFiEventInfo_t info) {

  switch (event) {

    case ARDUINO_EVENT_WIFI_AP_STACONNECTED:
      Serial.println();
      Serial.println(">>> Device connected to NerdBishop AP");
      Serial.printf(
        "MAC: %02X:%02X:%02X:%02X:%02X:%02X\n",
        info.wifi_ap_staconnected.mac[0],
        info.wifi_ap_staconnected.mac[1],
        info.wifi_ap_staconnected.mac[2],
        info.wifi_ap_staconnected.mac[3],
        info.wifi_ap_staconnected.mac[4],
        info.wifi_ap_staconnected.mac[5]
      );
      break;

    case ARDUINO_EVENT_WIFI_AP_STADISCONNECTED:
      Serial.println();
      Serial.println("<<< Device disconnected from NerdBishop AP");
      break;

    default:
      break;
  }
}