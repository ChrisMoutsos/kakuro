/*
 * cell.cpp
 * See cell.h for more information
 */

#include "cell.h"
#include <QDebug>
#include <QMouseEvent>
#include <QStaticText>

Cell::Cell(int s, int r, int c) {
    this->setMouseTracking(true);
    selected = false;
    for (int i = 0; i < 10; i++) {
        notes[i] = 0;
    }
    type = 0;
    downClue = rightClue = value = 0;
    numInDownSum = numInRightSum = 0;
    fixed = false;
    size = s;
    row = r;
    col = c;
    label = new QLabel;
}

void Cell::makePixmap() {
    QPixmap pixmap = QPixmap(size, size);
    label->setPixmap(pixmap);
}

void Cell::fill(QColor c) {
    QPixmap pixmap = *(label->pixmap());

    QPainter painter(&pixmap);
    painter.setBrush(QBrush(c, Qt::SolidPattern));
    QRectF rect = label->rect();
    painter.drawRect(rect);

    label->setPixmap(pixmap);

    drawBorder();
}

void Cell::drawBorder() {
    QPixmap pixmap = *(label->pixmap());
    QPainter painter(&pixmap);
    painter.setPen(QPen(*colors[BORDERCOLOR], 0));
    painter.drawRect(0, 0, size-1, size-1);
    label->setPixmap(pixmap);
}

void Cell::draw(qreal numberAlpha) {
    if (label->pixmap()->width() == 0 ||
            label->pixmap()->height() == 0)
        return;

    if (type == 0) {
        if (notes[0] == 0) {
            drawValue(numberAlpha);
            drawNotes(0.5);
        }
        else {
            drawValue(0.2);
            drawNotes();
        }
    }
    else {
        drawClueBox();
    }
}

void Cell::drawNotes(qreal alpha) {
    QPixmap pixmap = *(label->pixmap());
    QPainter painter(&pixmap);
    QRectF rect;

    //Set painter attributes
    painter.setOpacity(alpha);
    painter.setRenderHint(QPainter::TextAntialiasing);
    painter.setFont(QFont("Arial", size*0.25, 60));
    painter.setPen(*colors[NOTECOLOR]);

    int pos = 0;
    for (int i = 1; i <= 9; i++) {
        if (notes[i] == 0)
            continue;
        rect = QRectF(QPointF(2+(pos%3*(size/3)), 1+(pos/3*(size/3))), QSizeF(size/3.5, size/3.5));
        painter.drawText(rect, Qt::AlignCenter, QString::number(i));
        pos++;
    }

    if (notes[0] == 1 && pos == 0) {
        painter.setFont(QFont("Arial", size*0.1, 60, 1));
        painter.setPen(*colors[NOTECOLOR]);
        painter.drawText(label->rect(), Qt::AlignCenter, "type a note...");
    }

    label->setPixmap(pixmap);
}

void Cell::drawValue(qreal alpha) {
    drawValue(value, alpha);
}

void Cell::drawValue(int v, qreal alpha) {
    fill(*colors[NONCLUECOLOR]);

    QPixmap pixmap = *(label->pixmap());
    QPainter painter(&pixmap);
    QRectF rect = label->rect();

    if (selected) {
        painter.setPen(*colors[SELECTCOLOR]);
        painter.setBrush(QBrush(*colors[SELECTCOLOR], Qt::SolidPattern));
        painter.drawRect(1, 1, size-3, size-3);
    }

    if (v == 0) {
        label->setPixmap(pixmap);
        drawBorder();
        return;
    }

    //Set painter attributes
    painter.setOpacity(alpha);
    painter.setRenderHint(QPainter::TextAntialiasing);
    painter.setFont(QFont("Arial", size*0.65));
    //painter.setPen(Qt::black);
    painter.setPen(*colors[NONCLUETEXTCOLOR]);

    painter.drawText(rect, Qt::AlignCenter, QString::number(v));

    label->setPixmap(pixmap);
}

void Cell::drawClueBox() {
    drawClueBox(downClue, rightClue);
}

void Cell::drawClueBox(int dClue, int rClue) {
    fill(QColor(Qt::white));

    QPixmap pixmap = *(label->pixmap());
    QPainter painter(&pixmap);

    //Set painter attributes
    painter.setRenderHint(QPainter::TextAntialiasing);
    painter.setFont(QFont("Arial", size*0.3));

    //Fill in black
    painter.setPen(QPen(*colors[CLUECOLOR], 2));
    painter.setBrush(QBrush(*colors[CLUECOLOR], Qt::SolidPattern));
    painter.drawRect(label->rect());

    painter.setPen(*colors[CLUETEXTCOLOR]);
    if (dClue) {
        QRectF bottomLeftRect = QRectF(QPointF(size/9, 0.5*size), QPointF(size/2, size));
        QPointF bottomLeftPoint(size/9, 0.5*size);
        if (dClue < 10) bottomLeftPoint += QPointF(size/8, 0);

        QStaticText d(QString::number(dClue));
        d.setTextWidth(bottomLeftRect.width());
        d.setTextFormat(Qt::RichText);
        painter.drawStaticText(bottomLeftPoint, d);
    }

    if (rClue) {
        QRectF topRightRect = QRectF(QPointF(size/2-size/20, size/10), QPointF(size, size/2));
        QPointF topRightPoint(size/2, size/20);
        if (rClue < 10) topRightPoint += QPointF(size/8, 0);

        QStaticText r(QString::number(rClue));
        r.setTextWidth(topRightRect.width());
        r.setTextFormat(Qt::RichText);
        painter.drawStaticText(topRightPoint, r);
    }

    //Draw diagonal line
    if (dClue || rClue) {
        painter.setPen(QPen(*colors[CLUETEXTCOLOR], 1));
        painter.drawLine(QPointF(0.5, 0.5), QPointF(size-0.5, size-0.5));
    }

    label->setPixmap(pixmap);
}

void Cell::handleNumPress(int n) {
    if (fixed)
        return;
    if (notes[0] == 0) {
        value = n;
        draw();
    }
    else {
        notes[n] = !notes[n];
        draw();
    }
}

void Cell::select() {
    selected = true;
    notes[0] = 0;

    draw();
}

void Cell::unselect() {
    selected = false;
    notes[0] = 0;

    draw();
}

void Cell::setValue(int v) {
    if (v < 0 || v > 9)
        return;

    value = v;
}
