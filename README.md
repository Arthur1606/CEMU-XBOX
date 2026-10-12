# CEMU XBOX

Emulador de Wii U (Cemu) empaquetado como aplicación UWP para **Xbox Series X|S en Modo Desarrollador** (y Windows x64). Proyecto comunitario y **sin ánimo de lucro**.

Este repositorio reúne en un solo lugar las tres piezas necesarias para compilar:

| Carpeta | Origen | Licencia |
| --- | --- | --- |
| [`Cemu/`](Cemu/) | [rodrigoandrigo/Cemu](https://github.com/rodrigoandrigo/Cemu) (rama `build-msvc-d3d11`, fork de [cemu-project/Cemu](https://github.com/cemu-project/Cemu)): núcleo del emulador con renderer D3D11 y la ABI `CemuEmbed` | MPL-2.0 |
| [`Cemu-UWP-Host/`](Cemu-UWP-Host/) | [rodrigoandrigo/Cemu-UWP-Host](https://github.com/rodrigoandrigo/Cemu-UWP-Host): aplicación UWP (interfaz, mandos, almacenamiento, swap chain) | Apache-2.0 |
| `SDL3-uwp/` (submódulo) | [rodrigoandrigo/SDL3_UWP](https://github.com/rodrigoandrigo/SDL3_UWP): SDL3 con backend Windows.Gaming.Input | zlib |

Ambos proyectos se importaron con `git subtree`, así que su historial y su autoría originales se conservan. Este repositorio no distribuye juegos, claves, firmware ni ningún otro contenido de Nintendo.

## Cambios respecto a los repositorios originales

- **Error de compilación del host corregido:** `Cemu_UWP_HostMain::ReplaceD3D11Surface` se usaba y definía, pero no estaba declarado.
- **Suspensión/reanudación:** antes, al suspender la app (botón Guía, Inicio, reposo) se destruía la instancia de Cemu. Como el runtime solo admite una instancia por proceso, al volver el emulador quedaba inutilizable hasta reiniciar y se perdía la partida. Ahora la sesión se conserva y continúa al reanudar.
- **JIT (recompilador PPC):**
  - el manifiesto declara `codeGeneration`;
  - en UWP, si `VirtualAlloc` rechaza páginas ejecutables, se reintenta con `VirtualAlloc2FromApp`;
  - si aun así no hay memoria ejecutable, el recompilador ya no escribe sobre un puntero nulo: esa función se ejecuta con el intérprete.
- **Pantalla del GamePad:** nueva función `CemuEmbed_ToggleGamePadScreen` y atajo **Vista + clic del stick derecho** para intercambiar TV y GamePad, como `Ctrl+Tab` en Cemu de PC. Mientras se ve el GamePad, el ratón virtual (L+R) actúa como pantalla táctil.
- **README del host:** ya no dice que D3D12 se pueda seleccionar desde Ajustes.
- **CI:** [`.github/workflows/build-xbox.yml`](.github/workflows/build-xbox.yml) compila el runtime y el paquete UWP en Windows y publica el `.msixbundle` como artefacto descargable.

## Controles en partida

| Acción | Mando |
| --- | --- |
| Activar/desactivar ratón virtual | `LB + RB` |
| Clic / toque | `A` (con el ratón virtual activo) |
| Intercambiar pantalla TV ↔ GamePad | `Vista + R3` |
| Mostrar/ocultar opciones | `Vista + Menú` |

## Compilar

### Opción A: GitHub Actions (no necesitas Windows)

Cada `push` a `main` lanza el workflow **Build CEMU XBOX**. Al terminar, descarga el artefacto `CEMU-XBOX-x64` desde la pestaña *Actions*. La primera ejecución tarda bastante porque vcpkg compila todas las dependencias; las siguientes usan caché.

### Opción B: PC con Windows

Requisitos: Visual Studio 2026 (o 2022) con C++ de escritorio y de UWP, Windows SDK 10.0.26100 y CMake 3.21 o superior.

> Clona el repositorio en una ruta **sin espacios** (por ejemplo, `C:\dev\CEMU-XBOX`). Algunos ports de vcpkg fallan con espacios en la ruta.

```powershell
git clone --recursive <url-de-este-repo> C:\dev\CEMU-XBOX
cd C:\dev\CEMU-XBOX

cmake -S Cemu -B Cemu\build-msvc-d3d11 -G "Visual Studio 18 2026" -A x64 `
  -DCEMU_UWP=ON -DENABLE_D3D11=ON -DENABLE_D3D12=ON `
  -DSDL3_UWP_SOURCE_DIR="$PWD\SDL3-uwp"
cmake --build Cemu\build-msvc-d3d11 --config Release --target CemuBin --parallel

msbuild Cemu-UWP-Host\Cemu-UWP-Host.vcxproj /restore /m /p:Configuration=Release /p:Platform=x64
```

Con Visual Studio 2022, usa `-G "Visual Studio 17 2022"` y añade `/p:PlatformToolset=v143` a msbuild.

## Instalar en Xbox (Modo Desarrollador)

1. Abre el **Xbox Device Portal** (`https://<ip-de-la-xbox>:11443`).
2. Ve a *Add* y sube el `.msixbundle` del artefacto, junto con las dependencias de la carpeta `Dependencies\x64` (VCLibs).
3. En **Dev Home**, abre las opciones de la app y cambia su tipo a **Game** (no *App*). Con el tipo *App*, la consola le asigna mucha menos memoria, CPU y GPU, y el rendimiento cae mucho.
4. Primer arranque: importa `keys.txt` y tus juegos, actualizaciones y DLC volcados legalmente. Consulta [`Cemu-UWP-Host/README.md`](Cemu-UWP-Host/README.md).

## Pendiente para igualar a Cemu de PC

- **Renderer:** el backend D3D11 todavía no cubre todo (geometry shaders arbitrarios, stream output, formatos poco comunes). El backend D3D12 interno está en el núcleo, pero el host no lo ofrece. Terminarlo es lo que más compatibilidad aportaría.
- **Varios mandos:** el host solo envía un mando (jugador 1). Faltan los jugadores 2 a 5 para el multijugador local.
- **Giroscopio:** el mando de Xbox no tiene, y no hay emulación por stick. Juegos como Star Fox Zero o Splatoon lo necesitan.
- **Amiibo/NFC** y **vibración:** sin implementar.
- **Perfiles por juego:** los `gameProfiles` se empaquetan, pero no se pueden editar desde la interfaz.
# CEMU-XBOX
