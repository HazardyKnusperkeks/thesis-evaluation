#include "hauptfenster.hpp"

#include "encodingfenster.hpp"
#include "qcustomplot/qcustomplot.h"

#include <QCheckBox>
#include <QDir>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QTabBar>
#include <QTabWidget>
#include <QTimer>
#include <QVBoxLayout>

void Hauptfenster::fuegeHauptTabHinzu(void) {
	addTab(HauptTab, "Zusammenfassung");
	tabBar()->setTabButton(0, QTabBar::RightSide, nullptr);
	return;
}

void Hauptfenster::setzePfad(const QString& pfad) {
	Pfad->setText(pfad);
	
	while ( !Encodings.isEmpty() ) {
		entferneEncoding(*Encodings.keyBegin());
	} //while ( !Encodings.isEmpty() )
	
	while ( EncodingLayout->count() ) {
		auto item = EncodingLayout->takeAt(0);
		Q_ASSERT(item->widget());
		delete item->widget();
		delete item;
	} //while ( EncodingLayout->count() )
	
	QDir verzeichnis(pfad);
	QVector<QCheckBox*> boxes;
	
	for ( const auto& encoding : verzeichnis.entryList(QDir::Dirs | QDir::NoDotAndDotDot) ) {
		auto box = new QCheckBox(encoding, this);
		EncodingLayout->addWidget(box);
		boxes.append(box);
		
		connect(box, &QCheckBox::toggled, this, [this,box](const bool aktiv) {
				if ( aktiv ) {
					fuegeEncodingHinzu(box->text());
				} //if ( aktiv )
				else {
					entferneEncoding(box->text());
				} //else -> if ( aktiv )
			});
	} //for ( const auto& encoding : verzeichnis.entryList(QDir::Dirs) )
	
	auto knopf = new QPushButton("Alle", this);
	connect(knopf, &QPushButton::clicked, this, [this,boxes](void) {
			for ( auto& box : boxes ) {
				if ( !box->isChecked() ) {
					box->setChecked(true);
				} //if ( !box->isChecked() )
			} //for ( auto& box : boxes )
			return;
		});
	
	EncodingLayout->addStretch();
	EncodingLayout->addWidget(knopf);
	return;
}

void Hauptfenster::fuegeEncodingHinzu(const QString& encoding) {
	auto fenster = new EncodingFenster(false);
	fenster->setzePfad(Pfad->text() + QDir::separator() + encoding);
	Encodings.insert(encoding, fenster);
	addTab(fenster, encoding);
	UpdateTimer->start();
	return;
}

void Hauptfenster::entferneEncoding(const QString& encoding) {
	auto fenster = Encodings.take(encoding);
	int index = 0;
	for ( ; fenster != widget(index); ++index ) { }
	removeTab(index);
	delete fenster;
	UpdateTimer->start();
	return;
}

