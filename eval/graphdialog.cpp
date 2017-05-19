#include "graphdialog.hpp"

#include "qcustomplot/qcustomplot.h"

#include <QDebug>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QGridLayout>
#include <QLabel>
#include <QSpinBox>

#include <unordered_map>

void LegendenDialog::speichereGraph(void) {
	const auto pfad = QFileDialog::getSaveFileName(this, "Speichere Graph", QString(), "*.pdf");
	if ( pfad.isEmpty() ) {
		return;
	} //if ( pfad.isEmpty() )
	
	Graph->savePdf(pfad);
	return;
}

LegendenDialog::LegendenDialog(QCustomPlot *graph, QWidget *parent) : QDialog(parent), Graph(new QCustomPlot(this)) {
	Graph->xAxis->setVisible(false);
	Graph->yAxis->setVisible(false);
	Graph->legend->setVisible(true);
	Graph->legend->layout()->take(Graph->legend);
	auto legendenLayout = new QCPLayoutInset;
	legendenLayout->addElement(Graph->legend, Qt::AlignCenter);
	Graph->plotLayout()->clear();
	Graph->plotLayout()->addElement(legendenLayout);
	
	const auto plots = graph->plottableCount();
	
	for ( auto i = 0; i < plots; ++i ) {
		auto plot = graph->plottable(i);
		auto item = graph->legend->itemWithPlottable(plot);
		
		if ( item ) {
			auto neuesItem = new QCPPlottableLegendItem(Graph->legend, plot);
			Graph->legend->addItem(neuesItem);
		} //if ( item )
	} //for ( auto i = 0; i < plots; ++i )
	
	auto knoepfe = new QDialogButtonBox(QDialogButtonBox::Ok, this);
	
	connect(knoepfe, &QDialogButtonBox::accepted, this, &StatistikGraphDialog::accept);
	connect(knoepfe->addButton("Speichern", QDialogButtonBox::NoRole), &QPushButton::clicked, this, &LegendenDialog::speichereGraph);
	
	auto layout = new QGridLayout(this);
	layout->addWidget(Graph,                       0, 0, 1, -1);
	layout->addWidget(knoepfe,                     3, 0, 1, -1);
	layout->setColumnStretch(4, 1);
	
	setWindowTitle("Legende speichern");
	return;
}

void GraphDialog::speichereGraph(void) {
	const auto pfad = QFileDialog::getSaveFileName(this, "Speichere Graph", QString(), "*.pdf");
	if ( pfad.isEmpty() ) {
		return;
	} //if ( pfad.isEmpty() )
	
	Graph->savePdf(pfad);
	return;
}

void GraphDialog::zeigeLegende(void) {
	LegendenDialog dialog(Graph, this);
	dialog.exec();
	return;
}

GraphDialog::GraphDialog(QCustomPlot *graph, QWidget *parent) : QDialog(parent), Breite(new QSpinBox(this)),
		Hoehe(new QSpinBox(this)), Graph(new QCustomPlot(this)) {
	Graph->xAxis->setTicker(graph->xAxis->ticker());
	Graph->yAxis->setTicker(graph->yAxis->ticker());
	Graph->xAxis2->setTicker(graph->xAxis2->ticker());
	Graph->yAxis2->setTicker(graph->yAxis2->ticker());
	
	Graph->xAxis->setSubTicks(graph->xAxis->subTicks());
	Graph->yAxis->setSubTicks(graph->yAxis->subTicks());
	Graph->xAxis2->setSubTicks(graph->xAxis2->subTicks());
	Graph->yAxis2->setSubTicks(graph->yAxis2->subTicks());
	
	Graph->xAxis->setTickLength(graph->xAxis->tickLengthIn(),   graph->xAxis->tickLengthOut());
	Graph->yAxis->setTickLength(graph->yAxis->tickLengthIn(),   graph->yAxis->tickLengthOut());
	Graph->xAxis2->setTickLength(graph->xAxis2->tickLengthIn(), graph->xAxis2->tickLengthOut());
	Graph->yAxis2->setTickLength(graph->yAxis2->tickLengthIn(), graph->yAxis2->tickLengthOut());
	
	Graph->xAxis->setRange(graph->xAxis->range());
	Graph->yAxis->setRange(graph->yAxis->range());
	Graph->xAxis2->setRange(graph->xAxis2->range());
	Graph->yAxis2->setRange(graph->yAxis2->range());
	
	Breite->setMaximum(15000);
	Hoehe->setMaximum(5000);
	
	Graph->setFixedSize(graph->size());
	Breite->setValue(graph->width());
	Hoehe->setValue(graph->height());
	
	connect(Breite, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), Graph, &QCustomPlot::setFixedWidth);
	connect(Hoehe,  static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), Graph, &QCustomPlot::setFixedHeight);
	
	auto knoepfe = new QDialogButtonBox(QDialogButtonBox::Ok, this);
	
	connect(knoepfe, &QDialogButtonBox::accepted, this, &StatistikGraphDialog::accept);
	connect(knoepfe->addButton("Speichern", QDialogButtonBox::NoRole), &QPushButton::clicked, this, &GraphDialog::speichereGraph);
	connect(knoepfe->addButton("Legende",   QDialogButtonBox::NoRole), &QPushButton::clicked, this, &GraphDialog::zeigeLegende);
	
	auto layout = new QGridLayout(this);
	layout->addWidget(Graph,                       0, 0, 1, -1);
	layout->addWidget(new QLabel("Breite:", this), 2, 0);
	layout->addWidget(Breite,                      2, 1);
	layout->addWidget(new QLabel("Höhe:", this),   2, 2);
	layout->addWidget(Hoehe,                       2, 3);
	layout->addWidget(knoepfe,                     3, 0, 1, -1);
	layout->setColumnStretch(4, 1);
	layout->setRowStretch(1, 1);
	
	setWindowTitle("Graph speichern");
	return;
}

