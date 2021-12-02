#include <EEPROM.h>

#include <OneWire.h>
#include <DallasTemperature.h>
#define ONE_WIRE_BUS 9
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
#include <SerialCommand.h>
SerialCommand sCmd;
#define oxi 7
#define btn_start 5
#define btn_stop 6
int i;
byte ky_tu[8] = {
  B01100,
  B10010,
  B01100,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
};
#include<Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 20, 4);
unsigned int cap_nhat,on_time, off_time, bit_temp, allow_motor , cap_nhat_esp, st_oxi , status_oxi, allow_oxi , time_count, time_on_oxi , time_off_oxi , st_device;
unsigned int time , on_stt , off_stt;
void hello()
{
  lcd.print("    XUONG TAO 3");
}
//.................................khai bao cac bien ngat cho cong tac vo cap
volatile unsigned int encoderPos = 0;  // Cho vị trí đầu bằng 0 a counter for the dial
unsigned int lastReportedPos = 1;   // Vị trí cuối bằng 1 i change management
static boolean rotating = false;    // Quản lý debounce (giống như là chống nhiễu) debounce management

// các biến cho trình phục vụ ngắt interrupt service routine vars
boolean A_set = false;
boolean B_set = false;
#define data 3
#define clock_pin 2
#define sw 4
uint8_t set_mode, clear_lcd;
unsigned int count;
uint32_t time_press;
uint8_t bien_dem;
uint8_t da_nhan;
//..................................................................khai bao bien module nhiet do
float temp;
unsigned long get_time , last_time , time_check_node;
//..................................................................
unsigned int value , st_btn_start, st_btn_stop, tin_hieu_esp  , node_err;
int  check_node;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  sensors.begin();
  get_time = millis();
  last_time = millis();
  time_check_node = millis();
  sCmd.addCommand("OXI", get_control_esp);
  sCmd.addCommand("RUN", LCD_PRINT_START);
  sCmd.addCommand("STOP", LCD_PRINT_STOP);
  sCmd.addCommand("RUN_TIME", LCD_PRINT_RUN_TIME);
  sCmd.addCommand("STOP_TIME", LCD_PRINT_STOP_TIME);
  sCmd.addCommand("ESP_RESPON", RESPON);
  pinMode(oxi, OUTPUT);
  pinMode(btn_start, INPUT_PULLUP);
  pinMode(btn_stop, INPUT_PULLUP);
  pinMode(sw, INPUT);
  off_time = EEPROM.read(0);
  delay(10);
  on_time = EEPROM.read(1);
  delay(10);
  st_device = EEPROM.read(2);

  //  st_oxi = EEPROM.read(2);

  //........................................khoi dong LCD
  lcd.init();
  lcd.init();
  lcd.backlight();
  hello();
  //............................................khoi dong cong tac vo cap
  pinMode(clock_pin, INPUT_PULLUP); // INPUT-PULLUP tương đương Mode INPUT và tự động nhận trạng thái HIGH hoặc LOW
  pinMode(data, INPUT_PULLUP);
  // Chân encoder trên ngắt 0 (chân 2): encoder pin on interrupt 0 (pin 2)
  attachInterrupt(0, doEncoderA, CHANGE);

  // Chân encoder trên ngắt 1 (chân 3) encoder pin on interrupt 1 (pin 3)
  attachInterrupt(1, doEncoderB, CHANGE);
}

