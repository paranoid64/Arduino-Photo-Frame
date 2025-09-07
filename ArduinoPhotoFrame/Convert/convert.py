from PIL import Image
import os
import glob

def convert_to_rgb565(im):
    # Resize to 160x128
    im = im.resize((160, 128))

    # Convert to RGB565
    arr = im.convert('RGB').load()

    rgb565_data = bytearray()
    for y in range(128):
        for x in range(160):
            r, g, b = arr[x, y]
            rgb565 = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)
            rgb565_data.append(rgb565 & 0xFF)
            rgb565_data.append(rgb565 >> 8)

    return rgb565_data

def save_bmp_rgb565(filename, data, width=160, height=128):
    # BMP Header (54 bytes)
    header = bytearray(54)
    # Signature
    header[0:2] = b'BM'
    filesize = 54 + len(data)
    header[2:6] = filesize.to_bytes(4, 'little')
    header[10:14] = (54).to_bytes(4, 'little')  # Pixel data offset
    header[14:18] = (40).to_bytes(4, 'little')  # DIB header size
    header[18:22] = width.to_bytes(4, 'little')
    header[22:26] = height.to_bytes(4, 'little')
    header[26:28] = (1).to_bytes(2, 'little')  # planes
    header[28:30] = (16).to_bytes(2, 'little')  # bits per pixel
    header[30:34] = (3).to_bytes(4, 'little')  # Compression = BI_BITFIELDS
    header[34:38] = len(data).to_bytes(4, 'little')
    header[38:42] = (2835).to_bytes(4, 'little')  # horizontal resolution (pixels/meter)
    header[42:46] = (2835).to_bytes(4, 'little')  # vertical resolution
    header[46:50] = (0).to_bytes(4, 'little')  # colors in palette
    header[50:54] = (0).to_bytes(4, 'little')  # important colors

    # Bitfields for RGB565
    header += (0xF800).to_bytes(4, 'little')  # Red mask
    header += (0x07E0).to_bytes(4, 'little')  # Green mask
    header += (0x001F).to_bytes(4, 'little')  # Blue mask

    # Pixel data in BGR order, bottom to top (BMP format)
    # The pixel data must be padded per row to multiple of 4 bytes
    row_size = (width * 2 + 3) & ~3
    padding = row_size - width * 2

    with open(filename, 'wb') as f:
        f.write(header)
        for y in reversed(range(height)):
            start = y * width * 2
            end = start + width * 2
            f.write(data[start:end])
            f.write(b'\x00' * padding)

input_folder = '.'
output_folder = 'bmp_out'
os.makedirs(output_folder, exist_ok=True)

for filepath in glob.glob('*.jpg') + glob.glob('*.JPG'):
    im = Image.open(filepath)
    rgb565_data = convert_to_rgb565(im)
    filename = os.path.splitext(os.path.basename(filepath))[0]
    save_bmp_rgb565(os.path.join(output_folder, f"{filename}.bmp"), rgb565_data)
    print(f"Konvertiert {filepath} -> {output_folder}/{filename}.bmp")