void Hauptfenster::update(void) {
	delete ScrollWidget->takeWidget();
	auto parentWidget = new QWidget;
	
	auto punkteGraph = new QCustomPlot(parentWidget);
	auto punktePlot = new QCPStatisticalBox(punkteGraph->xAxis, punkteGraph->yAxis);
	auto planerPunktePlot = new QCPStatisticalBox(punkteGraph->xAxis, punkteGraph->yAxis);
	auto planerPunkteCompletePlot  = new QCPStatisticalBox(punkteGraph->xAxis, punkteGraph->yAxis);
	
	punktePlot->setBrush(Qt::green);
	planerPunktePlot->setBrush(Qt::blue);
	planerPunkteCompletePlot->setBrush(Qt::red);
	
	auto outlierPunkteGraph = new QCustomPlot(parentWidget);
	auto outlierPunktePlot = new QCPStatisticalBox(outlierPunkteGraph->xAxis, outlierPunkteGraph->yAxis);
	auto outlierPlanerPunktePlot = new QCPStatisticalBox(outlierPunkteGraph->xAxis, outlierPunkteGraph->yAxis);
	auto outlierPlanerPunkteCompletePlot  = new QCPStatisticalBox(outlierPunkteGraph->xAxis, outlierPunkteGraph->yAxis);
	
	outlierPunktePlot->setBrush(Qt::green);
	outlierPlanerPunktePlot->setBrush(Qt::blue);
	outlierPlanerPunkteCompletePlot->setBrush(Qt::red);
	
	auto graphen = {punkteGraph, outlierPunkteGraph};
	
	auto encodingTicker = QSharedPointer<QCPAxisTickerText>::create();
	
	double index = 2., max = 0.;
	
	for ( auto& graph : graphen ) {
		graph->xAxis->setRange(.5, 3. * Encodings.size());
		graph->xAxis->setSubTicks(false);
		graph->xAxis->setTicker(encodingTicker);
		graph->xAxis->setTickLength(0, 0);
		
		graph->setMinimumSize(320 * Encodings.size(), 480);
	} //for ( auto& graph : graphen )
	
	for ( auto iter = Encodings.begin(); iter != Encodings.end(); ++iter, index += 3. ) {
		encodingTicker->addTick(index, iter.key());
		
		auto encoding = iter.value();
		
		const auto& punkte = encoding->punkte();
		punktePlot->addData(index - .75, punkte.Min, punkte.ErstesQuartil, punkte.ZweitesQuartil, punkte.DrittesQuartil, punkte.Max);
		
		const auto& planerPunkte = encoding->planerPunkte();
		planerPunktePlot->addData(index, planerPunkte.Min, planerPunkte.ErstesQuartil, planerPunkte.ZweitesQuartil, planerPunkte.DrittesQuartil, planerPunkte.Max);
		
		const auto& planerPunkteComplete = encoding->planerPunkteNachSpiel();
		planerPunkteCompletePlot->addData(index + .75, planerPunkteComplete.Min, planerPunkteComplete.ErstesQuartil, planerPunkteComplete.ZweitesQuartil, planerPunkteComplete.DrittesQuartil, planerPunkteComplete.Max);
		
		max = std::max({max, punkte.Max, planerPunkte.Max, planerPunkteComplete.Max});
		
		const auto& outlierPunkte = encoding->outlierPunkte();
		outlierPunktePlot->addData(index - .75, outlierPunkte.first.Min, outlierPunkte.first.ErstesQuartil, outlierPunkte.first.ZweitesQuartil, outlierPunkte.first.DrittesQuartil, outlierPunkte.first.Max, outlierPunkte.second);
		
		const auto& outlierPlanerPunkte = encoding->outlierPlanerPunkte();
		outlierPlanerPunktePlot->addData(index, outlierPlanerPunkte.first.Min, outlierPlanerPunkte.first.ErstesQuartil, outlierPlanerPunkte.first.ZweitesQuartil, outlierPlanerPunkte.first.DrittesQuartil, outlierPlanerPunkte.first.Max, outlierPlanerPunkte.second);
		
		const auto& outlierPlanerPunkteComplete = encoding->outlierPlanerPunkteNachSpiel();
		outlierPlanerPunkteCompletePlot->addData(index + .75, outlierPlanerPunkteComplete.first.Min, outlierPlanerPunkteComplete.first.ErstesQuartil, outlierPlanerPunkteComplete.first.ZweitesQuartil, outlierPlanerPunkteComplete.first.DrittesQuartil, outlierPlanerPunkteComplete.first.Max, outlierPlanerPunkteComplete.second);
	} //for ( auto iter = Encodings.begin(); iter != Encodings.end(); ++iter )
	
	for ( auto& graph : graphen ) {
		graph->yAxis->setRange(0., max);
	} //for ( auto& graph : graphen )
	
	auto layout = new QGridLayout(parentWidget);
	layout->addWidget(punkteGraph,        0, 0);
	layout->addWidget(outlierPunkteGraph, 1, 0);
	
	ScrollWidget->setWidget(parentWidget);
	return;
}

void Hauptfenster::neuerTab(void) {
	setCurrentIndex(addTab(new EncodingFenster(true), "Frei"));
	return;
}

void Hauptfenster::schliesseTab(const int index) {
	const auto encoding = tabText(index);
	if ( encoding != "Frei" ) {
		int layoutIndex = 0;
		for ( ; layoutIndex < EncodingLayout->count(); ++layoutIndex ) {
			auto item = EncodingLayout->itemAt(layoutIndex);
			auto box = static_cast<QCheckBox*>(item->widget());
			if ( box->text() == encoding ) {
				box->blockSignals(true);
				box->setChecked(false);
				box->blockSignals(false);
				entferneEncoding(encoding);
				return;
			} //if ( box->text() == encoding )
		} //for ( ; layoutIndex < EncodingLayout->count(); ++layoutIndex )
	} //if ( encoding != "Frei" )
	
	auto w = widget(index);
	removeTab(index);
	delete w;
	return;
}

Hauptfenster::Hauptfenster(QWidget *parent) : QTabWidget(parent), UpdateTimer(new QTimer(this)) {
	UpdateTimer->setInterval(500);
	UpdateTimer->setSingleShot(true);
	connect(UpdateTimer, &QTimer::timeout, this, &Hauptfenster::update);
	
	HauptTab = new QWidget;
	
	Pfad = new QLineEdit(HauptTab);
	Pfad->setReadOnly(true);
	
	auto durchsuchen = new QPushButton("Durchsuchen", HauptTab);
	connect(durchsuchen, &QPushButton::clicked, this, [this](void) {
			QString pfad = QFileDialog::getExistingDirectory(this, "Wähle Pfad", Pfad->text());
			if ( !pfad.isEmpty() ) {
				setzePfad(pfad);
			} //if ( !pfad.isEmpty() )
			return;
		});
	
	auto obenLayout = new QHBoxLayout;
	obenLayout->addWidget(Pfad);
	obenLayout->addWidget(durchsuchen);
	
	EncodingLayout = new QHBoxLayout;
	
	ScrollWidget = new QScrollArea(HauptTab);
	
	auto layout = new QVBoxLayout(HauptTab);
	layout->addLayout(obenLayout);
	layout->addLayout(EncodingLayout);
	layout->addWidget(ScrollWidget);
	
	auto neu = new QPushButton("Neu");
	connect(neu, &QPushButton::clicked, this, &Hauptfenster::neuerTab);
	
	setCornerWidget(neu);
	setTabsClosable(true);
	fuegeHauptTabHinzu();
	
	connect(this, &Hauptfenster::tabCloseRequested, this, &Hauptfenster::schliesseTab);
	
	setWindowTitle("Evaluation");
	setWindowState(Qt::WindowMaximized);
	return;
}
