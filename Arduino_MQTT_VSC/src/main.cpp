// G my_proj

/*
   TimeRTC.pde
   example code illustrating Time library with Real Time Clock.
   pwm -  5, 6 : 9, 10 : 11  13
   
	Timer0 - используется millis (), micros (), delay () и ШИМ на пинах 5 и 6
	Timer1 - используется для Servo, библиотеки WaveHC и ШИМ на пинах 9 и 10
	Timer2 - используется Tone и ШИМ на пинах 11 и 13

*/
#include <Arduino.h>
#include "NewPing.h"
//#include <Ultrasonic.h>  //Дальнометр SR04

#include <Wire.h>
#include <DS1307RTC.h>  // a basic DS1307 library that returns time as a time_t

#include "IRremote.h"
#include <DHT.h>
//#include <avr/pgmspace.h> // ДлЯ Progem запись во флэш

// #include "read_serial.h"

// Прототипы функций

// Устанавливаем значения в регистры  0 - topic № сдвиговика в массиве , 0- № регистра, 1-значение,2-сообщение (отправлять или нет) //Уст бит в сдвиговый регистр 0 - topic, 1 - num, 2 - val, 3 - send отправлять или нет 
//void set_reg_shift(byte topic, byte num, byte val, bool send);
void set_reg_shift(bool send);

// получить значения регистров  0 - № PCF (array_name), 1 - № регистра PCF
byte get_value_regs_PCF(byte i);

// Чтение и установка данных из Serial
void read_serial();

// чтение регистров  и запись в active_reg
void read_array_regs ();

// проверка и установка сдвиговых регистров из нажатых PCF
void set_push_PCF_to_shiftreg ();

// очищаем переменные Serial
void serial_clear();

// Отправляем сообщение в Serial
void send_to_serial( byte topic, byte num, int value);


#include "my_lib.h"


//#include <LiquidCrystal_I2C.h>
//LiquidCrystal_I2C lcd(0x21,16,2);  

//КОНСТАНТЫ
#define DHTPIN 6
#define DHTTYPE           DHT11
//#define btnSpeac      2
#define BTN      9
//#define micD      7
#define sr04Trig  10
#define sr04Echo  11
//Ultrasonic ultrasonic(sr04Trig, sr04Echo);

//74HC595 
#define DS  2
#define ST_CP  3
#define SH_CP  4


#define speacRelay      8
//#define micLamp      3

const int btns = A0;   //496, 333, ,249, 199, 165
const int sensTherm = A2; 
const int sensLight = A3;  
   
DHT dht(DHTPIN, DHTTYPE);



//*******************************************
//ПЕРЕМЕННЫЕ ДЛЯ РЕГИСТРОВ
#define ARRAY_LENGTH 3

	//переменные 74HC595

byte array_shift_reg[] =
{
	0b00000000,
	0b00000000
};
byte count_reg_shift = sizeof(array_shift_reg); //Кол-во строк массива, 3 столбца 

// 0 - сдвиговые регистры, 1 -  не в маске, 2 - зажатые, 3 - маска 
 byte arr_shift_reg[][4]  = {
//	PCF			 без маски	 >3s		 маска, связь со сдвиговым
	{0b00000000, 0b00000000, 0b00000000, 0b11000011},
	{0b00000000, 0b00000000, 0b00000000, 0b11000011}
  };

byte count_reg_PCF = sizeof(arr_shift_reg)/4; //Кол-во строк массива, 4 столбца 

//Для циклов
byte i, j;
boolean bit_PCF_reg;
byte byte_PCF_reg;
//byte registers_shift_1 = 0b00000000;
//byte registers_shift_2 = 0b00000000;





#include <PCF8574.h>  // Расширитель портов
PCF8574 expander_21(0x21); //1+
PCF8574 expander_20(0x20);  //3-

// Храним значеие байта из PCF 
byte value_reg_PCF = 0;
// Храним значеие маски для PCF 
byte mask_PCF = 0;

//значение задержки, если переполнено, то прошло больше 3 сек и кнопка считается зажатой
byte value_time_delay = 0;
// временное значение сдвигового регичтра
byte value_reg_shift = 0;


// Объявление переменных
// для read_serial
char serial_char = 0;

struct serial_struct
{
	String id = "";
	String topic = "";
	String num = "";
	String value = "";

