// ==========================================================================================
// ARDUINO C++ FIRMWARE PARA NÓ IOT ESP32 - PROTOCOLO STORE-AND-FORWARD (CHRONOS DTN)
// IMPLEMENTAÇÃO DE ARMAZENAMENTO DE SEGURANÇA NA MEMÓRIA FLASH USANDO LITTLEFS
// ==========================================================================================

// Biblioteca essencial do framework Arduino que expõe as definições básicas da placa ESP32.
#include <Arduino.h>
// Biblioteca para controle da interface de comunicação de hardware I2C, utilizada pelo display.
#include <Wire.h>
// Biblioteca para manipulação de arquivos na memória de armazenamento Flash não-volátil LittleFS.
#include <FS.h>
#include <LittleFS.h>
// Biblioteca de comunicação de rede sem fio Wi-Fi nativa do chip ESP32.
#include <WiFi.h>
// Biblioteca cliente HTTP para efetuar requisições REST/JSON para a API backend.
#include <HTTPClient.h>
// Biblioteca gráfica Adafruit GFX para renderização de telas e figuras geométricas.
#include <Adafruit_GFX.h>
// Biblioteca de controle específica do display de cristal líquido OLED baseado no controlador SSD1306.
#include <Adafruit_SSD1306.h>

// Definição da largura da tela física em pixels do display OLED SSD1306 conectado.
#define LARGURA_TELA 128
// Definição da altura física em pixels da tela do display OLED SSD1306 conectado.
#define ALTURA_TELA 64
// Definição do pino de reset físico do OLED (compartilhado ou desabilitado via constante -1).
#define OLED_RESET -1
// Instanciação global da classe de controle do display OLED SSD1306 via barramento I2C.
Adafruit_SSD1306 display(LARGURA_TELA, ALTURA_TELA, &Wire, OLED_RESET);

// Pino digital configurado como a primeira entrada física do sistema (Botão para simular criação de transação).
const int PINO_BOTAO_TX = 12;
// Pino digital configurado como a segunda entrada física (Botão para forçar/cortar conectividade Wi-Fi).
const int PINO_BOTAO_LINK = 14;
// Pino digital de saída configurado para o primeiro indicador visual (LED Verde: Rede Terrestre Online).
const int PINO_LED_STATUS_OK = 2;
// Pino digital de saída configurado para o segundo indicador (LED Vermelho: Rede em Blecaute / Offline).
const int PINO_LED_BLECAUTE = 4;

// Nome da rede Wi-Fi local simulada (SSID) para estabelecimento de rede de gateway.
const char* WIFI_SSID = "Chronos_Luna_Mesh";
// Senha de autenticação WPA2 da rede Wi-Fi simulada localmente.
const char* WIFI_PASS = "InterplanetarySecureNet";

// Endereço de IP e porta lógica da API de sincronização do gateway Spring Boot na Terra.
const char* API_ENDPOINT_TX = "http://192.168.1.100:8080/api/transactions/sync";
// Caminho do arquivo JSON no LittleFS onde as transações financeiras serão persistidas localmente durante a queda.
const char* ARQUIVO_BUFFER_DTN = "/dtn_buffer_tx.json";

// Variável booleana de estado de simulação física do sinal (Simula sombra de transmissão orbital ou blecaute).
bool link_online = true;
// Armazena o timestamp da última leitura de verificação para controle de debouncing e temporização do loop.
unsigned long ultimo_tempo_loop = 0;
// Intervalo de tempo entre varreduras lógicas de rede (3000 milissegundos).
const unsigned long INTERVALO_VERIFICACAO = 3000;

// Declaração de funções auxiliares para inicialização e controle operacional modular.
void inicializarFileSystem();
void inicializarDisplay();
void criarTransacaoLocal();
void verificarConectividadeESincronizar();
void desenharInterfaceGrafica(String status_rede, int num_pendentes);

