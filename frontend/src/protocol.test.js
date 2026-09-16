import { test } from 'node:test';
import assert from 'node:assert/strict';
import { topicsFor, decodeMessage } from './protocol.js';
const t = topicsFor('lab');
test('matches firmware topics and preserves leading slash', () => {
  assert.equal(t.control, 'lab/led/control');
  assert.equal(topicsFor('/lab').led, '/lab/led/status');
  for (const p of ['', 'lab/', 'lab/#', 'lab/+', ' lab']) assert.throws(() => topicsFor(p));
});
test('receives plain-text sensor and physical LED updates', () => {
  assert.deepEqual(decodeMessage(t, t.temp, '28.5'), {key:'temp',value:28.5});
  assert.deepEqual(decodeMessage(t, t.humidity, '0'), {key:'humidity',value:0});
  assert.deepEqual(decodeMessage(t, t.led, 'OFF'), {key:'led',value:'OFF'});
  assert.equal(decodeMessage(t, t.control, 'ON'), null);
});
test('rejects invalid sensor readings and LED state', () => {
  for (const x of ['', 'NaN', 'Infinity', '{"temp":25}']) assert.throws(() => decodeMessage(t,t.temp,x));
  for (const x of ['-1','101']) assert.throws(() => decodeMessage(t,t.humidity,x));
  assert.throws(() => decodeMessage(t,t.led,'on'));
});
