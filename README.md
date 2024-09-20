# 🛰️ Projeto IoT: Rastreador GPS com TTGO T-Call ESP32 e Servidor Traccar


![Status](https://img.shields.io/badge/status-em%20desenvolvimento-yellow)
![Licença](https://img.shields.io/badge/licença-MIT-blue)

## Descrição

Este projeto implementa um rastreador GPS IoT usando o TTGO T-Call ESP32, em conjunto com um módulo GSM SIM800 e um módulo GPS Betian 250. O sistema coleta e transmite dados de localização em tempo real para um servidor Traccar, permitindo o rastreamento do dispositivo em um mapa.

Para garantir a integridade dos dados mesmo em momentos de instabilidade da conexão, o projeto inclui um sistema de buffer circular, que armazena temporariamente as informações e as envia assim que a conexão GPRS for restaurada.

---

## Funcionalidades

- 📍 **Monitoramento GPS** em tempo real
- 📶 **Conexão GPRS** para envio de dados
- 💾 **Buffer circular** para garantir que nenhum dado seja perdido

---

# 🛰️ IoT Project: GPS Tracker with TTGO T-Call ESP32 and Traccar Server

## Description

This project implements an IoT GPS tracker using the TTGO T-Call ESP32, along with a SIM800 GSM module and a Betian 250 GPS module. The system collects and transmits real-time location data to a Traccar server, allowing the device to be tracked on a map.

To ensure data integrity even during connection instability, the project includes a circular buffer system that temporarily stores information and sends it as soon as the GPRS connection is restored.

---

## Features
- 📍 Real-time GPS monitoring
- 📶 GPRS connection for data transmission
- 💾 Circular buffer to ensure no data is lost
  
--- 

## Instalação

1. Clone este repositório:
   ```bash
   git clone https://github.com/jpa-weels/traccar-tracker-with-ttgo-t-call-sim800.git
   cd projeto-gps-ttgo-tcall

2. Você pode usar um dos quatro servidor demo ou criar o seu.
   ```bash
   https://www.traccar.org/demo-server/

---