// Função de configuração inicial do ESP32 (chamada uma única vez pelo processador na inicialização).
void setup() {
  // Inicializa o barramento serial de comunicação física para envio de logs de telemetria via UART a 115200 bps.
  Serial.begin(115200);
  
  // Configura o pino do botão de transação como entrada utilizando resistor interno pull-up de pull-down virtual.
  pinMode(PINO_BOTAO_TX, INPUT_PULLUP);
  // Configura o pino do botão de conectividade como entrada utilizando resistor pull-up físico/lógico.
  pinMode(PINO_BOTAO_LINK, INPUT_PULLUP);
  
  // Configura o pino de status online como saída de sinal digital (LED Verde).
  pinMode(PINO_LED_STATUS_OK, OUTPUT);
  // Configura o pino de status offline/blecaute como saída de sinal digital (LED Vermelho).
  pinMode(PINO_LED_BLECAUTE, OUTPUT);
  
  // Escreve estado LOW (desligado) inicial no LED de sucesso.
  digitalWrite(PINO_LED_STATUS_OK, LOW);
  // Escreve estado HIGH (ligado) inicial no LED de blecaute de rede.
  digitalWrite(PINO_LED_BLECAUTE, HIGH);
  
  // Inicializa o barramento físico I2C e a comunicação visual do display OLED SSD1306.
  inicializarDisplay();
  // Monta e valida o sistema de arquivos seguro de partição interna Flash LittleFS.
  inicializarFileSystem();
  
  // Inicializa a biblioteca de hardware Wi-Fi definindo a placa em modo de conexão estação (STA).
  WiFi.mode(WIFI_STA);
  // Inicia a tentativa de comunicação física com o roteador usando credenciais espaciais configuradas.
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  
  // Escreve log de depuração serial indicando término da inicialização do setup principal do nó.
  Serial.println("[SYSTEM-INIT] No DTN Chronos Inicializado com Sucesso.");
}

// Loop principal de execução iterativa do ESP32 (chamado indefinidamente pelo microcontrolador).
void loop() {
  // Lê o estado lógico digital atual do botão físico de simulação de transação financeira.
  int estado_botao_tx = digitalRead(PINO_BOTAO_TX);
  // Lê o estado lógico digital atual do botão físico de controle de conectividade ativa.
  int estado_botao_link = digitalRead(PINO_BOTAO_LINK);
  
  // Se o botão de rede for pressionado (nível lógico LOW devido ao resistor pull-up de entrada).
  if (estado_botao_link == LOW) {
    // Inverte o estado atual da simulação de enlace físico/lógico de transmissão orbital.
    link_online = !link_online;
    // Emite log na UART indicando a alteração voluntária da qualidade do link de rádio.
    Serial.printf("[LINK-SIM] Alteracao manual do link de rede. Status atual: %s\n", link_online ? "ONLINE" : "OFFLINE_BLECAUTE");
    // Pequeno atraso (delay) preventivo para evitar ruídos de transição (debounce de chave física).
    delay(300);
  }
  
  // Se o botão de geração de transação for pressionado (solicitação de checkout de stablecoin).
  if (estado_botao_tx == LOW) {
    // Invoca rotina local para formatar e bufferizar uma nova transação financeira na partição Flash.
    criarTransacaoLocal();
    // Atraso preventivo de debounce físico para evitar múltiplas transações no mesmo gatilho mecânico.
    delay(300);
  }
  
  // Executa rotina cíclica de rede em intervalos regulares sem bloquear a execução do processador (Non-blocking timer).
  if (millis() - ultimo_tempo_loop >= INTERVALO_VERIFICACAO) {
    // Atualiza a variável de controle temporal com o instante atual medido em milissegundos.
    ultimo_tempo_loop = millis();
    // Executa a validação de canais de transmissão e sincronização de dados armazenados.
    verificarConectividadeESincronizar();
  }
}

// Inicializa a partição física do LittleFS para persistência em disco rígido flash interno.
void inicializarFileSystem() {
  // Executa a montagem lógica do sistema de arquivos seguro LittleFS na memória flash do chip.
  if (!LittleFS.begin(true)) {
    // Se falhar, exibe mensagem crítica no barramento serial.
    Serial.println("[FATAL] Erro ao inicializar o LittleFS. Memoria flash corrompida.");
    // Encerra imediatamente a rotina para evitar escrita de dados inválidos na placa.
    return;
  }
  // Exibe mensagem de validação de sucesso e disponibilidade do storage local no microcontrolador.
  Serial.println("[STORAGE] LittleFS montado e pronto para store-and-forward.");
}

