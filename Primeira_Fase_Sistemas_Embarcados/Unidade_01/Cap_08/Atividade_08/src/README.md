A razão pela qual existem dois blocos add_executable() Atividade_08 _background e Atividade_08 _poll  é para oferecer duas maneiras diferentes de compilar o seu código principal, resultando em dois executáveis com comportamentos distintos no que se refere à gestão da rede Wi-Fi.
Eles usam o mesmo arquivo fonte principal, Atividade_08.c, mas são linkados com bibliotecas de arquitetura do CYW43 (o chip Wi-Fi) diferentes:

1.	Um executável para o modo "background" ou "threadsafe":
-	Este geralmente tem _background no nome do alvo (target).
-	Ele é linkado com uma biblioteca como pico_cyw43_arch_lwip_threadsafe_background.
-	Neste modo, as tarefas de rede Wi-Fi e LwIP (a pilha de rede) são, em grande parte, executadas em uma thread separada ou gerenciadas por interrupções, ocorrendo em "segundo plano".
-	No código C, o macro PICO_CYW43_ARCH_POLL não estará definido (ou será 0). Isso significa que, no seu loop while, a parte do código dentro de #if PICO_CYW43_ARCH_POLL (como cyw43_arch_poll(); e cyw43_arch_wait_for_work_until(...)) não será compilada. Em vez disso, se houver um bloco #else, o código dentro dele (como sleep_ms(10);) será usado.

2.	Um executável para o modo "poll":
- 	Este geralmente tem _poll no nome do alvo (target).
- 	Ele é linkado com uma biblioteca como pico_cyw43_arch_lwip_poll.
- 	Neste modo, você é responsável por chamar explicitamente funções para processar os eventos de rede Wi-Fi e LwIP de dentro do seu loop principal.
-	No código C, o macro PICO_CYW43_ARCH_POLL estará definido como 1. Isso significa que o código dentro de #if PICO_CYW43_ARCH_POLL (como cyw43_arch_poll(); e cyw43_arch_wait_for_work_until(...)) será compilado e executado.

 
Por que ter os dois?

- Flexibilidade: Oferece ao desenvolvedor a escolha do modelo de concorrência para a rede que melhor se adapta à sua aplicação. \
    •	O modo "background" pode parecer mais simples inicialmente, pois você não precisa se preocupar em chamar cyw43_arch_poll() no seu loop. No entanto, ele introduz a complexidade de threads (multitarefa preemptiva) e a necessidade de garantir que o acesso a dados compartilhados entre o seu loop principal e a thread de rede seja seguro (thread-safe). \
    •	O modo "poll" dá a você controle explícito sobre quando o processamento de rede ocorre. Ele é mais simples de raciocinar em um contexto de loop único e pode ser mais eficiente se bem gerenciado, especialmente para aplicações onde você já tem um loop principal ativo fazendo outras coisas (como no simulador de alarme, que verifica o estado do alarme, atualiza LEDs, etc.). O código que desenvolvemos para o seu simulador de alarme foi estruturado pensando no modo "poll" para integrar a lógica do alarme e as chamadas de rede de forma não bloqueante.

Eles se referem aos mesmos arquivos fonte .c principais, mas não resultam no mesmo executável final. Devido à linkagem com bibliotecas diferentes e à definição (ou não) do macro PICO_CYW43_ARCH_POLL, o pré-processador C incluirá ou excluirá diferentes seções do seu código no Atividade_08.c. Portanto, você obtém dois binários .uf2 distintos, cada um otimizado para um modo de operação da rede.

Se você decidiu que o modo "poll" é o mais adequado para o projeto (o que eu recomendaria para a estrutura atual do seu Atividade_08.c), você pode simplificar seu CMakeLists.txt para gerar apenas esse executável.

No entanto, manter os dois targets como os exemplos da SDK fazem não é prejudicial; apenas lhe dá a opção de experimentar ambos os modos ou mudar no futuro se necessário. Se você estiver usando uma IDE como o VS Code, você pode selecionar qual dos targets deseja compilar e depurar. Para o seu projeto, você deve se concentrar em compilar e carregar o target _poll.
