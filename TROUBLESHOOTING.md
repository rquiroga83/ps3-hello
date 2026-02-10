# 🔧 Solución de Problemas (Troubleshooting)

## ❌ Error: "Datos Dañados" al Instalar PKG

### 🔍 Diagnóstico

El error "datos dañados" (corrupted data) en PS3 al instalar un PKG puede tener varias causas:

---

## ✅ Solución 1: Falta ICON0.PNG (Error al INSTALAR)

### Problema Principal: Falta ICON0.PNG

Los paquetes PKG de PS3 **requieren obligatoriamente** un archivo `ICON0.PNG` para ser instalados correctamente.

**Especificaciones del ícono:**
- Formato: PNG
- Tamaño: **320 x 176 píxeles**
- Profundidad: 8-bit RGBA (color + alpha)
- Ubicación: Raíz del PKG (junto a PARAM.SFO)

### Corrección Aplicada

Se agregó al proyecto:

1. **Script generador**: `scripts/create_icon.py`
   - Crea automáticamente un ICON0.PNG válido
   - Sin dependencias externas (PNG puro en Python)
   - Diseño: Fondo azul con texto "HOLA PS3"

2. **Makefile actualizado**:
   ```makefile
   pkg: all
       @python3 $(CURDIR)/scripts/create_icon.py pkg_temp/ICON0.PNG
       @cp -f pkg_temp/ICON0.PNG /ICON0.PNG
   ```

---

## ✅ Solución 2: EBOOT.BIN Mal Firmado (Error al EJECUTAR)

### 🔍 Síntoma

- ✓ El PKG se instala correctamente
- ✓ Aparece en el XMB con su ícono
- ✗ Al intentar ejecutarlo: "Datos dañados"

### Problema: Firma Incorrecta del EBOOT.BIN

El EBOOT.BIN debe estar firmado con **`make_self`**, NO con `fself`.

**Diferencia crucial:**
```bash
# ❌ INCORRECTO - produce archivo sin firmar adecuadamente
fself hello-ps3.elf EBOOT.BIN
# Resultado: ~1.1 MB, ejecutable sin comprimir/encriptar

# ✅ CORRECTO - firma para homebrew
make_self hello-ps3.elf EBOOT.BIN  
# Resultado: ~90 KB, comprimido y encriptado correctamente
```

### Corrección en Makefile

**Antes (incorrecto):**
```makefile
@fself $(OUTPUT).elf pkg_temp/USRDIR/EBOOT.BIN
```

**Después (correcto):**
```makefile
@make_self $(OUTPUT).elf pkg_temp/USRDIR/EBOOT.BIN
```

### ¿Cómo Verificar?

```bash
# El EBOOT.BIN correcto debe ser pequeño (comprimido)
ls -lh pkg_temp/USRDIR/EBOOT.BIN
# ✓ Correcto: ~90 KB
# ✗ Incorrecto: ~1+ MB

# Regenerar PKG con la firma correcta
make clean && make pkg
```

---

## 📋 Lista de Verificación PKG

Tu PKG debe contener estos archivos:

```
pkg_temp/
├── PARAM.SFO      ✓ Metadata del paquete
├── ICON0.PNG      ✓ Ícono 320x176 (OBLIGATORIO)
└── USRDIR/
    └── EBOOT.BIN  ✓ Ejecutable firmado con make_self
```

---

## Solución 3: PS3_SYSTEM_VER Incorrecto (PS3 4.88 HEN)

### Síntomas
- El PKG instala correctamente (icono visible)
- Al ejecutar: "datos dañados"
- EBOOT.BIN correcto (88KB, firmado con make_self)
- Problema persiste después de reinstalar

### Causa Principal
El campo `PS3_SYSTEM_VER` en PARAM.SFO debe coincidir con la versión del firmware.

**Según PSDevWiki:**
> *"PS3_SYSTEM_VER: Minimum PS3 System Software required for the content to be bootable. Format: XX.YYYY"*

Si el valor no coincide, el PS3 puede rechazar la ejecución con "datos dañados".

### Verificación

**Revisa tu firmware:**
- XMB → Settings → System Settings → System Information
- Versiones comunes HEN: 4.88, 4.89, 4.90, 4.91

**Verifica el PARAM.SFO actual:**
```bash
strings pkg_temp/PARAM.SFO | grep -A1 "PS3_SYSTEM_VER"
```

### Solución

**✅ Para PS3 4.88 HEN (YA CORREGIDO):**

El código actual usa `PS3_SYSTEM_VER = "04.8800"` en `scripts/create_sfo.py`.

**Para otras versiones**, modifica línea 28 en [scripts/create_sfo.py](scripts/create_sfo.py):

