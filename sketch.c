#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Endereço do LCD e dimensões
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Pinos do Sensor Ultrassônico Principal (Nível)
const int trigNivel = 2; // Renomeado para clareza
const int echoNivel = 3; 

// Pinos do Sensor Ultrassônico de Interação (Próximo)
const int trigInteracao = 7;
const int echoInteracao = 6; 

// Pinos do LED RGB
const byte RED = A1;
const byte GREEN = A2;
const byte BLUE = A3;

// Variáveis de medição
int durationNivel = 0;
int distanceNivel = 0;
int durationInteracao = 0;
int distanceInteracao = 0;
int i;

// Variável de estado para controlar a mensagem no LCD (resolve o "piscar")
String estadoAtual = ""; 

// Variável para controle de tempo para estabilizar a exibição (resposta devagar)
unsigned long tempoAnterior = 0;
const long intervaloMinimo = 1000; // 1 segundo (ajuste para a resposta mais lenta desejada)


void setup()
{
  // Sensores Ultrassônicos
  pinMode(trigNivel , OUTPUT);
  pinMode(echoNivel , INPUT);
  pinMode(trigInteracao , OUTPUT);
  pinMode(echoInteracao , INPUT);

  // LEDs (Pinos 8 a 13)
  for (i = 8; i <= 13; i++) {
    pinMode(i, OUTPUT);
    // digitalWrite(i, LOW); // Garante que todos os LEDs comecem desligados
  }

  // Display LCD
  lcd.begin();
  lcd.backlight();
  Serial.begin(9600);

  // LED RGB
  pinMode(RED, OUTPUT);
  pinMode(GREEN, OUTPUT);
  pinMode(BLUE, OUTPUT);

  delay(1000); // Aguarda a inicialização
  
  // Mensagem inicial
  lcd.setCursor(0, 0);
  lcd.print(" Inic. Coletor ");
  lcd.setCursor(0, 1);
  lcd.print(" TAMPINHA AMIGA ");
  delay(2000); 
  lcd.clear();
}


void loop()
{
  // 1. MEDIÇÃO DO NÍVEL (distanceNivel)
  // Medição sequencial para evitar interferência entre os sensores
  digitalWrite(trigNivel , LOW);
  delayMicroseconds(2);
  digitalWrite(trigNivel , HIGH);
  delayMicroseconds(10);
  digitalWrite(trigNivel , LOW);
  durationNivel = pulseIn(echoNivel , HIGH, 20000); // Timeout adicionado para segurança
  distanceNivel = durationNivel * 0.017;
  
  // 2. MEDIÇÃO DA INTERAÇÃO (distanceInteracao)
  digitalWrite(trigInteracao , LOW);
  delayMicroseconds(2);
  digitalWrite(trigInteracao , HIGH);
  delayMicroseconds(10);
  digitalWrite(trigInteracao , LOW);
  durationInteracao = pulseIn(echoInteracao , HIGH, 20000);
  distanceInteracao = durationInteracao * 0.017;
  
  // Debug no Serial
  Serial.print("Distancia Nivel: ");
  Serial.print(distanceNivel);
  Serial.print(" cm | Interacao: ");
  Serial.print(distanceInteracao);
  Serial.println(" cm");


  // 3. LÓGICA DE INTERAÇÃO (PRIORIDADE ALTA)
  if (distanceInteracao > 0 && distanceInteracao < 9) {
    if (estadoAtual != "INTERACAO") {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(" OBRIGADAAAAAAAAAAA ");
      lcd.setCursor(0, 1);
      lcd.print(" COLETOR FELIZ ");
      estadoAtual = "INTERACAO";

      analogWrite(RED, 256);
      analogWrite(GREEN, 0);
      analogWrite(BLUE, 0);

      delay(5000); // Mantém a mensagem e a luz por 5 segundos

      // Desliga o LED RGB após a interação
      analogWrite(RED, 0); 
      analogWrite(GREEN, 0); 
      analogWrite(BLUE, 0);
      
      // Força a atualização do nível após a interação
      tempoAnterior = 0; 
    }
  }


  // 4. LÓGICA DE NÍVEL (Responde mais devagar/estável)
  unsigned long tempoAtual = millis();
  
  // Só executa a lógica de nível se o tempo mínimo tiver passado OU se estiver no estado INTERACAO (para sair dele)
  if (tempoAtual - tempoAnterior >= intervaloMinimo || estadoAtual == "INTERACAO") {
    tempoAnterior = tempoAtual; // Atualiza o tempo

    // Desliga todos os LEDs de nível para redefinir
    for (i = 9; i <= 13; i++) {
        digitalWrite(i, LOW);
    }

    String novoEstado = "";

    // Usando if/else if para garantir que apenas UMA condição seja ativada
    if (distanceNivel < 2) { // 100% (mais cheio)
      novoEstado = "100%";
      digitalWrite(9, HIGH);
      digitalWrite(10, HIGH);
      digitalWrite(11, HIGH);
      digitalWrite(12, HIGH);
      digitalWrite(13, HIGH);
      
    } else if (distanceNivel < 5) { // 80%
      novoEstado = "80%";
      digitalWrite(9, LOW);
      digitalWrite(10, HIGH);
      digitalWrite(11, HIGH);
      digitalWrite(12, HIGH);
      digitalWrite(13, HIGH);
      
    } else if (distanceNivel < 8) { // 60%
      novoEstado = "60%";
      digitalWrite(9, LOW);
      digitalWrite(10, LOW);
      digitalWrite(11, HIGH);
      digitalWrite(12, HIGH);
      digitalWrite(13, HIGH);
      
    } else if (distanceNivel < 11) { // 40%
      novoEstado = "40%";
      digitalWrite(9, LOW);
      digitalWrite(10, LOW);
      digitalWrite(11, LOW);
      digitalWrite(12, HIGH);
      digitalWrite(13, HIGH);
      
    } else if (distanceNivel <= 12) { // 20%
      novoEstado = "20%";
      digitalWrite(9, LOW);
      digitalWrite(10, LOW);
      digitalWrite(11, LOW);
      digitalWrite(12, LOW);
      digitalWrite(13, HIGH);

    } 
    
    if(distanceNivel > 12) { // Vazio (distância >= 14)
      digitalWrite(9, LOW);
      digitalWrite(10, LOW);
      digitalWrite(11, LOW);
      digitalWrite(12, LOW);
      digitalWrite(13, LOW);
      novoEstado = "CONTRIBUA";
    }

    // VERIFICAÇÃO PRINCIPAL: Atualiza o LCD SOMENTE SE o estado de nível MUDOU
    if (novoEstado != estadoAtual) {
      lcd.clear();
      lcd.setCursor(0, 0);

      if (novoEstado == "CONTRIBUA") {
        lcd.print(" CONTRIBUA COM ");
        lcd.setCursor(0, 1);
        lcd.print(" SUA TAMPINHA ");
      } else {
        // Mensagem padrão de preenchimento
        lcd.print(novoEstado + " PREENCHIDO ");
        lcd.setCursor(0, 1);
        lcd.print(" DE TAMPINHAS ");
      }

      estadoAtual = novoEstado; // Salva o novo estado
    }
  }
}
