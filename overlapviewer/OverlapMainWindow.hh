#ifndef OVERLAP_MAINWINDOW_HH
#define OVERLAP_MAINWINDOW_HH

#include <QMainWindow>
#include <QWidget>

#include "ui_OverlapMainWindow.h"

#include "DataSet.hh"

namespace overlapviewer {

class OverlapMainWindow : public QMainWindow
{
	Q_OBJECT
public:
	OverlapMainWindow(SelectedFeaturesPtr selectedFeatures);
private:
	void updateFeatures();

	Ui::OverlapMainWindow d_overlapMainWindow;
	SelectedFeaturesPtr d_selectedFeatures;
};

}

#endif // OVERLAP_MAINWINDOW_HH

