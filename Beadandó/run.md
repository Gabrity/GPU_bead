# Fordítás és futtatás

## 1. Fejlesztői környezet betöltése
Minden új PowerShell-ablakban egyszer (a `cl`, `cmake`, `ninja` csak így érhető el):

```powershell
Set-ExecutionPolicy -Scope Process -ExecutionPolicy Bypass -Force
& "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\Common7\Tools\Launch-VsDevShell.ps1" -Arch amd64 -SkipAutomaticLocation
```

## 2. Fordítás
```powershell
cd C:\Users\komba\Desktop\GPU_bead\Beadandó
cmake -S . -B build -G Ninja
cmake --build build
```
A konfigurálást (`cmake -S . -B build ...`) elég egyszer lefuttatni, utána a `cmake --build build` újrafordítja a módosult fájlokat.

## 3. Futtatás
Munkakönyvtár: `Beadandó\`

```powershell
.\build\contours.exe .\heightmaps_for_contours\hdata2.txt
.\build\contours.exe .\heightmaps_for_contours\hdata.txt
```

Várt kimenet: a térkép mérete, min/max magassága és a beolvasási idő.
- `hdata2.txt`: 250 x 250, 769.488–1791.370 m
- `hdata.txt`: 1650 x 2244, 437.524–1791.610 m

## Hibaelhárítás
- Ha az `nvcc` nem található, nyiss új ablakot, vagy töltsd be a PATH-ot az adott ablakba:
  ```powershell
  $env:Path += ';' + [Environment]::GetEnvironmentVariable('Path','Machine')
  ```
- Ha a CMake a `build` mappában régi beállításokra panaszkodik, töröld a `build` mappát, és konfiguráld újra.