```python
# Firmware 4.88 (actual)
("PS3_SYSTEM_VER", TYPE_UTF8, "04.8800"),

# Firmware 4.89
("PS3_SYSTEM_VER", TYPE_UTF8, "04.8900"),

# Firmware 4.90
("PS3_SYSTEM_VER", TYPE_UTF8, "04.9000"),

# Firmware 4.91
("PS3_SYSTEM_VER", TYPE_UTF8, "04.9100"),
```

**Regenera el PKG:**
```bash
rm -rf pkg_temp hello-ps3.pkg
make pkg
```

---

## 🔧 Instrucciones Específicas para HEN (4.88)

Si usas **HEN** (Homebrew ENabler) en lugar de CFW completo:

### 1️⃣ Desinstalar PKG Anterior
```
XMB → Game → [HOLA PS3]
Presiona TRIÁNGULO → Borrar
Confirma eliminación
```

### 2️⃣ Reiniciar PS3
Obligatorio para limpiar caché del XMB.

### 3️⃣ Activar HEN
```
Navegador Web → PS3HEN → Activar
```
⚠️ **CRÍTICO:** HEN NO es persistente - se desactiva al apagar.

### 4️⃣ Instalar Nuevo PKG

**Método USB (recomendado):**
```
1. Copia hello-ps3.pkg a la raíz del USB
2. Conecta USB al PS3
3. XMB → Package Manager → Install Package Files
4. Selecciona Standard → hello-ps3.pkg
```

**Método FTP:**
```
1. Copia a /dev_hdd0/packages/hello-ps3.pkg
2. XMB → Package Manager → Install Package Files
```

### 5️⃣ Ejecutar

```
XMB → Game → HOLA PS3
Presiona X para ejecutar
```

### ⚠️ Limitaciones de HEN vs CFW

| Característica | HEN | CFW |
|----------------|-----|-----|
| Persistente después de apagar | ❌ No | ✅ Sí |
| Permisos completos del sistema | ⚠️ Limitado | ✅ Completo |
| Compatibilidad homebrew | ⚠️ ~80% | ✅ 100% |
| Requiere activación manual | ✅ Sí | ❌ No |

**Si el problema persiste después de estos pasos:**
- HEN puede tener restricciones con homebrew que usa RSX intensivamente
- Considera actualizar a CFW completo (Rebug, Evilnat, Ferrox)
- Algunos homebrew complejos no son 100% compatibles con HEN

---

### Verificar tu PKG

```bash
# Ver contenido del directorio temporal
ls -la pkg_temp/

# Debe mostrar:
# PARAM.SFO
# ICON0.PNG  <- ¡IMPORTANTE!
# USRDIR/EBOOT.BIN
```

---

## 🐛 Otros Errores Comunes

### 1. "Archivo no soportado" o "No se puede iniciar" (Obsoleto)

**Este error ya está resuelto** - era causado por usar `fself` en lugar de `make_self`.

**La solución correcta está implementada en el Makefile actual.**

Asegúrate de:
```bash
# Regenerar con la versión correcta
make clean && make pkg

# Verificar tamaño del EBOOT.BIN
ls -lh pkg_temp/USRDIR/EBOOT.BIN
# Debe ser ~90 KB (comprimido)
```

### 2. "No aparece en el XMB después de instalar"

**Causa**: PARAM.SFO con CATEGORY incorrecta

**Verificación**:
```bash
strings pkg_temp/PARAM.SFO | grep CATEGORY
# Debe mostrar: CATEGORY seguido de HG
```

**CATEGORY correcta para homebrew**: `HG`

### 3. "El ícono no se muestra correctamente"

**Causa**: Formato de imagen incorrecto

**Especificaciones ICON0.PNG**:
- Exactamente 320x176 píxeles
- No usar PNG progresivo
- No usar PNG con paleta indexada
- Usar RGBA de 8-bit por canal

**Verificar tu ícono**:
```bash
file pkg_temp/ICON0.PNG
# Debe decir: PNG image data, 320 x 176, 8-bit/color RGBA
```

### 4. "Error al crear PKG con pkg.py"

**Causa**: Archivos no están en la ubicación esperada

**El script pkg.py busca archivos en la raíz `/`**:
```bash
# Estos archivos DEBEN existir durante la creación:
/PARAM.SFO
/ICON0.PNG
/USRDIR/EBOOT.BIN
```

El Makefile los copia temporalmente y los borra después.

### 5. ContentID inválido

**Formato correcto**: `UP0001-ABCD12345_00-0123456789ABCDEF`

**Estructura**:
```
UP0001-PSL145310_00-0000000000000001
│     │          │  │
│     │          │  └─ Unique ID (16 caracteres)
│     │          └─ Siempre "00"
│     └─ TITLE_ID (9 caracteres, mismo que en PARAM.SFO)
└─ Prefijo (UP para homebrew)
```

