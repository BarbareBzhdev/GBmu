#ifndef OPENGLWINDOW_H
#define OPENGLWINDOW_H

#include <QWidget>
#include "htype.hpp"

#define WIN_WIDTH 160
#define WIN_HEIGHT 144

class QMenuBar;

class OpenGLWindow : public QWidget
{
	Q_OBJECT
	private:
		explicit OpenGLWindow(QWidget *parent = 0);
		~OpenGLWindow();

		void paintEvent(QPaintEvent *event) override;
		void closeEvent(QCloseEvent *event) override;

	public:
		static OpenGLWindow	*Instance();

		virtual void initialize();

		virtual void keyReleaseEvent(QKeyEvent *e) override;
		virtual void keyPressEvent(QKeyEvent *e) override;
		void drawPixel(uint16_t addr, uint8_t r, uint8_t g, uint8_t b);
		void drawPixel(uint16_t addr, uint32_t color);

		QMenuBar*	genMenuBar(void);
		void dragEnterEvent(QDragEnterEvent *event) override;
		void dropEvent(QDropEvent *event) override;

	private slots:
		void openSlot();
		void openStateSlot();
		void saveStateSlot();
		void gbDbSlot();
		void gbTypeAUTOSlot();
		void gbTypeGBSlot();
		void gbTypeGBCSlot();
		void gbComPlaySlot();
		void gbComPauseSlot();
		void gbComStopSlot();
		void gbSoundOnSlot();
		void gbSoundOffSlot();
		void updateSlot();

	signals:
		void openRomSign(std::string path);
		void openStateSign(std::string path);
		void saveStateSign(std::string path);
		void gbTypeSign(htype hardware);
		void gbDbSign();
		void keyPressSign(int key);
		void keyReleaseSign(int key);
		void gbComPlay();
		void gbComPause();
		void gbComStop();
		void gbSoundSign(bool on);
		void closeSign();

	private:
		QMenuBar			*_menuBar;
		QImage				*frameBuffer;
};

#endif // OPENGLWINDOW_H
