# spiele\

\![plan](https://github.com/momefilo/spiele/assets/106985138/b978ef56-136b-41f7-9567-b7b82bf22d3c)
\![IMG_20240224_152710_HDR](https://github.com/momefilo/spiele/assets/106985138/7a2d6fe8-1b43-4b3c-8723-8d83b51fd97c)
\\![Uploading IMG_20240224_152922_HDR.jpg…]()

\![Uploading IMG_20240224_153044_HDR.jpg…]()
\![Uploading IMG_20240224_153110_HDR.jpg…]()

Tetris, Klotski und Snake für ili9341, passiv-Buzzer und vier Tastern\
auf../libs aufbauend\
Spiele benoigt Grafikdateien die aufgrund ihrer Groesse nicht simpel mit einem Programm in den Flash geschrieben werden koennen.\
Es muessen nacheinander vier Programme auf den Pico kopiert werden um alle Daten auf den Flash des Pico zu bekommen \
Kopieren sie per dag und drop nacheinander \
mit flash_spiele0.uf2 beginnend \
dann flash_spiele1.uf2 \
dann flash_spiele3.uf2 \
und spiele.uf2 endend auf den Pico um Tetris, Klotski und Snake auf dem Pico spielen zu koennen.\

Die Datei "yd_rp2040.h" ist eine Board-Spezifikationsdatei für dieses Program
und sollte nach pico-sdk/src/boards/include/boards/ kopiert werden, \
ansonsten sind alle "CMakeLists.txt" Dateien anzupassen.
