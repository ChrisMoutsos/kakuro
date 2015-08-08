/*
 * puzzleboard.h
 *
 * The PuzzleBoard is where the Kakuro is actually located.
 * It handles keyboard and mouse input used to play the game.
 * It also includes the solving and generating functions.
 *
 * Its data includes a two-dimensional array of Cells (cellArray),
 * a vector of CellInfos that is a barebones representation
 * of cellArray (used for faster calculations), and an array of
 * vectors of vectors of ints (sumInNumCombo[SUM][NUM])
 * used to store the combinations of every "sum" in "num."
 *
 * See note in mainwindow.h about KAKStrings.
 *
 */

#ifndef PUZZLEBOARD_H
#define PUZZLEBOARD_H

#include <QWidget>
#include <QMainWindow>
#include <QGridLayout>
#include <QPixmap>
#include <QPainter>
#include <QTextStream>
#include "cell.h"
#include "common.h"

class PuzzleBoard : public QWidget {
    Q_OBJECT

public:
    PuzzleBoard(QString s = 0);
    ~PuzzleBoard();

    void clearBoard();

    //Events
    void keyPressEvent(QKeyEvent * event);
    void handleMouse(QMouseEvent * mouseEvent);

    //KAKString and saving/loading
    void makeBoardFromKAKString(QString s);
    bool lazyValidateKAKString(QString s) const;
    QString getKAKString() const;
    QString getTimeFormatted() const;

    //Solving related
    bool solve(bool useBruteForce = true);
    bool checkSolved() const;

    //Returns KAKString of generated board with unique solution
    QString generateBoard(int rows, int cols) const;

    //Accessors
    int getCellSize() const { return cellSize; }
    int getRows() const { return rows; }
    int getCols() const { return cols; }
    int getSeconds() const { return seconds; }
    QVector<QVector<int>> getSumInNum(int s, int n) const { return sumInNumCombo[s][n]; }

    //Mutators
    void setCellSize(int s);
    void setRows(int r) { if (r < 0) return; rows = r; }
    void setCols(int c) { if (c < 0) return; cols = c; }
    void setSeconds(int s) { if (s < 0) return; seconds = s; }
    void setColor(int whichColor, QColor *color);

public slots:
    void drawBoard();

private:
    //Events
    void handleMouseMove(CellPos pos);
    void handleMouseLeftPress(CellPos pos);
    void handleMouseRightPress(CellPos pos);
    void handleMouseLeftRelease(CellPos pos);

    //General utility
    void initSumInNumCombos();
    void toggleSelectOnCell(CellPos pos);
    void clearColumnFrom(CellPos pos);
    void clearRowFrom(CellPos pos);
    CellPos getFirstNonClueCell() const;
    CellPos getNextNonClueCell(int dir) const;
    int getValidDownSumFromCell(CellPos pos) const;
    int getValidRightSumFromCell(CellPos pos) const;
    int getMinNoteForCell(CellPos pos) const;
    int getMaxNoteForCell(CellPos pos) const;

    //KAKString and saving/loading
    void makeNewCellArray(int newRows, int newCols);
    QVector<CellInfo> getCellsInfoFromCellArray() const;
    void updateCellArray();
    void updateCellArray(QVector<CellInfo> info);
    void setCellsInfo(QString s);
    QString convertCellsInfoToKAKString(int rows, int cols, QVector<CellInfo> info) const;

    //Solving related
    void putBackFixedValues();
    void giveMetaKnowledgeToCells();
    bool updateClueCellCombos();
    void setCellValueAndEraseNeighborNoteDups(CellPos cell, int val);
    bool* getIntersectNotesForCell(CellPos pos) const;
    bool writeNotesFromIntersections();
    void removeNotesFixedValues();
    bool logicSolve(bool lazy);
    bool writeCellsWithOneNoteAndRemoveDupNotes();
    bool adjustNotesByLogicalRange();
    bool solveUniquesWithOneEmpty();
    bool removeExtraNotesFromUniques();
    bool solveCellsWithNecessaryValue();
    bool removeNotesNotInPossibleCombos();
    bool removeNotesNotInPossiblePerms();
    bool smartBruteForceSolve();
    bool hasEmptyCells() const;

    //Number of rows, cols, and cellSize
    int rows, cols, cellSize;

    CellPos selectedCell, draggingCell;

    //Seconds passed
    int seconds;

    //Info about the cells, used for storing/loading board
    //configurations. Not guaranteed to be in sync with cellArray cells
    QVector<CellInfo> cellsInfo;
    //2D array for cells
    Cell **cellArray;
    //Grid layout for cells
    QGridLayout *gridLayout;

    //Sum combinations
    QVector<QVector<int>> sumInNumCombo[46][10];
    //Does the sum in num have only one combo?
    bool sumInNumIsUnique[46][10];

    const qreal DRAG_OPACITY = 0.7;

    //Colors
    QColor *colors[7];
};

#endif
