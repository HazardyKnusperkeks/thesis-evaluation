#ifndef ROBOTERPLOT_HPP
#define ROBOTERPLOT_HPP

#include "../common/common-qt.hpp"
#include "graphlegende.hpp"

enum class TaskTyp : int;

class RoboterPlot : public GraphZuLegende {
	Q_OBJECT
	private:
	QCPBars* generateBar(const TaskTyp typ, const bool mitName, const bool fail);
	
	public:
	explicit RoboterPlot(const std::unordered_map<std::string, TaskList>& plan, QWidget *parent = nullptr);
	
	std::vector<QCPAbstractPlottable*> legendePlottable(void) override;
};

#endif
