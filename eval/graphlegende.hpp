#ifndef GRAPHLEGENDE_HPP
#define GRAPHLEGENDE_HPP

#include "qcustomplot/qcustomplot.h"

#include <vector>

class GraphZuLegende : public QCustomPlot {
	Q_OBJECT
	private:
	
	public:
	explicit GraphZuLegende(QWidget *parent = nullptr) : QCustomPlot(parent) {
		return;
	}
	
	virtual std::vector<QCPAbstractPlottable*> legendePlottable(void) = 0;
};

#endif

