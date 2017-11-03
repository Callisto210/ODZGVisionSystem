## ODZGVisionSystem
---
### Requirements

* Gstreamer-1.0 >= 1.12 (gstreamer, gstreamer-video, gstreamer-audio)
* pistache https://pistache.io
* GTest https://github.com/google/googletest
* spdlog https://github.com/gabime/spdlog
* rapidJSON - C++ json library
* jsmn - C json library
* CMake (3.5.1) for building project
* Autotools for building dependencies (gstreamer libraries)
* C/C++ compiler with C++14 support (tested on gcc)
* GNU Make
* HTTP server like nginx or apache for serving static files



## Opis developerski ODZGVisionSystem
---
### Budowanie projektu
 Projekt budujemy przy pomocy cmake dwuetapowo:
 
 Etap 1:
 * Utwórz katalog na biblioteki/zależności
 * Wykonaj cmake dla katalogu lib
 * Zbuduj i zainstaluj potrzebne biblioteki
 
 np. 
 
 ```bash 
    mkdir cmake-build-lib
    cd cmake-build-lib
    cmake /path/to/project/lib -DCMAKE_INSTALL_DIR=/path/to/install 
    # Opcjonalny parametr -DCORES
    # make all wywołuje od razu instalację zależności z autotoolsów, stąd sudo
    [sudo] make all 
    [sudo] make install
 ```
 Projekt zbuduje tylko te zależności, które nie zostały znalezione.
 Pozostałe mogą być pominięte.
 
 Etap 2:
 * Utwórz katalog do budowania
 * Wykonaj cmake dla katalogu głównego
 * Zbuduj projekt komendą make
 
 np.
 
 ```bash
    mkdir cmake-build
    cd cmake-build
    cmake /path/to/project -DCMAKE_INSTALL_DIR=/path/to/target
    make all -j$(nproc)
    [sudo] make install
```
    
    
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
├── data       // pliki statyczne
│   └── html   // pliki statyczne frontendu
├── docs       // Dokumentacja - jeszcze pusty
├── include    // zebrane linki do plików nagłówkowych (nie wszystkie)
├── lib        // repozytoria z zależnymi bibliotekami
│   ├── gstreamer
│   ├── gst-plugins-base
│   ├── gst-plugins-good
│   ├── jsmn
│   ├── pistache
│   └── spdlog
├── src        // źródła projektu
└── tests      // Przewidziany na testy, np. jednostkowe

```
