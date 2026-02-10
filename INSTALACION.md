# 🎮 Instalación en PS3 - Guía Rápida

## ✅ El PKG está Listo

**Archivo:** `hello-ps3.pkg` (90 KB)

Este PKG ha sido corregido para funcionar correctamente:
- ✅ Incluye ICON0.PNG (instala sin error)
- ✅ EBOOT.BIN firmado con make_self (ejecuta sin error)

---

## 📋 Instrucciones de Instalación

### Opción 1: USB (Más Simple)

1. **Preparar USB:**
   ```
   - Formatear USB en FAT32
   - Copiar hello-ps3.pkg a la raíz del USB
   ```

2. **En la PS3:**
   ```
   - Conectar USB a la PS3
   - Ir a: Game → Install Package Files → Standard
   - Seleccionar el dispositivo USB
   - Elegir hello-ps3.pkg
   - Presionar X para instalar
   ```

3. **Ejecutar:**
   ```
   - Ir a: Game → PlayStation 3
   - Buscar "Hola Mundo PS3" con su ícono
   - Presionar X para ejecutar
   ```

### Opción 2: FTP (Para Desarrollo)

1. **Subir PKG:**
   ```bash
   # Usar FileZilla o comando FTP
   ftp 192.168.X.X  # IP de tu PS3
   cd /dev_hdd0/packages
   put hello-ps3.pkg
   ```

2. **Instalar:**
   ```
   - En PS3: Package Manager → Install Package Files
   - Seleccionar hello-ps3.pkg
   - Instalar
   ```

### Opción 3: Navegador Web

1. **Servidor local:**
   ```bash
   # En tu PC (mismo directorio que el PKG)
   python3 -m http.server 8000
   ```

2. **En la PS3:**
   ```
   - Abrir navegador web
   - Ir a: http://IP-DE-TU-PC:8000/hello-ps3.pkg
   - Descargar e instalar
   ```

---

## 🔄 Si Ya Instalaste una Versión Anterior

**IMPORTANTE:** Desinstala primero la versión anterior

1. En XMB, sobre "Hola Mundo PS3"
2. Presionar **△ (Triangle)**
3. Seleccionar **Delete** o **Eliminar**
4. Confirmar
5. Instalar la nueva versión

---

## ⚠️ Requisitos Previos

Tu PS3 debe tener:
- ✅ **CFW** (Custom Firmware): Rebug, Ferrox, IRISMAN, etc.
  - O **HEN** (Homebrew Enabler) para consolas no hackeables

Si no tienes CFW/HEN:
- No podrás instalar este PKG
- Busca guías de instalación de CFW/HEN según tu modelo de PS3

---

## 🎮 ¿Qué Hace el Programa?

Al ejecutar verás:
- Pantalla negra de fondo
- Rectángulo azul en el centro con borde blanco
- Barra verde decorativa
- Bloques de colores (amarillo, rojo, blanco)
- Texto "HOLA PS3" visual

**Controles:**
- **✖ (X/Cross):** Salir del programa

---

## 🐛 Si Algo Sale Mal

### Error al Instalar

**Mensaje:** "Datos dañados" antes de instalar

**Solución:** 
- El nuevo PKG ya tiene el ICON0.PNG
- Si persiste, verifica que tu PS3 tenga CFW/HEN activo

### Error al Ejecutar

**Mensaje:** "Datos dañados" al intentar ejecutar

**Solución:**
1. Desinstala la versión actual
2. Verifica que el PKG sea de 90 KB (no 1+ MB)
3. Reinstala el PKG nuevo

### Pantalla Negra

**Si el programa inicia pero solo se ve negro:**
- Espera 2-3 segundos (puede tardar en inicializar RSX)
- Si persiste, revisa logs en TTY si tienes acceso

### No Aparece en XMB

**Si no aparece después de instalar:**
- Espera 10 segundos y navega por el XMB
- Reinicia la PS3
- Verifica que se instaló en: Game → PlayStation 3

---

## 📝 Logs de Depuración (Avanzado)

Si tienes acceso TTY o FTP, puedes revisar:

```bash
# Logs del sistema
/dev_hdd0/tmp/

# Log de instalación
/dev_hdd0/tmp/installpkg.log
```

El programa imprime mensajes en TTY:
```
===========================================
   HOLA PS3 - Homebrew Mínimo
===========================================
Inicializando sistema gráfico RSX...
Resolución: 1280x720
Inicializando sistema de control...
Dibujando mensaje...
===========================================
Programa iniciado correctamente!
Presiona X en el control para salir
===========================================
```

---

## ✅ Verificación de Éxito

1. **Instalación exitosa:**
   - No hubo error "datos dañados"
   - Aparece en Game → PlayStation 3

2. **Ejecución exitosa:**
   - Se inicia sin error
   - Ves el mensaje visual en pantalla
   - Puedes salir con el botón X

3. **Funcionamiento correcto:**
   - El mensaje se ve centrado y claro
   - Los colores son correctos
   - El control responde

---

## 🎯 Tamaño de Archivos Correctos

Para verificar que tienes la versión correcta:

```bash
# El PKG debe ser pequeño
ls -lh hello-ps3.pkg
# ✓ Correcto: ~90 KB

# El EBOOT.BIN debe estar comprimido
ls -lh pkg_temp/USRDIR/EBOOT.BIN
# ✓ Correcto: ~88 KB
```

Si ves tamaños de ~1 MB o más, regenera el PKG:
```bash
make clean && make pkg
```

---

## 📚 Más Ayuda

- [TROUBLESHOOTING.md](TROUBLESHOOTING.md) - Guía completa de problemas
- [README.md](README.md) - Documentación general
- [EXPLICACION.md](EXPLICACION.md) - Cómo funciona el código

---

## 🎉 ¡Listo!

Una vez instalado correctamente, habrás compilado e instalado tu primer homebrew para PS3. 

**¿Qué hacer ahora?**
- Revisa [MODIFICACIONES.md](MODIFICACIONES.md) para personalizar el programa
- Experimenta con los colores y formas
- Crea tu propio homebrew basado en este ejemplo

¡Disfruta tu homebrew PS3! 🎮✨
