Projeto IoT: Rastreador GPS com TTGO T-Call ESP32 e Servidor Traccar
Descrição
Este projeto IoT utiliza o TTGO T-Call ESP32, um módulo GSM SIM800 e um módulo GPS Betian 250 para criar um rastreador de localização em tempo real. O sistema coleta dados de localização via GPS e envia para um servidor Traccar para monitoramento. Além disso, implementa uma lógica de buffer para armazenar dados quando a conexão com o servidor estiver indisponível, enviando-os assim que a conectividade for restaurada.

Características Principais
Hardware:
TTGO T-Call ESP32 com módulo GSM SIM800.
Módulo GPS Betian 250.
Comunicação GPRS com a operadora Claro (APN: claro.com.br).
Funcionalidades:
Envio de dados de latitude, longitude, número de satélites, velocidade, altitude, carga da bateria e intensidade de sinal (RSSI).
Armazenamento de dados em buffer quando não há conexão GPRS e envio assim que reconectar.
Controle de buffer circular para garantir que nenhum dado seja perdido.
Reconexão automática em caso de falha na conexão GPRS.
Reset automático do ESP32 a cada 6 horas para garantir estabilidade.
Servidor de Localização:
Envio de dados de localização para o servidor Traccar, configurado para receber e monitorar o dispositivo.
Configuração do Hardware
Componentes Utilizados:
TTGO T-Call ESP32: Placa de desenvolvimento com ESP32 e SIM800 integrados.
Módulo GPS Betian 250: Fornece as coordenadas de localização via GPS.
Ligações:
O módulo GPS deve ser conectado nas portas TX e RX do TTGO T-Call ESP32.
Alimentação:
O TTGO T-Call ESP32 pode ser alimentado via porta USB ou bateria de 3,7V.
Requisitos
Software:
Arduino IDE ou Plataforma de desenvolvimento de sua escolha.
Bibliotecas necessárias:
TinyGSM
TinyGPS++
Configuração do Projeto
Passo a Passo
Clonar o Repositório:

bash
Copiar código
git clone https://github.com/usuario/projeto-gps-ttgo-tcall.git
cd projeto-gps-ttgo-tcall
Configurar o Código: No arquivo principal, configure as credenciais da APN de acordo com sua operadora de rede (atualmente configurada para Claro):

cpp
Copiar código
#define APN "claro.com.br"
#define USER "claro"
#define PASS "claro"
Carregar o Código: Abra o código no Arduino IDE, selecione a placa ESP32 e carregue o código no dispositivo.

Conectar ao Servidor Traccar: Certifique-se de ter um servidor Traccar configurado e ativo para receber os dados do dispositivo. As configurações de servidor e porta estão definidas no código:

cpp
Copiar código
const char server[] = "xxx.xxx.x.xxx";
const int port = 5055;
Como Funciona
Ao iniciar, o dispositivo se conecta à rede GPRS via SIM800 e começa a coletar dados de localização através do módulo GPS.
Os dados de localização são enviados para o servidor Traccar periodicamente.
Se a conexão GPRS falhar, os dados são armazenados em um buffer circular e enviados posteriormente.
O sistema realiza um reset automático do ESP32 a cada 6 horas para manter a estabilidade.
Estrutura do Projeto
main.ino: Código principal do projeto.
utilities.h: Funções auxiliares usadas no projeto.
Autor
Desenvolvido por [Jean ANdrade].

