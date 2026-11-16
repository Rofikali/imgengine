/** Synthetic TIFF metadata; no private image fixtures. */
export function oriented(bytes: Buffer, orientation: number, png: boolean, little: boolean) {
  const tiff = Buffer.alloc(26)
  tiff.writeUInt16BE(little ? 0x4949 : 0x4d4d)
  const view = new DataView(tiff.buffer, tiff.byteOffset, tiff.length)
  view.setUint16(2, 42, little); view.setUint32(4, 8, little); view.setUint16(8, 1, little)
  view.setUint16(10, 0x112, little); view.setUint16(12, 3, little)
  view.setUint32(14, 1, little); view.setUint16(18, orientation, little)
  if (!png) return Buffer.concat([bytes.subarray(0, 2), Buffer.from([255, 225, 0, 34, 69, 120, 105, 102, 0, 0]), tiff, bytes.subarray(2)])
  const chunk = Buffer.alloc(38)
  chunk.writeUInt32BE(26); chunk.write('eXIf', 4); tiff.copy(chunk, 8)
  let crc = 0xffffffff
  for (const byte of chunk.subarray(4, 34)) {
    crc ^= byte
    for (let bit = 0; bit < 8; bit++) crc = (crc >>> 1) ^ ((crc & 1) ? 0xedb88320 : 0)
  }
  chunk.writeUInt32BE((crc ^ 0xffffffff) >>> 0, 34)
  return Buffer.concat([bytes.subarray(0, 33), chunk, bytes.subarray(33)])
}
