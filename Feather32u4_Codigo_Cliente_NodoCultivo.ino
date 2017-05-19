// -----------------------------------------------------------------
// Código de Cliente
// -----------------------------------------------------------------

// -----------------------------------------------------------------
// Libreiras
// -----------------------------------------------------------------
#include <RHReliableDatagram.h>
#include <RH_RF95.h>
#include <SPI.h>
#include <SFE_BMP180.h>
#include <Wire.h>
#include <DHT11.h>


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
#define NUM_MAT 255
#define TOTAL_DAT RH_RF95_MAX_MESSAGE_LEN

// -----------------------------------------------------------------
// Configuracion del dirver
// -----------------------------------------------------------------

// Singleton instance of the radio driver
RH_RF95 driver(RFM95_CS, RFM95_INT);
//RH_RF95 rf95(5, 2); // Rocket Scream Mini Ultra Pro with the RFM95W

// Class to manage message delivery and receipt, using the driver declared above
RHReliableDatagram manager(driver, CLIENT_ADDRESS);

// bufer maximo envio,recepcion
uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];

int sensorPin = A1;
uint8_t sensorValur = 0;

// -----------------------------------------------------------------
// Configuracion sensor precion
// -----------------------------------------------------------------
//Se declara una instancia de la librería
SFE_BMP180 pressure;
//Se declaran las variables. Es necesario tomar en cuenta una presión inicial
//esta será la presión que se tome en cuenta en el cálculo de la diferencia de altura
double PresionBase;
//Leeremos presión y temperatura. Calcularemos la diferencia de altura
double Presion = 0;
double Altura = 0;
double Temperatura = 0;
char status;

// -----------------------------------------------------------------
// configuracion sesor humedad
// -----------------------------------------------------------------
int pin=A5;
DHT11 dht11(pin);
int error ;
float temp, humi;

// -----------------------------------------------------------------
// Configuracion encoder
// -----------------------------------------------------------------
const byte ENCODER_A= 0; 
double tiempo1 = 0;
double tiempo2 = 0;
double difTiempo = 0;
double omega = 0;
double vel = 0;
const double  intervalo=1570.796;
const double radio=0.0254;


void setup()
{
   
    Serial.begin(9600);
    Serial.println("Iniciando programa");
    delay(100);
    inicializadorRFM95();
    analogReadResolution(8);
    analogWriteResolution(8);
    SensorStart();
    
    pinMode(ENCODER_A, INPUT);
    // initialize hardware interrupts
    attachInterrupt(digitalPinToInterrupt(ENCODER_A), event, CHANGE);
}

void loop()
{   
    error  =  dht11.read(humi, temp);
    ReadSensor();
    
    String humedad = String("erro:"+String(error,DEC)+";humedad:"+humi+";temperatura:"+temp+";Presion:"+Presion+";Altura:"+Altura+";velocidad:"+vel);
    Serial.println(humedad);

    uint8_t data[humedad.length()];
    humedad.toCharArray((char*) data, humedad.length());
    
    
    if(manager.sendtoWait(data, sizeof(data),SERVER_ADDRESS)){
        uint8_t len = sizeof(buf);
        uint8_t from;   
        if (manager.recvfromAckTimeout(buf, &len, 2000, &from))
        {
            Serial.print("got reply from : 0x");
            Serial.print(from, HEX);
            Serial.print(": ");
            Serial.println((char*)buf);
        }
        else
        {
            Serial.println("No reply, is rf95_reliable_datagram_server running?");
        }
    }
    else{
        Serial.println("sendtoWait failed");
    }
    delay(500);
}

void event() {
  tiempo2=millis();
  difTiempo=tiempo2-tiempo1;
  omega=intervalo/difTiempo;
  vel=omega*radio;
  tiempo1=tiempo2;
}

void ReadSensor()
{
    //En este método se hacen las lecturas de presión y temperatura y se calcula la altura
    //Se inicia la lectura de temperatura
    status = pressure.startTemperature();
    if (status != 0)
    {
        delay(status);
        //Se realiza la lectura de temperatura
        status = pressure.getTemperature(Temperatura);
        if (status != 0)
        {
            //Se inicia la lectura de presión
            status = pressure.startPressure(3);
            if (status != 0)
            {
                delay(status);
                //Se lleva a cabo la lectura de presión,</span>
                //considerando la temperatura que afecta el desempeño del sensor</span>
                status = pressure.getPressure(Presion, Temperatura);
                if (status != 0)
                {
                    //Cálculo de la altura en base a la presión leída en el Setup
                    Altura = pressure.altitude(Presion, PresionBase);
                }
                else
                    Serial.println("Error en la lectura de presion\n");
            }
            else
                Serial.println("Error iniciando la lectura de presion\n");
        }
        else
            Serial.println("Error en la lectura de temperatura\n");
    }
    else
        Serial.println("Error iniciando la lectura de temperatura\n");
}

void SensorStart()
{
    //Secuencia de inicio del sensor
    if (pressure.begin())
        Serial.println("BMP180 init success");
    else
    {
        Serial.println("BMP180 init fail (disconnected?)\n\n");
        while (1)
            ;
    }
    //Se inicia la lectura de temperatura
    status = pressure.startTemperature();
    if (status != 0)
    {
        delay(status);
        //Se lee una temperatura inicial
        status = pressure.getTemperature(Temperatura);
        if (status != 0)
        {
            //Se inicia la lectura de presiones
            status = pressure.startPressure(3);
            if (status != 0)
            {
                delay(status);
                //Se lee la presión inicial incidente sobre el sensor en la primera ejecución
                status = pressure.getPressure(PresionBase, Temperatura);
            }
        }
    }
}

void enviarDatos(uint8_t* datos)
{
    Serial.println(sizeof(datos));
    if(manager.sendtoWait(datos, sizeof(datos),SERVER_ADDRESS)){
        uint8_t len = sizeof(buf);
        uint8_t from;   
        if (manager.recvfromAckTimeout(buf, &len, 2000, &from))
        {
            Serial.print("got reply from : 0x");
            Serial.print(from, HEX);
            Serial.print(": ");
            Serial.println((char*)buf);
        }
        else
        {
            Serial.println("No reply, is rf95_reliable_datagram_server running?");
        }
    }
    else{
        Serial.println("sendtoWait failed");
    }
}

void inicializadorRFM95()
{
    pinMode(LED, OUTPUT);
    pinMode(RFM95_RST, OUTPUT);
    digitalWrite(RFM95_RST, HIGH);

    Serial.println("Iniciar Cliente " + CLIENT_ADDRESS);

    // manual reset
    digitalWrite(RFM95_RST, LOW);
    delay(10);
    digitalWrite(RFM95_RST, HIGH);
    delay(10);

    if (!manager.init())
        Serial.println("init failed");

    // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM
    if (!driver.setFrequency(RF95_FREQ))
    {
        Serial.println("setFrequency failed");
        while (1)
            ;
    }
    Serial.print("Set Freq to: ");
    Serial.println(RF95_FREQ);
}
