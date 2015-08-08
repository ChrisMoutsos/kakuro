/*
 * mainwindow.cpp
 * See mainwindow.h for more information
 */

#include "mainwindow.h"
#include "combohelperdialog.h"
#include <QDebug>
#include <stdlib.h>
#include <time.h>
#include <QTextDocument>

MainWindow::MainWindow() {
    installEventFilter(this);
    this->setMouseTracking(true);
    srand(time(NULL));

    setWindowTitle("Kakuro");

    AUTOSAVE_INTERVAL = 60;
    AUTOSAVE_DISPLAY_TIME = 3;
    timerId = 0;
    board = 0;
    newGameD = 0;
    settingsD = 0;

    setColorsDefault();

    makePuzzleBoard(QString("10x10 50 "
                            "- 7/0 34/0 - 28/0 23/0 41/0 - 32/0 10/0 "
                            "0/7 0 0 0/15 0 0 0 0/3 0 0 "
                            "0/14 0 0 41/23 0 0 0 28/15 0 0 "
                            "- 7/28 0 0 0 0 0 0 0 11/0 "
                            "0/29 0 0 0 0 0/14 0 0 0 0 "
                            "0/18 0 0 0 0 3/29 0 0 0 0 "
                            "- - 14/16 0 0 0 0 0 16/0 - "
                            "- 8/41 0 0 0 0 0 0 0 4/0 "
                            "0/9 0 0 0 - - 0/7 0 0 0 "
                            "0/22 0 0 0 - - 0/17 0 0 0 "
                            "t00:00:00"));

    createStatusBar();
    createActions();
    createMenus();
    createToolBars();
}

MainWindow::~MainWindow() {
    killTimer(timerId);
}

void MainWindow::makePuzzleBoard(QString KAKString) {
    if (!board) {
        board = new PuzzleBoard(KAKString);
    }
    else {
        board->makeBoardFromKAKString(KAKString);
    }

    if (timerId) killTimer(timerId);
    timerId = startTimer(1000);

    setFixedSize(board->width()+0, board->height()+75);
    setCentralWidget(board);

    updateLastSavedKAKString(KAKString);
    setBoardColors();
}

void MainWindow::setColorsDefault() {
    //Set colors to their defaults
    colors[CLUECOLOR] = new QColor(0, 0, 0, 255);
    colors[NONCLUECOLOR] = new QColor(255, 255, 255, 255);
    colors[CLUETEXTCOLOR] = new QColor(255, 255, 255, 255);
    colors[NONCLUETEXTCOLOR] = new QColor(0, 0, 0, 255);
    colors[NOTECOLOR] = new QColor(95, 95, 95, 255);
    colors[SELECTCOLOR] = new QColor(121, 213, 252, 100);
    colors[BORDERCOLOR] = new QColor(0, 0, 0, 255);
}

void MainWindow::newGame() {
    //Make the dialog
    newGameD = new QDialog;
    newGameD->setWindowTitle(tr("New Game"));

    //Create content
    //Group box for entire page
    QGroupBox *configGroup = new QGroupBox(tr("Board settings"));

    //Group box for board size
    QGroupBox *boardSizeGroup = new QGroupBox(tr("Board size:"));
    //Row and column combo boxes
    QLabel *rowLabel = new QLabel(tr("Rows:"));
    rowCombo = new QComboBox;
    for (int i = 3; i <= 17; i++) {
        rowCombo->addItem(QString::number(i), i);
    }
    QLabel *colLabel = new QLabel(tr("Columns:"));
    colCombo = new QComboBox;
    for (int i = 3; i <= 17; i++) {
        colCombo->addItem(QString::number(i), i);
    }
    //Layout for boardSizeGroup
    QGridLayout *boardSizeLayout = new QGridLayout;
    boardSizeLayout->addWidget(rowLabel, 0, 0);
    boardSizeLayout->addWidget(rowCombo, 0, 1);
    boardSizeLayout->addWidget(colLabel, 1, 0);
    boardSizeLayout->addWidget(colCombo, 1, 1);
    boardSizeGroup->setLayout(boardSizeLayout);

    //Add everything to the config grid
    QGridLayout *configLayout = new QGridLayout;
    configLayout->addWidget(boardSizeGroup, 0, 0);
    configGroup->setLayout(configLayout);

    //Control buttons
    QPushButton *makeButton = new QPushButton(tr("Generate board"));
    QPushButton *cancelButton = new QPushButton(tr("Cancel"));
    //Add buttons to their layout
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(makeButton);
    buttonLayout->addWidget(cancelButton);
    //Make a holder widget for buttons
    QWidget *buttonWidget = new QWidget;
    buttonWidget->setLayout(buttonLayout);

    //Add configGroup and buttonWidget to mainLayout
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(configGroup);
    mainLayout->addWidget(new QLabel(tr("Note: generating a new board may take a while")));
    mainLayout->addWidget(buttonWidget);
    mainLayout->addStretch(1);

    newGameD->setLayout(mainLayout);

    //Connect all the buttons
    connect(makeButton, SIGNAL(clicked()), this, SLOT(makeNewGame()));
    connect(cancelButton, SIGNAL(clicked()), newGameD, SLOT(close()));

    connect(rowCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(setNewRows()));
    connect(colCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(setNewCols()));

    //Set pre-selected values
    newRows = board->getRows() < 17 ? board->getRows() : 8;
    int index = rowCombo->findData(newRows);
    if (index != -1) {
        rowCombo->setCurrentIndex(index);
    }
    newCols = board->getCols() < 17 ? board->getCols() : 8;
    index = colCombo->findData(newCols);
    if (index != -1) {
        colCombo->setCurrentIndex(index);
    }

    newGameD->exec();
}

