/*
 * puzzleboard.cpp
 * See puzzleboard.h for more information
 */

#include "puzzleboard.h"
#include <QPainter>
#include <QPixmap>
#include <QString>
#include <QDebug>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QTimer>
#include <cstdlib>
#include <ctime>

//Helper functions for reading KAKstrings
unsigned int getUInt(QString & s) {
    unsigned int x;

    if (!s.size()) return 0;

    if (s[0] < '0' || s[0] > '9')
        return 0;

    x = s[0].digitValue();
    s.remove(0, 1);
    while (s.size() && s[0].digitValue() != -1) {
        x *= 10;
        x += s[0].digitValue();
        s.remove(0, 1);
    }
    return x;
}

void removeSpaces(QString & s) {
    while (s.size() && s[0] == ' ') {
        s.remove(0, 1);
    }
}

//Used for board generation
void floodFill(int index, QVector<bool> & filled, QVector<bool> map, int rows, int cols);

PuzzleBoard::PuzzleBoard(QString s) {
    srand(time(NULL));
    this->setMouseTracking(true);

    seconds = 0;
    selectedCell = draggingCell = { -1, -1 };
    cellSize = 50;
    cellArray = 0;
    gridLayout = 0;
    rows = cols = 1;

    //Set colors default
    colors[CLUECOLOR] = new QColor(0, 0, 0, 255);
    colors[NONCLUECOLOR] = new QColor(255, 255, 255, 255);
    colors[CLUETEXTCOLOR] = new QColor(15, 15, 15, 255);
    colors[NONCLUETEXTCOLOR] = new QColor(0, 0, 0, 255);
    colors[NOTECOLOR] = new QColor(95, 95, 95, 255);
    colors[SELECTCOLOR] = new QColor(121, 213, 252, 100);
    colors[BORDERCOLOR] = new QColor(0, 0, 0, 255);

    //Make the board
    makeBoardFromKAKString(s);

    //Initialize sumInNumCombos[SUM][NUM] with combinations
    initSumInNumCombos();
}

PuzzleBoard::~PuzzleBoard() {
    for (int r = 0; r < rows; r++) {
        delete [] cellArray[r];
    }
    delete [] cellArray;
}

void PuzzleBoard::makeNewCellArray(int newRows, int newCols) {
    if (rows == newRows && cols == newCols)
        return;

    //Delete old cellArray
    if (cellArray) {
        for (int r = 0; r < rows; r++)
            delete [] cellArray[r];
        delete [] cellArray;
    }
    //Delete old layout
    if (gridLayout) {
        delete gridLayout;
    }
    //Create new layout
    gridLayout = new QGridLayout;
    gridLayout->setSpacing(0);
    gridLayout->setContentsMargins(0, 0, 0, 0);
    setLayout(gridLayout);

    //Create new cellArray
    rows = newRows;
    cols = newCols;
    cellArray = new Cell*[rows];
    for (int r = 0; r < rows; r++) {
        cellArray[r] = new Cell[cols];
    }
    //Put each cell on the grid,
    //update its settings, and draw it
    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            cellArray[r][c].setSize(cellSize);
            cellArray[r][c].setRowCol(r, c);
            cellArray[r][c].makePixmap();
            for (int i = 0; i < 7; i++) {
                cellArray[r][c].setColor(i, colors[i]);
            }
            gridLayout->addWidget(cellArray[r][c].getLabel(), r, c);
        }
    }
}

void PuzzleBoard::setColor(int whichColor, QColor *color) {
    colors[whichColor] = color;
    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            cellArray[r][c].setColor(whichColor, colors[whichColor]);
        }
    }
}

void PuzzleBoard::initSumInNumCombos() {
    bool done;
    int summy;

    for (int NUM = 0; NUM <= 9; NUM++) {
        for (int SUM = 0; SUM <= 45; SUM++) {
            if (NUM == 0 || SUM == 0)
                continue;

            if (9*NUM < SUM)
                continue;
            if (NUM > SUM)
                continue;

            //digits will hold 1, 2, .. , NUM
            //and will always be ascending order.
            //We will count up, carrying over when necessary.
            //This guarantees check of each combo
            //of NUM size with digits 1-9,
            //with the added ease of no repeat digits
            QVector<int> digits;
            for (int i = 1; i <= NUM; i++) {
                digits.push_back(i);
            }
            //Decrease last digit because we will
            //increase it at the start of the while loop
            digits[NUM-1]--;

            done = false;
            while (!done) {

                //PHASE ONE
                //Increase to the next combo-set, with carry if needed

                //Get safe digit to increase
                int index = (NUM-1);
                while (index >= 0 && digits[index] >= 9-((NUM-1)-index)) {
                    index--;
                }
                //If there was no safe digit,
                //we've hit every combination and can break
                if (index < 0) {
                    done = true;
                    break;
                }
                //Increase the digit
                digits[index]++;
                //Put the rest of the set in increasing order
                for (int i = index+1; i < NUM; i++) {
                    digits[i] = digits[i-1]+1;
                }


                //Phase TWO
                //Check if it's a valid como-set

                //Get sum of all of digits
                summy = 0;
                for (int i = 0; i < int(digits.size()); i++) {
                    summy += digits[i];
                }

                //If it's correct,
                if (summy == SUM) {
                    //..add to final vector, if it's unique

                    bool dup = false;
                    //For every set already found that solves SUM in NUM
                    for (int i = 0; i < int(sumInNumCombo[SUM][NUM].size()); i++) {
                        //If the sets aren't the same size, they're not the same
                        if (sumInNumCombo[SUM][NUM][i].size() != digits.size()) {
                            continue;
                        }
                        //Check each number in the set and digits,
                        //and if they're different, they're different sets
                        for (int j = 0; j < int(digits.size()); j++) {
                            if (sumInNumCombo[SUM][NUM][i][j] != digits[j]) {
                                dup = false;
                                break;
                            }
                        }
                        if (!dup) break;
                    }

                    //Made it this far?
                    //It's a new combo-set, so add it to final vector
                    if (!dup) {
                        sumInNumCombo[SUM][NUM].push_back(digits);
                    }
                }

            } //end while(!done)
        } //end for SUM
    } //end for NUM

    for (int NUM = 0; NUM <= 9; NUM++) {
        for (int SUM = 0; SUM <= 45; SUM++) {
            sumInNumIsUnique[SUM][NUM] = (sumInNumCombo[SUM][NUM].size() == 1);
        }
    }

}

bool PuzzleBoard::lazyValidateKAKString(QString s) const {
    //Minimum KAK: 1x1 1 -
    if (s.size() < 8)
        return false;

    //Make sure it doesn't contain illegal characters
    for (char c = '!'; c < '~'; c++) {
        if (c == '(' || c == ')' || c == ':' ||
                c == '-' || (c >= '0' && c <= '9') ||
                c == 'x' || c == '/' || c == 'n' ||
                c == 't' || c == 'f') {
            continue;
        }
        if (s.contains(c))
            return false;
    }

    //Make sure it starts with NUMxNUM
    int r, c;
    r = getUInt(s);
    if (s[0] != 'x')
        return false;
    s.remove(0, 1);
    c = getUInt(s);
    if (!(r && c))
        return false;

    return true;
}

void PuzzleBoard::setCellsInfo(QString s) {
    //Set the board size
    rows = getUInt(s);
    //Remove the x
    s.remove(0, 1);
    cols = getUInt(s);
    removeSpaces(s);
    cellSize = getUInt(s);
    removeSpaces(s);

    int v;
    CellInfo c;

    cellsInfo.clear();

    //Get cells until we have enough,
    //we hit the time 't',
    //or run out of KAKString to read from
    while (s.size() && s[0] != 't' && rows*cols != cellsInfo.size()) {

        //Clear our CellInfo c
        for (int i = 0 ; i < 10; i++) {
            c.notes[i] = 0;
        }
        c.valueOrClues[0] = c.valueOrClues[1] = 0;
        c.fixed = false;

        //Empty clue
        if (s[0] == '-') {
            s.remove(0, 1);
            c.type = CLUE;
            cellsInfo.push_back(c);
        }
        //Non-empty clue, or nonclue
        else if (s[0] != ' ') {
            //Get number
            v = getUInt(s);

            //Non-empty clue
            if (s[0] == '/') {
                c.type = CLUE;
                c.valueOrClues[0] = v;
                //Remove slash
                s.remove(0, 1);
                //Get right clue
                c.valueOrClues[1] = getUInt(s);

                cellsInfo.push_back(c);
            }
            //Nonclue
            else if (s.size() == 0 || s[0] == ' ' || s[0] == 'n' || s[0] == 'f') {
                c.type = NONCLUE;

                c.valueOrClues[0] = v;

                //If there are notes, get them
                if (s[0] == 'n') {
                    //Remove the 'n'
                    s.remove(0, 1);

                    //Set c.notes
                    while (s.size() && s[0] != ' ') {
                        v = s[0].digitValue();
                        s.remove(0, 1);
                        c.notes[v] = 1;
                    }
                }
                else if (s[0] == 'f') {
                    //Remove the 'f'
                    s.remove(0, 1);
                    c.fixed = true;
                }

                cellsInfo.push_back(c);
            }
        }
        if (s.size()) {
            removeSpaces(s);
        }
    }

    removeSpaces(s);

    //Get the time
    if (s.size() && s[0] == 't') {
        s.remove(0, 1);
        seconds = 3600*getUInt(s);
        s.remove(0, 1);
        seconds += 60*getUInt(s);
        s.remove(0, 1);
        seconds += getUInt(s);
    }

    //If we have too many or too little,
    //do some dirty adjustment so the program
    //doesn't clear the user's boot sector
    if (rows*cols != cellsInfo.size()) {
        //Add empty cells
        if (rows*cols > cellsInfo.size()) {
            CellInfo c;
            c.type = NONCLUE;
            c.valueOrClues[0] = 0;
            for (int i = 0; i < 10; i++)
                c.notes[i] = 0;
            for (int i = cellsInfo.size(); i <= rows*cols; i++) {
                cellsInfo.push_back(c);
            }
        }
        //Delete extra cells
        else {
            cellsInfo.erase(cellsInfo.begin()+rows*cols, cellsInfo.end());
        }
    }
}

