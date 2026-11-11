#include <LiquidCrystal.h>
#include <avr/pgmspace.h>

// Ordem: RS, E, D4, D5, D6, D7
LiquidCrystal lcd(2, 3, 4, 5, 6, 7);

const byte LED_VERDE = 8;
const byte LED_VERMELHO = 9;
const byte PIEZO = 11;

enum Botao {
    ANTERIOR,
    PROXIMA,
    PLAY,
    STOP,
    TOTAL_BOTOES
};

const byte PINOS_BOTOES[TOTAL_BOTOES] = {
    A0, A1, A2, A3
};

enum Estado {
    MENU,
    TOCANDO,
    PAUSADO
};

const byte TOTAL_MUSICAS = 5;
const byte MAX_NOTAS = 47;

const unsigned long DEBOUNCE_MS = 30;
const unsigned int INTERVALO_MS = 25;

struct Nota {
    unsigned int frequencia;
    unsigned int duracao;
};

Estado estado = MENU;

byte musicaAtual = 0;
byte notaAtual = 0;

bool emIntervalo = false;

unsigned int frequenciaAtual = 0;
unsigned int duracaoEtapa = 0;

unsigned long inicioEtapa = 0;
unsigned long inicioPausa = 0;

byte ultimaLeitura[TOTAL_BOTOES] = {
    HIGH, HIGH, HIGH, HIGH
};

byte leituraEstavel[TOTAL_BOTOES] = {
    HIGH, HIGH, HIGH, HIGH
};

unsigned long ultimaMudanca[TOTAL_BOTOES] = {
    0, 0, 0, 0
};