void loop() {
  // put your main code here, to run repeatedly:
  sCmd.readSerial();
  cap_nhat_lcd();
  btn_star_stop();
  cong_tac_vo_cap();
  cam_bien_nhiet();
  oxi_control();
  check_st_esp();
  // cap_nhat_du_lieu_esp();
}
void check_st_esp()
{
  if(millis()-time_check_node > 60000)
  {
    node_err+=1;
    Serial.println("ARDUINO_CALL 1");
    time_check_node = millis();
  }
    if(node_err==3)
    {
      delay(10000);
      asm volatile ( "jmp 0");
    }
}
void RESPON()
{
  char *arg ;
  arg = sCmd.next();
  check_node  = atoi(arg); // Chuyển chuỗi thành số
   if(check_node  == 1)
  {
    node_err=0;
    check_node=0;
    }
  
}
void cap_nhat_du_lieu_esp()
{
  if (cap_nhat_esp == 0)
  {
    //delay(10000);
//    Serial.print("off_r ");
//    Serial.println(off_time);
//    delay(1);
//    Serial.print("on_r ");
//    Serial.println(on_time);
   // delay(1);
    cap_nhat_esp = 1;
  }
}
void get_control_esp()
{
  char *arg ;
  arg = sCmd.next();
  value = atoi(arg); // Chuyển chuỗi thành số
}
void oxi_control()
{
  if (value == 1 && tin_hieu_esp == 0)
  {
    allow_oxi = 1;
    Serial.print("ALLOW_OXI ");
    Serial.println(!allow_oxi);
    digitalWrite(oxi, 1);
    time_count = 0;
    tin_hieu_esp = 1;
  }

  if ((unsigned long) (millis() - last_time) > 1000 && value == 1)
  {

    time_count += 1;
    Serial.print("TIME_OXI ");
    Serial.println(time_count);
    if (time_count >= on_time * 60 && allow_oxi == 1 )
    {
      Serial.print("ALLOW_OXI ");
      Serial.println(allow_oxi);
      digitalWrite(oxi, 0);
      time_count = 0;
      allow_oxi = 0;

    }
    if (time_count >= off_time * 60 && allow_oxi == 0 )
    {
      Serial.print("ALLOW_OXI ");
      Serial.println(allow_oxi);
      digitalWrite(oxi, 1);
      time_count = 0;
      allow_oxi = 1;

    }

    last_time = millis();
  }
  if (value == 0 &&  tin_hieu_esp == 1)
  {
    digitalWrite(oxi, 0);
    allow_oxi = 0;
    Serial.print("ALLOW_OXI ");
    Serial.print(allow_oxi);
    last_time = millis();
    time_count = 0;
    tin_hieu_esp = 0;

  }



}
void btn_star_stop()
{
  if (st_device == 1 && cap_nhat == 0)
  {
    Serial.print("off_r ");
    Serial.println(off_time);
    delay(100);
    Serial.print("on_r ");
    Serial.println(on_time);
    delay(100);
    //st_device = 1;
    allow_oxi = 1;
    value = 1;
    digitalWrite(oxi, 1);
    Serial.println("START_STOP 1");
    delay(200);
    cap_nhat = 1;
  }
  if (digitalRead(btn_start) == 0 && st_btn_start == 0 && on_time != 0 | off_time != 0)
  {
    lcd.clear();
    delay(200);
    st_btn_start += 1;
  }
  if (digitalRead(btn_start) == 0 && st_btn_start == 0 && on_time == 0 | off_time == 0)
  {

    lcd.setCursor(1, 3);
    lcd.print("chua dat thoi gian");

  }
  if (digitalRead(btn_start == 1) && st_btn_start == 1 && on_time != 0 | off_time != 0)
  {
    EEPROM.write(2, 1);
    st_device = 1;
    allow_oxi = 1 ;
    value = 1;
    digitalWrite(oxi, 1);
    Serial.println("START_STOP 1");
    delay(200);
    st_btn_start = 0;
  }
  if (digitalRead(btn_stop) == 0 && st_btn_stop == 0 && on_time != 0 | off_time != 0)
  {
    delay(200);
    st_btn_stop += 1;
  }
  if (digitalRead(btn_stop == 1) && st_btn_stop == 1 && on_time != 0 | off_time != 0)
  {
    EEPROM.write(2, 0);
    cap_nhat=0;
    st_device = 0;
    value = 0;
    digitalWrite(oxi, 0);
    allow_oxi = 0;
    Serial.println("START_STOP 2");
    delay(200);
    st_btn_stop = 0;

  }
}
void cap_nhat_lcd()
{
  //.....................xoa man hinh lcd
  if (set_mode != clear_lcd)
  {
    lcd.clear();
    count = 0;
    encoderPos = 0;
    clear_lcd = set_mode;

  }
  if (set_mode == 0 )
  {
    // man hinh chinh
    lcd.setCursor(4, 0);
    lcd.print("XUONG TAO 3");
    lcd.print("  0/5");
    lcd.setCursor(0, 1);
    lcd.print("TRANG THAI OXI: ");
    if (value == 1)
    {
      lcd.print("ON   ");
    }
    else
    {
      lcd.print("OFF  ");
    }
    if (allow_oxi == 1 )
    {
      lcd.setCursor(0, 2);
      lcd.print("T/ON:");
      lcd.print(round(time_count) );
      if (time_count <= 10)
      {
        lcd.print("    ");
      }
      lcd.setCursor(10, 2);
      lcd.print("s");
    }
    else
    {
      lcd.setCursor(0, 2);
      lcd.print("T/OFF:");
      lcd.print(round(time_count));
      if (time_count <= 10)
      {
        lcd.print("    ");
      }
      lcd.setCursor(10, 2);
      lcd.print("s");
    }
    if (on_time != 0 | off_time != 0)
    {
      lcd.setCursor(0, 3);
      lcd.print(" ON:");
      lcd.print(on_time);
      lcd.print("   M");
      lcd.setCursor(9, 3);
      lcd.print("| OFF:");
      lcd.print(off_time);
      lcd.print("   M");
    }

  }
  if (set_mode == 1)
  {
    //man hinh setting 1

    lcd.setCursor(4, 0);
    lcd.print("XUONG TAO 3");
    lcd.print("  1/5");
    lcd.setCursor(0, 1);
    lcd.print("Time On : ");
    if (count < 10)
    {
      lcd.print(round(count));
      lcd.print(" ");
    }
    if ((count >= 10) && (count < 100))
    {
      lcd.print(round(count));
      lcd.print("  ");
    }
    if ((count >= 100) && (count <= 999))
    {
      lcd.print(round(count));
      lcd.print("   ");
    }
    lcd.print(" phut");
    on_time = count;

  }
  if (set_mode == 2)
  {
    //man hinh setting 1

    lcd.setCursor(4, 0);
    lcd.print("XUONG TAO 3");
    lcd.print("  2/5");
    lcd.setCursor(0, 1);
    lcd.print("Time Off : ");
    if (count < 10)
    {
      lcd.print(round(count));
      lcd.print(" ");
    }
    if ((count >= 10) && (count < 100))
    {
      lcd.print(round(count));
      lcd.print("  ");
    }
    if ((count >= 100) && (count <= 999))
    {
      lcd.print(round(count));
      lcd.print("   ");
    }
    lcd.print(" phut");
    off_time = count;

  }
  if (set_mode == 3)
  {
    lcd.setCursor(4, 0);
    lcd.print("XUONG TAO 3");
    lcd.print("  3/5");
    lcd.setCursor(0, 1);
    lcd.print("TEMPERATURE AUTO    MODE: ");
    bit_temp = count;
    if (bit_temp >= 2)
    {
      encoderPos = 0;
      count = 0;
      bit_temp = 0;
    }
    if (bit_temp == 0)
    {
      lcd.print("OFF");
    }
    if (bit_temp == 1)
    {
      lcd.print("ON ");
    }
  }
  if ((set_mode == 0) && (bit_temp == 1))
  {
    if (temp < 27)
    {
      allow_motor = 0;
    }
    if ((temp >= 27) && (temp < 35))
    {
      on_time = 15;
      off_time = 60;
    }
    if ((temp >= 35) && (temp < 45))
    {
      on_time = 20;
      off_time = 35;
    }
  }
  if (set_mode == 4)
  {
    lcd.setCursor(0, 0);
    lcd.print("DANG PHAT TRIEN");
  }

}
void cong_tac_vo_cap()
{

  if (digitalRead(sw) == 0 && da_nhan == 0)
  {
    delay(200);
    da_nhan += 1;
  }
  if (digitalRead(sw) == 1 && da_nhan == 1)
  {

    delay(200);
    if (set_mode == 2)
    {

      Serial.print("off_r ");
      Serial.println(off_time);
      delay(10);
      Serial.print("on_r ");
      Serial.println(on_time);
      delay(10);
      EEPROM.write(1, on_time);
      delay(10);
      EEPROM.write(0, off_time);
    }
    if (set_mode == 3)
    {

    }
    set_mode += 1;
    if (set_mode >= 5)
    {
      set_mode = 0;
    }
    da_nhan = 0;
  }
  rotating = true;  // Khởi động bộ debounce (có thể hiểu như bộ chống nhiễu): reset the debouncer
  if ((encoderPos >= 0) && (encoderPos <= 999))
  {
    if (lastReportedPos != encoderPos) {
      count = encoderPos;
      lastReportedPos = encoderPos;
    }
  }
}
void doEncoderA() {
  // debounce
  if ( rotating ) delay (1);  // Chờ 1 chút wait a little until the bouncing is done

  // Kiểm tra việc chuyển đổi trạng thái, xem có thật sự thay đổi trạng thái chưa: Test transition, did things really change?
  if ( digitalRead(clock_pin) != A_set ) { // debounce một lần nữa: debounce once more
    A_set = !A_set;

    // Cộng 1 nếu có tín hiệu A rồi có tín hiệu B: adjust counter + if A leads B
    if ( A_set && !B_set )
      encoderPos += 1;

    rotating = false;  // Không cần debounce nữa cho đến khi được nhấn lần nữa: no more debouncing until loop() hits again
  }
}