void MainWindow::makeNewGame() {
    if (!newGameD) return;
    QString s = board->generateBoard(newRows, newCols);
    makePuzzleBoard(s);
    newGameD->close();
}

void MainWindow::setNewRows() {
    newRows = rowCombo->itemData(rowCombo->currentIndex()).toInt();
}

void MainWindow::setNewCols() {
    newCols = colCombo->itemData(colCombo->currentIndex()).toInt();
}

void MainWindow::setNewCellSize() {
    newCellSize = cellSizeCombo->itemData(cellSizeCombo->currentIndex()).toInt();
}

void MainWindow::open() {
    if (areYouSure()) {
        QString fName = QFileDialog::getOpenFileName(this,
                                                     tr("Load Kakuro game"),
                                                     "",
                                                     "Kakuro files (*.kak);;"
                                                     "Text files (*.txt)");

        if (!fName.isEmpty())
            loadFile(fName);
    }
}

void MainWindow::loadFile(const QString &fileName) {
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("Kakuro"),
                             tr("Cannot read file %1:\n%2.")
                             .arg(fileName)
                             .arg(file.errorString()));
        return;
    }

    QTextStream in(&file);
#ifndef QT_NO_CURSOR
    QApplication::setOverrideCursor(Qt::WaitCursor);
#endif

    QString saved = in.readAll();

#ifndef QT_NO_CURSOR
    QApplication::restoreOverrideCursor();
#endif

    if (!board->lazyValidateKAKString(saved)) {
        QMessageBox::warning(this, tr("Kakuro"),
                             tr("Invalid file! What are you even doing?"));
        return;
    }

    makePuzzleBoard(saved);
    updateLastSavedFileName(fileName);
    updateStatusTimer();
}

bool MainWindow::areYouSure() {
    QString currKAKString = board->getKAKString();
    currKAKString.truncate(currKAKString.size()-9);

    if (lastSavedKAKString != currKAKString) {
        QMessageBox::StandardButton ans;
        ans = QMessageBox::warning(this, tr("Kakuro"),
                                   tr("Do you want to save the current game?"),
                                   QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        if (ans == QMessageBox::Save)
            return save();
        else if (ans == QMessageBox::Cancel)
            return false;
    }
    return true;
}

bool MainWindow::save() {
    //Show the saveAs menu if you've either
    //never saved a file before, or you clicked
    //save when there was an backupFile
    if (lastSavedFileName.isEmpty() || !backupFile.isEmpty()) {
        bool saved = saveAs();

        //Delete autosaved file if it exists
        if (!backupFile.isEmpty()) {
            QFile file(backupFile);
            file.remove();
            backupFile = "";
        }

        return saved;
    }

    //If you already have a saved file, just save it
    return saveFile(lastSavedFileName);
}

bool MainWindow::saveAs() {
    QFileDialog dialog(this);
    dialog.setWindowModality(Qt::WindowModal);
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setDefaultSuffix((".kak"));

    QStringList filters;
    filters << "Kakuro files (*.kak)"
            << "Text files (*.txt)";
    dialog.setNameFilters(filters);

    QStringList files;
    if (dialog.exec())
        files = dialog.selectedFiles();
    else
        return false;

    return saveFile(files.at(0));
}

bool MainWindow::saveFile(const QString &fileName) {
    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("Application"),
                             tr("Cannot write file %1:\n%2.")
                             .arg(fileName)
                             .arg(file.errorString()));
        return false;
    }

    QTextStream out(&file);
