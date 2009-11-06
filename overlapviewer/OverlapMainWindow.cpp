#include "OverlapMainWindow.ih"

OverlapMainWindow::OverlapMainWindow(DataSetPtr selectedFeatures)
	: d_dataset(selectedFeatures)
{
	d_overlapMainWindow.setupUi(this);

	updateFeatures();

	connect(d_overlapMainWindow.featuresTreeWidget,
		SIGNAL(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)),
		this, SLOT(updateFeature(QTreeWidgetItem *, QTreeWidgetItem *)));
	connect(d_overlapMainWindow.featureRegexLineEdit, SIGNAL(returnPressed()),
		this, SLOT(featureRegExpChanged()));
}

void OverlapMainWindow::featureRegExpChanged()
{
	QString regexStr = d_overlapMainWindow.featureRegexLineEdit->text();

	// When the regexp string length is 0, change the pointer to the
	// regexp to a null pointer to signal that no regexp should be used.
	if (regexStr.size() == 0)
		d_featureFilterRegExp.clear();
	else {
		QRegExp *featureFilterRegExp = new QRegExp(QString("(") + regexStr + ")");

		// Check if the regexp is valid. If not, we leave the existing
		// regexp as-is.
		if (!featureFilterRegExp->isValid()) {
			d_overlapMainWindow.statusbar->showMessage(
				QString("Compilation of regular expression failed: ") +
				featureFilterRegExp->errorString(), 5000);
			delete featureFilterRegExp;
			return;
		}

		d_featureFilterRegExp = QSharedPointer<QRegExp>(featureFilterRegExp);
	}

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
		colTexts.push_back(QString::number(iter->delta, 'g', 3));
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
}

void OverlapMainWindow::updateFeatures()
{	
	QList<QTreeWidgetItem *> items;

	for (SelectedFeatures::const_iterator iter = d_dataset->selectedFeatures.begin();
		iter != d_dataset->selectedFeatures.end(); ++iter)
	{
		if (d_featureFilterRegExp.data() != 0 &&
				d_featureFilterRegExp->indexIn(iter->feature) == -1)
			continue;

		QStringList colTexts;
		colTexts.push_back(iter->feature);
		colTexts.push_back(QString::number(iter->gain, 'g', 3));
		colTexts.push_back(QString::number(iter->alpha, 'g', 3));

		items.push_back(new QTreeWidgetItem(colTexts));

	}

	d_overlapMainWindow.featuresTreeWidget->clear();
	d_overlapMainWindow.featuresTreeWidget->insertTopLevelItems(0, items);
}
