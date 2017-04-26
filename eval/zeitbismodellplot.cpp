#include "zeitbismodellplot.hpp"

ZeitBisModellPlot::ZeitBisModellPlot(const std::map<int, AvgSequenz<double>>& zeitBisModell,
		const std::map<int, AvgSequenz<double>>& zeitSeitModell, QWidget *parent) : QCustomPlot(parent) {
	if ( zeitBisModell.empty() ) {
		return;
	} //if ( zeitBisModell.empty() )
	
	auto bisPlot = new QCPBars(xAxis, yAxis);
	auto seitPlot = new QCPBars(xAxis, yAxis);
	
	auto gruppe = new QCPBarsGroup(this);
	
	gruppe->setSpacing(.15);
	
	bisPlot->setBarsGroup(gruppe);
	bisPlot->setWidth(.3);
	
	seitPlot->setBarsGroup(gruppe);
	seitPlot->setBrush(QColor(180, 00, 120, 50));
	seitPlot->setPen(QColor(180, 00, 120));
	seitPlot->setWidth(.3);
	
	auto ticker = QSharedPointer<QCPAxisTickerText>::create();
	ticker->setSubTickCount(0.);
	
	double max = 0.;
	
	for ( const auto& paar : zeitBisModell ) {
		const AvgSequenz<double>& avgSeq = paar.second;
		bisPlot->addData(paar.first, avgSeq.Avg);
		max = std::max(max, avgSeq.Avg);
		
		ticker->addTick(paar.first, QString::number(paar.first));
	} //for ( const auto& paar : zeitBisModell )
	
	for ( const auto& paar : zeitSeitModell ) {
		const AvgSequenz<double>& avgSeq = paar.second;
		seitPlot->addData(paar.first, avgSeq.Avg);
		max = std::max(max, avgSeq.Avg);
	} //for ( const auto& paar : zeitSeitModell )
	
	xAxis->setRange(0., zeitBisModell.size() + 1.);
	xAxis->setTicker(ticker);
	yAxis->setRange(0., max + 1.);
	return;
}

