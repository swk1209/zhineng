#include "app_service.h"
#include "app_data.h"
#include "wifi_service.h"
#include <string.h>

void app_service_init(void)
{
    g_app.wifi_ok = wifi_service_is_connected();
    strncpy(g_app.wifi_ip, wifi_service_get_ip_str(), sizeof(g_app.wifi_ip) - 1);
    g_app.wifi_ip[sizeof(g_app.wifi_ip) - 1] = '\0';
}

bool app_service_update(void)
{
    bool need_redraw = false;
    uint8_t wifi_now = wifi_service_is_connected();
    const char *ip_now = wifi_service_get_ip_str();

    if (g_app.wifi_ok != wifi_now)
    {
        g_app.wifi_ok = wifi_now;
        need_redraw = true;
    }

    if (strncmp(g_app.wifi_ip, ip_now, sizeof(g_app.wifi_ip)) != 0)
    {
        strncpy(g_app.wifi_ip, ip_now, sizeof(g_app.wifi_ip) - 1);
        g_app.wifi_ip[sizeof(g_app.wifi_ip) - 1] = '\0';
        need_redraw = true;
    }

    return need_redraw;
}