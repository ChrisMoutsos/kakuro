###Introduction
This is an application designed for Kakuro puzzles. Kakuro is a Japanese puzzle game that is similar to Sudoku. It is sometimes called a mathematical version of the crossword puzzle. The goal is to fill in each entry with some combination of digits 1-9 (inclusive, with no duplicate digits allowed) so that the sum of each group is equal to the 'clue' associated with the group. 

###Overview
Basic controls: use your moouse, arrow keys, or WASD to select a cell. Use number keys 1-9 to set the cells value (or -/+ to decrement/increment). To write notes on a cell, right click (or press 'n') and type the notes. To clear a column/row group, right click the clue for the group.

You can play puzzles yourself, or get the program to show you the solution to a puzzle. There is also a generator for generating a random board (guaranteed to have a unique solution).

Saving and loading puzzles is supported, using .KAK files (text files with a pre-defined format to identify a Kakuro board. Read more about KAKs in the file 'mainwindow.h', especially if you wish to manually load a puzzle).

The size of the board is adjustable in the settings menu, under the 'cell size' label. Also in the settings menu are options to enter hex values to change the colors of the board.

The icons are from @Templarian's [Modern UI Icons](https://github.com/Templarian/WindowsIcons) project.

####Screenshots

![Screenshot1](http://i.imgur.com/5KAfq2r.png)
![Screenshot2](http://i.imgur.com/EngWyDW.png)