	byte b_id = 0;
	byte b_topic = 0;
	byte b_num = 0;
	byte b_value = 0;
};

//Структура для хранения данных из сериала
serial_struct serial_vars;
byte serial_num = 0;
String arduino_id = "1";






void setup()  
{
	//Устанавливаем сначала все порты на вывод
	for (byte i = 1; i <= 14; i++)
	{
		pinMode(i, OUTPUT);  
		digitalWrite(i, LOW);
	}


	pinMode(sr04Trig, OUTPUT); 
	pinMode(sr04Echo, INPUT); 

	Serial.begin(115200);
	Serial.println("Start");

	//Расширитель портов
	expander_20.begin();
	expander_21.begin();


	//74HC595
	//устанавливаем режим OUTPUT 
	pinMode(ST_CP, OUTPUT);
	pinMode(SH_CP, OUTPUT);
	pinMode(DS, OUTPUT);
	digitalWrite(ST_CP, LOW);

	pinMode(BTN, INPUT);   //а 9й – входом кнопки

	pinMode(speacRelay, OUTPUT);  //8й вывод будет выходом
	digitalWrite(speacRelay, LOW);

	// время
	while (!Serial) ; // Только для платы Leonardo
	setSyncProvider(RTC.get);   // получаем время с RTC
	if (timeStatus() != timeSet)
		Serial.println("Unable to sync with the RTC"); //синхронизация не удаласть
	else
		Serial.println("RTC has set the system time");

	//dht.begin();


}

// TODO:  Основной цикл
void loop()
{
	//Для отладки PCF
	delay(2000);
	//for (int i = 0; i < 8; i++)
	//{
		/*Serial.print(i);
		Serial.print(get_value_regs_PCF(0, i));
		delay(10);
		Serial.println(get_value_regs_PCF(1, i));
		delay(10);*/
		Serial.println("--------");
		Serial.println(expander_21.read8(), BIN);
		Serial.println(expander_20.read8(), BIN);

		if (expander_21.read8() == 0b00000010)
		{
			array_shift_reg[0] = 255;
			array_shift_reg[1] = 255;
			//set_reg_shift(1);
		}
		else
		{
			array_shift_reg[0] = 0;
			array_shift_reg[1] = 0;
		}
		
			//Serial.println("1");

		
	//}
	//Serial.println("---------------");

	//Serial.println("1");
	// читаем данные с Serial порта и записываем в структуру
	///read_serial();
	//Serial.println("2");


	// проверяем все регистры и получаем данные. если что-то нажато
	///read_array_regs();
	//Serial.println("3");
	// Устанавливаем нажатые кнопки в сдвиговые регистры
	///set_push_PCF_to_shiftreg();
	//Serial.println("4");
	//  TODO: Установить значения в сдвиговые регистры
	set_reg_shift(1);
	//Serial.println("5");
	//serial_clear();
	//Serial.println("6");

	//Вывод данных 5000 - 20 минут
	/*if (counter == 5000)
	{	
		counter = 0;
		Serial.println("i1t50n0v"+String(float(Thermister(analogRead(sensTherm))),1)); 
		Serial.println("i1t51n0v"+String(float(analogRead(sensLight)),1));
		Serial.println("i1t52n0v"+digitalClockDisplay());
		//delay(5);
		val = !digitalRead(speacRelay);

		//Считываем влажность и температуру
		float h = dht.readHumidity();
		float t = dht.readTemperature();
		
		// Проверка удачно прошло ли считывание влажности и температуры
		if (isnan(h) || isnan(t)) 
		{
			Serial.println("/hall/dht11@off%");
			return;
		} 

		Serial.println("/hall/dht11t@"+String(t)+"%");
		Serial.println("/hall/dht11h@"+String(h)+"%"); 
		

		
	}
	counter++;*/
	//Serial.println("END");
	//Serial.println("df");
	//delay(200);	
}

//*********************************************************************************************
//***************************************ФУНКЦИИ***********************************************
//*********************************************************************************************


