#include <LiquidCrystal.h>
int ultimoValor = 0;        
int signalValue = 0; 
long ultimoTiempo = 0;
long periodo= 0;
float frecuencia = 0;
// Crear el objeto LCD con los pines correspondientes
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
//Pines de los punsadores
const int pulsador1 = 6; //Iniciar
const int pulsador2 = 7; //Detener
//Estado de los pulsadores
int estadoPulsador1 = LOW;
int estadoPulsador2 = LOW;
int n = 0;

// Control de adquisición de datos
bool adquiriendoDatos = false; // Estado de adquisición
bool ImprimirDatos=false;// Para imprimir datos
int tamArreglo = 2; // Tamaño inicial de arreglo, necesario para despues redimensionar
float* Valores = new float [tamArreglo]; // Puntero al arreglo dinámico
int cantidadElementos=0;//La cantidad de elementos ingresados en la variable

//Variables para almacenar informacion de la señal
float amplitud;
const char* formadeOnda;

void setup() {
  // Inicializar la pantalla LCD con 16x2
  lcd.begin(16, 2);
  lcd.clear();// Encender la retroiluminación
  lcd.println("Sistema listo.");// Mostrar un mensaje inicial
  
  // Inicializar el puerto serial para ver los datos en el monitor serial
  Serial.begin(9600);
  
  // Configurar los pulsadores como entradas
  pinMode(pulsador1, INPUT);
  pinMode(pulsador2, INPUT);
}

void loop() {
  //Generar los valores de la señal
  int valorSenal = analogRead(A0); // Lee la señal en el pin A0
  //Calcular la frecuencia   
  if (valorSenal > 512 && ultimoValor <= 512) {
      long tiempoActual = millis();  // Obtiene el tiempo actual
      periodo = tiempoActual - ultimoTiempo;       // Calcula el período
      ultimoTiempo = tiempoActual;                // Actualiza el último tiempo

      // Calcula la frecuencia (en Hz)
      if (periodo > 0) {//para no dividir entre 0
        frecuencia = 1000.0 / periodo;  // Convertir milisegundos a segundos
      }
    }

    ultimoValor = valorSenal;  // Actualiza el valor anterior
  float voltajeSenal = (valorSenal - 512) * (5.0 / 1023.0);  // Asumiendo que la referencia es de 5V
  //Iniciar adquisición
  if (estadoPulsador1 == HIGH && adquiriendoDatos==false){
    Serial.println("Iniciando la adquisicion de datos...");
    adquiriendoDatos = true; 
  }
  Serial.println(voltajeSenal); // Mostrar el valor en el monitor serial
  //Pulsadores
  estadoPulsador1 = digitalRead(pulsador1);
  estadoPulsador2 = digitalRead(pulsador2);
  if(adquiriendoDatos==true){
   	if(tamArreglo==cantidadElementos){
  		int nuevotamano=tamArreglo+1;//Definicion nuevo tamaño del puntero
      	float* nuevoarreglo_Valores = new float[nuevotamano];//Definicion de nuevo arreglo y asignación del nuevo tamaño
      	for(int i=0;i<tamArreglo;i++){//Ciclo para pasar los valores de un puntero a otro
      		nuevoarreglo_Valores[i]=Valores[i];
        }
      	delete[]Valores;//se quita la reserva de memoria del viejo arreglo 
  		Valores = nuevoarreglo_Valores; //Asignación nombre de variable antigua al nuevo puntero
    	tamArreglo=nuevotamano;//Asignación del nuevo tamaño
    }
  	Valores[cantidadElementos]=voltajeSenal;//
  	cantidadElementos++;
  }
  //Detener adquisición
  if(estadoPulsador2 == HIGH && adquiriendoDatos==true){
    Serial.println("Deteniendo la adquisicion...");
    adquiriendoDatos = false;
  }
  //Mostrar datos obtenidos
  if(estadoPulsador2 == HIGH){//Cuando se presiona el segundo pulsador se muestra instantaneamente los datos recolectados
    Serial.println("Estos son los valores que se recolectaron ");
    for(int i=0;i<tamArreglo;i++){
    	Serial.println(Valores[i]);
    }
    Serial.println("Fin de muestreo de datos ");
    n = 1;
  }
  if (n==1){ //Para calcular la amplitud
    Serial.print("Frecuencia: ");
    Serial.print(frecuencia);
    Serial.println(" Hz");
    
    amplitud = calcularAmplitud(Valores, cantidadElementos);
    Serial.print("La amplitud de la señal es: ");
    Serial.println(amplitud);
    
    formadeOnda = identificarOnda(Valores, cantidadElementos);
    Serial.print("La forma de onda de la señal es: ");
    Serial.println(formadeOnda);
    // Mostrar en el LCD
    mostrarDatos(frecuencia, amplitud, formadeOnda);
    
    n = 0;
  }
}

float calcularAmplitud(float *arr, int nelementos){
  float valorMax = arr[0];
  float valorMin = arr[0];
  //Encontrar el valor maximo y minimo en el arreglo
  for (int i=1; i<nelementos; i++){
    if(arr[i] > valorMax){
      valorMax = arr[i];
    }
    if(arr[i] <valorMin){
      valorMin = arr[i];
    }
  }
  //Calcular amplitud
  return (valorMax-valorMin) / 2;
}
//Identificar Forma de Onda
const char* identificarOnda(float* valores, int nelementos){
  int cambiosBruscos = 0;
  int transicionesSuaves = 0;
  for (int i = 0; i<nelementos; i++){
    float pendiente = valores[i]-valores[i-1];
    if (abs(pendiente)>0.8){//Cambios grandes, ondas cuadradas
      cambiosBruscos++;
    }
    else if (abs(pendiente)>0.05){//CAmbios suaves, ondas senoidales
      transicionesSuaves++;
    }
  }
  if (cambiosBruscos>transicionesSuaves){
    return "Cuadrada";
  } else{
    bool esTriangular = true;
    for (int i =2; i<nelementos; i++){
      float pendiente1 = valores[i-1]-valores[i-2];
      float pendiente2 = valores[i]-valores[i-1];
      if (abs(pendiente1-pendiente2)>0.01){
        esTriangular = false;
        break;
      }
    }
    if (esTriangular){
      return "Triangular";
    }else{
      if(transicionesSuaves>cambiosBruscos/2){
        return "Senoidal";
      }
      else{
        return "Desconocida";
      }
    }
  }
}
void mostrarDatos(float frecuencia, float amplitud, const char* formadeOnda){
  unsigned long tiempoAct = millis();
  static unsigned long tiempoAnt = 0;
  static int pantallaAct = 0;
   
  if (tiempoAct - tiempoAnt >= 30000){ //Si han pasado 30 seg
    tiempoAnt = tiempoAct;
    pantallaAct = (pantallaAct+1)%3;
    
    lcd.clear();
    
    if (pantallaAct == 0){
      lcd.setCursor(0,0);
      lcd.print("Frecuencia:");
      lcd.setCursor(0,1);
      lcd.print(frecuencia);
      lcd.print("Hz");
    }
    else if(pantallaAct == 1){
      lcd.setCursor(0,0);
      lcd.print("Amplitud:");
      lcd.setCursor(0,1);
      lcd.print(amplitud);
      lcd.print("V");
    }
    else{
      lcd.setCursor(0,0);
      lcd.print("Forma de Onda:");
      lcd.setCursor(0,1);
      lcd.print(formadeOnda);
    }
  }
}
      
      