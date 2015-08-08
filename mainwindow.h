/*
 * mainwindow.h
 *
 * The MainWindow class is the backbone of the application.
 * It holds all of the menus, toolbars, saving/loading
 * functions, settings, etc., and will hold a PuzzleBoard
 * as its central widget. Input on the PuzzleBoard is handled
 * in PuzzleBoard, not here.
 *
 * -----------------------------------------------------------
 *
 * Note on KAKStrings:
 * A 'KAKString' is a string that is in a format
 * settled on to represent and identify a Kakuro board.
 * It stores board size, cell size, clue values, nonclue
 * values and notes (and whether its fixed),
 * and the total time spent on the puzzle.
 * Format:
 * "$ROWSx$COLS $CELLSIZE "
 * (where $CELLSIZE is in pixels)
 * followed by either:
 * 1) "-" for a completely blank clue cell,
 * 2) "$DOWNCLUE/$RIGHTCLUE" for not-blank clue cells,
 * 3) "$VALUE" for non-clue cells, possibly followed by
 *    (if there are any notes turned on)
 *    "n" and then all of the notes turned on in that cell
 *    or
 *    (if the cell is fixed)
 *    "f"
 * The cells are separated by spaces, and every cell is
 * represented. There is a space after the last cell.
 * Note that "0/0" instead of "-" for a blank clue cell is
 * acceptable.
 * This is followed by the time currently spent on the puzzle,
 * in the format
 * "t$HOURS:$MINUTES:$SECONDS"
 * where $HOURS, $MINUTES, and $SECONDS are always 2 digits each.
 *
 * An example KAKString might look like this:
 * "3x3 50 - 12/0 3/0 0/11 0 0n12 0/4 3 1f t00:01:15"
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtWidgets>
#include "puzzleboard.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow();
    ~MainWindow();

    void makePuzzleBoard(QString KAKString);

protected:
    bool eventFilter(QObject *obj, QEvent *event);
    void timerEvent(QTimerEvent *event);
    void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;

private slots:
    void rules();
    void about();
    void reset();
    void settings();

    //Saving/loading
    void newGame();
    void open();
    bool save();
    bool saveAs();

    //Timer
    void updateStatusTimer();
    void toggleTimer();

    //Solver
    void checkSolved();
    void checkSolvable();
    void solveBoard();
    void comboHelper();

    //New game dialog
    void setNewRows();
    void setNewCols();
    void makeNewGame();

    //Settings dialog
    void setNewCellSize();
    void restoreDefaultSettings();
    void saveSettings();
    void setBoardColors();
    void setColorsDefault();

private:
    //Setup
    void createActions();
    void createMenus();
    void createToolBars();
    void createStatusBar();

    //Saving/loading
    bool areYouSure();
    void loadFile(const QString &fileName);
    bool saveFile(const QString &fileName);
    void updateLastSavedKAKString(const QString &KAKstring = "");
    void updateLastSavedFileName(const QString &fileName);

    PuzzleBoard *board;

    //Saving/loading
    QString lastSavedKAKString, lastSavedFileName, backupFile;
    int AUTOSAVE_INTERVAL, AUTOSAVE_DISPLAY_TIME;

    //Timer
    int timerId;
    QLabel *statusTimer, *autoSaveLabel;

    //New game dialog
    QDialog *newGameD;
    QComboBox *rowCombo, *colCombo;
    int newRows, newCols;

    //Settings dialog
    QDialog *settingsD;
    QComboBox *cellSizeCombo;
    int newCellSize;
    QLineEdit *colorLineEdits[7];
    QColor *colors[7];

    //Menus/toolbars
    QMenu *fileMenu;
    QMenu *solverMenu;
    QMenu *settingsMenu;
    QMenu *helpMenu;
    QToolBar *fileToolBar;
    QToolBar *helpToolBar;
    QToolBar *timerToolBar;
    QToolBar *toolsToolBar;
    //Acts for menus/toolbars
    QAction *checkSolvedAct;
    QAction *checkSolvableAct;
    QAction *solveBoardAct;
    QAction *newAct;
    QAction *openAct;
    QAction *saveAct;
    QAction *saveAsAct;
    QAction *exitAct;
    QAction *rulesAct;
    QAction *aboutAct;
    QAction *startTimerAct, *pauseTimerAct;
    QAction *resetAct;
    QAction *settingsAct;
    QAction *comboHelpAct;

};

#endif
