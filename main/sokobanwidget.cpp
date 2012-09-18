#include <QtDebug>
#include <QtGui/QPainter>
#include <QtGui/QMessageBox>
#include <QtGui/QKeyEvent>
#include "playingmode.h"
#include "messagemode.h"
#include "fademode.h"
#include "sokobanwidget.h"

SokobanWidget::SokobanWidget(QWidget * parent)
	: QWidget(parent), levelSet(0), gameMode(0), prematurePlayingMode(0)
{
	startFadeIn();
}

void SokobanWidget::resizeEvent(QResizeEvent*)
{
	gameMode->invalidateRect();
}

void SokobanWidget::keyPressEvent(QKeyEvent * event)
{
	bool isShiftDown = event->modifiers().testFlag(Qt::ShiftModifier);
	bool isCtrlDown = event->modifiers().testFlag(Qt::ControlModifier);

	//# CONTROLS
	int control = AbstractGameMode::CONTROL_NONE;
	switch(event->key()) { //#
		case Qt::Key_Q: emit wantsToQuit(); return;                         //# 'q' or Ctrl-Q - quit.
		case Qt::Key_Space: control = AbstractGameMode::CONTROL_SKIP; break;                  //# Space - skip intelevel message.
		case Qt::Key_Left:  case Qt::Key_H: control = AbstractGameMode::CONTROL_LEFT; break;  //# Left or 'h' - move left.
		case Qt::Key_Down:  case Qt::Key_J: control = AbstractGameMode::CONTROL_DOWN; break;  //# Down or 'j' - move down.
		case Qt::Key_Up:    case Qt::Key_K: control = AbstractGameMode::CONTROL_UP; break;    //# Up or 'k' - move .
		case Qt::Key_Right: case Qt::Key_L: control = AbstractGameMode::CONTROL_RIGHT; break; //# Right or 'l' - move left.
		case Qt::Key_Z:    if(isCtrlDown) control = AbstractGameMode::CONTROL_UNDO; break;    //# Ctrl-Z, Backspace or 'u' - undo last action.
		case Qt::Key_Backspace: control = AbstractGameMode::CONTROL_UNDO; break;
		case Qt::Key_U:    control = isShiftDown ? AbstractGameMode::CONTROL_HOME : AbstractGameMode::CONTROL_UNDO; break;
		case Qt::Key_R:    if(isCtrlDown) control = AbstractGameMode::CONTROL_HOME; break;    //# Ctrl-R, Home or 'U' - revert to the starting position.
		case Qt::Key_Home: control = AbstractGameMode::CONTROL_HOME; break;
		default: QWidget::keyPressEvent(event); return;
	} //#

	gameMode->processControl(control);
	update();
}

void SokobanWidget::loadNextLevel()
{
	QImage snapshot = QImage(size(), QImage::Format_RGB32);
	QPainter painter(&snapshot);
	gameMode->paint(&painter, snapshot.rect());

	gameMode = new FadeMode(snapshot, true, this);
	connect(gameMode, SIGNAL(fadeIsEnded()), this, SLOT(showMessage()));
	connect(gameMode, SIGNAL(update()), this, SLOT(update()));
	update();
}

void SokobanWidget::showMessage()
{
	levelSet.moveToNextLevel();

	QString message = levelSet.isOver() ? tr("Levels are over.") : tr("Next level: %1").arg(levelSet.getCurrentLevelIndex());
	gameMode = new MessageMode(!levelSet.isOver(), message, this);
	connect(gameMode, SIGNAL(messageIsEnded()), this, SLOT(startFadeIn()));
	update();
}

void SokobanWidget::startFadeIn()
{
	QImage snapshot = QImage(size(), QImage::Format_RGB32);
	prematurePlayingMode = new PlayingMode(levelSet.getCurrentLevel(), this);
	QPainter painter(&snapshot);
	prematurePlayingMode->paint(&painter, snapshot.rect());

	gameMode = new FadeMode(snapshot, false, this);
	connect(gameMode, SIGNAL(fadeIsEnded()), this, SLOT(startGame()));
	connect(gameMode, SIGNAL(update()), this, SLOT(update()));
	update();
}

void SokobanWidget::startGame()
{
	gameMode = prematurePlayingMode;// PlayingMode(levelSet.getCurrentLevel(), this);
	connect(gameMode, SIGNAL(levelIsSolved()), this, SLOT(loadNextLevel()));
	update();
}

void SokobanWidget::paintEvent(QPaintEvent*)
{
	QPainter painter(this);
	gameMode->paint(&painter, rect());
}
