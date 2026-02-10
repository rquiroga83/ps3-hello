# 🚀 Guía de Uso Rápido

## 📦 Tres formas de ejecutar tu homebrew

### 1️⃣ **PKG (Recomendado para distribución)**

```bash
# Generar PKG
make pkg

# Resultado: hello-ps3.pkg (89 KB)
```

**Instalación:**
- Copia `hello-ps3.pkg` a USB o FTP
- PS3: Package Manager → Install Package Files
- Aparece en XMB → Game con icono

✅ **Ventajas:** Instalación limpia, icono automático, fácil de compartir  
❌ **Desventajas:** Requiere reinstalar para cada cambio

---

### 2️⃣ **SELF (Recomendado para desarrollo/testing)**

```bash
# Generar SELF independiente
make self

# Resultado: hello-ps3.self (87 KB)
```

**Método A - Estructura manual (USB):**

```bash
# Estructura necesaria en USB o HDD:
/dev_hdd0/game/TEST12345/
├── PARAM.SFO      # Copia desde pkg_temp/PARAM.SFO
├── ICON0.PNG      # Copia desde pkg_temp/ICON0.PNG (opcional)
└── USRDIR/
    └── EBOOT.BIN  # Renombra hello-ps3.self a EBOOT.BIN
```

1. Copia `hello-ps3.self` a tu USB
2. Renombra a `EBOOT.BIN`
3. Crea la estructura de carpetas
4. Copia `pkg_temp/PARAM.SFO` e `ICON0.PNG` (opcional)
5. Conéctalo al PS3 → Aparece en XMB → Game

**Método B - FTP (más rápido):**

```bash
# Conecta por FTP a tu PS3 (192.168.1.X:21)
# Sube hello-ps3.self directamente a:
/dev_hdd0/game/TSTPS30001/USRDIR/EBOOT.BIN

# O reemplaza el EBOOT.BIN de cualquier app existente
```

✅ **Ventajas:** Cambios rápidos sin reinstalar, ideal para debugging  
❌ **Desventajas:** Requiere configuración manual inicial

---

### 3️⃣ **ELF (Solo para emuladores/debugging)**

```bash
# El archivo hello-ps3.elf ya existe después de compilar
make

# Resultado: hello-ps3.elf (1.1 MB sin comprimir)
```

**Uso:**
- Emuladores: RPCS3 puede cargar ELF directamente
- Debugging: Herramientas de desarrollo (ps3load, etc.)

⚠️ **NO funciona en PS3 real** - Necesitas convertirlo a SELF/PKG primero

---

## 🛠️ Comandos Makefile

```bash
make              # Compila → genera hello-ps3.elf
make self         # ELF → SELF (hello-ps3.self)
make pkg          # ELF → PKG completo (hello-ps3.pkg)
make clean        # Limpia todo
```

---

## 📊 Comparación de Métodos

| Método | Tamaño | Instalación | Actualización | Icono | Uso |
|--------|--------|-------------|---------------|-------|-----|
| **PKG** | 89 KB | Package Manager | Reinstalar PKG | ✅ Automático | Distribución |
| **SELF** | 87 KB | Manual/FTP | Reemplazar archivo | ❌ Manual | Desarrollo |
| **ELF** | 1.1 MB | No soportado | N/A | ❌ | Emulador |

---

## 🎯 ¿Cuál usar?

### 👨‍💻 **Desarrollando/Testing:**
```bash
make self
# → Usa FTP para reemplazar EBOOT.BIN rápidamente
```

### 📦 **Distribuyendo/Compartiendo:**
```bash
make pkg
# → Comparte hello-ps3.pkg con otros usuarios
```

### 🐛 **Debugging con RPCS3:**
```bash
make
# → Carga hello-ps3.elf en RPCS3 directamente
```

---

## 💡 Tips

**Desarrollo rápido con FTP:**
1. Instala el PKG una vez (para tener la estructura completa)
2. Modifica el código
3. `make self`
4. Sube `hello-ps3.self` por FTP como `EBOOT.BIN`
5. Reinicia la app en PS3 (sin reinstalar)

**Verificar tamaños correctos:**
```bash
ls -lh hello-ps3.*

# Esperado:
# hello-ps3.elf:  1.1 MB  (ELF sin comprimir)
# hello-ps3.self:  87 KB  (SELF comprimido)
# hello-ps3.pkg:   89 KB  (PKG con metadata)
```

---

## ⚠️ Requisitos PS3

- **Firmware:** 4.88 HEN o CFW compatible
- **HEN activo:** Si usas HEN, actívalo antes de ejecutar
- **Permisos:** FTP requiere servidor FTP instalado (multiMAN, Rebug Toolbox, etc.)

---

## 🔗 Más información

- [INSTALACION.md](INSTALACION.md) - Guía detallada de instalación
- [TROUBLESHOOTING.md](TROUBLESHOOTING.md) - Solución de problemas
- [MODIFICACIONES.md](MODIFICACIONES.md) - Cómo modificar el código
