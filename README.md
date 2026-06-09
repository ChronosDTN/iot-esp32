# Chronos DTN — Firmware IoT ESP32

> Protótipo IoT desenvolvido com **ESP32** para simular um nó de retransmissão lunar da plataforma **Chronos DTN**.
> O sistema utiliza botões, LEDs, display LCD I2C, Wi-Fi, WebServer, endpoints JSON e dashboard local para representar o comportamento de uma fila DTN em caso de blecaute de comunicação.

---

## 🛰️ Sobre o Projeto

O **Chronos DTN** é uma solução inspirada em cenários de comunicação Terra-Lua dentro do tema de **Economia Espacial**.

Neste módulo de IoT, o ESP32 simula um **nó de retransmissão lunar** responsável por gerar e transmitir pacotes de dados. Quando a comunicação está online, os pacotes são transmitidos normalmente. Quando ocorre um blecaute, os pacotes ficam retidos em uma fila DTN. Ao restabelecer a conexão, a fila é transmitida novamente.

O conceito utilizado é baseado em **DTN — Delay/Disruption Tolerant Networking**, uma abordagem usada para ambientes onde a comunicação pode sofrer atrasos, interrupções ou perda temporária de sinal.

---

## 🎯 Objetivo do Módulo IoT

O objetivo deste módulo é demonstrar, por meio de um protótipo com ESP32, o funcionamento de um sistema embarcado capaz de:

* Simular o status de comunicação entre `ONLINE` e `BLECAUTE`;
* Gerar pacotes de dados;
* Reter pacotes em uma fila DTN durante falhas de comunicação;
* Transmitir os pacotes quando a conexão volta ao modo online;
* Exibir informações em uma interface local;
* Disponibilizar informações via Wi-Fi por meio de WebServer;
* Expor endpoints JSON;
* Apresentar uma dashboard web com os dados do sistema.

---

## 🛠️ Componentes Utilizados

| Componente        | Função                                  |
| ----------------- | --------------------------------------- |
| ESP32             | Microcontrolador principal do projeto   |
| LCD 16x2 I2C      | Interface local para exibição do status |
| Botão 1           | Alterna entre `ONLINE` e `BLECAUTE`     |
| Botão 2           | Gera um novo pacote DTN                 |
| LED verde         | Indica comunicação online               |
| LED vermelho      | Indica blecaute ou pacotes retidos      |
| Resistores        | Proteção dos LEDs                       |
| Wokwi             | Ambiente de simulação do circuito       |
| Wi-Fi Wokwi-GUEST | Rede simulada para conexão do ESP32     |
| WebServer         | Servidor HTTP embarcado no ESP32        |

---

## 📂 Estrutura de Arquivos

```txt
iot-chronos-dtn/
├── sketch.ino
├── diagram.json
├── libraries.txt
└── README.md
```

---

## 🔌 Ligações do Circuito

| Função            | Pino no ESP32 |
| ----------------- | ------------- |
| LCD SDA           | GPIO 21       |
| LCD SCL           | GPIO 22       |
| LED verde         | GPIO 25       |
| LED vermelho      | GPIO 26       |
| Botão de blecaute | GPIO 18       |
| Botão de pacote   | GPIO 19       |

---

## 📥 Entradas do Sistema

O projeto possui duas entradas principais:

### Botão de Blecaute

Responsável por alternar o estado da comunicação:

```txt
ONLINE ↔ BLECAUTE
```

Quando o sistema entra em blecaute, os pacotes deixam de ser transmitidos imediatamente e passam a ser armazenados na fila DTN.

### Botão de Pacote

Responsável por gerar um novo pacote de dados.

* Se o sistema estiver `ONLINE`, o pacote é transmitido.
* Se o sistema estiver em `BLECAUTE`, o pacote é retido na fila DTN.

---

## 📤 Saídas do Sistema

O projeto possui duas saídas principais:

### LED Verde

Indica que o sistema está online.

```txt
LED verde aceso = comunicação ONLINE
```

### LED Vermelho

Indica que o sistema está em blecaute ou possui pacotes retidos.

```txt
LED vermelho aceso = BLECAUTE ou fila DTN com pacotes
```

---

## 🖥️ Interface Local

A interface local foi implementada com um **LCD 16x2 I2C**.

O display exibe:

* Nome do projeto;
* Status da conexão;
* Quantidade de pacotes na fila DTN.

Exemplo em modo online:

```txt
CHRONOS DTN
ONLINE   F:0
```

Exemplo em modo blecaute:

```txt
CHRONOS DTN
BLECAUTE F:2
```

Exemplo durante transmissão da fila:

```txt
TRANSMITINDO
Pacotes: 2
```

---

## 📡 Comunicação Wi-Fi

O ESP32 utiliza a rede simulada do Wokwi:

```txt
SSID: Wokwi-GUEST
Senha: vazia
```

Após a conexão, o ESP32 inicializa um **WebServer na porta 80**.

No Monitor Serial, o sistema exibe:

```txt
WiFi conectado!
IP do ESP32: 10.10.0.2
Servidor Web iniciado.
Endpoints disponiveis:
/
/status
/pacotes
/toggle-blecaute
/gerar-pacote
```

> Observação: no Wokwi, o IP `10.10.0.2` pertence à rede interna da simulação. O acesso externo ao dashboard pode exigir o uso do Wokwi IoT Gateway ou configuração de port forwarding. A execução do WebServer e dos endpoints é demonstrada no Monitor Serial.

---

## 🌐 Dashboard Web

O projeto implementa uma dashboard web na rota:

```txt
/
```

A dashboard apresenta:

