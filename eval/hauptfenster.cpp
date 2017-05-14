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
	
	while ( !Encodings.empty() ) {
		entferneEncoding(Encodings.front().first);
	} //while ( !Encodings.empty() )
	
	while ( EncodingLayout->count() ) {
		auto item = EncodingLayout->takeAt(0);
		Q_ASSERT(item->widget());
		delete item->widget();
		delete item;
	} //while ( EncodingLayout->count() )
	
	QDir verzeichnis(pfad);
	QVector<QCheckBox*> boxes;
	
	auto zeile = 0, spalte = 0;
	constexpr const auto maxSpalten = 15;
	
	for ( const auto& encoding : verzeichnis.entryList(QDir::Dirs | QDir::NoDotAndDotDot) ) {
		auto box = new QCheckBox(encoding, this);
		EncodingLayout->addWidget(box, zeile, spalte);
		boxes.append(box);
		
		if ( ++spalte == maxSpalten ) {
			spalte = 0;
			++zeile;
		} //if ( ++spalte == maxSpalten )
		
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
	
	EncodingLayout->setColumnStretch(maxSpalten, 1);
	EncodingLayout->addWidget(knopf, 0, maxSpalten + 1);
	return;
}

void Hauptfenster::fuegeEncodingHinzu(const QString& encoding) {
	auto fenster = new EncodingFenster(false);
	fenster->setzePfad(Pfad->text() + QDir::separator() + encoding);
	Encodings.push_back({encoding, fenster});
	addTab(fenster, encoding);
	UpdateTimer->start();
	return;
}

void Hauptfenster::entferneEncoding(const QString& encoding) {
	auto iter = std::find_if(Encodings.begin(), Encodings.end(), [&encoding](const auto& paar) noexcept { return paar.first == encoding; });
	auto fenster = iter->second;
	Encodings.erase(iter);
	int index = 0;
	for ( ; fenster != widget(index); ++index ) { }
	removeTab(index);
	delete fenster;
	UpdateTimer->start();
	return;
}

