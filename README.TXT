if you decided to try to play the game you should
add the png and the wav and the ttf files to your
program and change the loading stuff with your 
own path 

this is a linker  :  
g++ -c SuperMario.cpp -I"SFML-2.6.1\include" -I"C:\Program Files (x86)\OpenAL 1.1 SDK\include" -DSFML_STATIC


and this is a compiler :  
g++ SuperMario.o -o SuperMario -L"SFML-2.6.1\lib" -L"C:\Program Files (x86)\OpenAL 1.1 SDK\libs" -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio "C:\Program Files (x86)\OpenAL 1.1 SDK\libs\OpenAL32.lib" -lopengl32 -lfreetype -lwinmm -lgdi32 -mwindows 

and you should have the sfml dlls instaled in 
your computer  and edit the path of the linker 
and the compiler considering your own path 


and if you want to play the game directly
without all of this you can follow this link it
will take you to a set_up file where you can download
the game :
https://drive.google.com/drive/folders/1sJT1ZfmP2Gn0w0oqWdwAkt10fH2qpo3d?usp=sharing


