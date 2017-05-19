// -----------------------------------------------------------------
// Código de servidor
// -----------------------------------------------------------------

struct clientes
{
    uint8_t direccion;
    bool activo;
};

// -----------------------------------------------------------------
// Libreiras
// -----------------------------------------------------------------
#include <RHReliableDatagram.h>
#include <RH_RF95.h>
#include <SPI.h>

// -----------------------------------------------------------------
// configuracion RFM95
// -----------------------------------------------------------------
#define RFM95_CS 8
#define RFM95_RST 4
#define RFM95_INT 3

#define CLIENT_ADDRESS 1
#define SERVER_ADDRESS 2

#define RF95_FREQ 434.0

// -----------------------------------------------------------------
// definiciones extra
// -----------------------------------------------------------------
#define LED 13
#define TOTAL_COMANDOS 2

// -----------------------------------------------------------------
// Configuracion del dirver
// -----------------------------------------------------------------

// Singleton instance of the radio driver
RH_RF95 driver(RFM95_CS, RFM95_INT);
// Class to manage message delivery and receipt, using the driver declared above
RHReliableDatagram manager(driver, SERVER_ADDRESS);

// -----------------------------------------------------------------
// Constantes
// -----------------------------------------------------------------

// bufer maximo envio,recepcion
uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];

clientes conectados[5];

void setup()
{
    inicializadorRFM95();
}

void loop()
{
   recibir();
}

void recibir()
{
    uint8_t len = sizeof(buf);
    uint8_t from;
    if (manager.recvfromAck(buf, &len, &from)){
        Serial1.println((char *) buf);
        uint8_t data[] = "ok";
        if (!manager.sendtoWait(data, sizeof(data), from))
            Serial1.println("sendtoWait failed");
    }
}

// void validarComando(char *comandos[TOTAL_COMANDOS], uint8_t from)
// {
//     char *comando = comandos[0];
//     Serial1.println(comando);
//     if (strcmp(comando, "conectar") == 0)
//     {
//         Serial1.println("entro");
//         uint8_t data[] = "ok";
//         if (!manager.sendtoWait(data, sizeof(data), from))
//             Serial1.println("sendtoWait failed");
//     }
//     else
//         Serial1.println("no entro");
// }

void inicializadorRFM95()
{
    pinMode(LED, OUTPUT);
    pinMode(RFM95_RST, OUTPUT);
    digitalWrite(RFM95_RST, HIGH);

    while (!Serial1)
        ;
    Serial1.begin(9600);
    delay(100);

    Serial1.println("Iniciar servidor");

    // manual reset
    digitalWrite(RFM95_RST, LOW);
    delay(10);
    digitalWrite(RFM95_RST, HIGH);
    delay(10);

    if (!manager.init())
        Serial1.println("init failed");

    // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM
    if (!driver.setFrequency(RF95_FREQ))
    {
        Serial1.println("setFrequency failed");
        while (1)
            ;
    }
    Serial1.print("Set Freq to: ");
    Serial1.println(RF95_FREQ);
}