StatistikGraphDialog::StatistikGraphDialog(QCustomPlot *graph, QWidget *parent) : GraphDialog(graph, parent) {
	auto axe = [this,graph](const QCPAxis *toMap) noexcept {
			if ( toMap == graph->xAxis ) {
				return Graph->xAxis;
			} //if ( toMap == graph->xAxis )
			else if ( toMap == graph->yAxis ) {
				return Graph->yAxis;
			} //else if ( toMap == graph->yAxis )
			else if ( toMap == graph->xAxis2 ) {
				return Graph->xAxis2;
			} //else if ( toMap == graph->xAxis2 )
			return Graph->yAxis2;
		};
	
	const auto plots = graph->plottableCount();
	for ( auto i = 0; i < plots; ++i ) {
		auto alterPlot = qobject_cast<QCPStatisticalBox*>(graph->plottable(i));
		if ( !alterPlot ) {
			qWarning("Falscher Plot-Typ!");
			continue;
		} //if ( !alterPlot )
		
		auto neuerPlot = new QCPStatisticalBox(axe(alterPlot->keyAxis()), axe(alterPlot->valueAxis()));
		
		neuerPlot->setData(alterPlot->data());
		neuerPlot->setBrush(alterPlot->brush());
		neuerPlot->setPen(alterPlot->pen());
		neuerPlot->setWidth(alterPlot->width());
		neuerPlot->setAntialiased(alterPlot->antialiased());
		neuerPlot->setName(alterPlot->name());
	} //for ( auto i = 0; i < plots; ++i )
	return;
}

BoxGraphDialog::BoxGraphDialog(QCustomPlot *graph, QWidget *parent) : GraphDialog(graph, parent) {
	auto axe = [this,graph](const QCPAxis *toMap) noexcept {
		if ( toMap == graph->xAxis ) {
			return Graph->xAxis;
		} //if ( toMap == graph->xAxis )
		else if ( toMap == graph->yAxis ) {
			return Graph->yAxis;
		} //else if ( toMap == graph->yAxis )
		else if ( toMap == graph->xAxis2 ) {
			return Graph->xAxis2;
		} //else if ( toMap == graph->xAxis2 )
		return Graph->yAxis2;
	};
	
	const auto plots = graph->plottableCount();
	
	std::unordered_map<QCPBarsGroup*, QCPBarsGroup*> gruppenMap;
	
	for ( auto i = 0; i < plots; ++i ) {
		auto alterPlot = qobject_cast<QCPBars*>(graph->plottable(i));
		if ( !alterPlot ) {
			qWarning("Falscher Plot-Typ!");
			continue;
		} //if ( !alterPlot )
		
		auto neuerPlot = new QCPBars(axe(alterPlot->keyAxis()), axe(alterPlot->valueAxis()));
		
		auto gruppe = alterPlot->barsGroup();
		if ( gruppe ) {
			if ( !gruppenMap.count(gruppe) ) {
				auto neueGruppe = new QCPBarsGroup(Graph);
				
				neueGruppe->setSpacing(gruppe->spacing());
				neueGruppe->setSpacingType(gruppe->spacingType());
				
				gruppenMap.insert({gruppe, neueGruppe});
			} //if ( !gruppenMap.count(gruppe) )
			auto neueGruppe = gruppenMap[gruppe];
			neueGruppe->append(neuerPlot);
		} //if ( gruppe )
		
		neuerPlot->setData(alterPlot->data());
		neuerPlot->setBrush(alterPlot->brush());
		neuerPlot->setPen(alterPlot->pen());
		neuerPlot->setWidth(alterPlot->width());
		neuerPlot->setAntialiased(alterPlot->antialiased());
		neuerPlot->setBaseValue(alterPlot->baseValue());
		neuerPlot->setName(alterPlot->name());
		
		if ( !graph->legend->hasItemWithPlottable(alterPlot) ) {
			neuerPlot->removeFromLegend();
		} //if ( !graph->legend->hasItemWithPlottable(alterPlot) )
	} //for ( auto i = 0; i < plots; ++i )
	return;
}

