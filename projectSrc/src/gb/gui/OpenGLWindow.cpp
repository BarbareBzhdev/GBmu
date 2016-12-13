#include "OpenGLWindow.hpp"

#include <QCoreApplication>
#include <QPainter>
#include <QMenuBar>
#include <QKeyEvent>
#include <QTimer>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QActionGroup>
#include <QFileDialog>

#include <iostream>

QTimer timerScreen;

OpenGLWindow::OpenGLWindow(QWidget *parent)
	: QWidget(parent)
	, _menuBar(this->genMenuBar())
	, frameBuffer(new QImage(WIN_WIDTH, WIN_HEIGHT, QImage::Format_RGB32))
{
	setAcceptDrops(true);
	resize(500,500);

	connect(&timerScreen, &QTimer::timeout, this, &OpenGLWindow::updateSlot);
	timerScreen.start(16);
}

OpenGLWindow::~OpenGLWindow()
{
	delete _menuBar;
	delete frameBuffer;
}

void			OpenGLWindow::closeEvent(QCloseEvent *event)
{
	emit	closeSign();
	event->accept();
}

OpenGLWindow	*OpenGLWindow::Instance()
{
	static OpenGLWindow	*_instance = nullptr;

	if (!_instance)
		_instance = new OpenGLWindow();
	return (_instance);
}

QMenuBar	*OpenGLWindow::genMenuBar()
{
	QMenuBar*	menuBar		= new QMenuBar();
	QMenu*		menu		= new QMenu(tr("Actions"));
	QAction*	openAct		= new QAction(tr("Open"));
	QAction*	openState	= new QAction(tr("Open State"));
	QAction*	saveState	= new QAction(tr("Save State"));
	QAction*	gbDbAct		= new QAction(tr("Debbuger"));

	connect(openAct, &QAction::triggered, this, &OpenGLWindow::openSlot);
	connect(openState, &QAction::triggered, this, &OpenGLWindow::openStateSlot);
	connect(saveState, &QAction::triggered, this, &OpenGLWindow::saveStateSlot);
	connect(gbDbAct, &QAction::triggered, this, &OpenGLWindow::gbDbSlot);
	menu->addAction(openAct); openAct->setShortcut(tr("Ctrl+O"));
	menu->addAction(openState);
	menu->addAction(saveState);
	menu->addAction(gbDbAct); gbDbAct->setShortcut(tr("Ctrl+D"));
	menuBar->addMenu(menu);

	QMenu*		hard	= new QMenu(tr("Model"));
	QAction*	type0	= new QAction(tr("AUTO"));
	QAction*	type1	= new QAction(tr("GB"));
	QAction*	type2	= new QAction(tr("GBC"));

	connect(type0, &QAction::triggered, this, &OpenGLWindow::gbTypeAUTOSlot);
	connect(type1, &QAction::triggered, this, &OpenGLWindow::gbTypeGBSlot);
	connect(type2, &QAction::triggered, this, &OpenGLWindow::gbTypeGBCSlot);

	QActionGroup*	gmod = new QActionGroup(this);
	gmod->addAction(type0);
	gmod->addAction(type1);
	gmod->addAction(type2);
	gmod->setExclusive(true);
	type0->setCheckable(true);
	type0->setChecked(true);
	type1->setCheckable(true);
	type2->setCheckable(true);
	hard->addAction(type0);
	hard->addAction(type1);
	hard->addAction(type2);
	menuBar->addMenu(hard);

	QMenu*		com		= new QMenu(tr("Commande"));
	QAction*	com0	= new QAction(tr("PLAY"));
	QAction*	com1	= new QAction(tr("PAUSE"));
	QAction*	com2	= new QAction(tr("STOP"));

	connect(com0, &QAction::triggered, this, &OpenGLWindow::gbComPlaySlot);
	connect(com1, &QAction::triggered, this, &OpenGLWindow::gbComPauseSlot);
	connect(com2, &QAction::triggered, this, &OpenGLWindow::gbComStopSlot);

	QActionGroup*	gcom = new QActionGroup(this);
	gcom->setExclusive(true);
	gcom->addAction(com0);
	gcom->addAction(com1);
	gcom->addAction(com2);
	com0->setCheckable(true);
	com0->setChecked(true);
	com1->setCheckable(true);
	com2->setCheckable(true);
	com->addAction(com0); com0->setShortcut(tr("Ctrl+P"));
	com->addAction(com1); com1->setShortcut(tr("Ctrl+O"));
	com->addAction(com2); com2->setShortcut(tr("Ctrl+S"));
	menuBar->addMenu(com);

	QMenu*		sound		= new QMenu(tr("Son"));
	QAction*	sound0		= new QAction(tr("ON"));
	QAction*	sound1		= new QAction(tr("OFF"));

	connect(sound0, &QAction::triggered, this, &OpenGLWindow::gbSoundOnSlot);
	connect(sound1, &QAction::triggered, this, &OpenGLWindow::gbSoundOffSlot);

	QActionGroup*	gsound = new QActionGroup(this);
	gsound->setExclusive(true);
	gsound->addAction(sound0);
	gsound->addAction(sound1);
	sound0->setCheckable(true);
	sound0->setChecked(true);
	sound1->setCheckable(true);

	sound->addAction(sound0);
	sound->addAction(sound1);
	menuBar->addMenu(sound);

	return menuBar;
}

