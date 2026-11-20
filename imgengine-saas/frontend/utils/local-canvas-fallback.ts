import type { PixelCrop } from './local-crop'
import type { RightAngleRotation } from './local-rotate'

export interface LocalCanvasOutput { width: number, height: number }

/**
 * Private compatibility path for browsers without a usable Worker +
 * OffscreenCanvas encoder. Callers already bound dimensions and source URLs;
 * this function never uploads, persists, or changes the requested operation.
 */
export async function renderLocalCanvasFallback(
  sourceUrl: string,
  output: LocalCanvasOutput,
  crop?: PixelCrop,
  rotation?: RightAngleRotation,
): Promise<Blob> {
  const image = new Image()
  try {
    await new Promise<void>((resolve, reject) => {
      image.onload = () => resolve()
      image.onerror = () => reject(Error('image decode failed'))
      image.src = sourceUrl
    })
    const canvas = document.createElement('canvas')
    canvas.width = output.width
    canvas.height = output.height
    const context = canvas.getContext('2d')
    if (!context) throw Error('canvas unavailable')
    context.imageSmoothingEnabled = true
    context.imageSmoothingQuality = 'high'
    if (crop) {
      context.drawImage(image, crop.x, crop.y, crop.width, crop.height, 0, 0, crop.width, crop.height)
    } else if (rotation === 90) {
      context.translate(output.width, 0); context.rotate(Math.PI / 2); context.drawImage(image, 0, 0)
    } else if (rotation === 180) {
      context.translate(output.width, output.height); context.rotate(Math.PI); context.drawImage(image, 0, 0)
    } else if (rotation === 270) {
      context.translate(0, output.height); context.rotate(-Math.PI / 2); context.drawImage(image, 0, 0)
    } else context.drawImage(image, 0, 0, output.width, output.height)
    const blob = await new Promise<Blob>((resolve, reject) => {
      canvas.toBlob(value => value ? resolve(value) : reject(Error('PNG encode failed')), 'image/png')
    })
    if (blob.type !== 'image/png' || !blob.size || blob.size > 20 * 1024 * 1024) throw Error('invalid PNG output')
    return blob
  } finally {
    image.removeAttribute('src')
  }
}
