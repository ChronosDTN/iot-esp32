#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// =====================
// CONFIGURAÇÕES DO WIFI
// =====================
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// Servidor web na porta 80
WebServer server(80);

// =====================
// PINOS DO PROJETO
// =====================
#define LED_VERDE 25
#define LED_VERMELHO 26

#define BOTAO_BLECAUTE 18
#define BOTAO_PACOTE 19

// LCD I2C no endereco 0x27, 16 colunas e 2 linhas
LiquidCrystal_I2C lcd(0x27, 16, 2);

// =====================
// VARIÁVEIS DO SISTEMA
// =====================
bool blecaute = false;

int filaPacotes = 0;
int pacotesGerados = 0;
int pacotesTransmitidos = 0;
int pacotesRetidos = 0;

const int latenciaMs = 1280;

bool ultimoEstadoBlecaute = HIGH;
bool ultimoEstadoPacote = HIGH;

// =====================
// FUNÇÕES DO LCD E LEDS
// =====================
void atualizarTela() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("CHRONOS DTN");

  lcd.setCursor(0, 1);

  if (blecaute) {
    lcd.print("BLECAUTE F:");
  } else {
    lcd.print("ONLINE   F:");
  }

  lcd.print(filaPacotes);
}

void atualizarLeds() {
  if (blecaute || filaPacotes > 0) {
    digitalWrite(LED_VERDE, LOW);
    digitalWrite(LED_VERMELHO, HIGH);
  } else {
    digitalWrite(LED_VERDE, HIGH);
    digitalWrite(LED_VERMELHO, LOW);
  }
}

void transmitirFila() {
  if (!blecaute && filaPacotes > 0) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("TRANSMITINDO");
    lcd.setCursor(0, 1);
    lcd.print("Pacotes: ");
    lcd.print(filaPacotes);

    Serial.print("Transmitindo pacotes retidos: ");
    Serial.println(filaPacotes);

    pacotesTransmitidos += filaPacotes;
    filaPacotes = 0;

    delay(1500);
  }
}

// =====================
// LÓGICA DO PROJETO
// =====================
void alternarBlecaute() {
  blecaute = !blecaute;

  Serial.print("Status alterado para: ");
  Serial.println(blecaute ? "BLECAUTE" : "ONLINE");

  if (!blecaute) {
    transmitirFila();
  }

  atualizarTela();
  atualizarLeds();
}

void gerarPacote() {
  pacotesGerados++;

  if (blecaute) {
    filaPacotes++;
    pacotesRetidos++;

    Serial.println("Pacote retido na fila DTN.");
  } else {
    pacotesTransmitidos++;

    Serial.println("Pacote transmitido com sucesso.");
  }

  atualizarTela();
  atualizarLeds();
}

// =====================
// ENDPOINT: DASHBOARD
// =====================
void handleDashboard() {
  String statusAtual = blecaute ? "BLECAUTE" : "ONLINE";
  String corStatus = blecaute ? "#ef4444" : "#22c55e";

  String html = "";
  html += "<!DOCTYPE html>";
  html += "<html lang='pt-BR'>";
  html += "<head>";
  html += "<meta charset='UTF-8'>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  html += "<meta http-equiv='refresh' content='3'>";
  html += "<title>Chronos DTN</title>";
  html += "<style>";
  html += "body{font-family:Arial;background:#0f172a;color:#e5e7eb;margin:0;padding:20px;}";
  html += ".container{max-width:900px;margin:auto;}";
  html += "h1{color:#38bdf8;}";
  html += ".grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(180px,1fr));gap:15px;}";
  html += ".card{background:#1e293b;border-radius:12px;padding:18px;box-shadow:0 4px 12px rgba(0,0,0,.3);}";
  html += ".label{font-size:14px;color:#94a3b8;}";
  html += ".value{font-size:28px;font-weight:bold;margin-top:8px;}";
  html += ".status{color:" + corStatus + ";}";
  html += "a{display:inline-block;margin-top:20px;margin-right:10px;padding:12px 16px;background:#2563eb;color:white;text-decoration:none;border-radius:8px;}";
  html += "a.red{background:#dc2626;}";
  html += "a.green{background:#16a34a;}";
  html += ".desc{line-height:1.5;color:#cbd5e1;}";
  html += "</style>";
  html += "</head>";
  html += "<body>";
  html += "<div class='container'>";
  html += "<h1>Chronos DTN Dashboard</h1>";
  html += "<p class='desc'>Simulador IoT de um no de retransmissao lunar com fila DTN para retencao de pacotes durante blecautes de comunicacao.</p>";

  html += "<div class='grid'>";

  html += "<div class='card'>";
  html += "<div class='label'>Status da conexao</div>";
  html += "<div class='value status'>" + statusAtual + "</div>";
  html += "</div>";

  html += "<div class='card'>";
  html += "<div class='label'>Fila DTN</div>";
  html += "<div class='value'>" + String(filaPacotes) + "</div>";
  html += "</div>";

  html += "<div class='card'>";
  html += "<div class='label'>Pacotes gerados</div>";
  html += "<div class='value'>" + String(pacotesGerados) + "</div>";
  html += "</div>";

  html += "<div class='card'>";
  html += "<div class='label'>Pacotes transmitidos</div>";
  html += "<div class='value'>" + String(pacotesTransmitidos) + "</div>";
  html += "</div>";

  html += "<div class='card'>";
  html += "<div class='label'>Pacotes retidos</div>";
  html += "<div class='value'>" + String(pacotesRetidos) + "</div>";
  html += "</div>";

  html += "<div class='card'>";
  html += "<div class='label'>Latencia simulada</div>";
  html += "<div class='value'>" + String(latenciaMs) + "ms</div>";
  html += "</div>";

  html += "</div>";

  html += "<a class='green' href='/gerar-pacote'>Gerar pacote</a>";
  html += "<a class='red' href='/toggle-blecaute'>Alternar blecaute</a>";
  html += "<a href='/status'>Ver JSON /status</a>";
  html += "<a href='/pacotes'>Ver JSON /pacotes</a>";

  html += "</div>";
  html += "</body>";
  html += "</html>";

  server.send(200, "text/html", html);
}

