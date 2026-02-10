#!/usr/bin/env python3
"""
Crea un ícono ICON0.PNG simple para PKG de PS3
Sin dependencias externas, usando PNG puro
"""
import struct
import zlib

def create_png_icon(filename, width=320, height=176):
    """Crea un PNG simple con texto 'HOLA PS3'"""
    
    # Fondo azul oscuro
    bg_color = (0, 20, 64)  # RGB
    text_color = (255, 200, 0)  # Amarillo
    border_color = (255, 255, 255)  # Blanco
    
    # Crear datos de la imagen (RGBA)
    pixels = []
    for y in range(height):
        row = []
        for x in range(width):
            # Dibujar borde blanco (10 píxeles de grosor)
            if (10 <= x <= 309 and (10 <= y <= 14 or 161 <= y <= 165)) or \
               (10 <= y <= 165 and (10 <= x <= 14 or 305 <= x <= 309)):
                row.extend(border_color + (255,))  # RGBA
            # Dibujar texto "HOLA PS3" (simplificado con bloques)
            elif 120 <= x <= 200 and 60 <= y <= 115:
                # Área del texto
                if (125 <= x <= 145 or 155 <= x <= 175 or 185 <= x <= 195) and \
                   (65 <= y <= 75 or 90 <= y <= 95 or 100 <= y <= 110):
                    row.extend(text_color + (255,))
                else:
                    row.extend(bg_color + (255,))
            else:
                row.extend(bg_color + (255,))  # Fondo
        pixels.extend(row)
    
    # Convertir a bytes
    raw_data = bytes(pixels)
    
    # Crear scanlines (cada fila necesita un byte de filtro)
    scanlines = b''
    for y in range(height):
        scanlines += b'\x00'  # Sin filtro
        scanlines += raw_data[y * width * 4:(y + 1) * width * 4]
    
    # Comprimir datos
    compressed = zlib.compress(scanlines, 9)
    
    # Construir PNG
    def make_chunk(chunk_type, data):
        chunk = chunk_type + data
        crc = zlib.crc32(chunk) & 0xffffffff
        return struct.pack('>I', len(data)) + chunk + struct.pack('>I', crc)
    
    png = b'\x89PNG\r\n\x1a\n'  # Firma PNG
    
    # IHDR chunk
    ihdr = struct.pack('>IIBBBBB', width, height, 8, 6, 0, 0, 0)
    png += make_chunk(b'IHDR', ihdr)
    
    # IDAT chunk
    png += make_chunk(b'IDAT', compressed)
    
    # IEND chunk
    png += make_chunk(b'IEND', b'')
    
    with open(filename, 'wb') as f:
        f.write(png)

if __name__ == '__main__':
    import sys
    output = sys.argv[1] if len(sys.argv) > 1 else 'ICON0.PNG'
    create_png_icon(output)
    print(f'✓ {output} creado (320x176 PNG)')
