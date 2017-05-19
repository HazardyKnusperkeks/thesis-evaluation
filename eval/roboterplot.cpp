#include "roboterplot.hpp"
#include "structs.hpp"

#include <unordered_set>

QCPBars* RoboterPlot::generateBar(const TaskTyp typ, const bool mitName, const bool fail) {
	static const auto brushMap =
		[](void) {
			std::unordered_map<TaskTyp, QBrush> ret;
			ret.reserve(11);
			//Values got from http://colorbrewer2.org/#type=diverging&scheme=RdBu&n=11
			ret.insert({TaskTyp::UselessGoto, QColor(103,   0,  31)});
			ret.insert({TaskTyp::Idle,        QColor(178,  24,  43)});
			ret.insert({TaskTyp::LateDeliver, QColor(214,  96,  77)});
			ret.insert({TaskTyp::FillRS,      QColor(244, 165, 130)});
			ret.insert({TaskTyp::PrepareCS,   QColor(253, 219, 199)});
			ret.insert({TaskTyp::GetBase,     QColor(247, 247, 247)});
			ret.insert({TaskTyp::Goto,        QColor(209, 229, 240)});
			ret.insert({TaskTyp::GetProduct,  QColor(146, 197, 222)});
			ret.insert({TaskTyp::MountCap,    QColor( 67, 147, 195)});
			ret.insert({TaskTyp::MountRing,   QColor( 33, 102, 172)});
			ret.insert({TaskTyp::Deliver,     QColor(  5,  48,  97)});
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
		ret->setObjectName(QString::number(static_cast<int>(typ)));
	} //if ( mitName )
	else {
		ret->removeFromLegend();
	} //else -> if ( mitName )
	return ret;
}

RoboterPlot::RoboterPlot(const std::unordered_map<std::string, TaskList>& plan, QWidget *parent) :
		GraphZuLegende(parent) {
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
					
					return stringZuTyp(taskIter->Name) == TaskTyp::Goto || taskIter->location() != previous->location();
				} //if ( typ == TaskTyp::Goto )
				return false;
			};
		
		while ( taskIter != taskEnd ) {
			TaskTyp typ;
			int size;
			const double start = now - offset;
			bool fail = false;
			if ( taskIter->Begin > now ) {
				typ  = TaskTyp::Idle;
				size = taskIter->Begin - now;
				now  = taskIter->Begin;
			} //if ( taskIter->Begin > now )
			else {
				typ = stringZuTyp(taskIter->Name);
				if ( taskIter->End == 0 ) {
					size = 30;
					now  = taskIter->Begin + size;
				} //if ( taskIter->End == 0 )
				else {
					size = taskIter->End - now;
					now  = taskIter->End;
				} //else -> if ( taskIter->End == 0 )
				fail = taskIter->Failed;
				++taskIter;
				
				if ( uselessGoto(typ) ) {
					typ = TaskTyp::UselessGoto;
				} //if ( uselessGoto(typ) )
			} //else -> if ( taskIter->Begin > now )
			auto bar = getBar(typ, fail);
			bar->setBaseValue(start);
			bar->addData(iter.key(), size);
		} //while ( taskIter != taskEnd )
		
		if ( now < gameEndWithOffset ) {
			auto bar = getBar(TaskTyp::Idle, false);
			bar->setBaseValue(now - offset);
			bar->addData(iter.key(), gameEndWithOffset - now);
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

std::vector<QCPAbstractPlottable*> RoboterPlot::legendePlottable(void) {
	std::unordered_map<int, QCPAbstractPlottable*> map;
	map.reserve(static_cast<int>(TaskTyp::UselessGoto));
	
	const auto plots = plottableCount();
	
	for ( auto i = 0; i < plots; ++i ) {
		auto plot = plottable(i);
		auto item = legend->itemWithPlottable(plot);
		
		if ( item ) {
			map.insert({plot->objectName().toInt(), plot});
		} //if ( item )
	} //for ( auto i = 0; i < plots; ++i )
	
	std::vector<QCPAbstractPlottable*> ret;
	ret.reserve(map.size());
	ret.push_back(map.at(static_cast<int>(TaskTyp::Idle)));
	ret.push_back(map.at(static_cast<int>(TaskTyp::UselessGoto)));
	ret.push_back(map.at(static_cast<int>(TaskTyp::Goto)));
	ret.push_back(map.at(static_cast<int>(TaskTyp::GetBase)));
	ret.push_back(map.at(static_cast<int>(TaskTyp::GetProduct)));
	ret.push_back(map.at(static_cast<int>(TaskTyp::MountRing)));
	ret.push_back(map.at(static_cast<int>(TaskTyp::MountCap)));
	ret.push_back(map.at(static_cast<int>(TaskTyp::PrepareCS)));
	ret.push_back(map.at(static_cast<int>(TaskTyp::FillRS)));
	ret.push_back(map.at(static_cast<int>(TaskTyp::Deliver)));
	ret.push_back(map.at(static_cast<int>(TaskTyp::LateDeliver)));
	return ret;
}