void PuzzleBoard::makeBoardFromKAKString(QString s) {
    int newRows, newCols;
    //Set the board size
    newRows = getUInt(s);
    //Remove the x
    s.remove(0, 1);
    newCols = getUInt(s);
    removeSpaces(s);
    cellSize = getUInt(s);
    removeSpaces(s);

    int v;
    CellInfo c;

    cellsInfo.clear();

    //Get cells until we have enough,
    //we hit the time 't',
    //or run out of KAKString to read from
    while (s.size() && s[0] != 't' && newRows*newCols != cellsInfo.size()) {

        //Clear our CellInfo c
        for (int i = 0 ; i < 10; i++) {
            c.notes[i] = 0;
        }
        c.valueOrClues[0] = c.valueOrClues[1] = 0;
        c.fixed = false;

        //Empty clue
        if (s[0] == '-') {
            s.remove(0, 1);
            c.type = CLUE;
            cellsInfo.push_back(c);
        }
        //Non-empty clue, or nonclue
        else if (s[0] != ' ') {
            //Get number
            v = getUInt(s);

            //Non-empty clue
            if (s[0] == '/') {
                c.type = CLUE;
                c.valueOrClues[0] = v;
                //Remove slash
                s.remove(0, 1);
                //Get right clue
                c.valueOrClues[1] = getUInt(s);

                cellsInfo.push_back(c);
            }
            //Nonclue
            else if (s.size() == 0 || s[0] == ' ' || s[0] == 'n' || s[0] == 'f') {
                c.type = NONCLUE;

                c.valueOrClues[0] = v;

                //If there are notes, get them
                if (s[0] == 'n') {
                    //Remove the 'n'
                    s.remove(0, 1);

                    //Set c.notes
                    while (s.size() && s[0] != ' ') {
                        v = s[0].digitValue();
                        s.remove(0, 1);
                        c.notes[v] = 1;
                    }
                }
                else if (s[0] == 'f') {
                    //Remove the 'f'
                    s.remove(0, 1);
                    c.fixed = true;
                }

                cellsInfo.push_back(c);
            }
        }
        if (s.size()) {
            removeSpaces(s);
        }
    }

    removeSpaces(s);

    //Get the time
    if (s.size() && s[0] == 't') {
        s.remove(0, 1);
        seconds = 3600*getUInt(s);
        s.remove(0, 1);
        seconds += 60*getUInt(s);
        s.remove(0, 1);
        seconds += getUInt(s);
    }

    //If we have too many or too little,
    //do some dirty adjustment so the program
    //doesn't clear the user's boot sector
    if (newRows*newCols != cellsInfo.size()) {
        //Add empty cells
        if (newRows*newCols > cellsInfo.size()) {
            CellInfo c;
            c.type = NONCLUE;
            c.valueOrClues[0] = 0;
            for (int i = 0; i < 10; i++)
                c.notes[i] = 0;
            for (int i = cellsInfo.size(); i <= newRows*newCols; i++) {
                cellsInfo.push_back(c);
            }
        }
        //Delete extra cells
        else {
            cellsInfo.erase(cellsInfo.begin()+newRows*newCols, cellsInfo.end());
        }
    }

    makeNewCellArray(newRows, newCols);
    updateCellArray();
    giveMetaKnowledgeToCells();

    //Set size of board
    setFixedSize(newCols*cellSize, newRows*cellSize);

    //Draw the board
    QTimer::singleShot(25, this, SLOT(drawBoard()));
}

void PuzzleBoard::updateCellArray() {
    updateCellArray(cellsInfo);
}

void PuzzleBoard::updateCellArray(QVector<CellInfo> info) {
    CellInfo cellInfo;
    int index = 0;

    while (index != info.size()) {
        cellInfo = info[index];
        if (cellInfo.type == CLUE) {
            cellArray[index/cols][index%cols].setType(CLUE);
            cellArray[index/cols][index%cols].setDownClue(cellInfo.valueOrClues[0]);
            cellArray[index/cols][index%cols].setRightClue(cellInfo.valueOrClues[1]);
        }
        else if (cellInfo.type == NONCLUE) {
            cellArray[index/cols][index%cols].setType(NONCLUE);
            cellArray[index/cols][index%cols].setValue(cellInfo.valueOrClues[0]);
            for (int i = 0; i < 10; i++) {
                cellArray[index/cols][index%cols].setNote(i, cellInfo.notes[i]);
            }
        }
        cellArray[index/cols][index%cols].setFixed(cellInfo.fixed);
        index++;
    }
}

QVector<CellInfo> PuzzleBoard::getCellsInfoFromCellArray() const {
    QVector<CellInfo> info;

    CellInfo cellInfo;
    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            if (cellArray[r][c].getType() == CLUE) {
                cellInfo.type = CLUE;
                cellInfo.valueOrClues[0] = cellArray[r][c].getDownClue();
                cellInfo.valueOrClues[1] = cellArray[r][c].getRightClue();
            }
            else if (cellArray[r][c].getType() == NONCLUE) {
                cellInfo.type = NONCLUE;
                cellInfo.valueOrClues[0] = cellArray[r][c].getValue();
                for (int i = 0; i < 10; i++) {
                    cellInfo.notes[i] = cellArray[r][c].getNote(i);
                }
            }
            cellInfo.fixed = cellArray[r][c].getFixed();
            info.push_back(cellInfo);
        }
    }

    return info;
}

QString PuzzleBoard::getKAKString() const {
    QString s = convertCellsInfoToKAKString(rows, cols, getCellsInfoFromCellArray());
    s.remove(s.size()-8, 9);
    s += getTimeFormatted();
    return s;
}

QString PuzzleBoard::convertCellsInfoToKAKString(int rows, int cols, QVector<CellInfo> info) const {
    QString s;
    s += QString::number(rows) + "x" + QString::number(cols);
    s += " " + QString::number(cellSize) + " ";

    int dClue, rClue, v;
    for (int i = 0; i < int(info.size()); i++) {
        if (info[i].type == CLUE) {
            dClue = info[i].valueOrClues[0];
            rClue = info[i].valueOrClues[1];
            if (dClue || rClue) {
                s += QString::number(dClue) + "/" +
                        QString::number(rClue) + " ";
            }
            else {
                s += "- ";
            }
        }
        else if (info[i].type == NONCLUE) {
            v = info[i].valueOrClues[0];
            s += QString::number(v);

            //Does it have any notes?
            bool hasAnyNotes = false;
            for (int n = 1; n < 10; n++) {
                if (info[i].notes[n]) {
                    hasAnyNotes = true;
                    break;
                }
            }

            if (hasAnyNotes) {
                s += 'n';
                //Add each noted number
                for (int n = 1; n < 10; n++) {
                    if (info[i].notes[n])
                        s += QString::number(n);
                }
            }
            else if (info[i].fixed) {
                s += 'f';
            }

            s += " ";
        }
    }

    s += "t00:00:00";

    return s;
}

void PuzzleBoard::clearBoard() {
    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            if (cellArray[r][c].getType() == CLUE)
                continue;
            if (cellArray[r][c].getFixed())
                continue;
            cellArray[r][c].setValue(0);
            for (int i = 0; i < 10; i++)
                cellArray[r][c].setNote(i, 0);
            cellArray[r][c].draw();
        }
    }
    cellsInfo = getCellsInfoFromCellArray();
}

void PuzzleBoard::drawBoard() {
    cellsInfo = getCellsInfoFromCellArray();
    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            cellArray[r][c].draw();
        }
    }
}

bool PuzzleBoard::solve(bool useBruteForce) {
    clearBoard();
    giveMetaKnowledgeToCells();
    writeNotesFromIntersections();
    removeNotesFixedValues();

    bool solved;
    //Do as much as we can with logic
    solved = logicSolve(false);

    //Smart bruteforce
    if (!solved && useBruteForce) {
        solved |= smartBruteForceSolve();
    }

    cellsInfo = getCellsInfoFromCellArray();
    drawBoard();

    return solved;
}

bool PuzzleBoard::logicSolve(bool lazy) {
    if (!hasEmptyCells()) {
        return checkSolved();
    }

    // do {
    do {
        do {
            do {
                do {
                    do {
                        do {
                            do {
                            } while (writeCellsWithOneNoteAndRemoveDupNotes());
                        } while (adjustNotesByLogicalRange());
                    } while (updateClueCellCombos());
                } while (solveUniquesWithOneEmpty());
            } while (!lazy && solveCellsWithNecessaryValue());
        } while (!lazy && removeExtraNotesFromUniques());
    } while (!lazy && removeNotesNotInPossibleCombos());
    //} while (removeNotesNotInPossiblePerms());

    return checkSolved();
}


bool PuzzleBoard::smartBruteForceSolve() {
    if (!hasEmptyCells()) {
        return checkSolved();
    }

    CellPos cell;
    cell.row = -1;
    cell.col = -1;

    //Pick empty nonclue cell with lowest number of notes
    int noteCount, minNotes = 10;
    bool lowest = false;
    for (int r = 0; r < rows; r++) {
        if (lowest) break;
        for (int c = 0; c < cols; c++) {
            if (lowest) break;
            if (cellArray[r][c].getType() == CLUE)
                continue;
            if (cellArray[r][c].getValue() != 0)
                continue;

            noteCount = 0;
            for (int i = 1; i < 10; i++) {
                if (cellArray[r][c].getNote(i)) {
                    noteCount++;
                    if (noteCount > minNotes)
                        break;
                }
            }
            if (noteCount < minNotes) {
                minNotes = noteCount;
                //Save the cell
                cell.row = r;
                cell.col = c;
                //Can't get a lower number of notes, so stop
                if (minNotes == 0) {
                    lowest = true;
                    return false;
                }
            }
        }
    }

    //Save board state
    QVector<CellInfo> savedCellInfo = getCellsInfoFromCellArray();

    //Loop through the notes of the cell we picked
    for (int i = 1; i < 10; i++) {
        if (!cellArray[cell.row][cell.col].getNote(i))
            continue;

        //Try a note on the cell we picked
        setCellValueAndEraseNeighborNoteDups(cell, i);

        //Attempt to solve
        bool solved = false;
        solved |= logicSolve(true);
        solved |= smartBruteForceSolve();
        if (solved) return true;

        //Undo the previous brute force, since it didn't work
        updateCellArray(savedCellInfo);
    }

    return false;
}

