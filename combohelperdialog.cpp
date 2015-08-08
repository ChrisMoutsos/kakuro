/*
 * combohelperdialog.cpp
 * See combohelperdialog.h for more information
 */

#include "combohelperdialog.h"

ComboHelperDialog::ComboHelperDialog(PuzzleBoard *b) {
    board = b;

    for (int sum = 0; sum < 46; sum++) {
        for (int num = 0; num < 10; num++) {
            sumInNumCombo[sum][num] = board->getSumInNum(sum, num);
        }
    }

    currSum = 3;
    currNum = 2;

    //Make the dialog
    setWindowTitle(tr("Combo Helper"));

    QLabel *sumLabel = new QLabel(tr("Sum:"));
    sumCombo = new QComboBox;
    for (int i = 3; i <= 45; i++) {
        sumCombo->addItem(QString::number(i), i);
    }
    sumCombo->addItem("All sums", 46);
    QLabel *numLabel = new QLabel(tr("Number:"));
    numCombo = new QComboBox;
    for (int i = 2; i <= 9; i++) {
        numCombo->addItem(QString::number(i), i);
    }
    numCombo->addItem("All numbers", 10);

    //Set pre-selected values
    int index = sumCombo->findData(currSum);
    if (index != -1) {
        sumCombo->setCurrentIndex(index);
    }
    index = numCombo->findData(currNum);
    if (index != -1) {
        numCombo->setCurrentIndex(index);
    }

    textBox = new QPlainTextEdit;

    //Control buttons
    QPushButton *closeButton = new QPushButton(tr("Close"));

    //Add to mainLayout
    QGridLayout *mainLayout = new QGridLayout;
    mainLayout->addWidget(sumLabel, 0, 0);
    mainLayout->addWidget(sumCombo, 0, 1);
    mainLayout->addWidget(new QLabel("in"), 1, 1);
    mainLayout->addWidget(numLabel, 2, 0);
    mainLayout->addWidget(numCombo, 2, 1);
    mainLayout->addWidget(textBox, 3, 0, 1, 2);
    mainLayout->addWidget(closeButton, 4, 0, 1, 2);

    //Connections
    connect(sumCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(setCurrSum()));
    connect(sumCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(updateTextBox()));
    connect(numCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(setCurrNum()));
    connect(numCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(updateTextBox()));
    connect(closeButton, SIGNAL(clicked()), this, SLOT(close()));

    setLayout(mainLayout);

    updateTextBox();
}

void ComboHelperDialog::setCurrSum() {
    currSum = sumCombo->itemData(sumCombo->currentIndex()).toInt();
}

void ComboHelperDialog::setCurrNum() {
    currNum = numCombo->itemData(numCombo->currentIndex()).toInt();
}

void ComboHelperDialog::updateTextBox() {
    textBox->clear();
    QString s;

    int minSum, maxSum, minNum, maxNum;
    minSum = currSum < 46 ? currSum : 3;
    maxSum = currSum < 46 ? currSum : 45;
    minNum = currNum < 10 ? currNum : 2;
    maxNum = currNum < 10 ? currNum : 9;

    for (int num = minNum; num <= maxNum; num++) {
        for (int sum = minSum; sum <= maxSum; sum++) {
            QVector<QVector<int>> combos = sumInNumCombo[sum][num];

            for (int i = 0; i < int(combos.size()); i++) {
                if (minSum != maxSum) {
                    s += "(" + QString::number(sum) + ") ";
                }
                for (int j = 0; j < int(combos[i].size()); j++) {
                    s += QString::number(combos[i][j]);
                }
                s += "\n";
            }
        }
    }

    textBox->appendPlainText(s);
}
