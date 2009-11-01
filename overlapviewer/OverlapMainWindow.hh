#ifndef OVERLAP_MAINWINDOW_HH
#define OVERLAP_MAINWINDOW_HH

#include <QMainWindow>
#include <QWidget>

#include "ui_OverlapMainWindow.h"

namespace overlapviewer {

class OverlapMainWindow : public QMainWindow
{
	Q_OBJECT

public:
	OverlapMainWindow(QWidget *parent = 0);
private:
	Ui::OverlapMainWindow d_overlapMainWindow;
};

inline OverlapMainWindow::OverlapMainWindow(QWidget *)
{
	d_overlapMainWindow.setupUi(this);
}

}

#endif // OVERLAP_MAINWINDOW_HH