bool PuzzleBoard::removeNotesNotInPossiblePerms() {
    bool changed = false;

    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            if (cellArray[r][c].getType() == NONCLUE)
                continue;
            if (cellArray[r][c].getRightClue() == 0 &&
                    cellArray[r][c].getDownClue() == 0)
                continue;

            //Right clue
            if (cellArray[r][c].getRightClue()) {
                //Get the notes of all cells in this right group
                QVector<QVector<int>> cellsNotes;
                for (int c3 = c+1; c3 < cols; c3++) {
                    if (cellArray[r][c3].getType() == CLUE)
                        break;
                    cellsNotes.push_back(QVector<int>());
                    if (cellArray[r][c3].getFixed()) {
                        cellsNotes[cellsNotes.size()-1].push_back(cellArray[r][c3].getValue());
                    }
                    else {
                        for (int n = 0; n < 10; n++) {
                            if (cellArray[r][c3].getNote(n))
                                cellsNotes[c3 - (c+1)].push_back(n);
                        }
                    }
                }

                //For every unsolved cell in this right clue
                for (int c2 = c+1; c2 < cols; c2++) {
                    if (cellArray[r][c2].getType() == CLUE)
                        break;
                    if (cellArray[r][c2].getValue())
                        continue;

                    //Put all other cells notes in otherCellsNotes
                    QVector<QVector<int>> otherCellsNotes = cellsNotes;
                    otherCellsNotes.erase(otherCellsNotes.begin() + c2 - (c+1));

                    //For each of this cells notes
                    for (int n = 1; n < 10; n++) {
                        if (!cellArray[r][c2].getNote(n))
                            continue;

                        //Iterators for the current note we're using from each cell
                        QVector<QVector<int>::const_iterator> currNoteItr;
                        //Start them all off on the first note
                        for (int i = 0; i < int(otherCellsNotes.size()); i++) {
                            currNoteItr.push_back(otherCellsNotes[i].begin());
                        }

                        //Try every single perm of this note + other cells notes
                        bool noteIsInAPerm = false;
                        while (currNoteItr[0] != otherCellsNotes[0].end()) {
                            //Make sure there are no dup numbers being used
                            bool repeatedNumber = false;
                            QVector<int> notesBeingUsed(1, n);
                            for (int i = 0; i < int(currNoteItr.size()); i++) {
                                notesBeingUsed.push_back(*currNoteItr[i]);
                            }
                            for (int i = 0; i < int(notesBeingUsed.size()); i++) {
                                if (repeatedNumber) break;
                                for (int j = 0; j < int(notesBeingUsed.size()); j++) {
                                    if (i == j) continue;
                                    if (notesBeingUsed[i] == notesBeingUsed[j]) {
                                        repeatedNumber = true;
                                        break;
                                    }
                                }
                            }

                            if (!repeatedNumber) {
                                //Get the sum of all the notes being tested
                                int sum = n;
                                for (int i = 0; i < int(currNoteItr.size()); i++) {
                                    sum += *currNoteItr[i];
                                }

                                if (sum == cellArray[r][c].getRightClue()) {
                                    noteIsInAPerm = true;
                                    break;
                                }
                            }

                            //Update iterators to the next perm
                            //Update last iterator (cell) to next note
                            currNoteItr.last()++;
                            //If we finished this cells notes, go back to this cells
                            //first note and move the previous cell to the next note
                            int cellIndex = otherCellsNotes.size()-1;
                            while (currNoteItr[cellIndex] == otherCellsNotes[cellIndex].end()) {
                                //If we've reached the last note on the very first cell, we're done
                                if (cellIndex == 0) {
                                    break;
                                }
                                currNoteItr[cellIndex] = otherCellsNotes[cellIndex].begin();
                                cellIndex--;
                                currNoteItr[cellIndex]++;
                            }

                        }

                        if (!noteIsInAPerm) {
                            cellArray[r][c2].setNote(n, false);
                            changed = true;
                        }

                    }
                }
            }

            //Down clue
            if (cellArray[r][c].getDownClue()) {
                //Get the notes of all cells in this right group
                QVector<QVector<int>> cellsNotes;
                for (int r3 = r+1; r3 < rows; r3++) {
                    if (cellArray[r3][c].getType() == CLUE)
                        break;
                    cellsNotes.push_back(QVector<int>());
                    if (cellArray[r3][c].getFixed()) {
                        cellsNotes[cellsNotes.size()-1].push_back(cellArray[r3][c].getValue());
                    }
                    else {
                        for (int n = 0; n < 10; n++) {
                            if (cellArray[r3][c].getNote(n))
                                cellsNotes[r3 - (r+1)].push_back(n);
                        }
                    }
                }

                //For every unsolved cell in this right clue
                for (int r2 = r+1; r2 < rows; r2++) {
                    if (cellArray[r2][c].getType() == CLUE)
                        break;
                    if (cellArray[r2][c].getValue())
                        continue;

                    //Put all other cells notes in otherCellsNotes
                    QVector<QVector<int>> otherCellsNotes = cellsNotes;
                    otherCellsNotes.erase(otherCellsNotes.begin() + r2 - (r+1));

                    //For each of this cells notes
                    for (int n = 1; n < 10; n++) {
                        if (!cellArray[r2][c].getNote(n))
                            continue;

                        //Iterators for the current note we're using from each cell
                        QVector<QVector<int>::const_iterator> currNoteItr;
                        //Start them all off on the first note
                        for (int i = 0; i < int(otherCellsNotes.size()); i++) {
                            currNoteItr.push_back(otherCellsNotes[i].begin());
                        }

                        //Try every single perm of this note + other cells notes
                        bool noteIsInAPerm = false;
                        while (currNoteItr[0] != otherCellsNotes[0].end()) {
                            //Make sure there are no dup numbers being used
                            bool repeatedNumber = false;
                            QVector<int> notesBeingUsed(1, n);
                            for (int i = 0; i < int(currNoteItr.size()); i++) {
                                notesBeingUsed.push_back(*currNoteItr[i]);
                            }
                            for (int i = 0; i < int(notesBeingUsed.size()); i++) {
                                if (repeatedNumber) break;
                                for (int j = 0; j < int(notesBeingUsed.size()); j++) {
                                    if (i == j) continue;
                                    if (notesBeingUsed[i] == notesBeingUsed[j]) {
                                        repeatedNumber = true;
                                        break;
                                    }
                                }
                            }

                            if (!repeatedNumber) {
                                //Get the sum of all the notes being tested
                                int sum = n;
                                for (int i = 0; i < int(currNoteItr.size()); i++) {
                                    sum += *currNoteItr[i];
                                }

                                if (sum == cellArray[r][c].getDownClue()) {
                                    noteIsInAPerm = true;
                                    break;
                                }
                            }

                            //Update iterators to the next perm
                            //Update last iterator (cell) to next note
                            currNoteItr.last()++;
                            //If we finished this cells notes, go back to this cells
                            //first note and move the previous cell to the next note
                            int cellIndex = otherCellsNotes.size()-1;
                            while (currNoteItr[cellIndex] == otherCellsNotes[cellIndex].end()) {
                                //If we've reached the last note on the very first cell, we're done
                                if (cellIndex == 0) {
                                    break;
                                }
                                currNoteItr[cellIndex] = otherCellsNotes[cellIndex].begin();
                                cellIndex--;
                                currNoteItr[cellIndex]++;
                            }

                        }

                        if (!noteIsInAPerm) {
                            cellArray[r2][c].setNote(n, false);
                            changed = true;
                        }

                    }
                }
            }

        }
    }

    return changed;
}

bool PuzzleBoard::updateClueCellCombos() {
    bool changed = false;

    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            if (cellArray[r][c].getType() == NONCLUE)
                continue;
            if (cellArray[r][c].getRightClue() == 0 &&
                    cellArray[r][c].getDownClue() == 0)
                continue;

            QVector<QVector<int>> rCombos = cellArray[r][c].getRightCombos();
            QVector<QVector<int>> dCombos = cellArray[r][c].getDownCombos();

            //If a cell in this group is solved,
            //delete the combos that don't include the solved number
            //Right clue
            for (int c2 = c+1; c2 < cols; c2++) {
                if (cellArray[r][c2].getType() == CLUE)
                    break;
                int v = cellArray[r][c2].getValue();
                if (v == 0) continue;
                for (int i = 0; i < int(rCombos.size()); i++) {
                    bool found = false;
                    for (int j = 0; j < int(rCombos[i].size()); j++) {
                        if (rCombos[i][j] == v) {
                            found = true;
                            break;
                        }
                        if (rCombos[i][j] > v) {
                            break;
                        }
                    }
                    if (!found) {
                        rCombos.erase(rCombos.begin()+i);
                        i--;
                        changed = true;
                    }
                }
            }
            //Down clue
            for (int r2 = r+1; r2 < rows; r2++) {
                if (cellArray[r2][c].getType() == CLUE)
                    break;
                int v = cellArray[r2][c].getValue();
                if (v == 0) continue;
                for (int i = 0; i < int(dCombos.size()); i++) {
                    bool found = false;
                    for (int j = 0; j < int(dCombos[i].size()); j++) {
                        if (dCombos[i][j] == v) {
                            found = true;
                            break;
                        }
                        if (dCombos[i][j] > v) {
                            break;
                        }
                    }
                    if (!found) {
                        dCombos.erase(dCombos.begin()+i);
                        i--;
                        changed = true;
                    }
                }
            }

            //Delete combos that don't contain any of the cells notes
            //Right clue
            for (int c2 = c+1; c2 < cols; c2++) {
                if (cellArray[r][c2].getType() == CLUE)
                    break;
                if (cellArray[r][c2].getValue())
                    continue;

                //Get all of the notes for this cell
                QVector<int> cellsNotes;
                for (int i = 1; i < 10; i++) {
                    if (cellArray[r][c2].getNote(i)) {
                        cellsNotes.push_back(i);
                    }
                }

                //If the combo doesn't have any of this cells notes,
                //it's not possible, so remove it
                bool hasAnyOfCellsNotes = false;
                //For all combos,
                for (int i = 0; i < int(rCombos.size()); i++) {
                    //Check that at least one note is in the current combo
                    hasAnyOfCellsNotes = false;
                    for (int n = 0; n < int(cellsNotes.size()); n++) {
                        //For all numbers in the combo..
                        for (int j = 0; j < int(rCombos[i].size()); j++) {
                            //it has this note
                            if (rCombos[i][j] == cellsNotes[n]) {
                                hasAnyOfCellsNotes = true;
                                break;
                            }
                            //it didn't have this note
                            if (rCombos[i][j] > cellsNotes[n])
                                break;
                        }
                        if (hasAnyOfCellsNotes) break;
                    }
                    //If not, delete the combo
                    if (!hasAnyOfCellsNotes) {
                        rCombos.erase(rCombos.begin()+i);
                        i--;
                        changed = true;
                    }
                }
            }
            //Down  clue
            for (int r2 = r+1; r2 < rows; r2++) {
                if (cellArray[r2][c].getType() == CLUE)
                    break;
                if (cellArray[r2][c].getValue())
                    continue;

                //Get all of the notes for this cell
                QVector<int> cellsNotes;
                for (int i = 1; i < 10; i++) {
                    if (cellArray[r2][c].getNote(i)) {
                        cellsNotes.push_back(i);
                    }
                }

                //If the combo doesn't have any of this cells notes,
                //it's not possible, so remove it
                bool hasAnyOfCellsNotes = false;
                //For all combos,
                for (int i = 0; i < int(dCombos.size()); i++) {
                    //Check that at least one note is in the current combo
                    hasAnyOfCellsNotes = false;
                    for (int n = 0; n < int(cellsNotes.size()); n++) {
                        //For all numbers in the combo..
                        for (int j = 0; j < int(dCombos[i].size()); j++) {
                            //it has this note
                            if (dCombos[i][j] == cellsNotes[n]) {
                                hasAnyOfCellsNotes = true;
                                break;
                            }
                            //it didn't have this note
                            if (dCombos[i][j] > cellsNotes[n])
                                break;
                        }
                        if (hasAnyOfCellsNotes) break;
                    }
                    //If not, delete the combo
                    if (!hasAnyOfCellsNotes) {
                        dCombos.erase(dCombos.begin()+i);
                        i--;
                        changed = true;
                    }
                }
            }

            //Set combos
            cellArray[r][c].setRightCombos(rCombos);
            cellArray[r][c].setDownCombos(dCombos);

        }
    }

    return changed;
}

