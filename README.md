# CEMU XBOX

Port de [Cemu](https://github.com/cemu-project/Cemu), emulador de Wii U, empaquetado como aplicación UWP para **Xbox Series X|S en Modo Desarrollador** (y Windows x64).

> **Estado: experimental.** Compila y se empaqueta en GitHub Actions, pero todavía no está verificado en hardware de forma amplia. Muchos juegos tendrán fallos gráficos o de rendimiento. Úsalo bajo tu propia responsabilidad.

Proyecto sin ánimo de lucro, no afiliado a Nintendo, Microsoft ni al proyecto Cemu original.

## Descargar e instalar

**Requisitos**
- Xbox Series X|S con **Modo Desarrollador** activado (Dev Home).
- Tu PC o Mac en la misma red que la Xbox.
- Tus propios volcados de consola (`keys.txt`, juegos, actualizaciones y DLC). Este repositorio no incluye juegos, claves, firmware ni contenido de Nintendo.

**Pasos**
1. Descarga el paquete `CEMU-XBOX-x64` desde [**Releases**](https://github.com/Arthur1606/CEMU-XBOX/releases/latest) y descomprímelo.
2. En el navegador, abre el **Xbox Device Portal**: `https://<IP-de-la-Xbox>:11443`. Acepta la advertencia del certificado (es normal, el portal usa un certificado local).
3. Inicia sesión con el usuario y la contraseña del Device Portal.
4. Ve a **Add**, elige el archivo `.msixbundle` y, si lo pide, añade los paquetes de `Dependencies\x64` (VCLibs).
5. En **Dev Home**, abre las opciones de la app y cambia su tipo de **App** a **Game**. Con el tipo *App* la consola le asigna mucha menos memoria, CPU y GPU, y el rendimiento cae mucho.
6. Abre la app desde *Mis juegos y apps*. Al primer arranque, importa tus archivos desde la propia interfaz.

## Controles

| Acción | Mando |
| --- | --- |
| Activar/desactivar ratón virtual | `LB + RB` |
| Clic / toque (con ratón virtual) | `A` |
| Intercambiar pantalla TV ↔ GamePad | `Vista + R3` |
| Mostrar/ocultar opciones | `Vista + Menú` |

## Compilar desde el código

### Opción A: GitHub Actions (sin Windows)
Cada `push` a `main` lanza el workflow [**Build CEMU XBOX**](.github/workflows/build-xbox.yml). Al terminar, el artefacto `CEMU-XBOX-x64` queda disponible en la pestaña *Actions*. La primera ejecución tarda bastante porque vcpkg compila las dependencias; las siguientes usan caché.

### Opción B: PC con Windows
Requisitos: Visual Studio 2026 (o 2022) con C++ de escritorio y de UWP, Windows SDK 10.0.26100 y CMake 3.21 o superior.

> Clona el repositorio en una ruta **sin espacios** (por ejemplo, `C:\dev\CEMU-XBOX`). Algunos ports de vcpkg fallan con espacios en la ruta.

```powershell
git clone --recursive https://github.com/Arthur1606/CEMU-XBOX C:\dev\CEMU-XBOX
cd C:\dev\CEMU-XBOX

cmake -S Cemu -B Cemu\build-msvc-d3d11 -G "Visual Studio 18 2026" -A x64 `
  -DCEMU_UWP=ON -DENABLE_D3D11=ON -DENABLE_D3D12=ON `
  -DSDL3_UWP_SOURCE_DIR="$PWD\SDL3-uwp"
cmake --build Cemu\build-msvc-d3d11 --config Release --target CemuBin --parallel

msbuild Cemu-UWP-Host\Cemu-UWP-Host.vcxproj /restore /m /p:Configuration=Release /p:Platform=x64
```

Con Visual Studio 2022, usa `-G "Visual Studio 17 2022"` y añade `/p:PlatformToolset=v143` a msbuild.

## Qué contiene

| Carpeta | Origen | Licencia |
| --- | --- | --- |
| [`Cemu/`](Cemu/) | [rodrigoandrigo/Cemu](https://github.com/rodrigoandrigo/Cemu) (rama `build-msvc-d3d11`), fork de [cemu-project/Cemu](https://github.com/cemu-project/Cemu). Núcleo del emulador con renderer D3D11 y la ABI `CemuEmbed` | [MPL-2.0](Cemu/LICENSE.txt) |
| [`Cemu-UWP-Host/`](Cemu-UWP-Host/) | [rodrigoandrigo/Cemu-UWP-Host](https://github.com/rodrigoandrigo/Cemu-UWP-Host). Aplicación UWP: interfaz, mandos, almacenamiento y swap chain | [Apache-2.0](Cemu-UWP-Host/LICENSE) |
| `SDL3-uwp/` (submódulo) | [rodrigoandrigo/SDL3_UWP](https://github.com/rodrigoandrigo/SDL3_UWP). SDL3 con backend Windows.Gaming.Input | zlib |

Ambos proyectos se incorporaron con `git subtree`, así que su historial y su autoría originales se conservan. Los archivos de cada carpeta se distribuyen bajo su licencia original; si redistribuyes el código o los binarios, mantén esos avisos.

## Cambios respecto a los originales

- **Compilación del host:** declarado `ReplaceD3D11Surface` (usado pero no declarado) e implementado `DeviceResources::DetachSwapChainPanel` (llamado pero inexistente).
- **Suspensión y reanudación:** al suspender la app (Guía, Inicio, reposo) ya no se destruye la instancia de Cemu. El runtime admite una sola instancia por proceso, así que antes la partida quedaba inutilizable hasta reiniciar.
- **JIT (recompilador PPC):**
  - el manifiesto declara `codeGeneration`;
  - en UWP, si `VirtualAlloc` rechaza páginas ejecutables, se reintenta con `VirtualAlloc2FromApp`;
  - si no hay memoria ejecutable, la función se ejecuta con el intérprete en vez de escribir sobre un puntero nulo.
- **Pantalla del GamePad:** nueva función `CemuEmbed_ToggleGamePadScreen` y atajo `Vista + R3`.
- **CI:** [`.github/workflows/build-xbox.yml`](.github/workflows/build-xbox.yml) compila el runtime y el paquete UWP en Windows y publica el `.msixbundle`.

## Pendiente

- **Renderer:** el backend D3D11 no cubre todo (geometry shaders arbitrarios, stream output, formatos poco comunes). El backend D3D12 está en el núcleo, pero el host todavía no lo ofrece.
- **Varios mandos:** solo se envía el mando del jugador 1.
- **Giroscopio, Amiibo/NFC y vibración:** sin implementar.
- **Perfiles por juego:** se empaquetan, pero no se pueden editar desde la interfaz.
