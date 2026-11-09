// Definicao da frequencia do clock(mesma usada na simulaçao no tinkercad).
#ifndef F_CPU
#define F_CPU 16000000UL
#endif
//bibliotecas. 
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <stdint.h>

// Definicao do estado da reproducao.
enum Estado {
    MENU = 0,
    TOCANDO = 1,
    PAUSADO = 2
};

// Definicao dos botoes.
enum Botao {
    UP = 0,
    DOWN = 1,
    PLAY = 2,
    STOP = 3
};

enum {
    TOTAL_MUSICAS = 5,
    MAX_EVENTOS = 47
};

// Controle do tempo em milissegundos.
volatile uint32_t relogio_ms = 0;

// Controle do estado da reproducao.
enum Estado estado = MENU;

uint8_t musica = 0;
uint8_t indice_nota = 0;

uint16_t frequencia_atual = 0;
uint16_t restante_ms = 0;

uint8_t intervalo = 0;

uint32_t inicio_etapa = 0;
uint32_t inicio_pausa = 0;

// Controle dos botoes.
uint8_t leitura_anterior[4] = {1, 1, 1, 1};
uint8_t botao_estavel[4] = {1, 1, 1, 1};
uint32_t mudou_em[4] = {0, 0, 0, 0};