// =====================
// ENDPOINT JSON: STATUS
// =====================
void handleStatus() {
  String statusAtual = blecaute ? "BLECAUTE" : "ONLINE";

  String json = "{";
  json += "\"no\":\"SAT-LUA-01\",";
  json += "\"status\":\"" + statusAtual + "\",";
  json += "\"filaPacotes\":" + String(filaPacotes) + ",";
  json += "\"latenciaMs\":" + String(latenciaMs);
  json += "}";

  server.send(200, "application/json", json);
}

// =====================
// ENDPOINT JSON: PACOTES
// =====================
void handlePacotes() {
  String json = "{";
  json += "\"gerados\":" + String(pacotesGerados) + ",";
  json += "\"transmitidos\":" + String(pacotesTransmitidos) + ",";
  json += "\"retidos\":" + String(pacotesRetidos) + ",";
  json += "\"filaDTN\":" + String(filaPacotes);
  json += "}";

  server.send(200, "application/json", json);
}

// =====================
// ENDPOINT JSON: TOGGLE BLECAUTE
// =====================
void handleToggleBlecaute() {
  alternarBlecaute();

  String statusAtual = blecaute ? "BLECAUTE" : "ONLINE";

  String json = "{";
  json += "\"mensagem\":\"Status alterado\",";
  json += "\"statusAtual\":\"" + statusAtual + "\",";
  json += "\"filaPacotes\":" + String(filaPacotes);
  json += "}";

  server.send(200, "application/json", json);
}

// =====================
// ENDPOINT: GERAR PACOTE
// =====================
void handleGerarPacote() {
  gerarPacote();

  String json = "{";
  json += "\"mensagem\":\"Pacote gerado\",";
  json += "\"status\":\"" + String(blecaute ? "RETIDO_DTN" : "TRANSMITIDO") + "\",";
  json += "\"filaPacotes\":" + String(filaPacotes) + ",";
  json += "\"pacotesGerados\":" + String(pacotesGerados);
  json += "}";

  server.send(200, "application/json", json);
}

// =====================
// SETUP
// =====================
void setup() {
  Serial.begin(115200);

  // LCD I2C
  Wire.begin(21, 22);
  lcd.init();
  lcd.backlight();

  lcd.setCursor(0, 0);
  lcd.print("CHRONOS DTN");
  lcd.setCursor(0, 1);
  lcd.print("Iniciando...");

  // LEDs
  pinMode(LED_VERDE, OUTPUT);
  pinMode(LED_VERMELHO, OUTPUT);

  // Botões com pullup interno
  pinMode(BOTAO_BLECAUTE, INPUT_PULLUP);
  pinMode(BOTAO_PACOTE, INPUT_PULLUP);

  // Wi-Fi
  Serial.println("Conectando ao WiFi...");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi conectado!");
  Serial.print("IP do ESP32: ");
  Serial.println(WiFi.localIP());

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("WiFi conectado");
  lcd.setCursor(0, 1);
  lcd.print(WiFi.localIP());

  delay(2500);

  // Rotas do servidor
  server.on("/", handleDashboard);
  server.on("/status", handleStatus);
  server.on("/pacotes", handlePacotes);
  server.on("/toggle-blecaute", handleToggleBlecaute);
  server.on("/gerar-pacote", handleGerarPacote);

  server.begin();

  Serial.println("Servidor Web iniciado.");
  Serial.println("Endpoints disponiveis:");
  Serial.println("/");
  Serial.println("/status");
  Serial.println("/pacotes");
  Serial.println("/toggle-blecaute");
  Serial.println("/gerar-pacote");

  atualizarTela();
  atualizarLeds();
}

// =====================
// LOOP
// =====================
void loop() {
  server.handleClient();

  bool estadoBlecaute = digitalRead(BOTAO_BLECAUTE);
  bool estadoPacote = digitalRead(BOTAO_PACOTE);

  if (estadoBlecaute == LOW && ultimoEstadoBlecaute == HIGH) {
    alternarBlecaute();
    delay(300);
  }

  if (estadoPacote == LOW && ultimoEstadoPacote == HIGH) {
    gerarPacote();
    delay(300);
  }

  ultimoEstadoBlecaute = estadoBlecaute;
  ultimoEstadoPacote = estadoPacote;
}