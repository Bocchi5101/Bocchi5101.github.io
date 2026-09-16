# React IoT Dashboard

หน้า FrontEnd ภาษา JavaScript (React + Vite + MQTT.js) สำหรับ ESP32 หรือ ESP8266 ที่ใช้ topic/payload ตามแลบ

## เปิดใช้งาน

บนเครื่องนี้มี dependencies และชุดไฟล์เว็บที่สร้างแล้ว: ดับเบิลคลิก `Start-Dashboard.cmd` เพื่อเปิดหน้าเว็บ และคงหน้าต่างโปรแกรมไว้ระหว่างใช้งาน (เปิดพอร์ต 4174 หรือพอร์ตถัดไปที่ว่าง)

การตรวจในสภาพแวดล้อมผู้ช่วย: protocol tests ผ่าน 3 ข้อ และตรวจหน้าเว็บ/URL validation ใน browser แล้ว Vite build ติดข้อจำกัด spawn EPERM ของสภาพแวดล้อมนี้ จึงสร้าง dist ด้วย esbuild CLI โดยตรงแทน ยังไม่ได้ทดสอบ end-to-end กับ HiveMQ credentials และบอร์ดจริง

ติดตั้ง Node.js รุ่นที่ Vite รองรับ (20.19+ หรือ 22.12+) เปิด terminal ที่โฟลเดอร์นี้:

```powershell
cd F:\Shiro\workrmutp\IOT\Web\frontend
npm install
npm run dev
```

เปิด URL ที่ terminal แสดง (ปกติ http://localhost:5173) ไม่เปิด index.html ด้วยการดับเบิลคลิก เพราะ JSX ต้องผ่าน Vite

ใช้ pnpm แทน npm ได้: `pnpm install` และ `pnpm dev` มี pnpm-lock.yaml ล็อกเวอร์ชันที่ทดสอบ

## เชื่อมต่อบอร์ดจริง

1. ใส่ WebSocket URL จาก HiveMQ เช่น `wss://YOUR-CLUSTER.s1.eu.hivemq.cloud:8884/mqtt`
2. ใส่ MQTT username/password ที่มีสิทธิ์ publish/subscribe ไม่ใช่รหัส login เว็บไซต์
3. Topic prefix เริ่มต้นคือ `lab` ต้องตรงกับโค้ดบอร์ด
4. กดเชื่อมต่อ แล้วเปิดบอร์ดที่ต่อ Wi-Fi และ HiveMQ เดียวกัน

| Topic | Payload | ทิศทาง |
|---|---|---|
| lab/sensor/temp | 28.5 | ESP → เว็บ |
| lab/sensor/humidity | 65.0 | ESP → เว็บ |
| lab/led/status | ON / OFF | ESP → เว็บ |
| lab/led/control | ON / OFF | เว็บ → ESP |

เว็บแสดงค่าที่ได้รับจริง ไม่สร้างค่าจำลอง การกดปุ่มไม่เปลี่ยนสถานะ LED ทันที แต่รอข้อความ status จาก ESP และแจ้งหากยังไม่มีสถานะตรงกับคำสั่งภายใน 8 วินาที ปุ่มจริงที่บอร์ดต้อง publish status ด้วย เว็บจึงเปลี่ยนตามได้

retained status คือค่าที่ broker เก็บไว้ ไม่ยืนยันว่าบอร์ดออนไลน์ ข้อความสถานะที่ตรงกับคำสั่งเป็นเพียงรายงานสถานะ ไม่ใช่ acknowledgement ที่มี request ID; ถ้ามีหลายผู้ควบคุมพร้อมกันก็อาจได้ข้อความตรงกันจากอีกผู้ควบคุมได้ ตัวบ่งชี้ข้อมูลล่าสุดอิง sensor ที่รับภายใน 15 วินาที การเชื่อมต่อ broker ไม่เท่ากับบอร์ดออนไลน์

## จุดอ่านโค้ด

- `src/App.jsx`: useState แสดงค่า, useRef เก็บ MQTT client, connect()/events รับข้อความ, command() ส่ง ON/OFF และ useEffect cleanup
- `src/protocol.js`: ชื่อ topic และตรวจ payload
- `src/App.css`: รูปแบบหน้าเว็บและมือถือ
- `src/main.jsx`: เริ่ม React

รหัสผ่านใช้เฉพาะในหน่วยความจำของหน้านี้ ไม่เขียน localStorage หรือบันทึกลงไฟล์ แต่ผู้ใช้หน้าเว็บยังเข้าถึง credentials ใน browser ได้ ใช้บัญชี MQTT ที่จำกัดสิทธิ์สำหรับแลบ

## ตรวจสอบและ build

```text
npm test
npm run build
npm run preview
```

ไฟล์ build อยู่ใน dist ใช้ static web server เพื่อเปิด ไม่ใช่ double click HTML การทดสอบอัตโนมัติไม่ทดแทนการทดลองกับ HiveMQ และ ESP จริง

หน้าอ่านสรุปเดิมยังอยู่ที่ `../index.html` โปรเจกต์นี้ยังไม่ได้เผยแพร่เป็นเว็บไซต์ออนไลน์
