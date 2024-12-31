n1#include <Servo.h> // Thư viện servo
#include <LiquidCrystal_I2C.h> // Thư viện LCD I2C

// Khai báo LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);
// Khai báo servo
Servo myservo;

// Định nghĩa các chân cảm biến và nút
#define ir_enter 2
#define ir_back 4
#define ir_car1 5
#define ir_car2 6
#define ir_car3 7
#define ir_car4 8
#define pay_button 9

// Biến toàn cục
unsigned long enter_time[4] = {0, 0, 0, 0};   // Thời gian xe vào slot
int prev_state[4] = {-1, -1, -1, -1};        // Trạng thái trước đó của cảm biến slot
int parking_fee[4] = {0, 0, 0, 0};          // Phí đỗ xe của từng slot
int slot = 4;                               // Tổng số slot trống

void setup() {
    Serial.begin(9600);

    // Cấu hình các chân cảm biến và nút
    pinMode(ir_car1, INPUT);
    pinMode(ir_car2, INPUT);
    pinMode(ir_car3, INPUT);
    pinMode(ir_car4, INPUT);
    pinMode(ir_enter, INPUT);
    pinMode(ir_back, INPUT);
    pinMode(pay_button, INPUT);

    // Khởi tạo servo
    myservo.attach(3);
    myservo.write(90); // Đóng servo

    // Khởi tạo LCD
    lcd.init();
    lcd.backlight();
    lcd.setCursor(0, 0);
    lcd.print("  Car Parking  ");
    lcd.setCursor(0, 1);
    lcd.print("     System    ");
    delay(2000);
    lcd.clear();
}

void loop() {
    // Cập nhật trạng thái cảm biến
    UpdateSlotStatus();

    // Hiển thị trạng thái slot
    DisplaySlotStatus();

    // Điều khiển xe đi vào
    HandleEnteringCar();

    delay(1);
}

// Hàm cập nhật trạng thái cảm biến slot
void UpdateSlotStatus() {
    for (int i = 0; i < 4; i++) {
        int sensor_pin = ir_car1 + i;
        int current_state = digitalRead(sensor_pin);

        // Nếu phát hiện xe vào slot
        if (prev_state[i] == 1 && current_state == 0) {
            enter_time[i] = millis(); // Ghi lại thời gian xe vào
            slot--;                   // Giảm số slot trống
        }

        // Nếu phát hiện xe rời khỏi slot
        if (prev_state[i] == 0 && current_state == 1) {
            DisplayFee(i);        // Hiển thị phí đỗ xe
            HandlePayment(i);    // Đợi thanh toán và mở cổng
        }

        prev_state[i] = current_state; // Cập nhật trạng thái cảm biến
    }
}

// Hiển thị trạng thái slot lên LCD
void DisplaySlotStatus() {
    lcd.setCursor(0, 0);
    lcd.print(prev_state[0] == 0 ? "S1:Fill " : "S1:Empty");
    lcd.setCursor(8, 0);
    lcd.print(prev_state[1] == 0 ? "S2:Fill " : "S2:Empty");

    lcd.setCursor(0, 1);
    lcd.print(prev_state[2] == 0 ? "S3:Fill " : "S3:Empty");
    lcd.setCursor(8, 1);
    lcd.print(prev_state[3] == 0 ? "S4:Fill " : "S4:Empty");
}

// Xử lý xe vào bãi
void HandleEnteringCar() {
    if (digitalRead(ir_enter) == 0 && slot > 0) {
        myservo.write(180);  // Mở cổng
        delay(2000);         // Đợi xe vào
        myservo.write(90);   // Đóng cổng
    } else if (digitalRead(ir_enter) == 0 && slot == 0) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("     Sorry     ");
        lcd.setCursor(0, 1);
        lcd.print("  Parking Full  ");
        delay(2000);
        lcd.clear();
    }
}

// Hiển thị phí đỗ xe
void DisplayFee(int index) {
    unsigned long duration = millis() - enter_time[index];
    if (duration < 10000) parking_fee[index] = 5000;       // Dưới 10s: 5k
    else if (duration < 60000) parking_fee[index] = 10000; // 10s - 1 phút: 10k
    else parking_fee[index] = 20000;                      // Trên 1 phút: 20k

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Slot "); lcd.print(index + 1); lcd.print(" Fee:");
    lcd.setCursor(0, 1);
    lcd.print(parking_fee[index]); lcd.print(" VND");
    delay(2000);
}

// Xử lý thanh toán
void HandlePayment(int index) {
      // Chờ người dùng nhấn nút thanh toán
      while (digitalRead(pay_button) == LOW) {
          delay(10); // Chờ người dùng nhấn nút thanh toán
      }

      // Hiển thị thông báo thanh toán thành công
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Payment Success");
      lcd.setCursor(0, 1);
      lcd.print("   Thank You!   ");
      delay(2000); // Hiển thị thông báo thanh toán thành công

      // Mở servo cho xe đi ra khi thanh toán
      myservo.write(180); // Mở servo
      delay(2000);        // Đợi xe đi qua
      myservo.write(90);  // Đóng servo

      // Reset slot và tăng slot trống
      enter_time[index] = 0;
      parking_fee[index] = 0;
      slot++;
      Serial.print(slot);
}
