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

## Instalação

1. Clone este repositório:
   ```bash
   git clone https://github.com/usuario/projeto-gps-ttgo-tcall.git
   cd projeto-gps-ttgo-tcall
