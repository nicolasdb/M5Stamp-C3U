## This is an example project for M5Stamp C3U on PlatformIO

Project forked to explore how to use this board.   

### This program
`main.cpp` : This code will read the state of 2 push buttons, then control the rotation of a stepper.  
I use stepper nema17 + driver A4988


My goal is to make something like the cheese game. 
![image](https://user-images.githubusercontent.com/12049360/195614139-9ec97789-3e6b-4e2f-af12-52e6a6ae80e8.png)


> For ARDUINO IDE: You can copy/paste the code from `main.cpp` 
> Don't forget to install libraries. 

#### Notes:
- I'm using freeRTOS tasks. The game need to be reactive and I want to pratice freeRTOS ^^

For more, check [issue #11](https://github.com/nicolasdb/M5Stamp-C3U/issues/11)