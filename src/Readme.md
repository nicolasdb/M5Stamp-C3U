Readme

Donc, je peux découper en 3 principales fonctions à interconnecter. 
Sachant que la gestion du SPIFFS ne fonctionne pas ici dans PIO, donc il faut ensuite passer par Arduino IDE pour l'upload. 

1. Timer > analogREAD > append to log.txt on SPIFFS
2. on PushButton > print last log on OLED
3. Access to or publish log.txt on webserver or Bluetooth. 


En utilisant FreeRTOS, je règle le problème de timer et chaques fonctions peuvent être définie comme une "task", mais c'est probablement overkill pour un tel projet et je ne sais pas comment le serveurWEB réagira.... 

### First: 
follow this one https://randomnerdtutorials.com/esp32-esp8266-plot-chart-web-server/

note: this will request sensor reading from the html page.... donc pas de sauvegarde d'un log long terme.... sauf si le site tourne en background?? Mais ça serait plus logique d'écrire les données dans un log.txt

https://techexplorations.com/blog/esp32/blog-esp32-how-to-retrieve-a-file-from-the-spiffs/

### then, 
how to create a wifi access point? instead of connecting to local wifi. 
ah! 
https://randomnerdtutorials.com/esp32-access-point-ap-web-server/