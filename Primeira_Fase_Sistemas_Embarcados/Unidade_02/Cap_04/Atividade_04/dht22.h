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
 * @file dht22.h
 * @brief Interface do driver para o sensor de temperatura e umidade DHT22/AM2302.
 * * @details Este arquivo define a interface pública para comunicação com o sensor
 * DHT22, também conhecido como AM2302. Ele abstrai a complexidade do protocolo
 * de comunicação de 1-fio (1-Wire) do sensor, fornecendo funções simples para
 * inicialização e leitura dos dados ambientais.
 * * O DHT22 é um sensor digital popular que fornece leituras de:
 * - **Temperatura:** Faixa de -40°C a 80°C com uma precisão de ±0.5°C.
 * - **Umidade Relativa:** Faixa de 0% a 100% com uma precisão de ±2%.
 * * A aplicação que utilizar este driver precisa apenas incluir este arquivo,
 * chamar `dht22_init()` uma vez e, em seguida, chamar `dht22_read()` para
 * obter os valores de temperatura e umidade.
 */

 #ifndef DHT22_H
 #define DHT22_H
 
 #include <stdint.h>
 
 /**
  * @brief Códigos de retorno para as operações do driver DHT22.
  * * @details Estas constantes são usadas como valores de retorno pelas funções do driver
  * para indicar o resultado da operação. Isso permite que a aplicação que chama
  * as funções possa tratar adequadamente os casos de sucesso e de erro.
  */
 #define DHT22_OK 0                        ///< Operação realizada com sucesso. Os dados de temperatura e umidade são válidos.
 #define DHT22_ERROR_CHECKSUM -1           ///< Falha na verificação do checksum. Indica que os dados recebidos foram corrompidos durante a transmissão.
 #define DHT22_ERROR_TIMEOUT -2            ///< Timeout durante a comunicação. O sensor não respondeu no tempo esperado, o que pode indicar um problema de conexão ou falha do sensor.
 #define DHT22_ERROR_INVALID_DATA -3       ///< Os dados recebidos estão fora dos limites físicos esperados (ex: umidade > 100%). Isso pode indicar uma leitura incorreta ou falha do sensor.
 #define DHT22_ERROR_NOT_INITIALIZED -4    ///< Tentativa de uso do driver sem antes chamar a função de inicialização `dht22_init()`.
 
 /**
  * @brief Inicializa o driver do sensor DHT22.
  * * @details Esta função deve ser chamada obrigatoriamente antes de qualquer tentativa de leitura do sensor.
  * Ela realiza as seguintes configurações essenciais:
  * - Configura o pino GPIO especificado como uma entrada.
  * - Ativa o resistor de pull-up interno no pino GPIO, que é necessário para manter a linha de dados em nível alto quando o sensor não está transmitindo.
  * - Inicializa a estrutura de estado interno do driver.
  * * @param[in] pin O número do pino GPIO do Raspberry Pi Pico onde o pino de dados do sensor DHT22 está conectado.
  * * @return Retorna `DHT22_OK` se a inicialização for bem-sucedida.
  * * @note O pino de dados do sensor DHT22 deve estar conectado ao pino GPIO especificado. Além disso,
  * o sensor requer alimentação (VCC, tipicamente 3.3V a 5.5V) e uma conexão com o terra (GND) para funcionar.
  */
 int dht22_init(uint32_t pin);
 
 /**
  * @brief Realiza uma leitura completa dos dados de temperatura e umidade do sensor DHT22.
  * * @details Esta é a função principal do driver. Ela executa todo o protocolo de comunicação
  * com o sensor, que consiste nos seguintes passos:
  * 1. Envia um sinal de início (start signal) para "acordar" o sensor.
  * 2. Aguarda e valida o sinal de resposta do sensor.
  * 3. Recebe 40 bits de dados (5 bytes) contendo as informações de umidade, temperatura e um checksum.
  * 4. Verifica a integridade dos dados usando o checksum.
  * 5. Converte os dados brutos recebidos em valores de ponto flutuante para temperatura (em graus Celsius) e umidade (em percentual).
  * * A função também respeita automaticamente o intervalo mínimo de 2 segundos entre as leituras,
  * conforme recomendado pelo fabricante do sensor para garantir a estabilidade das medições.
  * Se esta função for chamada antes de 2 segundos terem passado desde a última leitura,
  * ela aguardará internamente o tempo necessário.
  * * @param[out] temperature Ponteiro para uma variável do tipo `float` onde o valor da temperatura em graus Celsius será armazenado.
  * A faixa de medição é de -40°C a 80°C.
  * @param[out] humidity    Ponteiro para uma variável do tipo `float` onde o valor da umidade relativa em percentual será armazenado.
  * A faixa de medição é de 0% a 100%.
  * * @return Retorna um dos códigos de status definidos anteriormente:
  * - `DHT22_OK`: Leitura realizada com sucesso. Os valores em `temperature` e `humidity` são válidos.
  * - `DHT22_ERROR_CHECKSUM`: Erro de checksum, os dados recebidos estão corrompidos.
  * - `DHT22_ERROR_TIMEOUT`: Falha na comunicação com o sensor, ele não respondeu.
  * - `DHT22_ERROR_INVALID_DATA`: Os valores lidos, embora corretamente recebidos, estão fora da faixa de operação do sensor.
  * - `DHT22_ERROR_NOT_INITIALIZED`: O driver ainda não foi inicializado com `dht22_init()`.
  * * @note Os valores de temperatura e umidade só devem ser considerados válidos se a função
  * retornar `DHT22_OK`. Em caso de qualquer erro, os conteúdos das variáveis apontadas por
  * `temperature` e `humidity` não são modificados e devem ser ignorados.
  * * @par Exemplo de uso:
  * @code
  * #include "dht22.h"
  * #include <stdio.h>
  * * #define DHT_PIN 15
  * * int main() {
  * stdio_init_all();
  * dht22_init(DHT_PIN);
  * * while(1) {
  * float temp, humid;
  * int result = dht22_read(&temp, &humid);
  * * if (result == DHT22_OK) {
  * printf("Temperatura: %.1f°C, Umidade: %.1f%%\n", temp, humid);
  * } else {
  * printf("Erro na leitura do sensor DHT22: %d\n", result);
  * }
  * // A própria função dht22_read já garante o intervalo de 2 segundos.
  * }
  * }
  * @endcode
  */
 int dht22_read(float *temperature, float *humidity);
 
 #endif // DHT22_H