void Hauptfenster::update(void) {
	const auto x = ScrollWidget->horizontalScrollBar()->value();
	const auto y = ScrollWidget->verticalScrollBar()->value();
	delete ScrollWidget->takeWidget();
	auto parentWidget = new QWidget;
	
	auto punkteGraph = new QCustomPlot(parentWidget);
	auto punktePlot = new QCPStatisticalBox(punkteGraph->xAxis, punkteGraph->yAxis);
	auto planerPunktePlot = new QCPStatisticalBox(punkteGraph->xAxis, punkteGraph->yAxis);
	auto planerPunkteCompletePlot  = new QCPStatisticalBox(punkteGraph->xAxis, punkteGraph->yAxis);
	
	punktePlot->setBrush(Qt::green);
	planerPunktePlot->setBrush(Qt::yellow);
	planerPunkteCompletePlot->setBrush(Qt::red);
	
	auto outlierPunkteGraph = new QCustomPlot(parentWidget);
	auto outlierPunktePlot = new QCPStatisticalBox(outlierPunkteGraph->xAxis, outlierPunkteGraph->yAxis);
	auto outlierPlanerPunktePlot = new QCPStatisticalBox(outlierPunkteGraph->xAxis, outlierPunkteGraph->yAxis);
	auto outlierPlanerPunkteCompletePlot  = new QCPStatisticalBox(outlierPunkteGraph->xAxis, outlierPunkteGraph->yAxis);
	
	outlierPunktePlot->setBrush(Qt::green);
	outlierPlanerPunktePlot->setBrush(Qt::yellow);
	outlierPlanerPunkteCompletePlot->setBrush(Qt::red);
	
	auto nurPktGraph = new QCustomPlot(parentWidget);
	auto nurPktPlot = new QCPStatisticalBox(nurPktGraph->xAxis, nurPktGraph->yAxis);
	nurPktPlot->setBrush(Qt::green);
	
	auto nurOutlierPktGraph = new QCustomPlot(parentWidget);
	auto nurOutlierPktPlot = new QCPStatisticalBox(nurOutlierPktGraph->xAxis, nurOutlierPktGraph->yAxis);
	nurOutlierPktPlot->setBrush(Qt::green);
	
	auto idleGraph = new QCustomPlot(parentWidget);
	auto idlePlot = new QCPStatisticalBox(idleGraph->xAxis, idleGraph->yAxis);
	idlePlot->setBrush(QColor(255, 128, 0)); //orange
	
	auto outlierIdleGraph = new QCustomPlot(parentWidget);
	auto outlierIdlePlot = new QCPStatisticalBox(outlierIdleGraph->xAxis, outlierIdleGraph->yAxis);
	outlierIdlePlot->setBrush(QColor(255, 128, 0)); //orange
	
	auto startUpGraph = new QCustomPlot(parentWidget);
	auto startUpPlot = new QCPStatisticalBox(startUpGraph->xAxis, startUpGraph->yAxis);
	//startUpPlot->setBrush();
	
	auto graphen1 = {nurPktGraph, nurOutlierPktGraph, idleGraph, outlierIdleGraph, startUpGraph};
	auto graphen3 = {punkteGraph, outlierPunkteGraph};
	auto graphen = {graphen1, graphen3};
	
	auto pktGraphen = {punkteGraph, outlierPunkteGraph, nurPktGraph, nurOutlierPktGraph};
	auto idleGraphen = {idleGraph, outlierIdleGraph};
	
	auto encodingTicker1 = QSharedPointer<QCPAxisTickerText>::create();
	auto encodingTicker3 = QSharedPointer<QCPAxisTickerText>::create();
	
	double index1 = 1., index3 = 2., pktMax = 0., idleMax = 0., startUpMax = 0.;
	
	for ( auto& graph : graphen1 ) {
		graph->xAxis->setRange(.5, Encodings.size() + .5);
		graph->xAxis->setTicker(encodingTicker1);
		
		graph->setMinimumSize(100 * Encodings.size() + 10 * (Encodings.size() - 1) + 50, 480);
	} //for ( auto& graph : graphen1 )
	
	for ( auto& graph : graphen3 ) {
		graph->xAxis->setRange(.5, 3. * Encodings.size());
		graph->xAxis->setTicker(encodingTicker3);
		
		graph->setMinimumSize(300 * Encodings.size() + 10 * (Encodings.size() - 1) + 50, 480);
	} //for ( auto& graph : graphen3 )
	
	for ( auto& list : graphen ) {
		for ( auto& graph : list ) {
			graph->xAxis->setSubTicks(false);
			graph->xAxis->setTickLength(0, 0);
		} //for ( auto& graph : list )
	} //for ( auto& list : graphen )
	
	for ( auto iter = Encodings.begin(); iter != Encodings.end(); ++iter, index1 += 1., index3 += 3. ) {
		encodingTicker1->addTick(index1, iter->first);
		encodingTicker3->addTick(index3, iter->first);
		
		auto encoding = iter->second;
		
		const auto& punkte = encoding->punkte();
		punktePlot->addData(index3 - .75, punkte.Min, punkte.ErstesQuartil, punkte.ZweitesQuartil, punkte.DrittesQuartil, punkte.Max);
		nurPktPlot->addData(index1, punkte.Min, punkte.ErstesQuartil, punkte.ZweitesQuartil, punkte.DrittesQuartil, punkte.Max);
		
		const auto& planerPunkte = encoding->planerPunkte();
		planerPunktePlot->addData(index3, planerPunkte.Min, planerPunkte.ErstesQuartil, planerPunkte.ZweitesQuartil, planerPunkte.DrittesQuartil, planerPunkte.Max);
		
		const auto& planerPunkteComplete = encoding->planerPunkteNachSpiel();
		planerPunkteCompletePlot->addData(index3 + .75, planerPunkteComplete.Min, planerPunkteComplete.ErstesQuartil, planerPunkteComplete.ZweitesQuartil, planerPunkteComplete.DrittesQuartil, planerPunkteComplete.Max);
		
		pktMax = std::max({pktMax, punkte.Max, planerPunkte.Max, planerPunkteComplete.Max});
		
		const auto& outlierPunkte = encoding->outlierPunkte();
		outlierPunktePlot->addData(index3 - .75, outlierPunkte.first.Min, outlierPunkte.first.ErstesQuartil, outlierPunkte.first.ZweitesQuartil, outlierPunkte.first.DrittesQuartil, outlierPunkte.first.Max, outlierPunkte.second);
		nurOutlierPktPlot->addData(index1, outlierPunkte.first.Min, outlierPunkte.first.ErstesQuartil, outlierPunkte.first.ZweitesQuartil, outlierPunkte.first.DrittesQuartil, outlierPunkte.first.Max, outlierPunkte.second);
		
		const auto& outlierPlanerPunkte = encoding->outlierPlanerPunkte();
		outlierPlanerPunktePlot->addData(index3, outlierPlanerPunkte.first.Min, outlierPlanerPunkte.first.ErstesQuartil, outlierPlanerPunkte.first.ZweitesQuartil, outlierPlanerPunkte.first.DrittesQuartil, outlierPlanerPunkte.first.Max, outlierPlanerPunkte.second);
		
		const auto& outlierPlanerPunkteComplete = encoding->outlierPlanerPunkteNachSpiel();
		outlierPlanerPunkteCompletePlot->addData(index3 + .75, outlierPlanerPunkteComplete.first.Min, outlierPlanerPunkteComplete.first.ErstesQuartil, outlierPlanerPunkteComplete.first.ZweitesQuartil, outlierPlanerPunkteComplete.first.DrittesQuartil, outlierPlanerPunkteComplete.first.Max, outlierPlanerPunkteComplete.second);
		
		const auto& idle = encoding->idle();
		idlePlot->addData(index1, idle.Min, idle.ErstesQuartil, idle.ZweitesQuartil, idle.DrittesQuartil, idle.Max);
		
		idleMax = std::max(idleMax, idle.Max);
		
		const auto& outlierIdle = encoding->outlierIdle();
		outlierIdlePlot->addData(index1, outlierIdle.first.Min, outlierIdle.first.ErstesQuartil, outlierIdle.first.ZweitesQuartil, outlierIdle.first.DrittesQuartil, outlierIdle.first.Max, outlierIdle.second);
		
		const auto& startUp = encoding->startUp();
		startUpPlot->addData(index1, startUp.Min, startUp.ErstesQuartil, startUp.ZweitesQuartil, startUp.DrittesQuartil, startUp.Max);
		
		startUpMax = std::max(startUpMax, startUp.Max);
	} //for ( auto iter = Encodings.begin(); iter != Encodings.end(); ++iter )
	
	for ( auto& graph : pktGraphen ) {
		graph->yAxis->setRange(0., pktMax);
	} //for ( auto& graph : pktGraphen )
	
	for ( auto& graph : idleGraphen ) {
		graph->yAxis->setRange(0., idleMax);
	} //for ( auto& graph : idleGraphen )
	
	startUpGraph->yAxis->setRange(0., startUpMax);
	
	auto layout = new QGridLayout(parentWidget);
	int zeile = -1;
	for ( auto& list : graphen ) {
		for ( auto& graph : list ) {
			auto zeilenLayout = new QHBoxLayout;
			layout->addLayout(zeilenLayout, ++zeile, 0);
			zeilenLayout->addWidget(graph);
			zeilenLayout->addStretch();
		} //for ( auto& graph : list )
	} //for ( auto& list : graphen )
	
	ScrollWidget->setWidget(parentWidget);
	ScrollWidget->horizontalScrollBar()->setValue(x);
	ScrollWidget->verticalScrollBar()->setValue(y);
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
	
	EncodingLayout = new QGridLayout;
	
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
