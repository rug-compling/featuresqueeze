#ifndef OVERLAP_MAINWINDOW_HH
#define OVERLAP_MAINWINDOW_HH

#include <QMainWindow>
#include <QString>
#include <QTreeWidgetItem>
#include <QWidget>

#include "ui_OverlapMainWindow.h"

#include "DataSet.hh"

namespace overlapviewer {

class OverlapMainWindow : public QMainWindow
{
	Q_OBJECT
public:
	OverlapMainWindow(DataSetPtr selectedFeatures);
private slots:
	void updateFeature(QTreeWidgetItem *item, QTreeWidgetItem *);
private:
	void showFeature(QString const &feature);
	void updateFeatures();

	Ui::OverlapMainWindow d_overlapMainWindow;
	DataSetPtr d_dataset;
};

}

#endif // OVERLAP_MAINWINDOW_HH

