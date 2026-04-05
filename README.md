# 🌊 IoT Water Level Monitoring (ESP8266 + HC-SR04)

Sistem monitoring ketinggian air berbasis IoT menggunakan **ESP8266**, **sensor ultrasonik HC-SR04**, dan **Blynk** untuk monitoring dan kontrol secara real-time.

---

## 🚀 Fitur Utama

* 📡 Monitoring level air real-time via IoT
* 🔄 Mode **Manual & Otomatis**
* 💧 Kontrol pompa air (relay)
* 📟 Tampilan LCD 16x2
* 🔔 Alarm buzzer saat air penuh
* 💡 Indikator LED (Low, Medium, High)

---

## 🧠 Cara Kerja Sistem

1. Sensor **HC-SR04** mengukur jarak permukaan air
2. Data dikonversi menjadi tinggi air
3. Sistem menentukan level:

   * LOW
   * MEDIUM
   * HIGH
   * FULL
4. Data dikirim ke aplikasi **Blynk**
5. Pompa dikontrol:

   * ON saat air rendah
   * OFF saat penuh

---

## 📐 Rumus yang Digunakan

Jarak dihitung dengan:

```
jarak (cm) = t / 58.3
```

Kalibrasi hasil:

```
y = 0.9889x
```

---

## 🛠️ Komponen

* ESP8266 (NodeMCU)
* Sensor HC-SR04
* LCD 16x2 + I2C
* Relay 1 Channel
* Buzzer
* LED indikator
* Breadboard & kabel jumper
* Aplikasi Blynk

---

## 📊 Hasil Pengujian

* Akurasi tinggi dengan error sekitar **3.6%**
* Korelasi sangat baik (R² ≈ 0.9997)
* Sistem berjalan stabil dan real-time

---

## 📁 Struktur Project

```
water-level-iot/
│
├── codewater_level.ino      # Program utama
├── HCSR04_regression_analysis  # Data analisis
├── Practical Report         # Laporan PDF
└── README.md
```

---

## 📷 Dokumentasi
AutoMode
![Image](https://github.com/user-attachments/assets/ae757d42-a4c3-403c-a593-138057178915)

Auto & Manual Mode x
![Image](https://github.com/user-attachments/assets/3c8fba9e-236a-457f-99d9-ec31a3aab903)

---

## 🎥 Demo Video

https://drive.google.com/file/d/1d3YwxX_hWlrr8VKdUcmZmYyVtV1oBn3o/view

---

## ⚙️ Cara Menjalankan

1. Upload program ke ESP8266
2. Masukkan WiFi & Auth Token Blynk
3. Hubungkan rangkaian sesuai skema
4. Jalankan sistem
5. Monitor dan kontrol melalui aplikasi Blynk

---

## 👤 Author

**Hema Tata Nugraha**
Politeknik Negeri Semarang
Teknik Elektronika
