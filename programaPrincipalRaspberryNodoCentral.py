# -*- coding: utf-8 -*-

import serial

import time

import pymongo

import os.path

#Funcion de crear archivo .txt
def creaciontxt(nombretxt):
    archivo = open(nombretxt,'w')
    archivo.close()

#Funcion de escribir sobre el archivo
def grabartxt(nombretxt,tiempo,hora,error,hum,temp,pre,alt,vel):
    archivo = open(nombretxt,'a')
    archivo.write(tiempo+';'+hora+';'+error+';' + hum + ';' + temp + ';' + pre + ';' + alt + ';'+vel+'\n')
    archivo.close()

#Funcion de leer archivo de texto
def leertxt(nombretxt):
    archivo = open(nombretxt,'r')
    linea = archivo.readline()
    while linea != "":
        print linea
	linea = archivo.readline()
    archivo.close()

#Se actualiza los datos que son enviados por el Arduino
def actualizarDatos():
    arduino=serial.Serial('/dev/ttyACM0',baudrate=9600,timeout=3.0)
    datos=""
    #Espera a que se establezca la comunicación
    time.sleep(0.5)
    while arduino.inWaiting()>0:
        tiempo=str(int(round(time.time()*1000)))
        hora = time.strftime("%H:%M:%S")
        datos= tiempo+';'+hora + ';' + arduino.readline()
	#datos.append(recibido)
        print "Dato Recibido"
	print datos
    arduino.close()
    return datos

#Sube los datos a la base de datos
def subirDatos(lista):
    SEED_DATA=lista
    #Comunicacion base de datos
    uri ='mongodb://user1:conectar17@ds145380.mlab.com:45380/raspberrymongo'
    client = pymongo.MongoClient(uri)
    db = client.get_default_database()
    #Coleccion
    meteo = db['meteo']
    meteo.insert(SEED_DATA)
    client.close()
    
### Principal
#sudo service ntp restart
#sudo date --set "2017-04-13 13:06"

#Creacion de Archivo .txt
fecha = time.strftime("%y_%m_%d")
encabezado = 'DatosEstacion'
nomArchivo = encabezado+fecha+'.txt'
ruta="/home/pi/Documents/proyecto/"+nomArchivo
if not os.path.isfile(ruta):
    creaciontxt(nomArchivo)

#Manejo de Datos
while True:
    darduino=actualizarDatos()
    datos_Meteo = darduino.split(";")

    if len(datos_Meteo)==8:    
        tiempo= datos_Meteo[0]
        hora = datos_Meteo[1]
        
        t_error = datos_Meteo[2]
        d_error = t_error.split(":")
        error = d_error[1]
        
        t_hum = datos_Meteo[3]
        d_hum = t_hum.split(":")
        hum = d_hum[1]
    
        t_temp = datos_Meteo[4]
        d_temp = t_temp.split(":")
        temp = d_temp[1]
    
        t_pre = datos_Meteo[5]
        d_pre = t_pre.split(":")
        pre = d_pre[1]
        
        t_alt = datos_Meteo[6]
        d_alt = t_alt.split(":")
        alt = d_alt[1]

        t_vel = datos_Meteo[7]
        d_vel = t_vel.split(":")
        vel = d_vel[1]
    
        muestra = [tiempo,hora,error,hum,temp,pre,alt,vel]
        print "Organizacion de la Informacion"
        print muestra
        grabartxt(nomArchivo,*muestra)
        dicc={'time':int(tiempo),'fecha':fecha,
              'hora':hora,'humedad':float(hum),
              'temperatura':float(temp),'presion':float(pre),
              'altura':float(alt),'vel':float(vel)}
        print "Listo para subir a Base de Datos"
        #subirDatos(dicc)
        print "OK"
        print ""
#Lectura de Datos
#leertxt(nomArchivo)
