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

const char nomes[TOTAL_MUSICAS][17] PROGMEM = {
    "Mario",
    "Sonic - Green",
    "Megalovania",
    "Zelda",
    "Tetris"
};

const byte quantidades[TOTAL_MUSICAS] PROGMEM = {
    47, 36, 44, 41, 38
};

// Trechos simplificados para piezo.
const Nota musicas[TOTAL_MUSICAS][MAX_NOTAS] PROGMEM = {

    // Mario
    {
        {659,140}, {659,140}, {0,140},   {659,140},
        {0,140},   {523,140}, {659,140}, {0,140},
        {784,140}, {0,140},   {0,140},   {0,140},
        {392,140}, {0,140},   {0,140},   {0,140},
        {523,140}, {0,140},   {0,140},   {392,140},
        {0,140},   {0,140},   {330,140}, {0,140},
        {0,140},   {440,140}, {0,140},   {494,140},
        {0,140},   {466,140}, {440,140}, {0,140},
        {392,140}, {659,140}, {784,140}, {880,140},
        {0,140},   {698,140}, {784,140}, {0,140},
        {659,140}, {0,140},   {523,140}, {587,140},
        {494,140}, {0,140},   {0,140}
    },

    // Sonic
    {
        {1047,150}, {880,150},  {1047,150}, {988,150},
        {880,150},  {784,150},  {880,150},  {988,150},
        {659,150},  {0,150},

        {1047,150}, {880,150},  {1047,150}, {988,150},
        {880,150},  {784,150},  {880,150},  {988,150},
        {659,150},  {0,150},

        {1175,150}, {1047,150}, {988,150},  {1047,150},
        {1175,150}, {1319,150}, {1047,150}, {880,150},
        {1047,150}, {988,150},  {880,150},  {784,150},
        {880,150},  {988,150},  {659,150},  {0,150}
    },

    // Undertale - Megalovania
    {
        {294,120}, {294,120}, {587,240}, {440,360},
        {0,120},   {415,240}, {392,240}, {349,240},
        {294,120}, {349,120}, {392,120},

        {262,120}, {262,120}, {587,240}, {440,360},
        {0,120},   {415,240}, {392,240}, {349,240},
        {294,120}, {349,120}, {392,120},

        {247,120}, {247,120}, {587,240}, {440,360},
        {0,120},   {415,240}, {392,240}, {349,240},
        {294,120}, {349,120}, {392,120},

        {233,120}, {233,120}, {587,240}, {440,360},
        {0,120},   {415,240}, {392,240}, {349,240},
        {294,120}, {349,120}, {392,120}
    },

    // Zelda
    {
        {466,600}, {349,450}, {466,150}, {466,150},
        {523,150}, {587,150}, {622,150}, {698,900},
        {0,150},   {698,150}, {698,150}, {740,150},
        {831,150}, {932,900}, {0,150},   {932,150},
        {932,150}, {831,150}, {740,150}, {831,150},
        {740,300}, {698,900}, {698,300}, {622,150},
        {622,150}, {698,150}, {740,900}, {698,150},
        {622,150}, {554,150}, {554,150}, {622,150},
        {698,900}, {622,150}, {554,150}, {523,150},
        {523,150}, {587,150}, {659,900}, {784,300},
        {698,1200}
    },

    // Tetris
    {
        {659,320}, {494,160}, {523,160}, {587,320},
        {523,160}, {494,160}, {440,320}, {440,160},
        {523,160}, {659,320}, {587,160}, {523,160},
        {494,480}, {523,160}, {587,320}, {659,320},
        {523,320}, {440,320}, {440,640}, {0,160},

        {587,480}, {698,160}, {880,320}, {784,160},
        {698,160}, {659,480}, {523,160}, {659,320},
        {587,160}, {523,160}, {494,320}, {494,160},
        {523,160}, {587,320}, {659,320}, {523,320},
        {440,320}, {440,640}
    }
};

