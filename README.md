# Chronos DTN — Firmware IoT (ESP32 + Arduino C++)

> Módulo de firmware embarcado do gateway financeiro **Chronos DTN**. Simula um nó de borda físico da rede DTN cislunar usando um microcontrolador **ESP32**, com buffer local em **LittleFS** (memória Flash), display **OLED SSD1306**, dois botões físicos e transmissão HTTP para a API backend.

---

## 🛰️ Sobre o Módulo

O nó IoT representa um **gateway de borda espacial** — o ponto físico onde as transações financeiras são geradas e enfileiradas antes de serem transmitidas à Terra. O firmware implementa o padrão **Store-and-Forward** da arquitetura DTN:

1. **Modo Online:** Transações são enviadas imediatamente à API via HTTP.
2. **Modo Offline (Blecaute):** Transações são armazenadas localmente no Flash (LittleFS) sem perda de dados.
3. **Reconexão:** Ao restabelecer o Wi-Fi, o buffer local é esvaziado automaticamente em rajada.

O display OLED exibe em tempo real o status do enlace (`ONLINE` / `OFFLINE`), a contagem de transações no buffer e confirmações de envio.

---

## 🛠️ Tecnologias e Hardware

| Item | Especificação | Função |
|---|---|---|
| Microcontrolador | ESP32 (Espressif) | Processamento, Wi-Fi e armazenamento Flash |
| Linguagem | C++ (Arduino Framework) | Firmware embarcado |
| Sistema de arquivos | LittleFS | Persistência local de transações no Flash |
| Display | OLED SSD1306 (I²C, 128×64) | Interface visual do status do nó |
| Biblioteca OLED | Adafruit SSD1306 + GFX | Driver do display |
| Comunicação | HTTP (WiFiClientSecure) | Transmissão das transações para a API |
| Botão 1 (pino 14) | Push button | Simula blecaute de sinal (toggle Wi-Fi) |
| Botão 2 (pino 12) | Push button | Gera uma nova transação de USDC no buffer |

---

## 📂 Estrutura de Arquivos

```
iot-esp32/
└── dtn_node/
    └── dtn_node.ino    # Firmware principal: setup, loop, buffer, OLED e HTTP
```

---

## ▶️ Como Executar

### Pré-requisitos

- [Arduino IDE 2.x](https://www.arduino.cc/en/software) ou [PlatformIO](https://platformio.org/) instalado
- Placa **ESP32** configurada no gerenciador de placas (adicione o repositório da Espressif)
- Bibliotecas instaladas via Gerenciador de Bibliotecas:
  - `Adafruit SSD1306`
  - `Adafruit GFX Library`
  - `ArduinoJson`
  - `LittleFS` (integrado ao ESP32 Arduino Core)

### 1. Configurar credenciais Wi-Fi e endpoint da API

No arquivo `dtn_node.ino`, localize e edite as constantes no início do arquivo:

```cpp
const char* WIFI_SSID     = "SUA_REDE_WIFI";
const char* WIFI_PASSWORD = "SUA_SENHA_WIFI";
const char* API_ENDPOINT  = "http://192.168.x.x:8080/api/transactions";
```

### 2. Gravar o firmware no ESP32

1. Conecte o ESP32 via USB.
2. Selecione a placa correta no Arduino IDE (`ESP32 Dev Module`).
3. Selecione a porta COM correspondente.
4. Clique em **Upload** (▶️).

### 3. Monitorar via Serial

Abra o **Monitor Serial** na baudrate `115200` para acompanhar os logs:

```
[DTN-NODE] Wi-Fi conectado: 192.168.1.42
[DTN-NODE] Enlace: ONLINE
[DTN-BUFFER] Transacao de USDC enviada com sucesso.
[DTN-NODE] Enlace: OFFLINE (simulado)
[DTN-BUFFER] Transacao de USDC armazenada localmente. Buffer: 3 txs
[DTN-NODE] Reconexão detectada — despejando buffer local...
```

---

## 🧪 Roteiro de Teste (Store-and-Forward)

1. Ligue o ESP32 e aguarde a conexão Wi-Fi (display mostra `Enlace: ONLINE`).
2. Pressione o **Botão 1 (pino 14)** para simular o blecaute — display muda para `Enlace: OFFLINE`.
3. Pressione o **Botão 2 (pino 12)** várias vezes para gerar transações — o display incrementa `Buffer DTN: X txs`.
4. Pressione novamente o **Botão 1** para restaurar o enlace — o buffer é despejado automaticamente para a API.

---

## 🔗 Repositórios do Projeto Chronos DTN

| Módulo | Descrição |
|---|---|
| [backend-java](https://github.com/seu-usuario/chronos-backend-java) | API principal Spring Boot 3 + JWT |
| [backend-dotnet](https://github.com/seu-usuario/chronos-backend-dotnet) | API secundária .NET 8 + EF Core |
| [database](https://github.com/seu-usuario/chronos-database) | Scripts Oracle SQL e Procedure PL/SQL |
| [devops](https://github.com/seu-usuario/chronos-devops) | Docker Compose e Dockerfile |
| [mobile-app](https://github.com/seu-usuario/chronos-mobile-app) | App React Native com Expo Router |

---

## 👤 Autores

Projeto desenvolvido para a **Global Solution — FIAP 2026**.
