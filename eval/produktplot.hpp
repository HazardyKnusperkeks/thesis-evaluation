#ifndef PRODUKTPLOT_HPP
#define PRODUKTPLOT_HPP

#include "qcustomplot/qcustomplot.h"

class Produkt;

enum class ProduktStatus : int;

class ProduktPlot : public QCustomPlot {
	Q_OBJECT
	private:
	const std::vector<Produkt>& Produkte;
	
	QCPBars* generateBar(const ProduktStatus status, const bool mitName);
	
	public:
	explicit ProduktPlot(const std::vector<Produkt>& produkte, QWidget *parent = nullptr);
};

#endif
