#include "OverlapMainWindow.ih"

OverlapMainWindow::OverlapMainWindow(DataSetPtr selectedFeatures)
	: d_dataset(selectedFeatures)
{
	d_overlapMainWindow.setupUi(this);
	connect(d_overlapMainWindow.featuresTreeWidget,
		SIGNAL(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)),
		this, SLOT(updateFeature(QTreeWidgetItem *, QTreeWidgetItem *)));
	updateFeatures();
}

void OverlapMainWindow::showFeature(QString const &feature)
{
	d_overlapMainWindow.nameQLabel->setText(feature);
	d_overlapMainWindow.overlapQLabel->setText(
		QString::number(d_dataset->overlappingFeatures[feature].overlap));
	d_overlapMainWindow.activationQLabel->setText(
			QString::number(d_dataset->overlappingFeatures[feature].activation));

	QVector<OverlappingFeature> &overlappingFs =
		d_dataset->overlappingFeatures[feature].features;

	QList<QTreeWidgetItem *> items;
	for (QVector<OverlappingFeature>::const_iterator iter = overlappingFs.begin();
		iter != overlappingFs.end(); ++iter)
	{
		QStringList colTexts;
		colTexts.push_back(iter->feature);
		colTexts.push_back(QString::number(iter->delta));
		items.push_back(new QTreeWidgetItem(colTexts));
	}

	d_overlapMainWindow.overlapTreeWidget->clear();
	d_overlapMainWindow.overlapTreeWidget->insertTopLevelItems(0, items);
}

void OverlapMainWindow::updateFeature(QTreeWidgetItem *item, QTreeWidgetItem *)
{
	if (item == 0)
		return;

	QString selectedFeature = item->text(0);
	showFeature(selectedFeature);
	qDebug(item->text(0).toLatin1().constData());
}

void OverlapMainWindow::updateFeatures()
{
	QList<QTreeWidgetItem *> items;

	for (SelectedFeatures::const_iterator iter = d_dataset->selectedFeatures.begin();
		iter != d_dataset->selectedFeatures.end(); ++iter)
	{
		QStringList colTexts;
		colTexts.push_back(iter->feature);
		colTexts.push_back(QString::number(iter->gain));
		colTexts.push_back(QString::number(iter->alpha));

		items.push_back(new QTreeWidgetItem(colTexts));

	}

	d_overlapMainWindow.featuresTreeWidget->insertTopLevelItems(0, items);
}
