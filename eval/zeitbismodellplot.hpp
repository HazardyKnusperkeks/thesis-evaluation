#ifndef ZEITBISMODELLPLOT_HPP
#define ZEITBISMODELLPLOT_HPP

#include "../common/common-qt.hpp"
#include "qcustomplot/qcustomplot.h"

class ZeitBisModellPlot : public QCustomPlot {
	Q_OBJECT
	public:
	ZeitBisModellPlot(const std::map<int, AvgSequenz<double>>& zeitBisModell,
		const std::map<int, AvgSequenz<double>>& zeitSeitModell, QWidget *parent = nullptr);
};

#endif