// TODO: чтение регистров  и запись в active_reg
void read_array_regs ()
{
	// биты, которые нажаты и подходят по маске
	byte act_mask = 0;
	// биты, которые нажаты и не подходят по маске
	byte not_act_mask = 0;
	// биты, которые нажаты и не подходят по маске
	byte num_set_reg_pcf = 0;
	// флаг загрузки в ренистры сдвига
	byte flag_set_shift = 0;



	

	//Serial.println("Start arr");
	for (i=0; i < count_reg_PCF; i++)
	{
		value_reg_PCF = get_value_regs_PCF(i);
		//получаем маску
		mask_PCF = arr_shift_reg[i][2];
		// Выделяем биты, которые подходят по маске
		act_mask = value_reg_PCF & mask_PCF;
		// Выделяем биты, которые не подходят по маске
		not_act_mask = value_reg_PCF xor act_mask; 
		num_set_reg_pcf = 255;

		//Вычисляем номер регистра, который установлен
		for ( j = 0; j < 8; j++)
		{
			if (bitRead(act_mask, j) == 1)
			{
				num_set_reg_pcf = j;
				break;
			}
		}

		while(value_reg_PCF > 0 )
		{
			value_reg_PCF = get_value_regs_PCF(i);
			//если отпустили кнопку, то устанавливаем регистры
			if (value_reg_PCF == 0)
			{
				
				//Установка в регистр, нажатия кнопки

				if(value_time_delay == 255) // Если была зажата
				{
					//bitSet(arr_shift_reg[i][1], num_set_reg_pcf);
					arr_shift_reg[i][2] = arr_shift_reg[i][2] xor act_mask;
				}
				else  // если не зажата
				{
					//bitSet(arr_shift_reg[i][0], num_set_reg_pcf);
					arr_shift_reg[i][0] = arr_shift_reg[i][0] xor act_mask;
					if (act_mask !=0)
					{
						flag_set_shift = 1;	
					}
					arr_shift_reg[i][1] = arr_shift_reg[i][1] xor not_act_mask;
				}

				
			}
			//Считаем время задержки
			delay(11);
			if (value_time_delay != 255)
				{
					value_time_delay++;
				}	

		}
	}

	// Устанавливаем регистры сдвига, если есть нажатия кнопок
	if (flag_set_shift == 1)
	{
		set_reg_shift(0);

	}
}

void set_push_PCF_to_shiftreg()
{
	//Serial.println("Start push");
	for (i = 0; i < count_reg_PCF; i++)
	{
		byte_PCF_reg = arr_shift_reg[i][0];

		// Если не нажата ни одна кнопка, то переходим к следующей микросхеме
		if (byte_PCF_reg != 0)
		{
			
			
			
			
			//////////////////////////////////////////
			for (j=0; j < 8; j++)
			{
				/*// Отладка
				Serial.println("i - " + String(i));
				Serial.println("j - " + String(j));
				*/

				bit_PCF_reg = bitRead(byte_PCF_reg, j);
				if (bit_PCF_reg != 0)
				{
					// Проверяем нажатие кнопки и если есть связь со сдвиговым регистром, то инвертируем синганл
					if (bit_PCF_reg == bitRead(arr_shift_reg[i][2], j))
					{
						value_reg_shift = bitRead(array_shift_reg[i], j);
						bitWrite(array_shift_reg[i], j, !value_reg_shift);
						send_to_serial(i,j,!value_reg_shift);						
					}
					else
					{
						/* code */
					}
					
				}
				
			}
		}
	}



	//устанавливаем отдельные регистры, которые не связаны с PCF
	for (i = count_reg_PCF; i < count_reg_shift; i++)
	{
		for (j=0; j < 8; j++)
		{	
			//bit_PCF_reg = bitRead(array_shift_reg[i], j);

		}
	}

}


