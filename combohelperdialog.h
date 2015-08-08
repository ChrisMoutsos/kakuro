/*
 * combohelperdialog.h
 *
 * The ComboHelperDialog class is used to create a pop-up
 * dialog that lets the user choose a certain sum and a
 * certain number of numbers, and then displays the
 * possible combinations for that "sum" in "num."
 * There are two QComboBoxes (for choosing "sum"
 * and "num"), and a QPlainTextEdit to show the combinations.
 *
 * A pointer to a PuzzleBoard is present so that on construction,
 * we can copy the data from the PuzzleBoards sumInNumCombo[SUM][NUM]
 * into ours. After the copy, the pointer is not used.
 */

#ifndef COMBOHELPERDIALOG_H
#define COMBOHELPERDIALOG_H

#include <QWidget>
#include <QtWidgets>
#include "puzzleboard.h"

class ComboHelperDialog : public QDialog {
    Q_OBJECT

public:
    ComboHelperDialog(PuzzleBoard *b);

private slots:
    void setCurrSum();
    void setCurrNum();
    void updateTextBox();

private:
    PuzzleBoard *board;
    QVector<QVector<int>> sumInNumCombo[46][10];
    QComboBox *sumCombo, *numCombo;
    QPlainTextEdit *textBox;
    int currSum, currNum;
};

#endif