// Inicializa e limpa a tela de exibição gráfica OLED conectada ao barramento I2C.
void inicializarDisplay() {
  // Inicia o display OLED alocando memória interna usando o endereço I2C padrão de mercado 0x3C.
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    // Exibe erro na console UART caso não encontre o periférico no barramento físico.
    Serial.println(F("[FATAL] Display SSD1306 nao detectado no barramento I2C. Verifique fiacao."));
    // Entra em loop infinito de segurança para indicar falha de inicialização de hardware (POST).
    for(;;);
  }
  // Limpa o buffer de vídeo interno do display.
  display.clearDisplay();
  // Configura a cor de texto padrão como branco de alto contraste para visibilidade.
  display.setTextColor(SSD1306_WHITE);
  // Desenha a mensagem inicial no display de depuração.
  display.setTextSize(1);
  display.setCursor(0,0);
  display.println("CHRONOS DTN");
  display.println("SWARM NODE v1.0");
  display.display();
}

// Simula a criação de um pagamento financeiro do tipo USDC e salva em buffer local LittleFS se offline.
void criarTransacaoLocal() {
  // Abre o arquivo de lote de transações locais no modo de leitura e escrita (APPEND / WRITE).
  File arquivo = LittleFS.open(ARQUIVO_BUFFER_DTN, FILE_APPEND);
  // Se houver falha na abertura ou criação física do arquivo de transações no LittleFS.
  if (!arquivo) {
    Serial.println("[ERROR] Nao foi possivel abrir o arquivo de buffer na Flash.");
    return;
  }
  
  // Estrutura a payload JSON da transação financeira utilizando tokens de stablecoins e dados de nó.
  // Formato: {"tx_id":<millis>,"asset":"USDC","amount":150.00,"origin":"Lunar-Node-01"}
  String payload = "{\"tx_id\":" + String(millis()) + ",\"asset\":\"USDC\",\"amount\":150.00,\"origin\":\"Lunar-Node-01\"}\n";
  
  // Grava fisicamente a string formatada de dados JSON na partição Flash não-volátil do ESP32.
  arquivo.print(payload);
  // Fecha o ponteiro do arquivo aberto garantindo a escrita segura e flush dos blocos no disco.
  arquivo.close();
  
  // Imprime no canal serial que a transação foi retida de forma segura no buffer devido a políticas de DTN.
  Serial.println("[DTN-BUFFER] Transacao de USDC armazenada localmente (Store-and-Forward).");
}