bool PuzzleBoard::solveUniquesWithOneEmpty() {
    bool changed = false;

    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            if (cellArray[r][c].getType() == NONCLUE)
                continue;
            if (cellArray[r][c].getDownClue() == 0 &&
                    cellArray[r][c].getRightClue() == 0)
                continue;

            //Down
            QVector<QVector<int>> dCombos = cellArray[r][c].getDownCombos();
            //Is there only one combo to solve this clue?
            if (dCombos.size() == 1) {
                //Get all the nums we need
                QVector<int> numsLeft = dCombos[0];

                CellPos unsolvedCell = { -1, -1 };
                for (int r2 = r+1; r2 < rows; r2++) {
                    if (cellArray[r2][c].getType() == CLUE)
                        break;
                    int v = cellArray[r2][c].getValue();
                    //Solved nonclue
                    if (v) {
                        //Remove this num from our list
                        int index = numsLeft.indexOf(v);
                        if (index > -1) {
                            numsLeft.erase(numsLeft.begin()+index);
                        }
                    }
                    else {
                        //Break on second empty cell
                        if (unsolvedCell.row != -1 ||
                                unsolvedCell.col != -1)
                            break;
                        //Save location of empty cell
                        unsolvedCell = { r2, c };
                    }
                }

                //Set if there was only a single empty cell
                if (numsLeft.size() == 1 && unsolvedCell.row != -1 && unsolvedCell.col != -1) {
                    setCellValueAndEraseNeighborNoteDups(unsolvedCell, numsLeft[0]);
                    changed = true;
                }
            }

            //Right
            QVector<QVector<int>> rCombos = cellArray[r][c].getRightCombos();
            //Is there only one combo to solve this clue?
            if (rCombos.size() == 1) {
                //Get all the nums we need
                QVector<int> numsLeft = rCombos[0];

                CellPos unsolvedCell = { -1, -1 };
                for (int c2 = c+1; c2 < cols; c2++) {
                    if (cellArray[r][c2].getType() == CLUE)
                        break;
                    int v = cellArray[r][c2].getValue();
                    //Solved nonclue
                    if (v) {
                        //Remove this num from our list
                        int index = numsLeft.indexOf(v);
                        if (index > -1) {
                            numsLeft.erase(numsLeft.begin()+index);
                        }
                    }
                    else {
                        //Break on second empty cell
                        if (unsolvedCell.row != -1 ||
                                unsolvedCell.col != -1)
                            break;
                        //Save location of empty cell
                        unsolvedCell = { r, c2 };
                    }
                }

                //Set if there was only a single empty cell
                if (numsLeft.size() == 1 && unsolvedCell.row != -1 && unsolvedCell.col != -1) {
                    setCellValueAndEraseNeighborNoteDups(unsolvedCell, numsLeft[0]);
                    changed = true;
                }
            }


        }
    }

    return changed;
}

bool PuzzleBoard::solveCellsWithNecessaryValue() {
    bool changed = false;
    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            if (cellArray[r][c].getType() == NONCLUE)
                continue;
            if (cellArray[r][c].getDownClue() == 0 &&
                    cellArray[r][c].getRightClue() == 0)
                continue;

            QVector<bool> necessary(10, true);
            int timesSeen[10];
            CellPos cell[10];

            //Right clue
            if (cellArray[r][c].getRightClue()) {
                QVector<QVector<int>> rCombos = cellArray[r][c].getRightCombos();

                //Figure out which numbers are in every combo
                //For every number,
                for (int n = 1; n < 10; n++) {
                    //look in every combo,
                    bool found = false;
                    for (int i = 0; i < int(rCombos.size()); i++) {
                        found = false;
                        //and check to make sure the number is in the combo
                        for (int j = 0; j < int(rCombos[i].size()); j++) {
                            if (rCombos[i][j] == n) {
                                found = true;
                                break;
                            }
                            if (rCombos[i][j] > n) {
                                break;
                            }
                        }
                        //If it wasn't in a combo, it's not necessary
                        if (!found) {
                            necessary[n] = false;
                        }
                    }
                }

                //Set cells that are the only ones with necessary values
                bool hasAnyNecessaries = false;
                for (int n = 1; n < 10; n++) {
                    if (necessary[n]) {
                        hasAnyNecessaries = true;
                        break;
                    }
                }
                if (hasAnyNecessaries) {
                    //Count occurences of each note
                    for (int i = 0; i < 10; i++) {
                        timesSeen[i] = 0;
                    }
                    for (int c2 = c+1; c2 < cols; c2++) {
                        if (cellArray[r][c2].getType() == CLUE)
                            break;

                        int v = cellArray[r][c2].getValue();

                        if (v == 0) {
                            for (int i = 1; i < 10; i++) {
                                if (cellArray[r][c2].getNote(i)) {
                                    timesSeen[i]++;
                                    if (timesSeen[i] == 1) {
                                        cell[i] = { r, c2 };
                                    }
                                }
                            }
                        }
                    }

                    //Set necessary vales
                    for (int i = 1; i < 10; i++) {
                        if (timesSeen[i] == 1 &&
                                necessary[i] == true) {
                            setCellValueAndEraseNeighborNoteDups(cell[i], i);
                            changed = true;
                        }
                    }
                }
            }

            //Down clue
            if (cellArray[r][c].getDownClue()) {
                QVector<QVector<int>> dCombos = cellArray[r][c].getDownCombos();
                for (int i = 0; i < 10; i++) {
                    necessary[i] = true;
                    cell[i] = { -1, -1 };
                }
                //Figure out which numbers are in every combo
                //For every number,
                for (int n = 1; n < 10; n++) {
                    //look in every combo,
                    bool found = false;
                    for (int i = 0; i < int(dCombos.size()); i++) {
                        found = false;
                        //and check to make sure the number is in the combo
                        for (int j = 0; j < int(dCombos[i].size()); j++) {
                            if (dCombos[i][j] == n) {
                                found = true;
                                break;
                            }
                            if (dCombos[i][j] > n) {
                                break;
                            }
                        }
                        //If it wasn't in a combo, it's not necessary
                        if (!found) {
                            necessary[n] = false;
                        }
                    }
                }

                //Set cells that are the only ones with necessary values
                bool hasAnyNecessaries = false;
                for (int n = 1; n < 10; n++) {
                    if (necessary[n]) {
                        hasAnyNecessaries = true;
                        break;
                    }
                }
                if (hasAnyNecessaries) {
                    //Count occurences of each note
                    for (int i = 0; i < 10; i++) {
                        timesSeen[i] = 0;
                    }
                    for (int r2 = r+1; r2 < rows; r2++) {
                        if (cellArray[r2][c].getType() == CLUE)
                            break;

                        int v = cellArray[r2][c].getValue();
                        if (v == 0) {
                            for (int i = 1; i < 10; i++) {
                                if (cellArray[r2][c].getNote(i)) {
                                    timesSeen[i]++;
                                    if (timesSeen[i] == 1) {
                                        cell[i] = { r2, c };
                                    }
                                }
                            }
                        }
                    }

                    //Set necessary vales
                    for (int i = 1; i < 10; i++) {
                        if (timesSeen[i] == 1 &&
                                necessary[i] == true) {
                            setCellValueAndEraseNeighborNoteDups(cell[i], i);
                            changed = true;
                        }
                    }
                }
            }

        }
    }

    return changed;
}

bool PuzzleBoard::adjustNotesByLogicalRange() {
    bool changed = false;

    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            if (cellArray[r][c].getType() == NONCLUE)
                continue;
            if (cellArray[r][c].getDownClue() == 0
                    && cellArray[r][c].getRightClue() == 0)
                continue;

            //For every unsolved nonclue cell this clue cell has,
            //remove all notes less than
            //the clue - (sum of its row/col neighbors max notes),
            //and all notes higher than
            //the clue - (sum of its row/col neighbors min notes)

            //Row / right clue
            //Get min and max notes for each cell
            int maxNoteSum = 0, minNoteSum = 0;
            QVector<int> minNotes, maxNotes;
            int cellMin, cellMax;

            if (cellArray[r][c].getRightClue()) {
                for (int c2 = c+1; c2 < cols; c2++) {
                    if (cellArray[r][c2].getType() == CLUE)
                        break;
                    minNotes.push_back(getMinNoteForCell({r, c2}));
                    maxNotes.push_back(getMaxNoteForCell({r, c2}));
                }
                for (int i = 0; i < int(minNotes.size()); i++) {
                    minNoteSum += minNotes[i];
                }
                for (int i = 0; i < int(maxNotes.size()); i++) {
                    maxNoteSum += maxNotes[i];
                }

                //For each unsolved cell (for right clue),
                //delete the notes not in the logical range
                for (int c2 = c+1; c2 < cols; c2++) {
                    if (cellArray[r][c2].getType() == CLUE)
                        break;
                    if (cellArray[r][c2].getValue())
                        continue;
                    cellMin = cellArray[r][c].getRightClue()
                            - (maxNoteSum - maxNotes[c2 - (c+1)]);
                    cellMax = cellArray[r][c].getRightClue()
                            - (minNoteSum - minNotes[c2 - (c+1)]);

                    //Remove the notes too low
                    for (int i = cellMin-1; i > 0; i--) {
                        if (i > 9) i = 9;
                        if (cellArray[r][c2].getNote(i)) {
                            cellArray[r][c2].setNote(i, false);
                            changed = true;
                        }
                    }
                    //Remove the notes too high
                    for (int i = cellMax+1; i < 10; i++) {
                        if (i < 1) i = 1;
                        if (cellArray[r][c2].getNote(i)) {
                            cellArray[r][c2].setNote(i, false);
                            changed = true;
                        }
                    }
                }

            }

            if (cellArray[r][c].getDownClue()) {
                //Column / down clue
                //Get min and max notes for each cell
                maxNoteSum = minNoteSum = 0;
                minNotes.clear();
                maxNotes.clear();
                for (int r2 = r+1; r2 < rows; r2++) {
                    if (cellArray[r2][c].getType() == CLUE)
                        break;
                    minNotes.push_back(getMinNoteForCell({r2, c}));
                    maxNotes.push_back(getMaxNoteForCell({r2, c}));
                }
                for (int i = 0; i < int(minNotes.size()); i++) {
                    minNoteSum += minNotes[i];
                }
                for (int i = 0; i < int(maxNotes.size()); i++) {
                    maxNoteSum += maxNotes[i];
                }

                //For each unsolved cell (for down clue),
                //delete the notes not in the logical range
                for (int r2 = r+1; r2 < rows; r2++) {
                    if (cellArray[r2][c].getType() == CLUE)
                        break;
                    if (cellArray[r2][c].getValue())
                        continue;
                    cellMin = cellArray[r][c].getDownClue()
                            - (maxNoteSum - maxNotes[r2 - (r+1)]);
                    cellMax = cellArray[r][c].getDownClue()
                            - (minNoteSum - minNotes[r2 - (r+1)]);

                    //Remove the notes too low
                    for (int i = cellMin-1; i > 0; i--) {
                        if (i > 9) i = 9;
                        if (cellArray[r2][c].getNote(i)) {
                            cellArray[r2][c].setNote(i, false);
                            changed = true;
                        }
                    }
                    //Remove the notes too high
                    for (int i = cellMax+1; i < 10; i++) {
                        if (i < 1) i = 1;
                        if (cellArray[r2][c].getNote(i)) {
                            cellArray[r2][c].setNote(i, false);
                            changed = true;
                        }
                    }
                }
            }


        }
    }

    return changed;
}

