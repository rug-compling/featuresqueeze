#include "OverlapMainWindow.ih"

OverlapMainWindow::OverlapMainWindow(SelectedFeaturesPtr selectedFeatures)
	: d_selectedFeatures(selectedFeatures)
{
	d_overlapMainWindow.setupUi(this);
	updateFeatures();
}

void OverlapMainWindow::updateFeatures()
{
	QList<QTreeWidgetItem *> items;

	for (SelectedFeatures::const_iterator iter = d_selectedFeatures->begin();
		iter != d_selectedFeatures->end(); ++iter)
	{
		QStringList colTexts;
		colTexts.push_back(QString::number(iter->feature));
		colTexts.push_back(QString::number(iter->gain));
		colTexts.push_back(QString::number(iter->alpha));

		items.push_back(new QTreeWidgetItem(colTexts));

	}

	d_overlapMainWindow.featuresTreeWidget->insertTopLevelItems(0, items);
}