#ifndef QT_NO_CURSOR
    QApplication::setOverrideCursor(Qt::WaitCursor);
#endif

    QString saved = board->getKAKString();
    out << saved;

#ifndef QT_NO_CURSOR
    QApplication::restoreOverrideCursor();
#endif

    updateLastSavedKAKString(saved);
    lastSavedFileName = fileName;

    return true;
}

void MainWindow::updateLastSavedKAKString(const QString &KAKstring) {
    //If no KAKstring provided, just get it from the board
    if (KAKstring == "" && board) {
        lastSavedKAKString = board->getKAKString();
        lastSavedKAKString.truncate(lastSavedKAKString.size() - 9);
    }
    else {
        lastSavedKAKString = KAKstring;
        //If there's a time, cut it out
        if (lastSavedKAKString.contains(':')) {
            lastSavedKAKString.truncate(lastSavedKAKString.size() - 9);
        }
    }
}

void MainWindow::updateLastSavedFileName(const QString &fileName) {
    lastSavedFileName = fileName;
}

void MainWindow::closeEvent(QCloseEvent *event) {
    if (areYouSure()) {
        event->accept();
    }
    else {
        event->ignore();
    }
}

void MainWindow::rules() {
    QString rulesStr = "Kakuro is a logic puzzle game, similar to Sudoku. "
                       "Some call it the mathematical version of a crossword puzzle. \n\n"
                       "Each cell on a Kakuro grid is either a clue or a nonclue ('entry'). "
                       "A clue cell can have up to 2 clues. If the clue number is above the slash, "
                       "it's a clue for the row. If it's below the slash, it's a clue for "
                       "the column.\n\n"
                       "The objective is to insert a digit from 1 to 9 (inclusive) into "
                       "each entry cell such that the sum of the numbers in each "
                       "group matches the clue associated with it and that no digit "
                       "is duplicated in any entry.\n\n"
                       "How to use the program:\n"
                       "Cell selection/navigation is controlled by your mouse, arrows keys, or WASD. "
                       "Once you have selected a cell, enter a number by typing it (0-9). "
                       "To write 'notes' on the cell, right click or press 'n', and then enter the notes. "
                       "This is useful for more difficult puzzles. "
                       "Right click the clue number to quickly clear the group associated with it.\n\n"
                       "Have fun!";
    QMessageBox::information(this, "Help", rulesStr);
}

void MainWindow::about() {
    QString aboutStr = "Thanks for your support!\n\n- Chris Moutsos"
                       "\n\nhttps://www.github.com/ChrisMoutsos";

    QMessageBox::information(this, "About", aboutStr);
}

void MainWindow::toggleTimer() {
    if (!board)
        return;

    timerToolBar->clear();

    //Timer is ticking, so stop it
    if (timerId) {
        killTimer(timerId);
        timerId = 0;
        timerToolBar->addAction(startTimerAct);

        statusTimer->setText("PAUSED! Time: " + board->getTimeFormatted());
    }
    //Timer is not ticking, so start it
    else {
        timerId = startTimer(1000);
        timerToolBar->addAction(pauseTimerAct);
        updateStatusTimer();
    }
}

void MainWindow::reset() {
    if (board) {
        QMessageBox::StandardButton ans;
        ans = QMessageBox::warning(this, tr("Kakuro"),
                                   tr("This will clear all values and the timer!\n"
                                      "Are you sure you want to reset?"),
                                   QMessageBox::Yes | QMessageBox::No);
        if (ans == QMessageBox::Yes) {
            board->clearBoard();
            board->setSeconds(0);
            //Start timer if it's paused
            if (!timerId) {
                timerId = startTimer(1000);
                timerToolBar->clear();
                timerToolBar->addAction(pauseTimerAct);
            }
            updateStatusTimer();
        }
    }
}

void MainWindow::checkSolved() {
    if (!board)
        return;

    QString info;
    if (board->checkSolved()) {
        info = "Solved!";
    }
    else {
        info = "Not solved yet. Keep trying!";
    }

    QMessageBox::information(this, "Kakuro", info);
}

