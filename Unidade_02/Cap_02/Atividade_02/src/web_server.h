/**
 * @file    web_server.h
 * @brief   Interface pública para o módulo de servidor web.
 * @details Declara as funções para iniciar, parar e pollar o servidor HTTP.
 */

#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <stdbool.h>

/**
 * @brief Inicia o servidor HTTP e começa a escutar por conexões.
 * @param port O número da porta TCP na qual o servidor irá operar.
 * @return `true` se o servidor foi iniciado com sucesso, `false` caso contrário.
 */
bool  web_server_start(int port);

/**
 * @brief Processa eventos de rede pendentes.
 * @details Esta função deve ser chamada periodicamente no loop principal da aplicação
 * para permitir que a pilha de rede processe pacotes recebidos e enviados.
 * Neste caso, com callbacks, ela pode estar vazia, mas é uma boa prática mantê-la.
 */
void  web_server_poll(void);

/**
 * @brief Para o servidor HTTP e libera os recursos associados.
 */
void  web_server_stop(void);

#endif // WEB_SERVER_H