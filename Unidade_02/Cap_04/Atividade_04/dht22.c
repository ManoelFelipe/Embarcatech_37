/*
 * Copyright (C) 2025 Lucas Jundi Hikazudani
 * Contact: lhjundi <at> outlook <dot> com
 *
 * This file is part of Smart Bag (Transport of thermosensitive medicines).
 *
 * Smart Bag is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Smart Bag is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Smart Bag.  If not, see <https://www.gnu.org/licenses/>
 */

/**
 * @file dht22.c
 * @brief Implementação do driver para o sensor DHT22/AM2302 no Raspberry Pi Pico.
 * * @details Este arquivo contém a implementação detalhada das funções necessárias para
 * a comunicação com o sensor de temperatura e umidade DHT22. Ele utiliza o
 * protocolo de 1-fio (1-Wire) específico do sensor, que depende de uma
 * temporização precisa para a troca de dados. As funções aqui são de baixo nível
 * e manipulam diretamente os pinos GPIO e os temporizadores do Raspberry Pi Pico.
 */

 // --- Inclusão de Bibliotecas ---
 #include "dht22.h"         // Inclui a interface pública do próprio driver.
 #include "pico/stdlib.h"   // Inclui funções padrão do SDK do Pico, como `sleep_us` e `time_us_32`.
 #include "hardware/gpio.h" // Inclui funções para controle dos pinos de Entrada/Saída de Propósito Geral (GPIO).
 
 // --- Constantes de Temporização (Timing) para o Protocolo do DHT22 ---
 // Estes valores são críticos e baseados no datasheet do sensor.
 #define DHT22_START_SIGNAL_DELAY 18000  ///< Duração do sinal de início enviado pelo microcontrolador (18ms = 18000μs).
 #define DHT22_RESPONSE_WAIT_TIMEOUT 200 ///< Tempo máximo (em μs) para aguardar uma resposta do sensor após o sinal de início.
 #define DHT22_BIT_THRESHOLD 50          ///< Limite (em μs) para diferenciar um bit '0' de um bit '1'. Pulsos mais curtos que isso são '0', mais longos são '1'.
 #define DHT22_MIN_INTERVAL_MS 2000      ///< Intervalo mínimo recomendado entre leituras (2s = 2000ms) para evitar autoaquecimento do sensor.
 
 /**
  * @brief Estrutura para armazenar o estado interno e a configuração do driver DHT22.
  * @details Esta estrutura mantém as informações vitais sobre o estado do driver
  * entre as chamadas de função, como o pino utilizado e o tempo da última leitura.
  * É definida como `static` para ser visível apenas dentro deste arquivo.
  */
 typedef struct {
     uint32_t last_read_time_ms;  ///< Armazena o timestamp (em milissegundos) da última leitura bem-sucedida.
     uint32_t pin;                ///< Armazena o número do pino GPIO utilizado para a comunicação.
     bool initialized;            ///< Flag que indica se a função `dht22_init` já foi chamada.
 } dht22_state_t;
 
 // Instância global e estática da estrutura de estado do driver.
 // 'static' garante que esta variável só seja acessível dentro deste arquivo.
 static dht22_state_t dht22_state = {0, 0, false};
 
 /**
  * @brief Aguarda uma mudança de estado em um pino GPIO, com um tempo limite.
  * @details Função auxiliar para esperar até que um pino atinja um estado específico (alto ou baixo).
  * Se o estado não for alcançado dentro do timeout, a função retorna um erro.
  * É crucial para sincronizar a comunicação com o sensor.
  * @param[in] pin O número do pino GPIO a ser monitorado.
  * @param[in] state O estado esperado (`true` para alto, `false` para baixo).
  * @param[in] timeout_us O tempo máximo de espera em microssegundos.
  * @return Retorna `0` se o estado foi alcançado a tempo, ou `-1` se ocorreu um timeout.
  */
 static inline int wait_for_pin_state(uint32_t pin, bool state, uint32_t timeout_us) {
     uint32_t start = time_us_32(); // Captura o tempo inicial.
     // Continua no loop enquanto o estado do pino for diferente do desejado.
     while (gpio_get(pin) != state) {
         // Verifica se o tempo decorrido excedeu o timeout.
         if ((time_us_32() - start) > timeout_us) {
             return -1; // Timeout atingido.
         }
     }
     return 0; // Estado desejado alcançado com sucesso.
 }
 
 /**
  * @brief Implementação da função de inicialização do driver (visível publicamente).
  */
 int dht22_init(uint32_t pin) {
     // Inicializa o pino GPIO especificado para que o SDK do Pico possa usá-lo.
     gpio_init(pin);
     // Configura o pino com um resistor de pull-up interno. A linha de dados do DHT22 precisa
     // ser mantida em nível alto quando ociosa. O pull-up garante isso sem a necessidade
     // de um resistor externo. O segundo e terceiro parâmetros desativam o pull-down.
     gpio_set_pulls(pin, true, false);
     
     // Armazena as informações de configuração na estrutura de estado global.
     dht22_state.pin = pin;
     dht22_state.last_read_time_ms = 0; // Zera o tempo da última leitura.
     dht22_state.initialized = true;    // Marca o driver como inicializado.
     
     return DHT22_OK;
 }
 
 /**
  * @brief Envia o sinal de "início de comunicação" para o sensor DHT22.
  * @details Para iniciar uma leitura, o microcontrolador deve primeiro derrubar a linha de dados
  * para nível baixo por um período, e depois elevá-la, sinalizando ao sensor que
  * deseja receber dados.
  * @param[in] pin O número do pino GPIO conectado ao sensor.
  * @return Sempre retorna `DHT22_OK`.
  */
 static int dht22_send_start_signal(uint32_t pin) {
     // Configura o pino como saída para poder controlar seu nível.
     gpio_set_dir(pin, GPIO_OUT);
     
     // --- Sequência de início da comunicação ---
     gpio_put(pin, 0);                         // 1. Coloca o pino em nível baixo.
     sleep_us(DHT22_START_SIGNAL_DELAY);       // 2. Mantém em nível baixo por 18ms.
     gpio_put(pin, 1);                         // 3. Coloca o pino em nível alto.
     sleep_us(30);                             // 4. Mantém em nível alto por 30μs.
     
     // Configura o pino de volta para entrada para poder ler a resposta do sensor.
     gpio_set_dir(pin, GPIO_IN);
     
     return DHT22_OK;
 }
 
 /**
  * @brief Aguarda e verifica a resposta de confirmação do sensor.
  * @details Após o sinal de início, o sensor DHT22 deve responder com uma sequência
  * específica de níveis baixo e alto para confirmar que está pronto para enviar os dados.
  * Esta função verifica se essa sequência ocorre dentro do tempo esperado.
  * @param[in] pin O número do pino GPIO.
  * @return Retorna `DHT22_OK` se a resposta for válida, ou `DHT22_ERROR_TIMEOUT` se falhar.
  */
 static int dht22_wait_for_response(uint32_t pin) {
     // O sensor deve responder com: baixo por ~80μs, depois alto por ~80μs.
     // Usamos a função wait_for_pin_state para verificar cada transição.
     if (wait_for_pin_state(pin, 0, DHT22_RESPONSE_WAIT_TIMEOUT) != 0) return DHT22_ERROR_TIMEOUT;
     if (wait_for_pin_state(pin, 1, DHT22_RESPONSE_WAIT_TIMEOUT) != 0) return DHT22_ERROR_TIMEOUT;
     if (wait_for_pin_state(pin, 0, DHT22_RESPONSE_WAIT_TIMEOUT) != 0) return DHT22_ERROR_TIMEOUT;
     
     return DHT22_OK; // Sequência de resposta recebida com sucesso.
 }
 
 /**
  * @brief Lê os 40 bits de dados transmitidos pelo sensor.
  * @details Após a resposta, o sensor envia 5 bytes (40 bits) de dados. Cada bit é
  * codificado pela duração de um pulso em nível alto. Esta função mede a duração
  * de cada pulso para decodificar os bits.
  * @param[in] pin O número do pino GPIO.
  * @param[out] data Um buffer (array de 8 bits) para armazenar os 5 bytes de dados lidos.
  * @return Retorna `DHT22_OK` se os 40 bits forem lidos, ou `DHT22_ERROR_TIMEOUT` se houver falha.
  */
 static int dht22_read_data(uint32_t pin, uint8_t *data) {
     for (int i = 0; i < 40; i++) {
         // Cada bit começa com um pulso baixo de ~50μs. Primeiro, esperamos a linha ir para alto.
         if (wait_for_pin_state(pin, 1, DHT22_RESPONSE_WAIT_TIMEOUT) != 0) return DHT22_ERROR_TIMEOUT;
         
         // Mede a duração do pulso em nível alto para determinar se é bit 0 ou 1.
         uint32_t pulse_start = time_us_32();
         if (wait_for_pin_state(pin, 0, DHT22_RESPONSE_WAIT_TIMEOUT) != 0) return DHT22_ERROR_TIMEOUT;
         uint32_t pulse_length = time_us_32() - pulse_start;
         
         // Se o pulso alto durou mais que o limiar (50μs), é um bit '1'. Caso contrário, é '0'.
         // O bit '0' já está no array `data` (inicializado com zeros), então só precisamos definir o bit '1'.
         if (pulse_length > DHT22_BIT_THRESHOLD) {
             // Define o bit apropriado no byte correto.
             // `i / 8` calcula o índice do byte (0 a 4).
             // `7 - (i % 8)` calcula a posição do bit dentro do byte (da esquerda para a direita).
             data[i / 8] |= (1 << (7 - (i % 8)));
         }
     }
     
     return DHT22_OK;
 }
 
 /**
  * @brief Verifica a integridade dos dados usando o checksum.
  * @details O quinto byte enviado pelo sensor é um checksum, que deve ser igual à
  * soma dos quatro bytes anteriores. Isso protege contra erros de transmissão.
  * @param[in] data O buffer de 5 bytes com os dados recebidos.
  * @return `DHT22_OK` se o checksum for válido, `DHT22_ERROR_CHECKSUM` caso contrário.
  */
 static int dht22_verify_checksum(const uint8_t *data) {
     // Soma os primeiros 4 bytes (umidade e temperatura).
     // A soma é feita em um uint8_t, então o estouro (overflow) é natural e esperado,
     // correspondendo ao comportamento do sensor.
     uint8_t checksum = data[0] + data[1] + data[2] + data[3];
     
     // Compara a soma calculada com o quinto byte (checksum recebido).
     if (checksum != data[4]) {
         return DHT22_ERROR_CHECKSUM; // Erro de checksum.
     }
     return DHT22_OK; // Checksum válido.
 }
 
 /**
  * @brief Converte os dados brutos em valores de temperatura e umidade.
  * @details Os dados vêm em um formato específico que precisa ser interpretado para
  * gerar os valores flutuantes de temperatura e umidade.
  * @param[in] data Buffer com os 5 bytes de dados brutos (checksum já verificado).
  * @param[out] temperature Ponteiro para armazenar o valor final da temperatura.
  * @param[out] humidity Ponteiro para armazenar o valor final da umidade.
  * @return `DHT22_OK` se a conversão for bem-sucedida, `DHT22_ERROR_INVALID_DATA` se os valores estiverem fora da faixa.
  */
 static int dht22_convert_data(const uint8_t *data, float *temperature, float *humidity) {
     // --- Conversão da Umidade ---
     // Os bytes 0 e 1 representam a umidade. Juntamos os dois para formar um número de 16 bits
     // e dividimos por 10 para obter o valor em %RH.
     // Ex: `(data[0] << 8)` desloca o byte mais significativo 8 bits para a esquerda.
     *humidity = ((data[0] << 8) | data[1]) * 0.1f;
     
     // --- Conversão da Temperatura ---
     // Os bytes 2 e 3 representam a temperatura. O bit mais significativo (MSB) do byte 2
     // indica se a temperatura é negativa.
     // `(data[2] & 0x7F)` mascara o bit de sinal para ler o valor absoluto.
     *temperature = (((data[2] & 0x7F) << 8) | data[3]) * 0.1f;
     
     // Verifica se o bit de sinal (MSB do byte 2) está ativo.
     if (data[2] & 0x80) {
         *temperature *= -1; // Aplica o sinal negativo.
     }
     
     // Validação final: verifica se os valores convertidos estão dentro da faixa de operação do sensor.
     if (*humidity < 0.0f || *humidity > 100.0f || *temperature < -40.0f || *temperature > 80.0f) {
         return DHT22_ERROR_INVALID_DATA;
     }
     
     return DHT22_OK;
 }
 
 /**
  * @brief Implementação da função principal de leitura do sensor DHT22 (visível publicamente).
  */
 int dht22_read(float *temperature, float *humidity) {
     int result;
     uint8_t data[5] = {0}; // Inicializa o buffer de dados com zeros.
     
     // 1. Verifica se o driver foi inicializado.
     if (!dht22_state.initialized) {
         return DHT22_ERROR_NOT_INITIALIZED;
     }
     
     // 2. Garante o intervalo mínimo de 2 segundos entre as leituras.
     uint32_t current_time = to_ms_since_boot(get_absolute_time());
     if ((current_time - dht22_state.last_read_time_ms) < DHT22_MIN_INTERVAL_MS && dht22_state.last_read_time_ms != 0) {
         // Se chamada muito cedo, a função bloqueia (aguarda) pelo tempo restante.
         sleep_ms(DHT22_MIN_INTERVAL_MS - (current_time - dht22_state.last_read_time_ms));
     }
     
     // --- Executa a sequência completa de comunicação ---
     
     // 3. Envia o sinal de início.
     result = dht22_send_start_signal(dht22_state.pin);
     if (result != DHT22_OK) return result;
     
     // 4. Aguarda e valida a resposta do sensor.
     result = dht22_wait_for_response(dht22_state.pin);
     if (result != DHT22_OK) return result;
     
     // 5. Lê os 40 bits de dados.
     result = dht22_read_data(dht22_state.pin, data);
     if (result != DHT22_OK) return result;
     
     // Atualiza o timestamp da última leitura IMEDIATAMENTE após a comunicação,
     // para garantir a temporização mais precisa possível para a próxima chamada.
     dht22_state.last_read_time_ms = to_ms_since_boot(get_absolute_time());
     
     // 6. Verifica a integridade dos dados com o checksum.
     result = dht22_verify_checksum(data);
     if (result != DHT22_OK) return result;
     
     // 7. Converte os dados brutos para temperatura e umidade.
     // Esta é a última etapa, e seu resultado é retornado diretamente.
     return dht22_convert_data(data, temperature, humidity);
 }