void	OpenGLWindow::gbTypeAUTOSlot()
{
	emit gbTypeSign(AUTO);
}

void	OpenGLWindow::gbTypeGBSlot()
{
	emit gbTypeSign(GB);
}

void	OpenGLWindow::gbTypeGBCSlot()
{
	emit gbTypeSign(GBC);
}

void	OpenGLWindow::gbComPlaySlot()
{
	emit	gbComPlay();
}

void	OpenGLWindow::gbComPauseSlot()
{
	emit	gbComPause();
}

void	OpenGLWindow::gbComStopSlot()
{
	emit	gbComStop();
}

void	OpenGLWindow::gbSoundOnSlot()
{
	emit	gbSoundSign(true);
}

void	OpenGLWindow::gbSoundOffSlot()
{
	emit	gbSoundSign(false);
}

void	OpenGLWindow::openSlot()
{
	QString path = QFileDialog::getOpenFileName(NULL, tr("Open Rom File"), "/home", tr("*.gb, *.gbc"));

	emit openRomSign(path.toStdString());
}
void	OpenGLWindow::openStateSlot()
{
	QString path = QFileDialog::getOpenFileName(NULL, tr("Open Save File"), "/home", tr("*.sgb"));

	emit openStateSign(path.toStdString());
}

void	OpenGLWindow::saveStateSlot()
{
	QString path = QFileDialog::getSaveFileName(NULL, tr("Save State File"), "/home", tr("*.sgb"));

	emit saveStateSign(path.toStdString());
}

void	OpenGLWindow::gbDbSlot()
{
	emit gbDbSign();
}

void OpenGLWindow::keyReleaseEvent(QKeyEvent* e)
{
	emit keyReleaseSign(e->key());
}

void OpenGLWindow::keyPressEvent(QKeyEvent* e)
{
	emit keyPressSign(e->key());
}

void OpenGLWindow::drawPixel(uint16_t addr, uint8_t r, uint8_t g, uint8_t b)
{
	drawPixel(addr, ((uint32_t)r << 16) | ((uint32_t)g << 8) | ((uint32_t)b << 0));
}

void OpenGLWindow::drawPixel(uint16_t addr, uint32_t color)
{
	((uint32_t*)frameBuffer->bits())[addr] = color;
}

void OpenGLWindow::initialize()
{
	unsigned int defaultColor = 0x00000000;

	for (int y = 0 ; y < WIN_HEIGHT; ++y)
		for (int x = 0 ; x < WIN_WIDTH; ++x)
			drawPixel(y * WIN_WIDTH + x, defaultColor);
}

void OpenGLWindow::dragEnterEvent(QDragEnterEvent *event)
{
	event->acceptProposedAction();
}

void OpenGLWindow::dropEvent(QDropEvent *event)
{
	const QMimeData *mimeData = event->mimeData();
	emit openRomSign(mimeData->urls().at(0).toLocalFile().toStdString());
}

void OpenGLWindow::paintEvent(__attribute__((unused)) QPaintEvent *event)
{
	QPainter painter(this);
	painter.drawImage(0, 0, frameBuffer->scaled(size()));
	painter.end();
}

void OpenGLWindow::updateSlot(void)
{
	update();
}