// Verifica canais físicos/lógicos de transmissão e efetua o upload em massa caso a rede esteja disponível.
void verificarConectividadeESincronizar() {
  // Conta a quantidade total de linhas (registros de transações) armazenados no arquivo local de persistência.
  int transacoes_pendentes = 0;
  // Abre o arquivo de buffer em modo de leitura padrão para contagem física de dados retidos.
  File arquivo_leitura = LittleFS.open(ARQUIVO_BUFFER_DTN, FILE_READ);
  
  // Se o arquivo existir e for carregado adequadamente pela API do LittleFS.
  if (arquivo_leitura) {
    // Loop de contagem que percorre todo o arquivo até encontrar o fim (End Of File).
    while (arquivo_leitura.available()) {
      // Lê cada string até que haja uma quebra de linha física correspondente ao fim do JSON.
      arquivo_leitura.readStringUntil('\n');
      // Incrementa o contador de pacotes de transações retidos fisicamente no buffer local.
      transacoes_pendentes++;
    }
    // Fecha o ponteiro de arquivo de leitura aberta de forma adequada.
    arquivo_leitura.close();
  }
  
  // Se o enlace lógico simulado estiver ONLINE e houver conexão estabelecida ao Wi-Fi.
  if (link_online && (WiFi.status() == WL_CONNECTED)) {
    // Acende o LED verde indicador de sinal interplanetário saudável (Terrestrial Link UP).
    digitalWrite(PINO_LED_STATUS_OK, HIGH);
    // Apaga o LED vermelho de blecaute de rede.
    digitalWrite(PINO_LED_BLECAUTE, LOW);
    
    // Atualiza a tela gráfica indicando rede saudável e processando fila de uploads.
    desenharInterfaceGrafica("LINK: ONLINE", transacoes_pendentes);
    
    // Se houver transações acumuladas no arquivo físico de buffer.
    if (transacoes_pendentes > 0) {
      Serial.printf("[DTN-GATEWAY] Conexao ativa detectada. Enviando lote de %d transacoes...\n", transacoes_pendentes);
      
      // Abre o arquivo de buffer em modo leitura para processar o despacho em lote para o endpoint Spring Boot.
      File buffer = LittleFS.open(ARQUIVO_BUFFER_DTN, FILE_READ);
      // Cria instância HTTP do client para iniciar o pipeline de requisições de rede.
      HTTPClient http;
      
      // Percorre todas as linhas e faz requisições JSON sequenciais (simulação de upload em rajada de rádio).
      while (buffer.available()) {
        // Lê a payload JSON da transação individual retida.
        String json_tx = buffer.readStringUntil('\n');
        
        // Configura o destino da requisição no objeto HTTP client.
        http.begin(API_ENDPOINT_TX);
        // Define o cabeçalho HTTP obrigatório informando tráfego de dados JSON.
        http.addHeader("Content-Type", "application/json");
        
        // Dispara requisição POST síncrona transmitindo a payload da transação financeira.
        int http_response_code = http.POST(json_tx);
        
        // Se o servidor de backend da Terra responder com código HTTP de sucesso (200 OK ou 201 Created).
        if (http_response_code > 0) {
          Serial.printf("[SYNC-SUCCESS] Transacao enviada. HTTP Status: %d\n", http_response_code);
        } else {
          // Exibe erro de rede na console UART indicando perda do pacote.
          Serial.printf("[SYNC-FAILED] Erro de rede ao despachar pacote: %s\n", http.errorToString(http_response_code).c_str());
        }
        // Encerra a requisição HTTP atual liberando recursos de soquetes da stack TCP/IP do ESP32.
        http.end();
      }
      // Fecha o arquivo após enviar todos os registros acumulados no buffer.
      buffer.close();
      
      // Exclui o arquivo temporário de transações enviadas para liberar espaço físico no chip.
      LittleFS.remove(ARQUIVO_BUFFER_DTN);
      // Reseta a contagem de transações pendentes uma vez que o lote foi despachado por completo.
      transacoes_pendentes = 0;
    }
  } else {
    // Se o canal físico estiver inativo ou a conexão Wi-Fi for perdida.
    // Apaga o LED verde de comunicação saudável.
    digitalWrite(PINO_LED_STATUS_OK, LOW);
    // Acende o LED vermelho indicando interrupção de enlace físico de rádio (DTN Blecaute).
    digitalWrite(PINO_LED_BLECAUTE, HIGH);
    
    // Atualiza a tela informando queda de comunicação e retenção ativa de dados.
    desenharInterfaceGrafica("LINK: OFFLINE", transacoes_pendentes);
  }
}

// Renderiza a interface gráfica de telemetria de rede diretamente na tela OLED.
void desenharInterfaceGrafica(String status_rede, int num_pendentes) {
  // Limpa o frame buffer interno do display para redesenhar a tela atualizada do ciclo.
  display.clearDisplay();
  // Configura a primeira linha de escrita no topo da tela do display.
  display.setCursor(0, 0);
  // Imprime título institucional do roteador de gateway financeiro de rede.
  display.println(">> CHRONOS GATEWAY <<");
  display.println("---------------------");
  // Imprime a string de status de enlace atual ("ONLINE" ou "OFFLINE").
  display.print("Enlace: ");
  display.println(status_rede);
  // Imprime a contagem de transações retidas localmente na partição flash do LittleFS.
  display.print("Buffer DTN: ");
  display.print(num_pendentes);
  display.println(" txs");
  // Exibe graficamente o estado operacional de sincronização de ativos.
  display.println("Moeda: USDC / USDT");
  // Despeja todo o buffer gráfico desenhado na tela física OLED externa do hardware.
  display.display();
}
