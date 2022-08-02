#include "mbed.h"
#include "stm32746g_discovery_lcd.h"
#include "stm32746g_discovery_ts.h"
#include <string>
#include <sstream>

// Default network interface object. Don't forget to change the WiFi SSID/password in mbed_app.json if you're using WiFi.
NetworkInterface *net = NetworkInterface::get_default_instance();

int main() {
    printf(" - - - Welcome - - - \n");

    // Connect to the Internet (DHCP is expected to be on)
    printf("Connecting to the network using the default network interface...\n");
    net = NetworkInterface::get_default_instance();

    nsapi_error_t net_status = NSAPI_ERROR_NO_CONNECTION;
    while ((net_status = net->connect()) != NSAPI_ERROR_OK) {
        printf("Unable to connect to network (%d). Retrying...\n", net_status);
    }

    printf("Connected to the network successfully! Trying to get ip...\n");

    nsapi_error_t ip_status = NSAPI_ERROR_NO_CONNECTION;
    SocketAddress localIP = SocketAddress();
    ip_status = net->get_ip_address(&localIP);

    printf("IP Address is %s\n", localIP.get_ip_address());

    TCPSocket TCP;
    SocketAddress targetIP = SocketAddress("10.0.0.238", 8009);

    /* ------------------------------------------------------------------------------ */
    printf("Setting up socket network stack\n");
    while (TCP.open(net) != NSAPI_ERROR_OK) {
        printf("Unable to connect to socket. Retrying...\n");
        TCP.close();
    }
    printf("Socket NetworkStack OK\n");
    /* ------------------------------------------------------------------------------ */

    while (true) {
        nsapi_error_t ip_status = TCP.connect(targetIP);
        
        if(ip_status == NSAPI_ERROR_OK) {
            printf("DONE\n");
            break;
        }
        printf("Unable to connect to socket. Retrying... (%d)\n", ip_status);
        TCP.close();
    }

    printf("TCP connected!\n");
    std::string msg = "hello! :3";
    TCP.send(msg.c_str(), msg.length());
    printf("Message sent!\n");

    BSP_TS_Init(BSP_LCD_GetXSize(), BSP_LCD_GetYSize());
    TS_StateTypeDef TS_State;
    BSP_TS_GetState(&TS_State);
    uint8_t oldval = TS_State.touchDetected;
    std::stringstream ss;

    nsapi_error_t tcp_send = NSAPI_ERROR_NO_CONNECTION;
    while(true) {
        BSP_TS_GetState(&TS_State);
        if(TS_State.touchDetected != oldval) {

            if(TS_State.touchDetected > 0) {
                ss << "Change detected! Active touches: " << std::to_string(TS_State.touchDetected) << " : (" << std::to_string(TS_State.touchX[0]) << "," << std::to_string(TS_State.touchY[0]) << ")";
                
                if(TS_State.touchX[0] > 240) {
                    printf("right");
                }else{
                    printf("left");
                }
                
            }else{
                ss << "Change detected! Active touches: " << std::to_string(TS_State.touchDetected);
            }


            tcp_send = TCP.send(ss.str().c_str(), ss.str().length());
            if(tcp_send != NSAPI_ERROR_OK) {
                printf("Error sending TCP message: (%d)\n", tcp_send);
            }
            ss.str("");
            ss.clear();

            oldval = TS_State.touchDetected;
        }

        if(TS_State.touchDetected == 5) {
            break;
        }
        HAL_Delay(15);
    }

    TCP.close();
    printf("Connection closed!\n");
}