bool PuzzleBoard::removeExtraNotesFromUniques() {
    bool changed = false;

    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            if (cellArray[r][c].getType() == NONCLUE)
                continue;
            if (cellArray[r][c].getDownClue() == 0 &&
                    cellArray[r][c].getRightClue() == 0)
                continue;

            QVector<QVector<int>> cellNotes;

            //Right clue
            QVector<QVector<int>> rCombos = cellArray[r][c].getRightCombos();
            //Is there only one possible combo?
            if (rCombos.size() == 1) {
                //Get all of the notes for every cell
                for (int c2 = c + 1; c2 < cols; c2++) {
                    if (cellArray[r][c2].getType() == CLUE)
                        break;
                    cellNotes.push_back(QVector<int>());
                    for (int i = 1; i < 10; i++) {
                        if (cellArray[r][c2].getNote(i)) {
                            cellNotes[c2 - (c+1)].push_back(i);
                        }
                    }
                }

                //If there was ever a case where there were N
                //cells with ONLY the same N notes, we can safely
                //remove those notes from other cells
                int count = 0;
                for (int i = 0; i < int(cellNotes.size()); i++) {
                    count = 0;
                    //Count times these notes match another cells notes
                    for (int j = 0; j < int(cellNotes.size()); j++) {
                        if (cellNotes[i] == cellNotes[j]) {
                            count++;
                        }
                    }
                    //If there were the same number of matching cells
                    //as there are notes on the cells
                    if (count > 1 && count == int(cellNotes[i].size())) {
                        //Remove those notes from other cells
                        for (int c2 = c + 1; c2 < cols; c2++) {
                            if (cellArray[r][c2].getType() == CLUE)
                                break;
                            if (cellNotes[c2 - (c+1)] != cellNotes[i]) {
                                for (int n = 0; n < int(cellNotes[i].size()); n++) {
                                    if (cellArray[r][c2].getNote(cellNotes[i][n])) {
                                        cellArray[r][c2].setNote(cellNotes[i][n], false);
                                        changed = true;
                                    }
                                }
                            }
                        }
                    }
                }
            }

            //Down clue

            QVector<QVector<int>> dCombos = cellArray[r][c].getDownCombos();
            //Is there only one possible combo?
            if (dCombos.size() == 1) {
                cellNotes.clear();
                //Down clue
                //Get all of the notes for every cell
                for (int r2 = r + 1; r2 < rows; r2++) {
                    if (cellArray[r2][c].getType() == CLUE)
                        break;
                    cellNotes.push_back(QVector<int>());
                    for (int i = 1; i < 10; i++) {
                        if (cellArray[r2][c].getNote(i)) {
                            cellNotes[r2 - (r+1)].push_back(i);
                        }
                    }
                }

                //If there was ever a case where there were N
                //cells with ONLY the same N notes, we can safely
                //remove those notes from other cells
                int count = 0;
                for (int i = 0; i < int(cellNotes.size()); i++) {
                    count = 0;
                    //Count times these notes match another cells notes
                    for (int j = 0; j < int(cellNotes.size()); j++) {
                        if (cellNotes[i] == cellNotes[j]) {
                            count++;
                        }
                    }
                    //If there were the same number of matching cells
                    //as there are notes on the cells
                    if (count > 1 && count == int(cellNotes[i].size())) {
                        //Remove those notes from other cells
                        for (int r2 = r + 1; r2 < rows; r2++) {
                            if (cellArray[r2][c].getType() == CLUE)
                                break;
                            if (cellNotes[r2 - (r+1)] != cellNotes[i]) {
                                for (int n = 0; n < int(cellNotes[i].size()); n++) {
                                    if (cellArray[r2][c].getNote(cellNotes[i][n])) {
                                        cellArray[r2][c].setNote(cellNotes[i][n], false);
                                        changed = true;
                                    }
                                }
                            }
                        }
                    }
                }
            }


        }
    }

    return changed;
}

void PuzzleBoard::removeNotesFixedValues() {
    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            if (cellArray[r][c].getType() == CLUE)
                continue;
            if (cellArray[r][c].getFixed()) {
                setCellValueAndEraseNeighborNoteDups({ r, c }, cellArray[r][c].getValue());
            }
        }
    }
}

void PuzzleBoard::putBackFixedValues() {
    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            if (cellArray[r][c].getType() == CLUE)
                continue;
            if (cellArray[r][c].getFixed()) {
                for (int n = 1; n < 10; n++) {
                    cellArray[r][c].setNote(n, false);
                }
            }

        }
    }
}

bool PuzzleBoard::removeNotesNotInPossibleCombos() {
    bool changed = false;

    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            if (cellArray[r][c].getType() == NONCLUE)
                continue;
            if (cellArray[r][c].getRightClue() == 0 &&
                    cellArray[r][c].getDownClue() == 0)
                continue;

            QVector<QVector<int>> rCombos = cellArray[r][c].getRightCombos();
            QVector<QVector<int>> dCombos = cellArray[r][c].getDownCombos();

            //Right clue
            //For this clue cells row
            for (int c2 = c+1; c2 < cols; c2++) {
                if (cellArray[r][c2].getType() == CLUE)
                    break;
                if (cellArray[r][c2].getValue())
                    continue;
                //For every note in this unsolved cell,
                for (int n = 1; n < 10; n++) {
                    if (!cellArray[r][c2].getNote(n))
                        continue;
                    //If the note isn't in any of the possible combos, remove it
                    bool foundInAnyCombo = false;
                    for (int i = 0; i < int(rCombos.size()); i++) {
                        bool found = false;
                        for (int j = 0; j < int(rCombos[i].size()); j++) {
                            if (rCombos[i][j] == n) {
                                found = true;
                                break;
                            }
                            if (rCombos[i][j] > n) {
                                break;
                            }
                        }
                        if (found) {
                            foundInAnyCombo = true;
                            break;
                        }
                    }
                    if (!foundInAnyCombo) {
                        cellArray[r][c2].setNote(n, false);
                        changed = true;
                    }
                }
            }

            //Down clue
            //For this clue cells col
            for (int r2 = r+1; r2 < rows; r2++) {
                if (cellArray[r2][c].getType() == CLUE)
                    break;
                if (cellArray[r2][c].getValue())
                    continue;
                //For every note in this unsolved cell,
                for (int n = 1; n < 10; n++) {
                    if (!cellArray[r2][c].getNote(n))
                        continue;
                    //If the note isn't in any of the possible combos, remove it
                    bool foundInAnyCombo = false;
                    for (int i = 0; i < int(dCombos.size()); i++) {
                        bool found = false;
                        for (int j = 0; j < int(dCombos[i].size()); j++) {
                            if (dCombos[i][j] == n) {
                                found = true;
                                break;
                            }
                            if (dCombos[i][j] > n) {
                                break;
                            }
                        }
                        if (found) {
                            foundInAnyCombo = true;
                            break;
                        }
                    }
                    if (!foundInAnyCombo) {
                        cellArray[r2][c].setNote(n, false);
                        changed = true;
                    }
                }
            }


            cellArray[r][c].setRightCombos(rCombos);
            cellArray[r][c].setDownCombos(dCombos);

        }
    }


    return changed;
}

bool PuzzleBoard::writeNotesFromIntersections() {
    bool changed = false;

    bool* possNotes;
    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            if (cellArray[r][c].getType() == CLUE)
                continue;
            if (cellArray[r][c].getFixed())
                continue;

            possNotes = getIntersectNotesForCell({r, c});

            for (int i = 1; i < 10; i++) {
                if (possNotes[i] != cellArray[r][c].getNote(i)) {
                    changed = true;
                }
                cellArray[r][c].setNote(i, possNotes[i]);
            }

            delete [] possNotes;

        }
    }

    return changed;
}

bool PuzzleBoard::writeCellsWithOneNoteAndRemoveDupNotes() {
    bool changed = false;
    int noteCount, val;
    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            if (cellArray[r][c].getType() == CLUE)
                continue;
            if (cellArray[r][c].getValue())
                continue;

            noteCount = 0;
            for (int i = 1; i < 10; i++) {
                if (cellArray[r][c].getNote(i)) {
                    noteCount++;
                    val = i;
                    if (noteCount > 1)
                        break;
                }
            }
            //Only one note
            if (noteCount == 1) {
                //Set the cells value
                setCellValueAndEraseNeighborNoteDups({r, c}, val);\
                changed = true;
            }

        }
    }
    return changed;
}

int PuzzleBoard::getMinNoteForCell(CellPos pos) const {
    if (cellArray[pos.row][pos.col].getValue()) {
        return cellArray[pos.row][pos.col].getValue();
    }
    for (int i = 1; i < 10; i++) {
        if (cellArray[pos.row][pos.col].getNote(i)) {
            return i;
        }
    }
    return 10;
}

int PuzzleBoard::getMaxNoteForCell(CellPos pos) const {
    if (cellArray[pos.row][pos.col].getValue()) {
        return cellArray[pos.row][pos.col].getValue();
    }
    for (int i = 9; i > 0; i--) {
        if (cellArray[pos.row][pos.col].getNote(i)) {
            return i;
        }
    }
    return 0;
}

