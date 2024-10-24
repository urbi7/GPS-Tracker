#include "gsm.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
//GPS
#include "NMEA.h"

//extern GPSSTRUCT gpsData;
//###############################################################################################################
void sendLocation() {

	char sms_message[256];
	sprintf(sms_message, "Obecna lokalizacja: %.2f%c, %.2f%c, Godzina: %02d:%02d:%02d, Data: %02d.%02d.%02d", gpsData.ggastruct.lcation.latitude, gpsData.ggastruct.lcation.NS,gpsData.ggastruct.lcation.longitude, gpsData.ggastruct.lcation.EW, gpsData.ggastruct.tim.hour, gpsData.ggastruct.tim.min, gpsData.ggastruct.tim.sec, gpsData.rmcstruct.date.Day, gpsData.rmcstruct.date.Mon, gpsData.rmcstruct.date.Yr);
    // Funkcja wysyłająca obecną lokalizację
    gsm_msg_send("+48669631922", sms_message);

}

void sendStatus() {
    // Funkcja wysyłająca obecny status modułu
    char statusMsg[256];
    sprintf(statusMsg, "Status modulu: Power=%d, Registered=%d, Signal=%d%%", gsm.status.power, gsm.status.registerd, gsm_getSignalQuality_0_to_100());
    gsm_msg_send("+48669631922", statusMsg);
}
void trimWhitespace(char *str) {
	// Funkcja usuwająca białe znaki z początku i końca łańcucha znaków
    char *end;
    // Usuń białe znaki z początku
    while (isspace((unsigned char)*str)) str++;
    if (*str == 0) {
        // Wszystkie znaki były białymi znakami
        return;
    }
    // Usuń białe znaki z końca
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;
    // Znajdź nowy null-terminator
    *(end + 1) = 0;
}
//###############################################################################################################
#if (_GSM_CALL == 1 || _GSM_MSG == 1 || _GSM_GPRS == 1)
//###############################################################################################################
void gsm_callback_simcardReady(void)
{
  gsm_printf("CALLBACK SIMCARD READY\r\n");
}
//###############################################################################################################
void gsm_callback_simcardPinRequest(void)
{
  gsm_printf("CALLBACK SIMCARD PIN\r\n");
}
//###############################################################################################################
void gsm_callback_simcardPukRequest(void)
{
  gsm_printf("CALLBACK SIMCARD PUK\r\n");
}
//###############################################################################################################
void gsm_callback_simcardNotInserted(void)
{
  gsm_printf("CALLBACK SIMCARD NOT DETECT\r\n");
}
//###############################################################################################################
void gsm_callback_networkRegister(void)
{
  gsm_printf("CALLBACK NETWORK REGISTER\r\n");
}
//###############################################################################################################
void gsm_callback_networkUnregister(void)
{
  gsm_printf("CALLBACK NETWORK UNREGISTERED\r\n");
}
//###############################################################################################################
#if (_GSM_MAIN_POWER == 1)
void gsm_callback_networkNotFound(void)
{
  gsm_printf("CALLBACK NETWORK NOT FOUND\r\n");
	// Reset GSM Power
	gsm_power(false);
	gsm_power(true);
}
#endif
//###############################################################################################################
#if (_GSM_CALL == 1)
void gsm_callback_newCall(const char *number)
{
  gsm_printf("CALLBACK NEW CALL FROM: %s\r\n", number);
  //gsm_call_answer();
}
//###############################################################################################################
void gsm_callback_endCall(void)
{
  gsm_printf("CALLBACK END CALL\r\n");
}
//###############################################################################################################
void gsm_callback_dtmf(char *string, uint8_t len)
{
  gsm_printf("CALLBACK DTMF %s, LEN:%d\r\n", string, len);
}
#endif
//###############################################################################################################
#if (_GSM_MSG == 1)
void gsm_callback_newMsg(char *number, gsm_time_t time, char *msg)
{
	const char *expectedNumber = "+48669631922";
	const char *locationRequest = "Lokalizacja";
	const char *statusRequest = "Status";

	// Sprawdź numer nadawcy
	if (strcmp(number, expectedNumber) != 0)
	{
		gsm_printf("Otrzymano wiadomość od nieznanego numeru: %s\r\n", number);
		return;
	}

	// Oczyść wiadomość z białych znaków
	trimWhitespace(msg);

	// Sprawdź zawartość wiadomości
	if (strcmp(msg, locationRequest) == 0)
	{
		// Zawartość wiadomości to "Lokalizacja"
		// Wyślij obecną lokalizację
		gsm_printf("Otrzymano żądanie 'Lokalizacja'. Wysyłam lokalizację...\r\n");
		sendLocation();
	}
	else if (strcmp(msg, statusRequest) == 0)
	{
		// Zawartość wiadomości to "Status"
		// Wyślij obecny status modułu
		gsm_printf("Otrzymano żądanie 'Status'. Wysyłam status...\r\n");
		sendStatus();
	}
	else
	{
		// Wiadomość nieznana, brak akcji
		gsm_printf("Otrzymano nieznane żądanie: %s\n", msg);
	}

}
#endif
//###############################################################################################################
#if (_GSM_GPRS == 1)
void gsm_callback_gprsConnected(void)
{
  gsm_printf("CALLBACK GPRS CONNECTED, IP: %s\r\n", gsm.gprs.ip);
}
//###############################################################################################################
void gsm_callback_gprsDisconnected(void)
{
  gsm_printf("CALLBACK GPRS DISCONNECTED\r\n");
}
//###############################################################################################################
void gsm_callback_mqttMessage(char *topic, char *message)
{
  gsm_printf("CALLBACK GPRS MQTT TOPIC: %s   ----   MESSAGE: %s\r\n", topic, message);
}
//###############################################################################################################
void gsm_callback_mqttDisconnect(void)
{
  gsm_printf("CALLBACK GPRS MQTT DISCONNECT\r\n");
}
//###############################################################################################################
#endif
#endif