// Ngắt khi thay đổi trạng thái ở B, tương tự như ở A: Interrupt on B changing state, same as A above
void doEncoderB() {
  if ( rotating ) delay (1);
  if ( digitalRead(data) != B_set ) {
    B_set = !B_set;
    //  Trừ 1 nếu B rồi đến A: adjust counter - 1 if B leads A
    if ( B_set && !A_set )
      encoderPos -= 1;
    rotating = false;
  }
}
void LCD_PRINT_STOP()
{
  char *arg ;
  arg = sCmd.next();
  time = atoi(arg); // Chuyển chuỗi thành số
  if (set_mode == 0)
  {

  }
}
void LCD_PRINT_START()
{
  char *arg ;
  arg = sCmd.next();
  time = atoi(arg); // Chuyển chuỗi thành số
  if (set_mode == 0)
  {

  }
}
void LCD_PRINT_RUN_TIME()
{
  char *arg ;
  arg = sCmd.next();
  on_stt = atoi(arg); // Chuyển chuỗi thành số
  on_time = on_stt / 60;
  EEPROM.write(1, on_time);
  if (set_mode == 0)
  {
    lcd.clear();
    lcd.setCursor(0, 3);
    lcd.print(" ON:");
    lcd.print(on_stt);
    lcd.setCursor(8, 3);
    lcd.print("M");

  }
}

void LCD_PRINT_STOP_TIME()
{
  char *arg ;
  arg = sCmd.next();
  off_stt = atoi(arg); // Chuyển chuỗi thành số
  off_time = off_stt / 60;
  EEPROM.write(0, off_time);
  if (set_mode == 0)
  {
    lcd.clear();
    lcd.setCursor(9, 3);
    lcd.print("| OFF:");
    lcd.print(off_stt);
    lcd.setCursor(19, 3);
    lcd.print("M");
  }
}
void cam_bien_nhiet()
{
  if ((unsigned long) (millis() - get_time) > 2000 && set_mode == 0)
  {
    sensors.requestTemperatures();
    //sensors.getTempCByIndex(0); // vì 1 ic nên dùng 0
    //chờ 1 s rồi đọc để bạn kiệp thấy sự thay đổi
    Serial.print("TEMP ");
    Serial.println(String(sensors.getTempCByIndex(0)));

    lcd.setCursor(11, 2);
    lcd.print("| ");
    lcd.print(sensors.getTempCByIndex(0));
    lcd.createChar(0, ky_tu);
    lcd.setCursor( 18, 2);
    lcd.write(byte(0));
    lcd.print("C");
    get_time = millis();

  }

}