void MainWindow::checkSolvable() {
    if (!board)
        return;

    QString savedKAK = board->getKAKString();

    QString info;
    if (board->solve()) {
        info = "This Kakuro is solvable.";
    }
    else {
        info = "Oops! This Kakuro is unsolvable.";
    }

    board->makeBoardFromKAKString(savedKAK);

    QMessageBox::information(this, "Kakuro", info);
}

void MainWindow::solveBoard() {
    if (!board)
        return;

    QString savedKAK = board->getKAKString();

    QString info;
    if (board->solve()) {
        info = "Solved!";
    }
    else {
        info = "Oops! This Kakuro is unsolvable.";
        board->makeBoardFromKAKString(savedKAK);
    }

    QMessageBox::information(this, "Kakuro", info);
}

void MainWindow::comboHelper() {
    ComboHelperDialog *d = new ComboHelperDialog(board);
    d->show();
}

void MainWindow::settings() {
    //Make the dialog
    settingsD = new QDialog;
    settingsD->setWindowTitle(tr("Settings"));
    settingsD->setFixedWidth(500);

    //Create content
    //Group box for display settings
    QGroupBox *settingsGroup = new QGroupBox(tr("Cell size"));

    //Cell size combo box
    QLabel *cellSizeLabel = new QLabel(tr("Cell size:"));
    cellSizeCombo = new QComboBox;
    for (int i = 20; i <= 100; i+=5) {
        cellSizeCombo->addItem(QString::number(i) + "px", i);
    }
    QGroupBox *colorGroup = new QGroupBox(tr("Colors"));
    QLabel *colorInfo = new QLabel("Enter colors using hexadecimal format.");
    QLabel *colorLabels[7];
    colorLabels[CLUECOLOR] = new QLabel(tr("Clue background color:"));
    colorLabels[NONCLUECOLOR] = new QLabel(tr("Non-clue background color:"));
    colorLabels[CLUETEXTCOLOR] = new QLabel(tr("Clue text color:"));
    colorLabels[NONCLUETEXTCOLOR] = new QLabel(tr("Non-clue text color:"));
    colorLabels[BORDERCOLOR] = new QLabel(tr("Border color:"));
    colorLabels[NOTECOLOR] = new QLabel(tr("Notes text color:"));
    colorLabels[SELECTCOLOR] = new QLabel(tr("Highlight color:"));
    for (int i = 0; i < 7; i++) {
        colorLineEdits[i] = new QLineEdit;
        colorLineEdits[i]->setText(colors[i]->name());
    }

    QGridLayout *colorLayout = new QGridLayout;
    colorLayout->addWidget(colorInfo, 0, 0);
    for (int i = 0; i < 7; i++) {
        colorLayout->addWidget(colorLabels[i], i+1, 0);
        colorLayout->addWidget(colorLineEdits[i], i+1, 1);
    }
    colorLayout->setHorizontalSpacing(0);
    colorGroup->setLayout(colorLayout);

    //Add everything to the config grid
    QGridLayout *configLayout = new QGridLayout;
    configLayout->addWidget(cellSizeLabel, 0, 0);
    configLayout->addWidget(cellSizeCombo, 0, 1);
    settingsGroup->setLayout(configLayout);

    //Control buttons
    QPushButton *makeButton = new QPushButton(tr("Adjust settings"));
    QPushButton *restoreButton = new QPushButton(tr("Restore defaults"));
    QPushButton *cancelButton = new QPushButton(tr("Cancel"));
    //Add buttons to their layout
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(makeButton);
    buttonLayout->addWidget(restoreButton);
    buttonLayout->addWidget(cancelButton);
    //Make a holder widget for buttons
    QWidget *buttonWidget = new QWidget;
    buttonWidget->setLayout(buttonLayout);

    //Add configGroup and buttonWidget to mainLayout
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(settingsGroup);
    mainLayout->addWidget(colorGroup);
    mainLayout->addWidget(buttonWidget);
    mainLayout->addStretch(1);

    settingsD->setLayout(mainLayout);

    //Connect all the buttons
    connect(makeButton, SIGNAL(clicked()), this, SLOT(saveSettings()));
    connect(restoreButton, SIGNAL(clicked()), this, SLOT(restoreDefaultSettings()));
    connect(cancelButton, SIGNAL(clicked()), settingsD, SLOT(close()));

    connect(cellSizeCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(setNewCellSize()));

    //Set pre-selected values
    newCellSize = board->getCellSize() > 0 ? board->getCellSize() : 50;
    int index = cellSizeCombo->findData(newCellSize);
    if (index != -1) {
        cellSizeCombo->setCurrentIndex(index);
    }

    settingsD->exec();
}

