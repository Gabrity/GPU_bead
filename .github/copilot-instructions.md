# Copilot instrukciók – GPU beadandó (ELTE IK, GPGPU)

> Élő dokumentum: minden új, a projekthez hasznos információt ide kell felvezetni (döntések, mérések, hibák, tanulságok). A felhasználó magyarul kommunikál; válaszolj magyarul, a kódkommentek és azonosítók lehetnek angolul.

## 1. Kontextus
- Kurzus: ELTE IK GPGPU. Oktató: Ivan Eichhardt (Teams), Vad Viktor (vad@inf.elte.hu). Kurzusoldal: cv.inf.elte.hu
- Választott feladat: **11. Contour lines on the GPU** (max 85%) – magassági térkép izovonalai marching squares algoritmussal GPU-n.
- **Jelenlegi cél:** a beadandó megértése beugró vizsgára (szóbeli, váratlan kérdések). A prezentáció kész és külön van kezelve (`Prezentáció/`), itt nem foglalkozunk vele.
- Egyéni munka (nincs társ megadva). Plágium = 0%.

## 2. Feladat leírása (11-es feladat, kivonat a PDF-ből)
- Bemenet: magassági térkép szövegfájlban (szóköz = értékek elválasztója, **dupla újsor** = sorok elválasztója), méterben. Mintafájlok: http://u235axe.web.elte.hu/GPUCourse/Contours/
- Algoritmus: **marching squares** (https://en.wikipedia.org/wiki/Marching_squares); használjunk **shared memory-t és textúrákat** a gyorsításhoz.
- Kimenet: magassági térkép **magasság szerint színezve**, rajta az izovonalak szakaszokként, **50 m egész többszöröseinél**. Statikus kép (png/jpeg, pl. `stb_image_write.h`) VAGY OpenGL képernyőkimenet kötelező.
- Pontozás: helyes marching squares (30%) + kép kimenet (+10%) + OpenGL kimenet (+15%).
- Kiegészítő: nyílt/zárt **Bézier-görbék** a szakaszok simítására. Ehhez sávdiagonális (tridiagonális/ciklikus) egyenletrendszert kell megoldani: CUDA könyvtárral +5%, **saját párhuzamos solver +15%**; a Bézier **valódi rasterizálása** (nem sűrű szakaszokkal közelítés) +10%.
- Források: codeproject "Draw a Smooth Curve through a Set of 2D Points with Bezier Primitives" és "Draw Closed Smooth Curve with Bezier Spline"; Bresenham-féle görbe rasterizálás: http://members.chello.at/easyfilter/bresenham.pdf
- A feladat plafonja 85%. A felsorolt tételek összege (30+10+15 alap, +15 saját solver, +10 rasterizálás) 80%; a maradék 5% forrása a PDF-ben nem szerepel egyértelműen (a könyvtáras solver +5% alternatíva a sajáthoz).

## 3. Általános minimum elfogadási feltételek (a PDF 1. oldalából)
1. **GIT**: iteratív fejlesztést tükröző history; nem fogadnak el egyetlen "final" commitot / kevés nagy commitot. Gyakori, kicsi, beszédes commitok kellenek.
2. **Technikai dokumentáció** (riport): tervezési szempontok és optimalizációs döntések indoklása.
3. **Teljesítményelemzés**: tesztelési eredmények, **grafikonok** több tesztesetre.
- Jegyek: 0–50% = 1, (50,60] = 2, (60,70] = 3, (70,80] = 4, 80%+ = 5. Az órai pontok százalékban hozzáadódnak.

### Opcionális extra pontok
- Korai félévi 5 perces prezentáció a párhuzamos algoritmus-/designtervekről (~10%).
- Tiszta, jól kommentezett kód (+2%).
- Shared memory + work group + szinkronizáció helyes használata (+3%); minimum: összehasonlítás naiv megoldással.
- Részletes dokumentáció (+2%): shared mem, work group, memóriahatékonyság, cache-barátság, work-efficiency, párhuzamos adatszerkezetek, streaming; minden mérföldkőnél indoklás + inkrementális teljesítmény-összehasonlítás.
- Platformfüggetlenség: több OS (+3%), több compute eszköz (+2%).
- Multi-GPU / multi-device (utóbbi csak OpenCL) (+5%).
- CPU-összehasonlítás: szekvenciális (+1%), többszálú (+1%), harmadik fél CPU/GPU implementációja (+1%).

## 4. Környezet (felmérve 2026-11-22)
- OS: Windows, munkakönyvtár: `C:\Users\komba\Desktop\GPU_bead`. Shell: Windows PowerShell 5 szintaxis (nincs `&&`, `||`).
- GPU: **NVIDIA GeForce RTX 2060** (compute capability 7.5, 30 SM, 6 GiB, driver 621.24) + AMD Radeon iGPU. **CUDA a választott API.**
- Telepítve: **VS 2022 Build Tools** (MSVC 14.44, CMake 3.31 + Ninja a VS-ben) és **CUDA Toolkit 13.5** (winget `Nvidia.CUDA`, rendszergazdai PowerShellből telepíthető; az UAC a háttérfolyamatban nem jelenik meg).
- A `cl`/`cmake`/`ninja` csak a VS Developer környezetben van a PATH-on. Új ablakban:
  ```powershell
  Set-ExecutionPolicy -Scope Process -ExecutionPolicy Bypass -Force
  & "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\Common7\Tools\Launch-VsDevShell.ps1" -Arch amd64 -SkipAutomaticLocation
  ```
  (a `.ps1` futtatása alapból tiltott → a `Process` scope-os Bypass kell). Ha az `nvcc` nem található, a Machine/User PATH-ot újra be kell tölteni, vagy új ablakot nyitni.
- Build: `cd Beadandó; cmake -S . -B build -G Ninja; cmake --build build` (a `CMAKE_CUDA_ARCHITECTURES` alapértelmezése `native`, így a célgép GPU-jára fordul).
- **Nincs Python** a gépen (a korábbi `pypdf`-es jegyzet elavult); PDF-olvasáshoz külön kell telepíteni, ha szükséges.
- A repóban 1 commit van (`repo init`), `main` ág.
## 5. PDF-ek olvasása
- A PDF-ek olvashatók: `python -m pip install pypdf`, majd `PdfReader(...).pages[i].extract_text(extraction_mode='layout')` (az alapmód szavanként tördel; a `layout` mód olvasható). `PYTHONIOENCODING=utf-8` kell a magyar kimenethez.
- A kinyert szöveget ideiglenesen a `%TEMP%` alá tedd, ne a repóba.
- Kurzusanyagok az `Órai anyag/` mappában (gitignore-olva): `GPGPU homework assignments.pdf` (feladatok), `EA diák/` (GPGPU Arch 01–07: számábrázolás, numerika, CPU, GPU architektúrák, AMD RDNA, NVIDIA generációk, Ampere; EA1-2, 2_ELTE_IK_GPU_Ea_3).

## 6. Repó szerkezet (jelenlegi / tervezett)
- `Prezentáció/` – a kész prezentáció és a jegyzetek (lezárva).
- `Beadandó/` – a megoldás: `CMakeLists.txt`, `src/` (`heightmap.h/.cpp` beolvasó, `main.cpp`, `hello.cu`), `heightmaps_for_contours/` (hdata.txt 1650×2244 – 29 MB, hdata2.txt 250×250), `stb_image_write.h`.
- `Órai anyag/` – kurzusanyag, **gitignore-olt**.
- `.gitignore` – C/C++ szabályok + `Órai anyag/` + ez az instrukciós fájl.
- Tervezett: `Beadandó/src/` (host + kernelek), `Beadandó/data/` (letöltött térképek, gitignore: `data/*.txt`), `Beadandó/docs/` (riport, grafikonok), `Beadandó/bench/` (mérőscriptek).

## 7. Munkamódszer / konvenciók
- Gyakori, kis commitok, egyértelmű üzenettel (a history maga követelmény). Commit trailer: `Co-authored-by: Copilot <223556219+Copilot@users.noreply.github.com>`.
- Minden optimalizációs lépésnél: naiv verzió → optimalizált verzió, mérés (idő, GB/s), indoklás. A méréseket mentsd CSV-be a grafikonokhoz.
- Kódban csak a nem triviális részeket kommentezd, de a kód legyen tiszta (extra pont).
- A kernel-implementáció legyen saját; külső könyvtár csak I/O-ra, ablakkezelésre és (opcionálisan) a Bézier-solver összehasonlítására.
- Ne commitolj titkot, nagy bináris adatot, a kurzusanyagot.

## 8. Nyitott kérdések / döntések
- [x] API: **CUDA** (döntés: felhasználó, 2026-11-09; a célgépen NVIDIA GPU van). Az Intel-gépen nem futtatható, csak szerkesztésre használjuk.
- [ ] Kimenet: PNG elsőként, OpenGL (OpenCL–GL interop) másodikként.
- [ ] Bézier rész: **kötelező a projektben** (felhasználó döntése, 2026-11-09), nem opcionális. Láncolás + tridiagonális solver + Bézier-rasterizálás, lásd 10. szekció.
- [x] Teszt adatok: `Beadandó/heightmaps_for_contours/`. **Formátum:** hdata2.txt soronként 1 CRLF, hdata.txt dupla CRLF a sorok között → a beolvasó az üres sorokat kihagyja, a szélességet az első nem üres sorból veszi, egyenetlen sort hibának jelez. hdata: 1650×2244, 437.5–1791.6 m (beolvasás ~0.76 s); hdata2: 250×250, 769.5–1791.4 m.

## 10. Teljes algoritmus (tisztázott, kötelező sorrend)
1. **Beolvasás (CPU):** szöveg → `h[]` float tömb (`H·W`), GPU-ra másolás.
2. **Count (GPU):** cellánként a 4 sarokból case, szintenként hány szakasz → `count[]`.
3. **Scan (GPU):** `count[]` → `offset[]` és az összméret (`N`).
4. **Write (GPU):** cellánként a szakaszok `(x1,y1,x2,y2)` koordinátái → `segments[N]`; minden szakasz él-azonosítókat is kap (`edgeA`, `edgeB`), a láncoláshoz.
5. **Láncolás (GPU vagy CPU):** a szakaszokat közös él-azonosító alapján összekötjük; minden szintvonal **láncok** halmaza (zárt vagy nyílt), a láncok pontjainak rendezett listája.
6. **Bézier-vezérlőpontok (GPU):** lánconként a pontokon átmenő sima görbe vezérlőpontjai; ehhez tridiagonális (nyílt lánc) vagy ciklikus tridiagonális (zárt lánc) egyenletrendszer megoldása. Párhuzamosítás: cyclic reduction vagy PCR.
7. **Háttér (GPU):** pixelenként magasság (bilineáris, lásd fent) → színskála → `rgb[]`.
8. **Vonalak (GPU):** nem sűrű szakaszokkal, hanem a Bézier-görbe valódi rasterizálásával (Bresenham-jellegű görbe-raszterizáló, forrás: easyfilter `bresenham.pdf`) a `rgb[]` fölé.
9. **Kimenet (CPU):** PNG mentés, vagy OpenGL-megjelenítés (`segments`/görbe pontok vertex bufferben).

*Megjegyzés:* a 4. lépés szakaszai és az 5. láncai ugyanazok az adatok, csak más rendezésben. A szakasz-lista csak az alap marching squares-hez (30%+10%) elég, a 5–8. lépés a Bézier-bővítés (+15% solver + 10% rasterizálás + láncok).

