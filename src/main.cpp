#include <Arduino.h>
#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>


int RELE_PORT = 53;
int SENSOR_PORT = A0;

// ----------------------
// CONFIGURAÇÃO ETHERNET
// ----------------------
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192, 168, 100, 177);
IPAddress dns(192, 168, 100, 254);
IPAddress gateway(192, 168, 100, 254);
IPAddress subnet(255, 255, 255, 0);


// Tempo entre as leituras
const long intervalo = 1000;      
unsigned long tempoAnterior = 0;

// MQTT Configuration 
const char* mqttServer = "192.168.100.150"; //Ex. "broker.hivemq.com";
const char* mqttUsername = "admin";
const char* mqttPassword = "hivemq";
const int mqtt_port = 1883;
const char* mqttTopic = "arduino/luz";

// Rede Ethernet
EthernetClient ethClient;
PubSubClient client(ethClient);

// ----------------------
// FUNÇÕES MQTT - Reconecta ao servidor MQTT
// ----------------------
void reconnect() {
  while (!client.connected()) {
    Serial.print("Tentando conectar ao MQTT...");
    
    // Gera um client ID único
    String clientId = "arduinoMegaTemp_";
    clientId += String(random(0xffff), HEX);

    // Conecta no servidor, se der certo fala o ID do Client
    if (client.connect(clientId.c_str(), mqttUsername, mqttPassword)) {
      Serial.println(" conectado!");
      Serial.print("Client ID: ");
      Serial.println(clientId);
      client.subscribe("arduino/lampada");
    } else {
      // Se deu errado apresenta o que falhou.
      Serial.print(" falhou, rc=");
      Serial.print(client.state());
      Serial.print(" - ");
      
      //Código de erro
      switch(client.state()) {
        case -4: Serial.println("Timeout na conexão"); break;
        case -3: Serial.println("Conexão perdida"); break;
        case -2: Serial.println("Falha na conexão de rede"); break;
        case -1: Serial.println("Cliente desconectado"); break;
        case 1: Serial.println("Protocolo inválido"); break;
        case 2: Serial.println("Client ID rejeitado"); break;
        case 3: Serial.println("Servidor indisponível"); break;
        case 4: Serial.println("Credenciais inválidas"); break;
        case 5: Serial.println("Não autorizado"); break;
        default: Serial.println("Erro desconhecido"); break;
      }
      
      // Se falhou aguarda 5s antes de tentar novamente.
      delay(5000);
    }
  }
}

// Função que roda quando chega mensagem no tópico assinado
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Mensagem chegou no tópico: ");
  Serial.print(topic);
  Serial.print(" | Conteúdo: ");

  // Converte o payload para char para facilitar a comparação
  char comando = (char)payload[0];
  Serial.println(comando);

  // Lógica de controle (Ex: '1' liga, '0' desliga)
  // Supondo que o RELÉ está no pino 4 e aciona com LOW (comum em módulos relé)
  if (comando == '1') {
    digitalWrite(RELE_PORT, LOW); // Liga (se for relé ativo baixo)
    Serial.println("Lâmpada LIGADA via MQTT");
  } 
  else if (comando == '0') {
    digitalWrite(RELE_PORT, HIGH); // Desliga
    Serial.println("Lâmpada DESLIGADA via MQTT");
  }
}

void setup() {
  pinMode(3, OUTPUT);
  digitalWrite(3, HIGH);

  pinMode(RELE_PORT, OUTPUT);

  //Configura a serial
  Serial.begin(9600);
  while (!Serial) {
    ; // Aguarda porta serial conectar
  }
  
  Serial.println("=== INICIANDO SISTEMA ===");
  Serial.println("Configurando Ethernet...");
  
  // Tenta DHCP primeiro
  if (Ethernet.begin(mac) == 0) {
    Serial.println("DHCP falhou, usando IP fixo...");
    Ethernet.begin(mac, ip, gateway, dns);
  
  } else {
    Serial.println("DHCP conectado com sucesso!");
  }

  // Aguarda estabilizar a conexão
  delay(2000);
  
  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    Serial.println("ERRO: Shield nao detectado. Verifique o encaixe.");
  } 
  else if (Ethernet.linkStatus() == LinkOFF) {
    Serial.println("ERRO: Shield detectado, mas CABO desconectado.");
  } 
  else {
    Serial.println("SUCESSO: Hardware OK e Cabo Conectado!");
    Serial.print("IP Configurado: ");
    Serial.println(Ethernet.localIP());
  }
  
  // Mostra a configuração de Rede
  Serial.print("IP obtido: ");
  Serial.println(Ethernet.localIP());
  Serial.print("Gateway: ");
  Serial.println(Ethernet.gatewayIP());
  Serial.print("DNS: ");
  Serial.println(Ethernet.dnsServerIP());
  
  // Teste de conectividade básica
  Serial.println("Testando conectividade...");
  Serial.print("Conectando ao broker ");
  Serial.print(mqttServer);
  Serial.print(":");
  Serial.println(mqtt_port);
  
  // Configura o servidor MQTT
  client.setServer(mqttServer, mqtt_port);
  client.setCallback(callback);

  // Configuração adicional do cliente MQTT
  client.setKeepAlive(60);
  client.setSocketTimeout(30);
  
  Serial.println("Sistema iniciado. Pronto para ler encoder e enviar via MQTT.");
  Serial.println("Envie 'Z' via serial para zerar o encoder.");
  Serial.println("=====================================");
}

void loop() {

  // Se desconectar do Broker MQTT reconecta
  if (!client.connected()) {
    reconnect();
  }
  // Você precisa executar essa função de tempos em tempos.
  // Essa função mantém a conexão com o broker MQTT
  client.loop();

  // Faz uma atualização a cada segundo sem travar o loop.
  unsigned long tempoAtual = millis();

  // Se passou 1 segundo entra no loop.
  if (tempoAtual - tempoAnterior >= intervalo) {
    tempoAnterior = tempoAtual;

    //Lê a temperatura
    int leituraBruta = analogRead(SENSOR_PORT);
    int porcentagem = map(leituraBruta, 0, 1023, 0, 100);

    // Constrói a mensagem para o MQTT
    char mensagem[10];
    itoa(porcentagem, mensagem, 10);
    
    Serial.print("Leitura LDR: ");
    Serial.print(leituraBruta);
    Serial.print(" | Luz: ");
    Serial.print(mensagem);
    Serial.println("%");

    // Publica a mensagem no broker MQTT se estiver conectado
    if (client.connected()) {
      client.publish(mqttTopic, mensagem);
    }
  }
}