void escreverLinha(byte linha, const char *texto)
{
    byte coluna = 0;

    lcd.setCursor(0, linha);

    while (coluna < 16 && texto[coluna] != '\0') {
        lcd.write(texto[coluna]);
        coluna++;
    }

    // Apaga caracteres restantes da mensagem anterior.
    while (coluna < 16) {
        lcd.write(' ');
        coluna++;
    }
}

void atualizarTela()
{
    char nome[17];

    for (byte i = 0; i < 16; i++) {
        nome[i] = (char)pgm_read_byte(
            &nomes[musicaAtual][i]
        );
    }

    nome[16] = '\0';

    escreverLinha(0, nome);

    if (estado == MENU) {
        escreverLinha(1, "Escolha e PLAY");
    } else if (estado == TOCANDO) {
        escreverLinha(1, "Tocando");
    } else {
        escreverLinha(1, "Pausado");
    }

    digitalWrite(
        LED_VERDE,
        estado == TOCANDO ? HIGH : LOW
    );

    digitalWrite(
        LED_VERMELHO,
        estado == PAUSADO ? HIGH : LOW
    );
}

bool botaoPressionado(byte botao, unsigned long agora)
{
    byte leitura = digitalRead(PINOS_BOTOES[botao]);

    if (leitura != ultimaLeitura[botao]) {
        ultimaLeitura[botao] = leitura;
        ultimaMudanca[botao] = agora;
    }

    if (agora - ultimaMudanca[botao] >= DEBOUNCE_MS) {

        if (leitura != leituraEstavel[botao]) {
            leituraEstavel[botao] = leitura;

            // INPUT_PULLUP: pressionado corresponde a LOW.
            if (leitura == LOW) {
                return true;
            }
        }
    }

    //implementa reproducao pausa e parada das musicas
    void definirSom(unsigned int frequencia)
{
    if (frequencia == 0) {
        noTone(PIEZO);
    } else {
        tone(PIEZO, frequencia);
    }
}

void carregarNota()
{
    unsigned int duracao;

    frequenciaAtual = pgm_read_word(
        &musicas[musicaAtual][notaAtual].frequencia
    );

    duracao = pgm_read_word(
        &musicas[musicaAtual][notaAtual].duracao
    );

    // Reserva uma pausa curta entre as notas.
    if (duracao > INTERVALO_MS) {
        duracaoEtapa = duracao - INTERVALO_MS;
    } else {
        duracaoEtapa = 1;
    }

    emIntervalo = false;

    definirSom(frequenciaAtual);
    inicioEtapa = millis();
}

void iniciarMusica()
{
    notaAtual = 0;
    estado = TOCANDO;

    atualizarTela();
    carregarNota();
}

void pararMusica()
{
    definirSom(0);

    estado = MENU;
    notaAtual = 0;
    frequenciaAtual = 0;
    emIntervalo = false;

    atualizarTela();
}

void alternarPausa()
{
    if (estado == TOCANDO) {
        inicioPausa = millis();

        definirSom(0);
        estado = PAUSADO;

        atualizarTela();

    } else if (estado == PAUSADO) {
        estado = TOCANDO;

        atualizarTela();

        // Desconta o tempo em que ficou pausado.
        inicioEtapa += millis() - inicioPausa;

        if (emIntervalo) {
            definirSom(0);
        } else {
            definirSom(frequenciaAtual);
        }
    }
}

void atualizarMusica()
{
    unsigned long agora = millis();

    if (agora - inicioEtapa < duracaoEtapa) {
        return;
    }

    if (!emIntervalo) {
        // A nota terminou: inicia o intervalo.
        definirSom(0);

        emIntervalo = true;
        duracaoEtapa = INTERVALO_MS;
        inicioEtapa = millis();

    } else {
        // O intervalo terminou: avanca uma nota.
        notaAtual++;

        byte total = pgm_read_byte(
            &quantidades[musicaAtual]
        );

        if (notaAtual >= total) {
            pararMusica();
        } else {
            carregarNota();
        }
    }
}

    return false;
}
