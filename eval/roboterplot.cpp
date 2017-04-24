#include "roboterplot.hpp"
#include "structs.hpp"

#include <unordered_set>

QCPBars* RoboterPlot::generateBar(const TaskTyp typ, const bool mitName, const bool fail) {
	static const auto brushMap =
		[](void) {
			std::unordered_map<TaskTyp, QBrush> ret;
			ret.reserve(11);
			//Values got from http://colorbrewer2.org/#type=diverging&scheme=Spectral&n=11
			ret.insert({TaskTyp::UselessGoto, QColor(158,   1,  66)});
			ret.insert({TaskTyp::Idle,        QColor(213,  62,  79)});
			ret.insert({TaskTyp::LateDeliver, QColor(244, 109,  67)});
			ret.insert({TaskTyp::FillRS,      QColor(253, 174,  97)});
			ret.insert({TaskTyp::PrepareCS,   QColor(254, 224, 139)});
			ret.insert({TaskTyp::GetBase,     QColor(255, 255, 191)});
			ret.insert({TaskTyp::Goto,        QColor(230, 245, 152)});
			ret.insert({TaskTyp::GetProduct,  QColor(171, 221, 164)});
			ret.insert({TaskTyp::MountCap,    QColor(102, 194, 165)});
			ret.insert({TaskTyp::MountRing,   QColor( 50, 136, 189)});
			ret.insert({TaskTyp::Deliver,     QColor( 94,  79, 162)});
			return ret;
		}();
	
	QCPBars *ret = new QCPBars(yAxis, xAxis);
	
	auto brush = brushMap.at(typ);
	
	if ( fail ) {
		brush.setStyle(Qt::Dense2Pattern);
	} //if ( fail )
	
	ret->setAntialiased(false);
	ret->setBrush(brush);
	ret->setPen(QPen(Qt::black, .25));
	ret->setWidth(.9);
	
	if ( mitName ) {
		switch ( typ ) {
			case TaskTyp::Idle        : ret->setName("Idle");         break;
			case TaskTyp::Goto        : ret->setName("Goto");         break;
			case TaskTyp::UselessGoto : ret->setName("Useless Goto"); break;
			case TaskTyp::GetBase     : ret->setName("Get Base");     break;
			case TaskTyp::GetProduct  : ret->setName("Get Product");  break;
			case TaskTyp::PrepareCS   : ret->setName("Prepare CS");   break;
			case TaskTyp::FillRS      : ret->setName("Feed RS");      break;
			case TaskTyp::MountRing   : ret->setName("Mount Ring");   break;
			case TaskTyp::MountCap    : ret->setName("Mount Cap");    break;
			case TaskTyp::Deliver     : ret->setName("Deliver");      break;
			case TaskTyp::LateDeliver : ret->setName("Late Deliver"); break;
			default : Q_UNREACHABLE(); break;
		} //switch ( typ )
	} //if ( mitName )
	else {
		ret->removeFromLegend();
	} //else -> if ( mitName )
	return ret;
}

RoboterPlot::RoboterPlot(const std::unordered_map<std::string, TaskList>& plan, QWidget *parent) : QCustomPlot(parent) {
	if ( plan.empty() ) {
		return;
	} //if ( plan.empty() )
	
	QMap<double, QString> roboterTicks = {{3., "Tuvok"}, {2., "T'Pol"}, {1., "Spock"}};
	auto roboterTicker = QSharedPointer<QCPAxisTickerText>::create();
	roboterTicker->setTicks(roboterTicks);
	
	yAxis->setTicker(roboterTicker);
	yAxis->setRange(0., 4.);
	yAxis->setSubTicks(false);
	yAxis->setTickLength(0, 0);
	
	std::unordered_set<TaskTyp> vorhandeneTypen;
	
	auto getBar = [this,&vorhandeneTypen](const TaskTyp typ, const bool fail) {
			const bool mitName = vorhandeneTypen.count(typ) == 0;
			if ( mitName ) {
				vorhandeneTypen.insert(typ);
			} //if ( mitName )
			return generateBar(typ, mitName, fail);
		};
	
	constexpr int gameEnd = 900, offset = 240, gameEndWithOffset = gameEnd + offset;
	int max = gameEnd;
	for ( auto iter = roboterTicks.begin(); iter != roboterTicks.end(); ++iter ) {
		QCPBars *oldBar = nullptr;
		int now = offset;
		
		const auto& robotPlan(plan.at(iter->toStdString()));
		auto taskIter      = robotPlan.begin();
		const auto taskEnd = robotPlan.end();
		
		auto uselessGoto = [&](const TaskTyp typ) {
				if ( typ == TaskTyp::Goto ) {
					if ( taskIter == taskEnd ) {
						return true;
					} //if ( taskIter == taskEnd )
					const auto previous = taskIter - 1;
					
					return stringZuTyp(previous->Name) == TaskTyp::Goto || taskIter->location() != previous->location();
				} //if ( typ == TaskTyp::Goto )
				return false;
			};
		
		while ( taskIter != taskEnd ) {
			TaskTyp typ;
			int size;
			bool fail = false;
			if ( taskIter->Begin > now ) {
				typ = TaskTyp::Idle;
				size = taskIter->Begin - now;
				now = taskIter->Begin;
			} //if ( taskIter->Begin > now )
			else {
				typ = stringZuTyp(taskIter->Name);
				if ( taskIter->End == 0 ) {
					size = 30;
					now = taskIter->Begin + size;
				} //if ( taskIter->End == 0 )
				else {
					size = taskIter->End - now;
					now = taskIter->End;
				} //else -> if ( taskIter->End == 0 )
				fail = taskIter->Failed;
				++taskIter;
				
				if ( uselessGoto(typ) ) {
					typ = TaskTyp::UselessGoto;
				} //if ( uselessGoto(typ) )
			} //else -> if ( taskIter->Begin > now )
			auto bar = getBar(typ, fail);
			bar->addData(iter.key(), size);
			
			if ( oldBar ) {
				bar->moveAbove(oldBar);
			} //if ( oldBar )
			
			oldBar = bar;
		} //while ( taskIter != taskEnd )
		
		if ( now < gameEndWithOffset ) {
			auto bar = getBar(TaskTyp::Idle, false);
			bar->addData(iter.key(), gameEndWithOffset - now);
			bar->moveAbove(oldBar);
			now = gameEndWithOffset;
		} //if ( now < gameEndWithOffset )
		max = std::max(max, now - offset);
	} //for ( auto iter = roboterTicks.begin(); iter != roboterTicks.end(); ++iter )
	
	auto zeitTicker = QSharedPointer<QCPAxisTickerTime>::create();
	zeitTicker->setTimeFormat("%m:%s");
	
	xAxis->setTicker(zeitTicker);
	xAxis->setRange(0., max);
	xAxis->setLabel("Production Phase Gametime");
	return;
}
