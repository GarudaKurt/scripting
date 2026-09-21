#include <MQTTClient.h>
#include "lib/api.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>

#define ADDRESS     "tcp://broker.hivemq.com:1883"
#define CLIENT_ID   "cpp_attendance_simulator"
#define TOPIC       "embedded/attendance"
#define QOS         0
#define TIMEOUT     10000L

int main() {
    attendance_request_t req = { "esp32-sim-02", "EMP-1024" };
    char payload[256];

    if (build_time_in_payload(&req, payload, sizeof(payload)) != API_SUCCESS) {
        fprintf(stderr, "Failed to build time-in payload\n");
        return EXIT_FAILURE;
    }

    printf("Payload built:\n%s\n", payload);

    // --- MQTT connect + publish (this part was missing) ---
    MQTTClient client;
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    MQTTClient_message pubmsg = MQTTClient_message_initializer;
    MQTTClient_deliveryToken token;
    int rc;

    MQTTClient_create(&client, ADDRESS, CLIENT_ID, MQTTCLIENT_PERSISTENCE_NONE, NULL);

    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;

    printf("Connecting to broker at %s...\n", ADDRESS);

    if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS) {
        printf("Failed to connect, return code %d\n", rc);
        return EXIT_FAILURE;
    }

    printf("Connected successfully!\n");

    pubmsg.payload = payload;
    pubmsg.payloadlen = (int)strlen(payload);
    pubmsg.qos = QOS;
    pubmsg.retained = 0;

    printf("Publishing to topic '%s'...\n", TOPIC);
    MQTTClient_publishMessage(client, TOPIC, &pubmsg, &token);
    rc = MQTTClient_waitForCompletion(client, token, TIMEOUT);

    if (rc == MQTTCLIENT_SUCCESS) {
        printf("Message delivered successfully (token: %d)\n", token);
    } else {
        printf("Message delivery failed, return code %d\n", rc);
    }

    MQTTClient_disconnect(client, 10000);
    MQTTClient_destroy(&client);

    return EXIT_SUCCESS;
}