* Status da conexão;
* Quantidade de pacotes na fila DTN;
* Pacotes gerados;
* Pacotes transmitidos;
* Pacotes retidos;
* Latência simulada;
* Botões para gerar pacote e alternar blecaute.

---

## 🔗 Endpoints JSON

O WebServer do ESP32 disponibiliza endpoints para consulta e controle do sistema.

### GET `/status`

Retorna o status atual do nó lunar.

Exemplo de resposta:

```json
{
  "no": "SAT-LUA-01",
  "status": "ONLINE",
  "filaPacotes": 0,
  "latenciaMs": 1280
}
```

---

### GET `/pacotes`

Retorna a quantidade de pacotes gerados, transmitidos, retidos e presentes na fila.

Exemplo de resposta:

```json
{
  "gerados": 5,
  "transmitidos": 3,
  "retidos": 2,
  "filaDTN": 2
}
```

---

### GET `/toggle-blecaute`

Alterna o estado da comunicação entre `ONLINE` e `BLECAUTE`.

Exemplo de resposta:

```json
{
  "mensagem": "Status alterado",
  "statusAtual": "BLECAUTE",
  "filaPacotes": 2
}
```

---

### GET `/gerar-pacote`

Gera um novo pacote DTN.

Exemplo de resposta quando o sistema está online:

```json
{
  "mensagem": "Pacote gerado",
  "status": "TRANSMITIDO",
  "filaPacotes": 0,
  "pacotesGerados": 1
}
```

Exemplo de resposta quando o sistema está em blecaute:

```json
{
  "mensagem": "Pacote gerado",
  "status": "RETIDO_DTN",
  "filaPacotes": 1,
  "pacotesGerados": 1
}
```

---

## 🔁 Funcionamento da Fila DTN

O fluxo principal do sistema é:

1. O ESP32 inicia em modo `ONLINE`;
2. O LED verde acende;
3. O LCD mostra o status e a fila atual;
4. O usuário pode gerar pacotes pelo botão físico ou endpoint web;
5. Em modo `ONLINE`, o pacote é transmitido;
6. Em modo `BLECAUTE`, o pacote é retido na fila DTN;
7. Quando o sistema volta para `ONLINE`, os pacotes retidos são transmitidos;
8. O LCD, os LEDs e o Monitor Serial são atualizados conforme o estado do sistema.

---

## ▶️ Como Executar no Wokwi

1. Acesse o projeto no Wokwi;
2. Clique em **Start Simulation**;
3. Aguarde o LCD mostrar o status inicial;
4. Pressione o botão de blecaute para alternar entre `ONLINE` e `BLECAUTE`;
5. Pressione o botão de pacote para gerar pacotes;
6. Observe os LEDs, o LCD e o Monitor Serial;
7. Verifique a conexão Wi-Fi e os endpoints listados no Serial Monitor.

---

## 🧪 Roteiro de Teste

### Teste 1 — Inicialização

Resultado esperado:

```txt
CHRONOS DTN
ONLINE   F:0
```

LED verde aceso e LED vermelho apagado.

---

### Teste 2 — Ativar Blecaute

Ao pressionar o botão de blecaute:

```txt
CHRONOS DTN
BLECAUTE F:0
```

LED vermelho aceso.

---

### Teste 3 — Gerar Pacotes em Blecaute

Ao pressionar o botão de pacote durante o blecaute:

```txt
CHRONOS DTN
BLECAUTE F:1
```

A cada novo pacote, o número da fila aumenta.

---

### Teste 4 — Retornar Online

Ao pressionar novamente o botão de blecaute:

```txt
TRANSMITINDO
Pacotes: 2
```

Depois o sistema volta para:

```txt
CHRONOS DTN
ONLINE   F:0
```

---

## 📚 Bibliotecas Utilizadas

```cpp
#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
```

No arquivo `libraries.txt`, incluir:

```txt
LiquidCrystal I2C
```

---

## ✅ Evidências de Funcionamento

Foram validados os seguintes pontos:

* ESP32 funcionando no Wokwi;
* LCD I2C exibindo informações;
* Dois botões funcionando como entradas;
* Dois LEDs funcionando como saídas;
* Simulação de blecaute;
* Geração de pacotes;
* Fila DTN aumentando durante o blecaute;
* Transmissão dos pacotes ao voltar online;
* Conexão Wi-Fi com `Wokwi-GUEST`;
* WebServer iniciado;
* Endpoints disponíveis no Monitor Serial;
* Dashboard web implementada no código.

---

## 🎥 Roteiro do Vídeo

No vídeo de demonstração, apresentar:

1. Nome do projeto e proposta;
2. Componentes utilizados;
3. LCD mostrando o status inicial;
4. Botão alternando para blecaute;
5. LED vermelho acendendo;
6. Geração de pacotes durante o blecaute;
7. Fila DTN aumentando;
8. Retorno para online;
9. Fila sendo transmitida;
10. Monitor Serial mostrando Wi-Fi conectado e endpoints disponíveis.

---

## 🔗 Links

* Link do Wokwi: **INSERIR AQUI**
* Link do vídeo: **INSERIR AQUI**


---

## 👥 Integrantes

| Nome             | RM       | Turma  |
| ---------------- | -------- | ------ |
| Evellyn Ferreira | RM562744 | 2TDSPW |
| Maicon Douglas   | RM561279 | 2TDSPW |
| Fernando Charles | RM566482 | 2TDSPW |

---

## 📌 Observações

Este projeto foi desenvolvido para fins acadêmicos na **Global Solution FIAP 2026**, demonstrando uma aplicação de IoT em um contexto de economia espacial.

O protótipo simula um nó de retransmissão lunar com fila DTN, permitindo visualizar, de forma prática, como sistemas embarcados podem lidar com interrupções temporárias de comunicação.