bool* PuzzleBoard::getIntersectNotesForCell(CellPos pos) const {
    Cell *cPtr = &cellArray[pos.row][pos.col];
    bool *possible = new bool[10];

    //Get possible down and right combos
    QVector<QVector<int>> possDownCombos = sumInNumCombo[cPtr->getDownClue()][cPtr->getNumInDownSum()];
    QVector<QVector<int>> possRightCombos = sumInNumCombo[cPtr->getRightClue()][cPtr->getNumInRightSum()];

    bool possDownNums[10];
    bool possRightNums[10];
    for (int i = 0; i < 10; i++) {
        possDownNums[i] = false;
        possRightNums[i] = false;
    }

    //If there was a down clue
    if (cPtr->getDownClue() > 0) {
        //Get possible down numbers
        for (int i = 0; i < int(possDownCombos.size()); i++) {
            for (int j = 0; j < int(possDownCombos[i].size()); j++) {
                possDownNums[possDownCombos[i][j]] = true;
            }
        }
    }
    //If there wasn't a down clue
    else {
        //Set 1-9 to possible
        for (int i = 1; i < 10; i++) {
            possDownNums[i] = true;
        }
    }

    //If there was a right clue
    if (cPtr->getRightClue() > 0) {
        //Get possible right numbers
        for (int i = 0; i < int(possRightCombos.size()); i++) {
            for (int j = 0; j < int(possRightCombos[i].size()); j++) {
                possRightNums[possRightCombos[i][j]] = true;
            }
        }
    }
    //If there wasn't a right clue
    else {
        //Set 1-9 to possible
        for (int i = 1; i < 10; i++) {
            possRightNums[i] = true;
        }
    }

    //Get the intersection of possible down and right numbers
    for (int i = 0; i < 10; i++) {
        possible[i] = possDownNums[i] & possRightNums[i];
    }
    return possible;
}

void PuzzleBoard::giveMetaKnowledgeToCells() {
    //Give clues their numInDownSum/numInRightSum and combos,
    //and give nonclues their downClue and rightClue
    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            if (cellArray[r][c].getType() == NONCLUE)
                continue;

            cellArray[r][c].setNumInDownSum(0);
            //Give nonclue cells in this column this clues downClue
            for (int r2 = r+1; r2 < rows; r2++) {
                if (cellArray[r2][c].getType() == CLUE)
                    break;
                cellArray[r2][c].setDownClue(cellArray[r][c].getDownClue());
                cellArray[r][c].setNumInDownSum(cellArray[r][c].getNumInDownSum()+1);
            }

            cellArray[r][c].setNumInRightSum(0);
            //Give nonclue cells in this row this clues downClue
            for (int c2 = c+1; c2 < cols; c2++) {
                if (cellArray[r][c2].getType() == CLUE)
                    break;
                cellArray[r][c2].setRightClue(cellArray[r][c].getRightClue());
                cellArray[r][c].setNumInRightSum(cellArray[r][c].getNumInRightSum()+1);
            }


            cellArray[r][c].setRightCombos(sumInNumCombo[cellArray[r][c].getRightClue()]
                    [cellArray[r][c].getNumInRightSum()]);
            cellArray[r][c].setDownCombos(sumInNumCombo[cellArray[r][c].getDownClue()]
                    [cellArray[r][c].getNumInDownSum()]);
        }
    }

    //Go back and give nonclues their numInDownSum and numInRightSum
    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            if (cellArray[r][c].getType() == CLUE)
                continue;

            //Find clue above cell, set current cells
            //numInDownSum to the clues numInDownSum
            cellArray[r][c].setNumInDownSum(0);
            for (int r2 = r-1; r2 >= 0; r2--) {
                if (cellArray[r2][c].getType() == NONCLUE)
                    continue;
                cellArray[r][c].setNumInDownSum(cellArray[r2][c].getNumInDownSum());
                break;
            }
            //Find clue to the left of cell, set current cells
            //numInRightSum to the clues numInRightSum
            cellArray[r][c].setNumInRightSum(0);
            for (int c2 = c-1; c2 >= 0; c2--) {
                if (cellArray[r][c2].getType() == NONCLUE)
                    continue;
                cellArray[r][c].setNumInRightSum(cellArray[r][c2].getNumInRightSum());
                break;
            }
        }
    }
}

void PuzzleBoard::setCellValueAndEraseNeighborNoteDups(CellPos cell, int val) {
    int r = cell.row, c = cell.col;

    cellArray[r][c].setValue(val);
    for (int i = 0; i < 10; i++) {
        cellArray[r][c].setNote(i, false);
    }
    if (!cellArray[r][c].getFixed()) {
        cellArray[r][c].setNote(val, true);
    }

    //Remove dup notes on this cells row and col
    for (int r2 = r-1; r2 >= 0; r2--) {
        if (cellArray[r2][c].getType() == CLUE)
            break;
        cellArray[r2][c].setNote(val, false);
    }
    for (int r2 = r+1; r2 < rows; r2++) {
        if (cellArray[r2][c].getType() == CLUE)
            break;
        cellArray[r2][c].setNote(val, false);
    }
    for (int c2 = c-1; c2 >= 0; c2--) {
        if (cellArray[r][c2].getType() == CLUE)
            break;
        cellArray[r][c2].setNote(val, false);
    }
    for (int c2 = c+1; c2 < cols; c2++) {
        if (cellArray[r][c2].getType() == CLUE)
            break;
        cellArray[r][c2].setNote(val, false);
    }
}

bool PuzzleBoard::hasEmptyCells() const {
    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            if (cellArray[r][c].getType() == CLUE)
                continue;
            if (cellArray[r][c].getValue() == 0)
                return true;
        }
    }

    return false;
}

bool PuzzleBoard::checkSolved() const {
    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            if (cellArray[r][c].getType() == NONCLUE) {
                if (cellArray[r][c].getValue() == 0) {
                    return false;
                }
                continue;
            }

            int dClue = cellArray[r][c].getDownClue();
            int rClue = cellArray[r][c].getRightClue();

            //Check down sum
            if (dClue && getValidDownSumFromCell({ r, c }) != dClue) {
                return false;
            }
            //Check right sum
            if (rClue && getValidRightSumFromCell({ r, c }) != rClue) {
                return false;
            }
        }
    }

    return true;
}

int PuzzleBoard::getValidDownSumFromCell(CellPos pos) const {
    int sum = 0, v;
    bool used[10];
    for (int i = 0; i < 10; i++)
        used[i] = 0;

    for (int r = pos.row+1; r < rows; r++) {
        if (cellArray[r][pos.col].getType() == CLUE) break;
        v = cellArray[r][pos.col].getValue();
        //Can't repeat numbers
        if (used[v])
            return -1;
        used[v] = true;
        sum += v;
    }

    return sum;
}

int PuzzleBoard::getValidRightSumFromCell(CellPos pos) const {
    int sum = 0, v;
    bool used[10];
    for (int i = 0; i < 10; i++)
        used[i] = 0;

    for (int c = pos.col+1; c < cols; c++) {
        if (cellArray[pos.row][c].getType() == CLUE) break;
        v = cellArray[pos.row][c].getValue();
        //Can't repeat numbers
        if (used[v])
            return -1;
        used[v] = true;
        sum += v;
    }

    return sum;
}

CellPos PuzzleBoard::getFirstNonClueCell() const {
    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            if (cellArray[r][c].getType() == CLUE)
                continue;
            if (cellArray[r][c].getFixed())
                continue;
            return { r, c };
        }
    }
    return { -1, -1 };
}

CellPos PuzzleBoard::getNextNonClueCell(int dir) const {
    if (selectedCell.row == -1 ||
            selectedCell.col == -1)
        return getFirstNonClueCell();

    if (dir == UP) {
        for (int r = selectedCell.row-1; r >= 0; r--) {
            if (cellArray[r][selectedCell.col].getType() == CLUE)
                continue;
            if (cellArray[r][selectedCell.col].getFixed())
                continue;
            return { r, selectedCell.col };
        }
    }
    else if (dir == DOWN) {
        for (int r = selectedCell.row+1; r < rows; r++) {
            if (cellArray[r][selectedCell.col].getType() == CLUE)
                continue;
            if (cellArray[r][selectedCell.col].getFixed())
                continue;
            return { r, selectedCell.col };
        }
    }
    else if (dir == LEFT) {
        for (int c = selectedCell.col-1; c >= 0; c--) {
            if (cellArray[selectedCell.row][c].getType() == CLUE)
                continue;
            if (cellArray[selectedCell.row][c].getFixed())
                continue;
            return { selectedCell.row, c };
        }
    }
    else if (dir == RIGHT) {
        for (int c = selectedCell.col+1; c < cols; c++) {
            if (cellArray[selectedCell.row][c].getType() == CLUE)
                continue;
            if (cellArray[selectedCell.row][c].getFixed())
                continue;
            return { selectedCell.row, c };
        }
    }

    return { -1, -1 };
}

void PuzzleBoard::keyPressEvent(QKeyEvent * event) {
    if (draggingCell.row != -1 && draggingCell.col != -1)
        return;

    CellPos next = { -1, -1 };
    switch (event->key()) {
    case Qt::Key_W:
    case Qt::Key_Up:
        next = getNextNonClueCell(UP);
        break;
    case Qt::Key_S:
    case Qt::Key_Down:
        next = getNextNonClueCell(DOWN);
        break;
    case Qt::Key_A:
    case Qt::Key_Left:
        next = getNextNonClueCell(LEFT);
        break;
    case Qt::Key_D:
    case Qt::Key_Right:
        next = getNextNonClueCell(RIGHT);
        break;
    case Qt::Key_N:
        //Toggle notes
        if (selectedCell.row != -1 && selectedCell.col != -1) {
            Cell *cPtr = &cellArray[selectedCell.row][selectedCell.col];
            if (cPtr->getType() == NONCLUE && !cPtr->getFixed()) {
                cPtr->setNote(0, !cPtr->getNote());
                cPtr->draw();
            }
        }
        break;
    case Qt::Key_0:
    case Qt::Key_1:
    case Qt::Key_2:
    case Qt::Key_3:
    case Qt::Key_4:
    case Qt::Key_5:
    case Qt::Key_6:
    case Qt::Key_7:
    case Qt::Key_8:
    case Qt::Key_9:
        if (selectedCell.row != -1 && selectedCell.col != -1) {
            cellArray[selectedCell.row][selectedCell.col].handleNumPress(event->key() - Qt::Key_0);
        }
        break;
    case Qt::Key_Shift:
    case Qt::Key_E:
    case Qt::Key_Plus:
    case Qt::Key_Equal:
        if (selectedCell.row != -1 && selectedCell.col != -1) {
            Cell *cellPtr = &cellArray[selectedCell.row][selectedCell.col];
            if (cellPtr->getFixed()) break;
            if (cellPtr->getNote() == 0) {
                cellPtr->setValue(cellPtr->getValue()+1);
                cellPtr->draw();
            }
        }
        break;
    case Qt::Key_Control:
    case Qt::Key_Q:
    case Qt::Key_Minus:
    case Qt::Key_Underscore:
        if (selectedCell.row != -1 && selectedCell.col != -1) {
            Cell *cellPtr = &cellArray[selectedCell.row][selectedCell.col];
            if (cellPtr->getFixed()) break;
            if (cellPtr->getNote() == 0) {
                cellPtr->setValue(cellPtr->getValue()-1);
                cellPtr->draw();
            }
        }
        break;
    default:
        break;
    }
    if (next.row != -1 && next.col != -1) {
        toggleSelectOnCell(selectedCell);
        toggleSelectOnCell(next);
    }
}

