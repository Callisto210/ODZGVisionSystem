## OZDGVisionSystem
---
### Requirements

* ffmpeg ver. 3 https://ffmpeg.org
* pistache https://pistache.io
* GTest https://github.com/google/googletest
* spdlog https://github.com/gabime/spdlog
* CMake (3.0) for building pistache
* Autotools for building ffmpeg
* nasm/yasm - requirement for compiling ffmpeg
* C/C++ compiler with C++14 support (tested on gcc)
* GNU Make
* HTTP server like nginx or apache for serving static files


## Opis developerski ODZGVisionSystem
---
### Budowanie projektu
 Domyślnie powinno wystarczyć uruchomienie komendą `make`.  
 *CMake nie jest w pełni skonfigurowany*  
 Najważniejsze zależności w formie bibliotek są załączone jako submoduły w repozytorium.
 Aby je pobrać, należy wykonać komendę `git submodule update --init` w katalogu projektu.
 Przykładowa instrukcja budowania ffmpeg:  
 ```bash
 cd lib/ffmpeg && ./configure \
 	--disable-programs --enable-gpl --libdir=../../bin/lib/ffmpeg \
 	--shlibdir=../../bin/lib/ffmpeg \
 	--enable-shared --enable-avresample --enable-libx264 --enable-libx265
```
**!Uwaga: projekt po zlinkowaniu zwykle wymaga podania ścieżki bibliotek z paczki ffmpeg.
Aby je dostarczyć, należy ustawić zmienną `LD_LIBRARY_PATH`.**  

Przykładowe uruchomienie z katalogu głównego:
`LD_LIBRARY_PATH=./bin/lib/ffmpeg ./bin/rest`.

Serwer backendowy uruchamia się na porcie `8090`. Frontend jest dostarczony za pomocą
statycznych plików umieszczonych w katalogu `data/html`.

#### Przykładowa konfiguracja serwera HTTP nginx dla łączenia frontendu i backendu.

**Uwaga: Konfiguracja jest prosta na potrzeby developerskie. Nie powinna być użyta w środowisku
produkcyjnym.**

W pliku konfiguracyjnym nginx-a tworzymy blok server:

```nginx
server {
  listen 80;
  server_name nazwa_własna;

  access_log /ścieżka/do/logów;
  error_log /ścieżka/do/logów warn;

  location ~ \.(html|css|js) {
    root_directory /Ścieżka/do/projektu/data/html/;  
  }
  location /rest {
    proxy_pass http://localhost:8090/;
  }
}
```

Struktura katalogów:
```
├── bin        //katalog wyjściowy dla plików
│   └── lib    // Biblioteki w formacie *.so
├── build      //katalog tymczasowy na pliki pośrednie
├── data       // pliki statyczne
│   └── html   // pliki statyczne frontendu
├── docs       // Dokumentacja - jeszcze pusty
├── include    // zebrane linki do plików nagłówkowych
├── lib        // repozytoria z zależnymi bibliotekami
│   ├── ffmpeg
│   ├── pistache
│   └── spdlog
├── src        // źródła projektu
└── tests      // Przewidziany na testy, np. jednostkowe

```