void MainWindow::saveSettings() {
    //Set new cell size
    board->setCellSize(newCellSize);

    //Set new colors
    QColor colorHolder;
    for (int i = 0; i < 7; i++) {
        colorHolder.setNamedColor(colorLineEdits[i]->text());
        if (colorHolder.isValid()) {
            colors[i]->setNamedColor(colorLineEdits[i]->text());
        }
    }
    setBoardColors();

    //Adjust sizes
    setFixedSize(board->width()+0, board->height()+75);

    //Redraw board
    board->drawBoard();

    settingsD->close();
}

void MainWindow::restoreDefaultSettings() {
    setColorsDefault();
    for (int i = 0; i < 7; i++) {
        colorLineEdits[i]->setText(colors[i]->name());
        board->setColor(i, colors[i]);
    }
}

void MainWindow::setBoardColors() {
    if (!board)
        return;

    for (int i = 0; i < 7; i++) {
        board->setColor(i, colors[i]);
    }
}

void MainWindow::createActions() {
    //Acts for fileMenu
    newAct = new QAction(QIcon(":/res/new.png"), tr("&New game"), this);
    newAct->setShortcuts(QKeySequence::New);
    newAct->setStatusTip(tr("Start a new game"));
    connect(newAct, SIGNAL(triggered()), this, SLOT(newGame()));

    openAct = new QAction(QIcon(":/res/open.png"), tr("&Open game"), this);
    openAct->setShortcuts(QKeySequence::Open);
    openAct->setStatusTip(tr("Open an existing game"));
    connect(openAct, SIGNAL(triggered()), this, SLOT(open()));

    saveAct = new QAction(QIcon(":/res/save.png"), tr("&Save game"), this);
    saveAct->setShortcuts(QKeySequence::Save);
    saveAct->setStatusTip(tr("Save current game"));
    connect(saveAct, SIGNAL(triggered()), this, SLOT(save()));

    saveAsAct = new QAction(QIcon(":/res/saveas.png"), tr("Save game &as"), this);
    saveAsAct->setShortcuts(QKeySequence::SaveAs);
    saveAsAct->setStatusTip(tr("Save current game as..."));
    connect(saveAsAct, SIGNAL(triggered()), this, SLOT(saveAs()));

    exitAct = new QAction(tr("&Exit"), this);
    exitAct->setShortcuts(QKeySequence::Quit);
    exitAct->setStatusTip(tr("Exit"));
    connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));

    //Acts for helpMenu
    rulesAct = new QAction(QIcon(":/res/rules.png"), tr("&Help"), this);
    rulesAct->setStatusTip(tr("Learn the rules of Kakuro"));
    connect(rulesAct, SIGNAL(triggered()), this, SLOT(rules()));

    aboutAct = new QAction(QIcon(":/res/about.png"), tr("&About"), this);
    aboutAct->setStatusTip(tr("About Kakuro"));
    connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));

    //Acts for timer
    startTimerAct = new QAction(QIcon(":/res/starttimer.png"), tr("Start timer"), this);
    startTimerAct->setStatusTip(tr("Start the timer"));
    connect(startTimerAct, SIGNAL(triggered()), this, SLOT(toggleTimer()));

    pauseTimerAct = new QAction(QIcon(":/res/pausetimer.png"), tr("Pause timer"), this);
    pauseTimerAct->setStatusTip(tr("Pause the timer"));
    connect(pauseTimerAct, SIGNAL(triggered()), this, SLOT(toggleTimer()));

    //Acts for solver
    checkSolvedAct = new QAction(tr("Check if solved"), this);
    checkSolvedAct->setStatusTip(tr("Check if the current board is solved"));
    connect(checkSolvedAct, SIGNAL(triggered()), this, SLOT(checkSolved()));

    checkSolvableAct = new QAction(tr("Check if solvable"), this);
    checkSolvableAct->setStatusTip(tr("Check if the current board is able to be solved"));
    connect(checkSolvableAct, SIGNAL(triggered()), this, SLOT(checkSolvable()));

    solveBoardAct = new QAction(tr("Solve the board"), this);
    solveBoardAct->setStatusTip(tr("Solve the board"));
    connect(solveBoardAct, SIGNAL(triggered()), this, SLOT(solveBoard()));

    comboHelpAct = new QAction(tr("Combo helper"), this);
    comboHelpAct->setStatusTip(tr("Open the combo helper"));
    connect(comboHelpAct, SIGNAL(triggered()), this, SLOT(comboHelper()));

    //Settings
    settingsAct = new QAction(QIcon(":/res/settings.png"), tr("Settings"), this);
    settingsAct->setStatusTip(tr("Adjust settings"));
    connect(settingsAct, SIGNAL(triggered()), this, SLOT(settings()));

    resetAct = new QAction(QIcon(":/res/reset.png"), tr("Reset game"), this);
    resetAct->setStatusTip(tr("Reset the current game"));
    connect(resetAct, SIGNAL(triggered()), this, SLOT(reset()));
}