void PuzzleBoard::toggleSelectOnCell(CellPos pos) {
    if (pos.row == -1 || pos.col == -1) {
        return;
    }

    if (cellArray[pos.row][pos.col].getType() == CLUE) {
        return;
    }

    if (cellArray[pos.row][pos.col].getSelected() == true) {
        cellArray[pos.row][pos.col].unselect();
    }
    else {
        cellArray[pos.row][pos.col].select();
        selectedCell = pos;
    }
}

void PuzzleBoard::handleMouseMove(CellPos pos) {
    //If you've moved to a different cell
    if (pos.row != selectedCell.row || pos.col != selectedCell.col) {
        //..and the cell is a clue or fixed
        if (cellArray[pos.row][pos.col].getType() == CLUE ||
                cellArray[pos.row][pos.col].getFixed()) {
            toggleSelectOnCell(selectedCell);
            selectedCell = { -1, -1 };
            //...and you're dragging something
            if (draggingCell.row != -1 && draggingCell.col != -1) {
                //....draw it at full opacity
                cellArray[draggingCell.row][draggingCell.col].draw();
            }
        }
        //..and the cell is NOT a clue
        else {
            //Update which cell is selected
            toggleSelectOnCell(selectedCell);
            toggleSelectOnCell(pos);

            //...and you're dragging something
            if (draggingCell.row != -1 && draggingCell.col != -1) {
                //Pointers to shave line length
                Cell *dPtr = &cellArray[draggingCell.row][draggingCell.col];
                Cell *pPtr = &cellArray[pos.row][pos.col];
                //Get the value of drag-from
                int dragN = dPtr->getValue();
                //Draw a 0 on dragFrom, and the drag value on dragTo
                dPtr->drawValue(0, DRAG_OPACITY);
                dPtr->drawNotes(0.5);
                pPtr->drawValue(dragN, DRAG_OPACITY);
                pPtr->drawNotes(0.5);
            }
        }
    }
}

void PuzzleBoard::handleMouseLeftPress(CellPos pos) {
    //If it's not a clue,
    if (cellArray[pos.row][pos.col].getType() == NONCLUE) {
        //...start dragging it, if it has a value
        if (cellArray[pos.row][pos.col].getValue() &&
                !cellArray[pos.row][pos.col].getFixed()) {
            cellArray[pos.row][pos.col].draw(DRAG_OPACITY);
            draggingCell = pos;
        }
    }
}

void PuzzleBoard::handleMouseRightPress(CellPos pos) {
    //If it is a clue,
    if (cellArray[pos.row][pos.col].getType() == CLUE) {
        //Clear the appropriate row/column
        //Down clue
        if (cellArray[pos.row][pos.col].getLabel()->mapFromGlobal(QCursor::pos()).y() >
                cellArray[pos.row][pos.col].getLabel()->mapFromGlobal(QCursor::pos()).x()) {
            clearColumnFrom(pos);
        }
        //Right clue
        else {
            clearRowFrom(pos);
        }
    }
    //If it's not a clue
    else if (!cellArray[pos.row][pos.col].getFixed()){
        //Toggle notes
        cellArray[pos.row][pos.col].setNote(0, !cellArray[pos.row][pos.col].getNote());
        cellArray[pos.row][pos.col].draw();
    }
}

void PuzzleBoard::handleMouseLeftRelease(CellPos pos) {
    //If you're releasing on the cell you started on
    if (pos.row == draggingCell.row && pos.col == draggingCell.col) {
        draggingCell = { -1, -1 };
        cellArray[pos.row][pos.col].setNote(0, false);
        cellArray[pos.row][pos.col].draw();
        return;
    }

    //If you're dragging and dropping
    if (draggingCell.row != -1 && draggingCell.col != -1) {
        cellArray[draggingCell.row][draggingCell.col].draw();
        //..and if you're not releasing on a clue
        if (cellArray[pos.row][pos.col].getType() == NONCLUE &&
                !cellArray[pos.row][pos.col].getFixed()) {
            //...drop the drag cell value on the drop cell
            int dragV = cellArray[draggingCell.row][draggingCell.col].getValue();
            cellArray[draggingCell.row][draggingCell.col].setValue(0);
            cellArray[draggingCell.row][draggingCell.col].draw();
            cellArray[pos.row][pos.col].setValue(dragV);
            cellArray[pos.row][pos.col].draw();

        }
        draggingCell = { -1, -1 };
    }
}

void PuzzleBoard::handleMouse(QMouseEvent * mouseEvent) {
    CellPos pos = { (mapFromGlobal(QCursor::pos()).y())/cellSize,
                    (mapFromGlobal(QCursor::pos()).x())/cellSize};

    //Mouse movement
    if (mouseEvent->type() == QEvent::MouseMove || mouseEvent->type() == QEvent::HoverMove) {
        handleMouseMove(pos);
    }
    //Mouse button press
    else if (mouseEvent->type() == QEvent::MouseButtonPress) {
        if (mouseEvent->button() == Qt::RightButton) {
            handleMouseRightPress(pos);
        }
        else if (mouseEvent->button() == Qt::LeftButton) {
            handleMouseLeftPress(pos);
        }
    }
    //Mouse button release
    else if (mouseEvent->type() == QEvent::MouseButtonRelease) {
        //Releasing left click
        if (mouseEvent->button() == Qt::LeftButton) {
            handleMouseLeftRelease(pos);
        }
    }
}

void PuzzleBoard::clearColumnFrom(CellPos pos) {
    for (int r = pos.row+1; r < rows; r++) {
        if (cellArray[r][pos.col].getType() == CLUE)
            break;
        if (cellArray[r][pos.col].getFixed())
            continue;
        cellArray[r][pos.col].setValue(0);
        for (int i = 0; i < 10; i++)
            cellArray[r][pos.col].setNote(i, 0);
        cellArray[r][pos.col].draw();
    }
}

void PuzzleBoard::clearRowFrom(CellPos pos) {
    for (int c = pos.col+1; c < cols; c++) {
        if (cellArray[pos.row][c].getType() == CLUE)
            break;
        if (cellArray[pos.row][c].getFixed())
            continue;
        cellArray[pos.row][c].setValue(0);
        for (int i = 0; i < 10; i++)
            cellArray[pos.row][c].setNote(i, 0);
        cellArray[pos.row][c].draw();
    }
}

QString PuzzleBoard::getTimeFormatted() const {
    QString t = "";
    int hours, minutes, seconds = this->seconds;

    minutes = seconds/60;
    seconds %= 60;
    hours = minutes/60;
    minutes %= 60;

    if (hours < 10)
        t += "0";
    t += QString::number(hours) + ":";
    if (minutes < 10)
        t += "0";
    t += QString::number(minutes) + ":";
    if (seconds < 10)
        t += "0";
    t += QString::number(seconds);

    return t;
}

void PuzzleBoard::setCellSize(int s) {
    if (cellSize == s)
        return;

    cellSize = s;

    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            cellArray[r][c].setSize(cellSize);
            cellArray[r][c].makePixmap();
        }
    }

    //Set size of board
    setFixedSize(cols*cellSize, rows*cellSize);
}

