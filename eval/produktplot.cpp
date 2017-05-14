#include "produktplot.hpp"
#include "../common/common-qt.hpp"

#include <unordered_set>

enum class ProduktStatus : int {
	Nix,
	AufMaschine,
	InVerarbeitung,
	AufRoboter,
};

namespace std {
template<>
struct hash<ProduktStatus> {
	auto operator()(const ProduktStatus typ) const noexcept {
		return hash<int>{}(static_cast<int>(typ));
	}
};
}

QCPBars* ProduktPlot::generateBar(const ProduktStatus status, const bool mitName) {
	static const auto brushMap =
		[](void) {
			std::unordered_map<ProduktStatus, QBrush> ret;
			ret.reserve(3);
			//Values from http://colorbrewer2.org/#type=qualitative&scheme=Set1&n=3
			ret.insert({ProduktStatus::AufMaschine,    QColor(228,  26,  28)});
			ret.insert({ProduktStatus::AufRoboter,     QColor( 55, 126, 184)});
			ret.insert({ProduktStatus::InVerarbeitung, QColor( 77, 175,  74)});
			return ret;
		}();
	
	QCPBars *ret = new QCPBars(yAxis, xAxis);
	
	auto brush = brushMap.at(status);
	
	ret->setAntialiased(false);
	ret->setBrush(brush);
	ret->setPen(QPen(Qt::black, .25));
	ret->setWidth(.9);
	
	if ( mitName ) {
		switch ( status ) {
			case ProduktStatus::AufMaschine    : ret->setName("Idle");         break;
			case ProduktStatus::AufRoboter     : ret->setName("Goto");         break;
			case ProduktStatus::InVerarbeitung : ret->setName("Useless Goto"); break;
			default : Q_UNREACHABLE(); break;
		} //switch ( status )
	} //if ( mitName )
	else {
		ret->removeFromLegend();
	} //else -> if ( mitName )
	return ret;
}

ProduktPlot::ProduktPlot(const std::vector<Produkt>& produkte, QWidget *parent) : QCustomPlot(parent),
		Produkte(produkte) {
	if ( Produkte.empty() ) {
		return;
	} //if ( Produkte.empty() )
	
	QMap<double, QString> produktTicks;
	for ( auto i = 0u; i < Produkte.size(); ++i ) {
		produktTicks.insert(Produkte.size() - i, "Product #" + QString::number(i + 1));
	} //for ( auto i = 0u; i < Produkte.size(); ++i )
	
	auto produktTicker = QSharedPointer<QCPAxisTickerText>::create();
	produktTicker->setTicks(produktTicks);
	
	yAxis->setTicker(produktTicker);
	yAxis->setRange(Produkte.size() + 1., 0.);
	yAxis->setSubTicks(false);
	yAxis->setTickLength(0, 0);
	
	std::unordered_set<ProduktStatus> vorhandeneTypen;
	
	auto getBar = [this,&vorhandeneTypen](const ProduktStatus typ) {
			const bool mitName = vorhandeneTypen.count(typ) == 0;
			if ( mitName ) {
				vorhandeneTypen.insert(typ);
			} //if ( mitName )
			return generateBar(typ, mitName);
		};
	
	constexpr int gameEnd = 900, offset = 240/*, gameEndWithOffset = gameEnd + offset*/;
	for ( auto i = 0u; i < Produkte.size(); ++i ) {
		const double key = Produkte.size() - i;
		
		const auto& produkt(Produkte[i]);
		int now = 0;
		ProduktStatus altStatus = ProduktStatus::Nix;
		bool vernichtet = false;
		
		for ( const auto& position : produkt.Positionen ) {
			if ( position.Wann == -1 ) {
				break;
			} //if ( position.Wann == -1 )
			
			auto update = [&](const ProduktStatus status) {
					const auto wann = position.Wann - offset;
					
					if ( altStatus != ProduktStatus::Nix ) {
						auto bar = getBar(altStatus);
						bar->setBaseValue(now);
						bar->addData(key, wann - now);
					} //if ( altStatus != ProduktStatus::Nix )
					
					if ( status != ProduktStatus::Nix ) {
						now = wann;
					} //if ( status != ProduktStatus::Nix )
					altStatus = status;
					return;
				};
			
			switch ( position.Was ) {
				case ProduktPosition::Aufgehoben : update(ProduktStatus::AufRoboter);     break;
				case ProduktPosition::Bearbeitet : update(ProduktStatus::InVerarbeitung); break;
				case ProduktPosition::Fertig     : update(ProduktStatus::AufMaschine);    break;
				case ProduktPosition::Zerstoert  : update(ProduktStatus::Nix); vernichtet = true; break;
				default : break;
			} //switch ( position.Was )
		} //for ( const auto& position : produkt.Positionen )
		
		if ( !vernichtet && altStatus != ProduktStatus::Nix ) {
			auto bar = getBar(altStatus);
			bar->setBaseValue(now);
			bar->addData(key, gameEnd - now);
		} //if ( !vernichtet && altStatus != ProduktStatus::Nix )
	} //for ( auto i = 0u; i < Produkte.size(); ++i )
	
	auto zeitTicker = QSharedPointer<QCPAxisTickerTime>::create();
	zeitTicker->setTimeFormat("%m:%s");
	
	xAxis->setTicker(zeitTicker);
	xAxis->setRange(0., gameEnd);
	xAxis->setLabel("Production Phase Gametime");
	return;
}
