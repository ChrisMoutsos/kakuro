/*
 * cell.h
 *
 * This is the Cell class, used to represent a single cell
 * on the board. Its data includes information about the cell,
 * some meta-information about its clue groups, and pointers to the
 * colors it should use to draw itself. Data is set by
 * the cells PuzzleBoard after construction.
 *
 * To draw the cell, we'll give it a QLabel that will hold a QPixmap.
 */

#ifndef CELL_H
#define CELL_H

#include <QWidget>
#include <QLabel>
#include <QPixmap>
#include <QPainter>
#include <QString>
#include "common.h"

class Cell : public QWidget {
    Q_OBJECT

public:
    Cell(int s = 50, int r = 0, int c = 0);

    //Display
    void makePixmap();
    void fill(QColor c);
    void drawBorder();
    void draw(qreal numberAlpha = 1);
    void drawNotes(qreal alpha = 1);
    void drawValue(qreal alpha = 1);
    void drawValue(int v, qreal alpha = 1);
    void drawClueBox();
    void drawClueBox(int dClue, int rClue);
    void handleNumPress(int n);
    void select();
    void unselect();

    //Mutators
    void setValue(int v);
    void setSize(int s) { size = s; }
    void setRowCol(int r, int c) { row = r; col = c; }
    void setType(bool t) { type = t; }
    void setDownClue(int d) { downClue = d; }
    void setRightClue(int r) { rightClue = r; }
    void setNote(int i, bool v) { notes[i] = v; }
    void setNumInDownSum(int x) { numInDownSum = x; }
    void setNumInRightSum(int x) { numInRightSum = x; }
    void setColor(int whichColor, QColor *c) { colors[whichColor] = c; }
    void setFixed(bool f) { fixed = f; }
    void setRightCombos(QVector<QVector<int>> c) { rightCombos = c; }
    void setDownCombos(QVector<QVector<int>> c) { downCombos = c; }

    //Accessors
    bool getNote(int i = 0) const { return notes[i]; }
    bool getType() const { return type; }
    int getSize() const { return size; }
    int getRow() const { return row; }
    int getCol() const { return col; }
    int getValue() const { return value; }
    bool getSelected() const { return selected; }
    int getDownClue() const { return downClue; }
    int getRightClue() const { return rightClue; }
    int getNumInDownSum() const { return numInDownSum; }
    int getNumInRightSum() const { return numInRightSum; }
    bool getFixed() const { return fixed; }
    QLabel *getLabel() const { return label; }
    QVector<QVector<int>> getRightCombos() { return rightCombos; }
    QVector<QVector<int>> getDownCombos() { return downCombos; }


private:
    QLabel *label;

    int size, row, col;

    //NONCLUE = 0, CLUE = 1
    bool type;

    //Nonclue cells current value
    int value;

    //For nonclue cells, if the value is fixed or not
    bool fixed;

    //notes[0] is whether or not note display is active.
    //The rest of the members are whether or not the
    //number (index) is turned 'on' in the notes
    bool notes[10];

    //If the cell is selected
    bool selected;

    //For clue-cells goals,
    //and for nonclue-cells metaknowledge
    int downClue, rightClue;
    int numInDownSum, numInRightSum;

    //Combos possible, for clue cells
    QVector<QVector<int>> rightCombos, downCombos;

    //Colors
    QColor *colors[7];
};

#endif