QString PuzzleBoard::generateBoard(int rows, int cols) const {
    QVector<CellInfo> cells;
    bool makeNewBoard;
    do {
        makeNewBoard = false;

        //PHASE ONE
        //Create a board with only blank clues and empty nonclues
        //so that the nonclues are completely connected,
        //every clue group size is >= 2 and <= 9
        bool contiguous;
        do {
            cells.clear();

            //Phase One: Part 1
            //Make board with blank clues and empty nonclues,
            //with moderate regard in limiting clue group size

            //Keeps track of last row and col we placed a clue cell
            int lastRow = 0, lastCol = 0;
            for (int r = 0; r < rows; r++) {
                for (int c = 0; c < cols; c++) {
                    int index = r*cols+c;

                    CellInfo info;
                    for (int i = 0; i < 10; i++) {
                        info.notes[i] = 0;
                    }

                    int rand100 = rand()%100+1;
                    int nonClueChance = 70+rand()%30;
                    //Make row 0 and col 0 all clues
                    if (r == 0 || c == 0) {
                        nonClueChance = 0;
                    }
                    else {
                        if (lastRow == r && c > 1) {
                            //Don't do CLUE NONCLUE CLUE,
                            //because that's a right-clue group of size 1
                            if (cells[index-1].type == NONCLUE &&
                                    cells[index-2].type == CLUE) {
                                nonClueChance = 100;
                            }
                            //Don't make right clue groups of size > 9
                            else if (lastCol == c - 10) {
                                nonClueChance = 0;
                            }
                        }

                        //Figure out the last row that had a clue in this col
                        int lastRowInColWithClue = -1;
                        for (int i = index-cols; i >= 0; i -= cols) {
                            if (cells[i].type == CLUE) {
                                lastRowInColWithClue = i/cols;
                                break;
                            }
                        }

                        //Don't make down-clue groups of size > 9
                        if (lastRowInColWithClue == r - 10 && r > 1) {
                            nonClueChance = 0;
                        }

                        //Don't leave down-clue groups of size 1
                        //on the second-to-last row
                        if (lastRowInColWithClue >= rows - 2 &&
                                r == rows - 1) {
                            nonClueChance = 0;
                        }
                        //Don't leave right-clue groups of size 1
                        //on the second-to-last col
                        if (lastCol >= cols - 2 && c == cols - 1) {
                            nonClueChance = 0;
                        }
                    }

                    //Make a clue
                    if (rand100 > nonClueChance) {
                        info.type = CLUE;
                        lastRow = r;
                        lastCol = c;
                    }
                    //Make a nonclue
                    else {
                        info.type = NONCLUE;
                    }

                    //Add to our CellInfo vector
                    info.valueOrClues[0] = 0;
                    info.valueOrClues[1] = 0;
                    info.fixed = 0;
                    cells.push_back(info);
                }
            }

            //Phase One: Part 2
            //Fix the (hopefully minimal) occurences of
            //CLUE NONCLUE CLUE, AKA clue groups of size 1

            bool changed = true;
            //Keep checking until there were no changes
            while (changed) {
                changed = false;

                //For every clue,
                for (int r = 0; r < rows; r++) {
                    for (int c = 0; c < cols; c++) {
                        int index = r*cols+c;

                        if (cells[index].type == NONCLUE)
                            continue;
                        //Loop in all 4 directions (if in bounds),
                        //patching up all NONCLUE CLUE patterns
                        for (int dir = UP; dir <= LEFT; dir++) {
                            if (dir == UP && r < 2) continue;
                            if (dir == RIGHT && c >= cols - 2) continue;
                            if (dir == DOWN && r >= rows - 2) continue;
                            if (dir == LEFT && c < 2) continue;

                            int offset;
                            switch (dir) {
                            case UP:
                                offset = -cols;
                                break;
                            case RIGHT:
                                offset = 1;
                                break;
                            case DOWN:
                                offset = cols;
                                break;
                            case LEFT:
                                offset = -1;
                                break;
                            }
                            //Fill in the middle NONCLUE
                            if (cells[index+offset].type == NONCLUE &&
                                    cells[index+2*offset].type == CLUE) {
                                cells[index+offset].type = CLUE;
                                changed = true;
                            }
                        }
                    }
                }
            }

            //Phase One: Part Three
            //Check that nonclues are completely connected.
            //If they're not, say "screw it" and generate a new board

            //This is a bool-board of our current board, with clues 'on'
            QVector<bool> clues;
            for (int i = 0; i < int(cells.size()); i++) {
                clues.push_back(cells[i].type);
            }

            //Find the first nonclue
            int firstNonClue = 0;
            for (int i = 0; i < int(cells.size()); i++) {
                if (cells[i].type == CLUE)
                    continue;
                firstNonClue = i;
                break;
            }

            //Flood-fill a bool-board from the first nonclue,
            //turning 'on' all of the nonclues
            QVector<bool> filled(clues.size(), 0);
            floodFill(firstNonClue, filled, clues, rows, cols);

            //If there was a discrepancy between the flood filled
            //and the real bool-boards, (AKA there is more than one
            //group of nonclues), generate a new board
            contiguous = true;
            for (int i = 0; i < int(filled.size()); i++) {
                if (!filled[i] != clues[i]) {
                    contiguous = false;
                    break;
                }
            }

            //A check for smaller boards...is it all clues? Regen if so
            if (contiguous) {
                bool hasNonClues = false;
                for (int i = 0; i < int(cells.size()); i++) {
                    if (cells[i].type == NONCLUE) {
                        hasNonClues = true;
                        break;
                    }
                }
                if (!hasNonClues)
                    contiguous = false;
            }

        } while (!contiguous);

        //PHASE TWO
        //Our board is currently filled with blank clues and empty nonclues.
        //Place numbers on all of the nonclues in a legal fashion,
        //and then get the clues from the numbers we placed.

        bool makeNewClues = false;
        int tries = 0;
        do {
            if (tries > 0.5*(rows+cols)) {
                makeNewBoard = true;
                break;
            }
            tries++;
            makeNewClues = false;

            //Phase Two: Part One
            //Start filling in nonclues as legally as possible.
            //If we run into a contradiction somewhere, just restart
            //with newly shuffled vectors.

            for (int i = 0; i < int(cells.size()); i++) {
                cells[i].valueOrClues[0] = 0;
                cells[i].valueOrClues[1] = 0;
                cells[i].fixed = 0;
            }

            bool quit = false, restart = false;

            for (int r = 0; r < rows; r++) {
                if (quit) break;
                //If we contradicted and need to restart
                if (restart) {
                    restart = false;
                    r = -1;
                    for (int i = 0; i < int(cells.size()); i++) {
                        cells[i].valueOrClues[0] = 0;
                    }
                    continue;
                }
                for (int c = 0; c < cols; c++) {
                    if (restart || quit) break;
                    int index = r*cols+c;

                    if (cells[index].type == NONCLUE)
                        continue;

                    //We will go down each clue's column,
                    //filling in numbers legally and randomly

                    //Numbers possible for the clues column
                    QVector<int> numsLeftCol;
                    numsLeftCol.clear();
                    for (int i = 1; i < 10; i++) {
                        numsLeftCol.push_back(i);
                    }
                    //Shuffle!
                    for (int i = 8; i >= 1; i--) {
                        int j = rand()%(i+1);
                        int temp = numsLeftCol[i];
                        numsLeftCol[i] = numsLeftCol[j];
                        numsLeftCol[j] = temp;
                    }

                    //Place numbers on the clues column
                    for (int r2 = r+1; r2 < rows; r2++) {
                        if (restart || quit) break;
                        if (cells[r2*cols+c].type == CLUE)
                            break;

                        //Numbers possible for this current cell,
                        //taking into account the nums used on its row
                        QVector<int> numsLeft = numsLeftCol;

                        //Erase numbers already used on row
                        for (int c2 = c-1; c2 >= 0; c2--) {
                            if (cells[r2*cols+c2].type == CLUE)
                                break;
                            if (cells[r2*cols+c2].valueOrClues[0] == 0)
                                continue;

                            int index2 = numsLeft.indexOf(cells[r2*cols+c2].valueOrClues[0]);
                            if (index2 > -1) {
                                numsLeft.erase(numsLeft.begin() + index2);
                            }
                        }
                        for (int c2 = c+1; c2 < cols; c2++) {
                            if (cells[r2*cols+c2].type == CLUE)
                                break;
                            if (cells[r2*cols+c2].valueOrClues[0] == 0)
                                continue;

                            int index2 = numsLeft.indexOf(cells[r2*cols+c2].valueOrClues[0]);
                            if (index2 > -1) {
                                numsLeft.erase(numsLeft.begin() + index2);
                            }
                        }

                        //If we still have a number to use, use it
                        if (numsLeft.size()) {
                            int v = numsLeft[0];
                            //Erase number from the possible nums left for the col
                            int index3 = numsLeftCol.indexOf(v);
                            if (index3 > -1 ) {
                                numsLeftCol.erase(numsLeftCol.begin() + index3);
                            }
                            //Place the number
                            cells[r2*cols+c].valueOrClues[0] = v;
                        }
                        //Otherwise, placing numbers like this didn't work
                        //and we should just try again
                        else {
                            restart = true;
                            break;
                        }
                    } //end placing col numbers

                } //end for all cols
            } //end for all rows

            //Phase Two: Part Two
            //Count up the numbers we filled the nonclues with,
            //and update our clues with the sums

            //Make clues
            for (int r = 0; r < rows; r++) {
                for (int c = 0; c < cols; c++) {
                    if (cells[r*cols+c].type == NONCLUE) {
                        cells[r*cols+c].notes[cells[r*cols+c].valueOrClues[0]] = 1;
                        continue;
                    }
                    int dSum = 0;
                    for (int r2 = r+1; r2 < rows; r2++) {
                        if (cells[r2*cols+c].type == CLUE)
                            break;
                        dSum += cells[r2*cols+c].valueOrClues[0];
                    }
                    cells[r*cols+c].valueOrClues[0] = dSum;

                    int rSum = 0;
                    for (int c2 = c+1; c2 < cols; c2++) {
                        if (cells[r*cols+c2].type == CLUE)
                            break;
                        rSum += cells[r*cols+c2].valueOrClues[0];
                    }
                    cells[r*cols+c].valueOrClues[1] = rSum;

                }
            }

            //PHASE THREE (last one!)
            //Now that we have a working Kakuro board,
            //we need it to have a unique solution. We'll accomplish
            //this by fixing nonclue cells on possible values until
            //the board is solvable using our logic, restarting when necessary

            PuzzleBoard *board = new PuzzleBoard(convertCellsInfoToKAKString(rows, cols, cells));

            //LOGIC METHOD
            bool solved = board->solve(false);


            //Do a little foresight -- are there a lot of notes?
            //If so, just make some new clues to save time
            int noteCount = 0, unsolvedCellCount = 0;
            for (int r = 0; r < board->rows; r++) {
                for (int c = 0; c < board->cols; c++) {
                    if (board->cellArray[r][c].getType() == CLUE)
                        continue;
                    if (board->cellArray[r][c].getValue())
                        continue;

                    unsolvedCellCount++;
                    for (int n = 1; n < 10; n++) {
                        if (board->cellArray[r][c].getNote(n)) {
                            noteCount++;
                        }
                    }
                }
            }
            if (noteCount > 8*unsolvedCellCount) {
                makeNewClues = true;
                continue;
            }


            //Keep fixing unsolved cells to possible values
            while (!solved) {
                bool quit = false;
                //Find the cell with the least number of notes
                int minNotes = 10, note = 0;
                CellPos cell = { -1, -1 };
                for (int r = 0; r < board->rows; r++) {
                    if (quit) break;
                    for (int c = 0; c < board->cols; c++) {
                        if (quit) break;
                        if (board->cellArray[r][c].getType() == CLUE)
                            continue;
                        if (board->cellArray[r][c].getValue())
                            continue;
                        if (board->cellArray[r][c].getFixed())
                            continue;

                        int noteCount = 0, lastNote = 0;
                        for (int n = 1; n < 10; n++) {
                            if (board->cellArray[r][c].getNote(n)) {
                                noteCount++;
                                if (noteCount > minNotes)
                                    break;
                                lastNote = n;
                            }
                        }
                        //We broke something, can't be solved anymore
                        if (noteCount == 0 || noteCount == 1) {
                            makeNewClues = true;
                            quit = true;
                            break;
                        }
                        if (noteCount < minNotes) {
                            minNotes = noteCount;
                            cell = { r, c };
                            note = lastNote;
                        }
                        if (minNotes == 2)
                            break;
                    }
                }
                if (cell.row == -1 || cell.col == -1 ||
                        board->cellArray[cell.row][cell.col].getFixed())
                    makeNewClues = true;
                if (makeNewClues)
                    break;

                //Fix that cell to one of its notes
                board->cellArray[cell.row][cell.col].setValue(note);
                board->cellArray[cell.row][cell.col].setFixed(true);
                for (int n2 = 1; n2 < 10; n2++) {
                    board->cellArray[cell.row][cell.col].setNote(n2, false);
                }

                //Try to solve again
                solved = board->solve(false);
            }
            //Unique solution! Get info
            board->clearBoard();
            cells = board->getCellsInfoFromCellArray();

            delete board;

        } while (makeNewClues);

    } while (makeNewBoard);

    return convertCellsInfoToKAKString(rows, cols, cells);
}

void floodFill(int index, QVector<bool> & filled, QVector<bool> map, int rows, int cols) {
    if (index < 0 || index >= rows*cols)
        return;

    if (filled[index] || map[index])
        return;

    filled[index] = true;

    if (index%cols != 0)
        floodFill(index-1, filled, map, rows, cols);
    if (index%cols != cols-1)
        floodFill(index+1, filled, map, rows, cols);
    if (index/cols != 0)
        floodFill(index-cols, filled, map, rows, cols);
    if (index/cols != rows-1)
        floodFill(index+cols, filled, map, rows, cols);

}