void read_serial()
{
	//Serial.println("Start serial");
	while (Serial.available() > 0)
	{
		//delay(1);
		// забираем первый символ
		serial_char = Serial.read();

	 	switch (serial_char) 
		{
			case 'i':
				serial_num = 1;
				continue;

			case 't':
				// если id сообщения равен устройству, то записываем следующее значение
				if (serial_vars.id == arduino_id)
				{
					serial_num = 2;
				}	
				// иначе, мы обнуляем переменные и очищаем кэш сериала
				else
				{
					serial_clear();

					while (Serial.available() > 0)
					{
						Serial.read();	
					}
				}	
				continue;

			case 'n':
				serial_num = 3;
				continue;

			case 'v':
				serial_num = 4;
				continue;
		}

		// ессли получаем не число, то сбрасываем переменные и буфер
		if (!isdigit(serial_char))
		{
			serial_clear();

			while (Serial.available() > 0)
			{
				Serial.read();	
			}

		}

		switch (serial_num) 
		{
			case 1:
				serial_vars.id += serial_char;
				break;
			case 2:
				serial_vars.topic += serial_char;
				break;
			case 3:
				serial_vars.num += serial_char;
				break;
			case 4:
				serial_vars.value += serial_char;
				break;
		}
	}

	// После проверки буфера сообщений проверяем id и если он есть, то ковертируем в цифры
	if (serial_vars.id != "" and serial_num == 4)
	{
		serial_vars.b_id = serial_vars.id.toInt();
		serial_vars.b_topic = serial_vars.topic.toInt();
		serial_vars.b_num = serial_vars.num.toInt();
		serial_vars.b_value = serial_vars.value.toInt();

		Serial.println(serial_vars.topic + "-" + serial_vars.num + "-" + serial_vars.value);
		
		//если номер регистра меньше 50, значит это сдвиговый регистр и ставим его сразу
		if (serial_vars.b_topic < 50)
		{
			//active_reg[0] = serial_vars.b_topic;
			//active_reg[1] = serial_vars.b_num;

			bitWrite(array_shift_reg[serial_vars.b_topic], serial_vars.b_num, serial_vars.b_value);

			//set_reg_shift(serial_vars.b_topic, serial_vars.b_num, serial_vars.b_value, 0);
			//serial_clear();
		}
		// иначе смотрим по обстановке
		else
		{
			/* code */
		}
		
	}
}

// получить значения регистров  0 - № PCF (array_name), 1 - № регистра PCF
byte get_value_regs_PCF(byte num_array_name)
{
	switch (num_array_name) 
	{
		case 0:
		{
			Serial.print("A");
			//return expander_20.readButton8(num_reg);
			
			return expander_21.read8();//expander_20.readButton8(num_reg);
			break;
		}
		case 1:
		{
			Serial.print("B");
			return expander_20.read8();//expander_21.readButton(num_reg);
			break;
		}
		default:
			return 0;
	}
}

//Уст бит в сдвиговый регистр 0 - topic, 1 - num, 2 - val, 3 - send отправлять или нет 
void set_reg_shift( bool send)
{
	//Serial.println("Start set_reg_shift");
	//bitWrite(array_shift_reg[topic], num,val);

	//Сдвиг через сдвиговый регистр 74HC595
	digitalWrite(ST_CP, LOW);

	//запись данных в сдвиговые регистры
	for ( i = count_reg_shift - 1; i < 255; i--)
	{
		/*// Отладка
		Serial.println("count_reg_shift - " + String(count_reg_shift));
		Serial.println("i - " + String(i));
		delay(2000);
		*/
		shiftOut(DS, SH_CP, MSBFIRST, array_shift_reg[i]);

	
	}
	
    digitalWrite(ST_CP, HIGH);

	if (send)
		Serial.println("0 - " + String(array_shift_reg[0]) +"1 - " + String(array_shift_reg[1]) );	

}

void serial_clear()
{
	//Serial.println("Start clear");
	serial_vars.id = "";
	serial_vars.topic = "";
	serial_vars.num = "";
	serial_vars.value = "";

	serial_vars.b_id = 0;
	serial_vars.b_topic = 0;
	serial_vars.b_num = 0;
	serial_vars.b_value = 0;

	// Обнуляем регистры нажатых кнопок
	for ( i = 0; i < count_reg_PCF; i++)
	{
		arr_shift_reg[i][0] = 0;
		arr_shift_reg[i][1] = 0;
	}

	serial_num = 0;
}
// i1t0n2v1    i1t0n2v0

byte reverse_bit (byte topic, byte num)
{
	// инвертируем бит
	return !bitRead(array_shift_reg[topic], num);
}


// Отправить в Serial
void send_to_serial( byte topic, byte num, int value)
{
	Serial.println("i"+ String(arduino_id)  
	+ 't' + String(topic) 
	+ 'n' + String(num )
	+ 'v' + String(value));
}

void read_set_pcf ()
{
	expander_21.read8();
} 