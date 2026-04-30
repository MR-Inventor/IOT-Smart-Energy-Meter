# ⚡ Smart IoT Energy Meter

An ESP32-based Smart Energy Meter that measures real-time voltage, current, power, and electricity cost. The system uses embedded signal processing (RMS calculation) and displays data on an I2C LCD.

# 🚀 Features

- Real-time Voltage and Current measurement (RMS)
- Power calculation (W)
- Electricity cost estimation
- Sensor calibration for improved accuracy
- 16x2 LCD display using I2C
- Automatic display switching (V/I and Power/Cost)

# 🛠️ Hardware Used

- ESP32 Microcontroller  
- ACS712 Current Sensor (5A)  
- Voltage Sensor Module  
- 16x2 LCD (I2C)  
- Connecting wires  
- Power supply  

# 💻 Software & Tools

- Arduino IDE  
- Embedded C  
- ESP32 Board Package  

# ⚙️ Working Principle

- Reads analog signals from voltage and current sensors  
- Performs calibration to remove offset errors  
- Computes RMS values for stable AC measurements  
- Calculates power using voltage and current  
- Estimates electricity cost based on usage  
- Displays data on LCD and Serial Monitor  

# 📊 Output Example

Serial Monitor:
V: 220 V | I: 0.50 A | P: 110 W | Cost: ₹0.02

LCD:
V:220V  
I:0.50A  

P:110W  
Rs:0.02  

# 🔧 Setup Instructions

- Connect current sensor to GPIO 34  
- Connect voltage sensor to GPIO 36  
- Connect LCD via I2C (SDA: 21, SCL: 22)  
- Install LiquidCrystal_I2C library  
- Upload code using Arduino IDE  
- Open Serial Monitor (115200 baud)  
- Keep system at no-load during calibration  

# 📈 Future Improvements

- IoT dashboard integration (Blynk / Firebase)  
- Mobile app connectivity  
- Cloud data storage  
- Overload alert system  

# 📚 Learning Outcomes

- Embedded Systems Programming (ESP32)  
- Sensor calibration techniques  
- RMS calculations for AC signals  
- Real-time power monitoring  
- Hardware-software integration  

# 👨‍💻 Author

Digvijay Patankar  

# ⭐ Contribution

Feel free to fork, improve, and contribute to this project!
