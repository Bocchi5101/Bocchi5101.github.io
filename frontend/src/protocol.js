export function topicsFor(prefix) {
  if (!prefix || prefix !== prefix.trim() || /[+#\s]/.test(prefix) || prefix.endsWith('/')) throw new Error('Topic prefix ต้องไม่ว่าง ไม่มีช่องว่าง + # และไม่ลงท้ายด้วย /');
  return { temp: `${prefix}/sensor/temp`, humidity: `${prefix}/sensor/humidity`, led: `${prefix}/led/status`, control: `${prefix}/led/control` };
}
export function decodeMessage(topics, topic, payload) {
  const value = payload.trim();
  if (topic === topics.led) {
    if (!['ON', 'OFF'].includes(value)) throw new Error('LED status ต้องเป็น ON หรือ OFF');
    return { key: 'led', value };
  }
  const key = topic === topics.temp ? 'temp' : topic === topics.humidity ? 'humidity' : null;
  if (!key) return null;
  const number = Number(value);
  if (!value || !Number.isFinite(number) || (key === 'humidity' && (number < 0 || number > 100))) throw new Error('ค่าเซนเซอร์ไม่ถูกต้อง');
  return { key, value: number };
}
