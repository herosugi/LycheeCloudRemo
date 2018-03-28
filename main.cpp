
#include "mbed.h"
#include "https_request.h"
#include "ESP32Interface.h"

ESP32Interface wifi(P5_3, P3_14, P7_1, P0_1);
Serial pc(USBTX, USBRX);

DigitalOut led1(LED1);

/* List of trusted root CA certificates
 * This is the root CA for Google from GeoTrustGlobalCA
 *
 * To add more root certificates, just concatenate them.
 */
const char SSL_CA_PEM[] = "-----BEGIN CERTIFICATE-----\n"
    "MIIDVDCCAjygAwIBAgIDAjRWMA0GCSqGSIb3DQEBBQUAMEIxCzAJBgNVBAYTAlVT\n"
    "MRYwFAYDVQQKEw1HZW9UcnVzdCBJbmMuMRswGQYDVQQDExJHZW9UcnVzdCBHbG9i\n"
    "YWwgQ0EwHhcNMDIwNTIxMDQwMDAwWhcNMjIwNTIxMDQwMDAwWjBCMQswCQYDVQQG\n"
    "EwJVUzEWMBQGA1UEChMNR2VvVHJ1c3QgSW5jLjEbMBkGA1UEAxMSR2VvVHJ1c3Qg\n"
    "R2xvYmFsIENBMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEA2swYYzD9\n"
    "9BcjGlZ+W988bDjkcbd4kdS8odhM+KhDtgPpTSEHCIjaWC9mOSm9BXiLnTjoBbdq\n"
    "fnGk5sRgprDvgOSJKA+eJdbtg/OtppHHmMlCGDUUna2YRpIuT8rxh0PBFpVXLVDv\n"
    "iS2Aelet8u5fa9IAjbkU+BQVNdnARqN7csiRv8lVK83Qlz6cJmTM386DGXHKTubU\n"
    "1XupGc1V3sjs0l44U+VcT4wt/lAjNvxm5suOpDkZALeVAjmRCw7+OC7RHQWa9k0+\n"
    "bw8HHa8sHo9gOeL6NlMTOdReJivbPagUvTLrGAMoUgRx5aszPeE4uwc2hGKceeoW\n"
    "MPRfwCvocWvk+QIDAQABo1MwUTAPBgNVHRMBAf8EBTADAQH/MB0GA1UdDgQWBBTA\n"
    "ephojYn7qwVkDBF9qn1luMrMTjAfBgNVHSMEGDAWgBTAephojYn7qwVkDBF9qn1l\n"
    "uMrMTjANBgkqhkiG9w0BAQUFAAOCAQEANeMpauUvXVSOKVCUn5kaFOSPeCpilKIn\n"
    "Z57QzxpeR+nBsqTP3UEaBU6bS+5Kb1VSsyShNwrrZHYqLizz/Tt1kL/6cdjHPTfS\n"
    "tQWVYrmm3ok9Nns4d0iXrKYgjy6myQzCsplFAMfOEVEiIuCl6rYVSAlk6l5PdPcF\n"
    "PseKUgzbFbS9bZvlxrFUaKnjaZC2mqUPuLk/IH2uSrW4nOQdtqvmlKXBx4Ot2/Un\n"
    "hw4EbNX/3aBd7YdStysVAq45pmp06drE57xNNB6pXE0zX5IJL4hmXXeXxx12E6nV\n"
    "5fEWCRE11azbJHFwLJhWC9kXtNHjUStedejV0NxPNO3CBWaAocvmMw==\n"
    "-----END CERTIFICATE-----";

void dump_response(HttpResponse* res) {
    mbedtls_printf("Status: %d - %s\n", res->get_status_code(), res->get_status_message().c_str());

    mbedtls_printf("Headers:\n");
    for (size_t ix = 0; ix < res->get_headers_length(); ix++) {
        mbedtls_printf("\t%s: %s\n", res->get_headers_fields()[ix]->c_str(), res->get_headers_values()[ix]->c_str());
    }
    mbedtls_printf("\nBody (%d bytes):\n\n%s\n", res->get_body_length(), res->get_body_as_string().c_str());
}

void halt() {
    // 異常終了した場合、LED1を高速で点滅する
    while (1) {
        led1 = !led1;
        Thread::wait(100);
    }
}

int main() {
    DigitalOut output_pin(D13);     // 制御を行うピン番号

    string previous_command = "";   // 前回のコマンド

    pc.baud(115200);

    printf("Connecting...\r\n");
    int ret = wifi.connect(MBED_CONF_APP_WIFI_SSID, MBED_CONF_APP_WIFI_PASSWORD, NSAPI_SECURITY_WPA_WPA2);
    if (ret != 0) {
        printf("Connection error\r\n");
        halt();
    }
    printf("Success\r\n\r\n");
    printf("MAC: %s\r\n", wifi.get_mac_address());
    printf("IP: %s\r\n", wifi.get_ip_address());
    printf("Netmask: %s\r\n", wifi.get_netmask());
    printf("Gateway: %s\r\n", wifi.get_gateway());
    printf("RSSI: %d\r\n\r\n", wifi.get_rssi());

    while (1) {
        {
            // printf("\r\n----- HTTPS GET request -----\r\n");
            HttpsRequest* get_req = new HttpsRequest(&wifi, SSL_CA_PEM, HTTP_GET, MBED_CONF_APP_SERVER_URL);
            // get_req->set_debug(true);

            HttpResponse* get_res = get_req->send();
            if (!get_res) {
                printf("HttpRequest failed (error code %d)\r\n", get_req->get_error());
                Thread::wait(30000);
            }

            // printf("\r\n----- HTTPS GET response -----\r\n");
            // dump_response(get_res);

            // コマンドの解析
            string command = get_res->get_body_as_string();
            if (previous_command != command) {
                mbedtls_printf("%s\r\n", command.c_str());

                if (command.find("ON") != string::npos) {
                    output_pin = 1;
                }
                else if (command.find("OFF") != string::npos) {
                    output_pin = 0;
                }

                previous_command = command;
            }

            delete get_req;
        }

        led1 = !led1;
        Thread::wait(500);
    }

    Thread::wait(osWaitForever);
}

