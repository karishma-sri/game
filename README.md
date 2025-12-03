Lane Dodger v3 - Windows Console Game

Version: 3
Platform: Windows (Console)
Language: C

Overview
Lane Dodger v3 is a simple and fun console-based game where the player controls a car to avoid oncoming obstacles. The game features multiple lanes, adjustable difficulty levels, scoring, lives, and a persistent highscore. The game is implemented entirely in C using ASCII graphics.

Features
ASCII car representation with 3-line sprite.
Multiple lanes (default: 5 lanes).
Multiple obstacles (up to 12 simultaneously).

Adjustable difficulty:
Easy: relaxed spawn, slower speed
Medium: normal
Hard: faster spawn and speed

Score and level system:
Score increases for each dodged obstacle
Level increases every 7 points, increasing speed and spawn rate
Lives system: player has 3 lives; collision reduces one life
Highscore system: saves highscore in highscore.txt
Pause functionality (P key)
Quit to menu (Q key)

Controls
Left Arrow: Move car left
Right Arrow: Move car right
P: Pause/resume game
Q: Quit to main menu
R (after game over): Restart game at the same difficulty

How to Run
Compile the game using a Windows-compatible C compiler (e.g., gcc in MinGW or Dev-C++):
gcc lane_dodger_v3.c -o lane_dodger_v3.exe


Run the executable:
lane_dodger_v3.exe
Select a difficulty level from the main menu to start playing.

Game Mechanics
Obstacles spawn randomly in any lane.
The car occupies the bottom three rows of the screen.
Collision detection is lane-based.
Each obstacle successfully dodged increases the score by 1.
The game speed and spawn rate increase as levels progress.
Game ends when all lives are lost.
File Structure
lane_dodger_v3.c: main game source code
highscore.txt: stores the persistent highscore (created automatically)

Notes
The game uses Windows-specific headers: <conio.h> and <windows.h>. Not compatible with Linux/macOS without modifications.
ASCII graphics designed for a standard console width; resizing the console may affect display.
Beep sounds require the Windows console.
Future Enhancements
Different obstacle types with unique behaviors
Power-ups or bonuses
More sophisticated graphics using extended ASCII
Highscore table with multiple players