**En el Makefile**:
```makefile
APPID       := PSL145310
CONTENTID   := UP0001-PSL145310_00-0000000000000001
```

---

## 🔍 Herramientas de Diagnóstico

### Verificar PARAM.SFO

```bash
# Ver contenido del PARAM.SFO
strings pkg_temp/PARAM.SFO

# Debe incluir:
# - CATEGORY (seguido de HG)
# - TITLE_ID (tu APPID)
# - TITLE (nombre de tu app)
# - APP_VER (01.00)
# - VERSION (01.00)
```

### Verificar estructura del PKG

```bash
# Listar archivos en pkg_temp
find pkg_temp -type f

# Salida esperada:
# pkg_temp/PARAM.SFO
# pkg_temp/ICON0.PNG
# pkg_temp/USRDIR/EBOOT.BIN
```

### Verificar tamaños

```bash
ls -lh pkg_temp/PARAM.SFO    # ~400-600 bytes
ls -lh pkg_temp/ICON0.PNG    # ~500 bytes - 50 KB
ls -lh pkg_temp/USRDIR/EBOOT.BIN  # ~1-2 MB típico
```

---

## 🔄 Regenerar PKG Limpiamente

Si sigues teniendo problemas:

```bash
# 1. Limpiar completamente
make clean
rm -rf pkg_temp/
rm -f hello-ps3.pkg

# 2. Recompilar desde cero
make

# 3. Crear PKG nuevo
make pkg

# 4. Verificar que incluye el ícono
ls -la pkg_temp/ICON0.PNG
```

---

## 📱 Instalación en PS3

### Métodos de Instalación

**Opción 1: USB**
```
1. Copiar hello-ps3.pkg a USB
2. Conectar USB a PS3
3. Ir a: Game > Install Package Files
4. Seleccionar el PKG
5. Instalar
```

**Opción 2: FTP**
```
1. Usar FileZilla o similar
2. Conectar a PS3 por FTP
3. Subir PKG a /dev_hdd0/packages/
4. En PS3: Package Manager > Install Package Files
```

**Opción 3: Navegador Web**
```
1. Colocar PKG en un servidor web local
2. En PS3, abrir navegador
3. Navegar a http://tu-ip/hello-ps3.pkg
4. Descargar e instalar
```

### Después de Instalar

1. El juego debe aparecer en **Game > PlayStation 3**
2. El ícono debe ser visible
3. Al ejecutar, debe mostrar el mensaje gráfico

---

## 🆘 Si Nada Funciona

### Verificación Final

1. **¿Tu PS3 tiene CFW o HEN?**
   - CFW: Rebug, Ferrox, IRISMAN, etc.
   - HEN: PS3HEN para consolas no hackeables completamente

2. **¿El PKG es para la región correcta?**
   - Generalmente no importa para homebrew

3. **¿Tienes espacio libre?**
   - Requiere ~2-3 MB mínimo

4. **¿Otros PKG sí se instalan?**
   - Prueba con un PKG oficial para verificar que el sistema funcione

### Logs de Depuración

Si tienes acceso a TTY/FTP, revisa:
```
/dev_hdd0/tmp/installpkg.log
```

---

## ✅ Checklist Final

Antes de reportar un problema, verifica:

- [ ] El PKG incluye ICON0.PNG (320x176)
- [ ] El PKG incluye PARAM.SFO válido
- [ ] El PKG incluye USRDIR/EBOOT.BIN
- [ ] EBOOT.BIN fue creado con `fself` (no make_self_npdrm)
- [ ] CATEGORY en PARAM.SFO es "HG"
- [ ] TITLE_ID coincide en PARAM.SFO y ContentID
- [ ] ContentID tiene formato correcto
- [ ] PS3 tiene CFW/HEN instalado
- [ ] Tienes espacio libre suficiente
- [ ] Has probado reinstalar después de borrar el PKG anterior

---

## 📚 Recursos Adicionales

- **PSDevWiki PKG Format**: https://www.psdevwiki.com/ps3/PKG_files
- **PARAM.SFO Format**: https://www.psdevwiki.com/ps3/PARAM.SFO
- **ps3dev Discord**: Comunidad de desarrollo PS3
- **GBAtemp PS3 Hacking**: Foros de soporte

---

## 🎯 Resumen de la Solución

**El problema "datos dañados" se resolvió agregando:**

1. ✅ Script `create_icon.py` para generar ICON0.PNG
2. ✅ Actualización del Makefile para incluir el ícono
3. ✅ Estructura PKG completa con todos los archivos requeridos

**Ahora el PKG se instala correctamente sin errores.**

Para regenerar el PKG con la corrección:
```bash
make clean && make pkg
```

El archivo `hello-ps3.pkg` resultante está listo para instalar en tu PS3. 🎮✨