void MainWindow::createMenus() {
    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(newAct);
    fileMenu->addAction(openAct);
    fileMenu->addAction(saveAct);
    fileMenu->addAction(saveAsAct);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAct);

    solverMenu = menuBar()->addMenu(tr("Solver"));
    solverMenu->addAction(comboHelpAct);
    solverMenu->addAction(checkSolvedAct);
    solverMenu->addAction(checkSolvableAct);
    solverMenu->addAction(solveBoardAct);
    solverMenu->addAction(resetAct);

    settingsMenu = menuBar()->addMenu(tr("Settings"));
    settingsMenu->addAction(settingsAct);

    helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(rulesAct);
    helpMenu->addSeparator();
    helpMenu->addAction(aboutAct);
}

void MainWindow::createToolBars() {
    fileToolBar = addToolBar(tr("File"));
    fileToolBar->addAction(newAct);
    fileToolBar->addAction(openAct);
    fileToolBar->addAction(saveAct);

    helpToolBar = addToolBar(tr("Help"));
    helpToolBar->addAction(rulesAct);
    helpToolBar->addAction(aboutAct);

    timerToolBar = addToolBar(tr("Timer"));
    timerToolBar->addAction(pauseTimerAct);

    toolsToolBar = addToolBar(tr("Tools"));
    toolsToolBar->addAction(settingsAct);
    toolsToolBar->addAction(resetAct);
}

void MainWindow::createStatusBar() {
    autoSaveLabel = new QLabel;
    statusBar()->addPermanentWidget(autoSaveLabel);

    statusTimer = new QLabel;
    updateStatusTimer();
    statusBar()->addPermanentWidget(statusTimer);
}

void MainWindow::updateStatusTimer() {
    if (!board) {
        statusTimer->setText("");
        return;
    }
    statusTimer->setText("Time: " + board->getTimeFormatted());
}

void MainWindow::timerEvent(QTimerEvent *event) {
    if (!board) return;

    board->setSeconds(board->getSeconds()+1);
    updateStatusTimer();

    //Autosave at intervals (if there's been a change)
    if (board->getSeconds()%AUTOSAVE_INTERVAL == 0) {
        QString currStr = board->getKAKString();
        currStr.truncate(currStr.size()-9);
        if (lastSavedKAKString != currStr) {
            //If there's no save file, make a backup one
            if (lastSavedFileName.isEmpty()) {
                backupFile = "backup" + QString::number(rand()%100000) + ".kak";
                saveFile(backupFile);
                autoSaveLabel->setText("Autosaved at " +
                                       board->getTimeFormatted() +
                                       " to file " + lastSavedFileName);
                return;
            }

            //If there was a previous file (either back or user-created), use it
            saveFile(lastSavedFileName);
            autoSaveLabel->setText("Autosaved at " +
                                   board->getTimeFormatted() +
                                   " to file " + lastSavedFileName);
        }
    }
    //Clear the status bar label after a certain time
    if (board->getSeconds()%AUTOSAVE_INTERVAL == AUTOSAVE_DISPLAY_TIME) {
        autoSaveLabel->setText("");
    }
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event) {
    //Don't let the user play if the clock is stopped
    if (!timerId) return false;
    if (!board) return false;
    //Send most mouse and keyboard inputs to the board
    if (event->type() == QEvent::MouseMove ||
            event->type() == QEvent::HoverMove ||
            event->type() == QEvent::MouseButtonPress ||
            event->type() == QEvent::MouseButtonRelease) {
        if (board->rect().contains(board->mapFromGlobal(QCursor::pos()))) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
            board->handleMouse(mouseEvent);
        }
    }
    else if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        board->keyPressEvent(keyEvent);
    }
    return false;
}
