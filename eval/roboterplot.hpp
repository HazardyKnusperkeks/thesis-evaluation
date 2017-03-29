#ifndef ROBOTERPLOT_HPP
#define ROBOTERPLOT_HPP

#include "../common/common-qt.hpp"
#include "qcustomplot/qcustomplot.h"

enum class TaskTyp : int;

class RoboterPlot : public QCustomPlot {
	Q_OBJECT
	private:
	QCPBars* generateBar(const TaskTyp typ, const bool mitName, const bool fail);
	
	public:
	explicit RoboterPlot(const std::unordered_map<std::string, TaskList>& plan, QWidget *parent = nullptr);
};

#endif
