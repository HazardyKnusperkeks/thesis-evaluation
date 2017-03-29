#include "../common/common-qt.hpp"

#include <cassert>
#include <fstream>
#include <iostream>
#include <map>

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QString>

static inline bool istNull(const Zeit& zeit) noexcept {
	return std::get<0>(zeit) == 0 && std::get<1>(zeit) == 0 && std::get<2>(zeit) == 0 && std::get<3>(zeit) == 0;
}

static inline Zeit& operator-=(Zeit& lhs, const Zeit& rhs) noexcept {
	auto& std(std::get<0>(lhs));
	auto& min(std::get<1>(lhs));
	auto& sek(std::get<2>(lhs));
	auto& mic(std::get<3>(lhs));
	
	mic -= std::get<3>(rhs);
	sek -= std::get<2>(rhs);
	min -= std::get<1>(rhs);
	std -= std::get<0>(rhs);
	
	while ( mic < 0 ) {
		sek -= 1;
		mic += 1'000'000;
	} //while ( mic < 0 )
	
	while ( sek < 0 ) {
		min -= 1;
		sek += 60;
	} //while ( sek < 0 )
	
	while ( min < 0 ) {
		std -= 1;
		min += 60;
	} //while ( min < 0 )
	
	assert(std >= 0);
	assert(min >= 0 && min < 60);
	assert(sek >= 0 && sek < 60);
	assert(mic >= 0 && mic < 1'000'000);
	return lhs;
}

static inline Zeit operator-(Zeit lhs, const Zeit& rhs) noexcept {
	return lhs -= rhs;
}

static inline Zeit& operator+=(Zeit& lhs, const Zeit& rhs) noexcept {
	auto& std(std::get<0>(lhs));
	auto& min(std::get<1>(lhs));
	auto& sek(std::get<2>(lhs));
	auto& mic(std::get<3>(lhs));
	
	mic += std::get<3>(rhs);
	sek += std::get<2>(rhs);
	min += std::get<1>(rhs);
	std += std::get<0>(rhs);
	
	while ( mic >= 1'000'000 ) {
		sek += 1;
		mic -= 1'000'000;
	} //while ( mic >= 1'000'000 )
	
	while ( sek >= 60 ) {
		min += 1;
		sek -= 60;
	} //while ( sek >= 60 )
	
	while ( min >= 60 ) {
		std += 1;
		min -= 60;
	} //while ( min >= 60 )
	
	assert(std >= 0);
	assert(min >= 0 && min < 60);
	assert(sek >= 0 && sek < 60);
	assert(mic >= 0 && mic < 1'000'000);
	return lhs;
}

struct Double {
	double D;
	
	Double(void) = default;
	
	Double(const double d) noexcept : D(d) {
		return;
	}
	
	Double(const Zeit& zeit) noexcept {
		const auto& std(std::get<0>(zeit));
		const auto& min(std::get<1>(zeit));
		const auto& sek(std::get<2>(zeit));
		const auto& mic(std::get<3>(zeit));
		
		D = std;
		D *= 60.;
		D += min;
		D *= 60;
		D += sek;
		D += mic / 1'000'000.;
		return;
	}
	
	inline operator double(void) const noexcept {
		return D;
	}
};

static inline std::ostream& operator<<(std::ostream& out, const Double& d) noexcept {
	out<<std::right<<std::setw(10)<<std::fixed<<d.D;
	return out;
}

static inline std::ostream& operator<<(std::ostream& out, const decltype(Modell().Benutzt)& e) noexcept {
	switch ( e ) {
		case Modell::Uebersprungen : out<<"Übersprungen"; break;
		case Modell::Geparst       : out<<"Geparst";      break;
		case Modell::PlanGebaut    : out<<"Plan gebaut";  break;
	} //switch ( e )
	return out;
}

int main(int argc, char *argv[]) {
	const char *pfad = "/home/theses/bschaepers/fawkes-robotino/bin/debug8.log";
	QString ausgabePfad = "./";
	
	if ( argc >= 3 ) {
		pfad = argv[2];
	} //if ( argc >= 3 )
	
	if ( argc >= 2 ) {
		ausgabePfad = argv[1];
	} //if ( argc >= 2 )
	
	std::ifstream datei(pfad);
	
	if ( !datei.is_open() ) {
		std::cerr<<"Konnte Eingabe-Datei nicht öffnen."<<std::endl;
		return 1;
	} //if ( !datei.is_open() )
	
	QString ausgabeLogPfad;
	int dateiCount = 0;
	bool weiter = true;
	do { //while ( weiter )
		ausgabeLogPfad = ausgabePfad + QString::number(++dateiCount) + "-filtered.log";
		weiter = QFileInfo::exists(ausgabeLogPfad);
	} while ( weiter );
	
	std::ofstream ausgabeLog(ausgabeLogPfad.toStdString().c_str());
	QFile ausgabeInfoDatei(ausgabePfad + QString::number(dateiCount) + ".parsed");
	
	if ( !ausgabeLog.is_open() || !ausgabeInfoDatei.open(QIODevice::WriteOnly) ) {
		std::cerr<<"Konnte mindestens eine Ausgabe-Datei nicht öffnen."<<std::endl;
		return 2;
	} //if ( !ausgabeLog.is_open() || !ausgabeInfoDatei.open(QIODevice::WriteOnly) )
	
	QFileInfo eingabePfad(pfad);
	if ( eingabePfad.fileName() == "debug8.log" ) {
		QDir dir(eingabePfad.dir());
		for ( int i = 1; i <= 3; ++i ) {
			const auto alterPfad(dir.filePath("debug" + QString::number(i) + ".log"));
			const auto neuerPfad(ausgabePfad + QString::number(dateiCount) + "-roboter" + QString::number(i) + ".log");
			if ( !QFile::copy(alterPfad, neuerPfad) ) {
				std::cerr<<"Fehler beim kopieren vom Roboter "<<i<<" Log."<<std::endl;
				return 3;
			} //if ( !QFile::copy(alterPfad, neuerPfad) )
		} //for ( int i = 1; i <= 3; ++i )
		
		auto alterPfad(dir.filePath("refbox.log"));
		auto neuerPfad(ausgabePfad + QString::number(dateiCount) + "-refbox.log");
		if ( !QFile::copy(alterPfad, neuerPfad) ) {
			std::cerr<<"Fehler beim kopieren vom Refbox Log."<<std::endl;
			return 3;
		} //if ( !QFile::copy(alterPfad, neuerPfad) )
		
		alterPfad = dir.filePath("debug8.log");
		neuerPfad = ausgabePfad + QString::number(dateiCount) + "-planer.log";
		if ( !QFile::copy(alterPfad, neuerPfad) ) {
			std::cerr<<"Fehler beim kopieren vom Planer Log."<<std::endl;
			return 4;
		} //if ( !QFile::copy(alterPfad, neuerPfad) )
		
		alterPfad = dir.filePath("Memory.png");
		neuerPfad = ausgabePfad + QString::number(dateiCount) + "-Memory.png";
		if ( !QFile::copy(alterPfad, neuerPfad) ) {
			std::cerr<<"Fehler beim kopieren vom Memory-Bild."<<std::endl;
			return 5;
		} //if ( !QFile::copy(alterPfad, neuerPfad) )
		
		alterPfad = dir.filePath("Memory.rrd");
		neuerPfad = ausgabePfad + QString::number(dateiCount) + "-Memory.rrd";
		if ( !QFile::copy(alterPfad, neuerPfad) ) {
			std::cerr<<"Fehler beim kopieren von der Memory-RRD."<<std::endl;
			return 6;
		} //if ( !QFile::copy(alterPfad, neuerPfad) )
	} //if ( eingabePfad.fileName() == "debug8.log" )
	
	QDataStream ausgabeInfo(&ausgabeInfoDatei);
	
	constexpr const char *robots[3] = {"Tuvok", "T'Pol", "Spock"};
	
	Infos infos;
	std::map<int, int> produktMap;
	
	auto loescheProdukt = [&produktMap](const int logischerIndex) noexcept {
			produktMap.erase(logischerIndex);
			auto kopie(produktMap);
			produktMap.clear();
			std::transform(kopie.begin(), kopie.end(), std::inserter(produktMap, produktMap.end()),
			               [logischerIndex](std::pair<int, int> paar) noexcept {
			               	if ( paar.first > logischerIndex ) {
			               		--paar.first;
			               	} //if ( paar.first > logischerIndex )
			               	return paar;
			               });
			return;
		};
	
	Zeit baseGrounding, teamGrounding, start;
	Zeit *groundingZeit = nullptr;
	bool ersteZeile = true, erstesModell = true;
	
	infos.AusgefuehrterPlan.reserve(3);
	TaskList *roboterPlan = nullptr;
	
	Modell::PlanT leererPlan;
	leererPlan.reserve(3);
	
	for ( const auto& robot : robots ) {
		leererPlan[robot];
		infos.AusgefuehrterPlan[robot];
	} //for ( const auto& robot : robots )
	
	std::string zeile;
	
	auto getZeit = [&zeile](void) noexcept { return zeitAusString({zeile.data() + 2}); };
	auto inZeile = [&zeile](const char *str) noexcept { return zeile.find(str) != std::string::npos; };
	
	while ( !datei.eof() ) {
		std::getline(datei, zeile);
		bool schreibeZeile = true;
		
		if ( ersteZeile ) {
			ersteZeile = false;
			start = getZeit();
		} //if ( ersteZeile )
		
		if ( !inZeile("ASP-Planer") ) {
			continue;
		} //if ( !inZeile("ASP-Planer") )
		else if ( inZeile("Part #1: base []") ) {
			baseGrounding = getZeit();
			groundingZeit = &baseGrounding;
		} //else if ( inZeile("Part #1: base []") )
		else if ( inZeile("Part #1: ourTeam [") ) {
			teamGrounding = getZeit();
			groundingZeit = &teamGrounding;
		} //else if ( inZeile("Part #1: ourTeam [") )
		else if ( inZeile("Grounding done.") ) {
			if ( groundingZeit ) {
				Zeit temp = getZeit();
				*groundingZeit = std::move(temp -= *groundingZeit);
				groundingZeit = nullptr;
			} //if ( groundingZeit )
		} //else if ( inZeile("Grounding done.") )
		else if ( inZeile("Start async solving.") ) {
			infos.Solves.emplace_back(getZeit());
		} //else if ( inZeile("Start async solving.") )
		else if ( inZeile("Cancel solving, new requests:") ) {
			//Zeile: W 14:45:18.844749 ASP-Planer-Thread: Cancel solving, new requests: 0, interrupt level: Critical, reason: New robot
			Solve& solve(infos.Solves.back());
			
			const auto levelBeginPos = zeile.find("el:") + 4, levelEndPos = zeile.find(',', levelBeginPos);
			const auto grundBeginPos = zeile.find(':', levelEndPos) + 2, grundEndPos = zeile.size();
			solve.CancelLevel = zeile.substr(levelBeginPos, levelEndPos - levelBeginPos);
			solve.CancelGrund = zeile.substr(grundBeginPos, grundEndPos - grundBeginPos);
			solve.CancelZeit = getZeit();
		} //else if ( inZeile("Cancel solving, new requests:") )
		else if ( inZeile("Solving done.") ) {
			infos.Solves.back().End = getZeit();
		} //else if ( inZeile("Solving done.") )
		else if ( inZeile("Part #1: startProduction []") ) {
			infos.Solves.clear();
		} //else if ( inZeile("Part #1: startProduction []") )
		else if ( inZeile("New model") ) {
			auto& solve(infos.Solves.back());
			
			if ( istNull(solve.End) ) {
				solve.Models.emplace_back(getZeit(), leererPlan);
				if ( erstesModell ) {
					erstesModell = false;
					infos.StartUp = Double{getZeit() - start};
				} //if ( erstesModell )
			} //if ( istNull(solve.End) )
		} //else if ( inZeile("New model") )
		else if ( inZeile("Extracted plan") ) {
			//Zeile: I 14:38:51.636230 ASP-Planer-Thread: Extracted plan size: 13 Start GT: 262
			auto& model(infos.Solves.back().Models.back());
			model.Benutzt = Modell::Geparst;
			model.PlanZeit = stringViewToInt({zeile.data() + zeile.find("GT") + 4});
		} //else if ( inZeile("Extracted plan") )
		else if ( inZeile("commit changes") ) {
			infos.Solves.back().Models.back().Benutzt = Modell::PlanGebaut;
		} //else if ( inZeile("commit changes") )
		else if ( inZeile("Plan element:") ) {
			//Zeile: I 14:38:38.981424 ASP-Planer-Thread: Plan element: (Spock, getProduct(m("C","CS2","O")), 320, 360)
			const auto namePosBegin = zeile.find('(') + 1,           namePosEnd    = zeile.find(',', namePosBegin);
			const auto endPosEnd    = zeile.size() - 1,              endPosBegin   = zeile.rfind(' ', endPosEnd) + 1;
			const auto beginPosEnd  = zeile.rfind(',', endPosBegin), beginPosBegin = zeile.rfind(' ', beginPosEnd) + 1;
			const auto taskPosBegin = namePosEnd + 2,                taskPosEnd    = beginPosBegin - 2;
			
			infos.Solves.back().Models.back().Plan[zeile.substr(namePosBegin, namePosEnd - namePosBegin)].push_back(
				Task{zeile.substr(taskPosBegin, taskPosEnd - taskPosBegin),
				     stringViewToInt({zeile.data() + beginPosBegin, beginPosEnd - beginPosBegin}),
				     stringViewToInt({zeile.data() + endPosBegin,   endPosEnd   - endPosBegin})});
		} //else if ( inZeile("Plan element:") )
		else if ( inZeile("Plan & idle") ) {
			//Zeile: I 15:00:07.130751 ASP-Planer-Thread: Plan & idle time for robot T'Pol
			const auto namePosBegin = zeile.find("robot") + 6, namePosEnd = zeile.size();
			roboterPlan = &infos.AusgefuehrterPlan[zeile.substr(namePosBegin, namePosEnd - namePosBegin)];
			
			auto& solve(infos.Solves.back());
			if ( istNull(solve.End) ) {
				solve.End = getZeit();
			} //if ( istNull(solve.End) )
		} //else if ( inZeile("Plan & idle") )
		else if ( inZeile("Task #") ) {
			//Zeile: I 15:00:07.130789 ASP-Planer-Thread: Task # 1: (prepareCS(m("C","CS1","I"))      ,  269,  330) Failed Idle: 29 seconds
			const auto taskPosBegin = zeile.find('(') + 1,           taskPosEnd    = zeile.find(' ', taskPosBegin);
			const auto endPosEnd    = zeile.rfind(')'),              endPosBegin   = zeile.rfind(' ', endPosEnd) + 1;
			const auto beginPosEnd  = zeile.rfind(',', endPosBegin), beginPosBegin = zeile.rfind(' ', beginPosEnd) + 1;
			
			const auto failed = inZeile("Failed");
			
			roboterPlan->push_back(Task{zeile.substr(taskPosBegin, taskPosEnd - taskPosBegin),
			                            stringViewToInt({zeile.data() + beginPosBegin, beginPosEnd - beginPosBegin}),
			                            stringViewToInt({zeile.data() + endPosBegin,   endPosEnd   - endPosBegin}),
			                            failed});
		} //else if ( inZeile("Task #") )
		else if ( inZeile("generated product") ) {
			//Zeile: I 14:39:43.979990 ASP-Planer-Thread: BS generated product #0 with base color SILVER at 312.
			const auto woPosBegin    = zeile.find("Thread:")         + 8, woPosEnd    = zeile.find(' ', woPosBegin);
			const auto idPosBegin    = zeile.find('#', woPosEnd)     + 1, idPosEnd    = zeile.find(' ', idPosBegin);
			const auto colorPosBegin = zeile.find("color", idPosEnd) + 6, colorPosEnd = zeile.find(' ', colorPosBegin);
			const auto wannPosBegin  = zeile.find("at", colorPosEnd) + 3, wannPosEnd  = zeile.find('.', wannPosBegin);
			
			const auto id = stringViewToInt({zeile.data() + idPosBegin, idPosEnd - idPosBegin});
			produktMap.insert({id, infos.Produkte.size()});
			infos.Produkte.emplace_back(zeile.substr(colorPosBegin, colorPosEnd - colorPosBegin),
			                            zeile.substr(woPosBegin, woPosEnd - woPosBegin),
			                            stringViewToInt({zeile.data() + wannPosBegin, wannPosEnd - wannPosBegin}));
		} //else if ( inZeile("generated product") )
		else if ( inZeile("picks up") ) {
			//Zeile: I 15:57:57.149765 ASP-Planer-Thread: Robot Tuvok picks up product #0 at 321.
			const auto woPosBegin   = zeile.find("Robot")        + 6, woPosEnd   = zeile.find(' ', woPosBegin);
			const auto idPosBegin   = zeile.find('#', woPosEnd)  + 1, idPosEnd   = zeile.find(' ', idPosBegin);
			const auto wannPosBegin = zeile.find("at", idPosEnd) + 3, wannPosEnd = zeile.find('.', wannPosBegin);
			
			const auto id = stringViewToInt({zeile.data() + idPosBegin, idPosEnd - idPosBegin});
			auto& positionen(infos.Produkte[produktMap[id]].Positionen);
			positionen.emplace_back(ProduktPosition{ProduktPosition::Aufgehoben,
			                                        zeile.substr(woPosBegin, woPosEnd - woPosBegin),
			                                        stringViewToInt({zeile.data() + wannPosBegin,
			                                                         wannPosEnd - wannPosBegin})});
		} //else if ( inZeile("picks up") )
		else if ( inZeile("works on") ) {
			//Zeile: I 15:58:02.799023 ASP-Planer-Thread: Machine CS2 works on product #1 from 328 until 333.
			const auto woPosBegin   = zeile.find("Machine")        + 8, woPosEnd   = zeile.find(' ', woPosBegin);
			const auto idPosBegin   = zeile.find('#', woPosEnd)    + 1, idPosEnd   = zeile.find(' ', idPosBegin);
			const auto wannPosBegin = zeile.find("from", idPosEnd) + 5, wannPosEnd = zeile.find(' ', wannPosBegin);
			
			const auto id = stringViewToInt({zeile.data() + idPosBegin, idPosEnd - idPosBegin});
			auto& positionen(infos.Produkte[produktMap[id]].Positionen);
			positionen.emplace_back(ProduktPosition{ProduktPosition::Bearbeitet,
			                                        zeile.substr(woPosBegin, woPosEnd - woPosBegin),
			                                        stringViewToInt({zeile.data() + wannPosBegin,
			                                                         wannPosEnd - wannPosBegin})});
		} //else if ( inZeile("works on") )
		else if ( inZeile("finished working on") ) {
			//Zeile: I 15:58:06.657518 ASP-Planer-Thread: Machine CS2 has finished working on product #1 at 333.
			const auto woPosBegin   = zeile.find("Machine")      + 8, woPosEnd   = zeile.find(' ', woPosBegin);
			const auto idPosBegin   = zeile.find('#', woPosEnd)  + 1, idPosEnd   = zeile.find(' ', idPosBegin);
			const auto wannPosBegin = zeile.find("at", idPosEnd) + 3, wannPosEnd = zeile.find('.', wannPosBegin);
			
			const auto id = stringViewToInt({zeile.data() + idPosBegin, idPosEnd - idPosBegin});
			auto& positionen(infos.Produkte[produktMap[id]].Positionen);
			positionen.emplace_back(ProduktPosition{ProduktPosition::Fertig,
			                                        zeile.substr(woPosBegin, woPosEnd - woPosBegin),
			                                        stringViewToInt({zeile.data() + wannPosBegin,
			                                                         wannPosEnd - wannPosBegin})});
		} //else if ( inZeile("finished working on") )
		else if ( inZeile("drops product") ) {
			//Zeile: I 15:58:30.854244 ASP-Planer-Thread: Robot Tuvok drops product #0 at 361.
			const auto woPosBegin   = zeile.find("Robot")        + 6, woPosEnd   = zeile.find(' ', woPosBegin);
			const auto idPosBegin   = zeile.find('#', woPosEnd)  + 1, idPosEnd   = zeile.find(' ', idPosBegin);
			const auto wannPosBegin = zeile.find("at", idPosEnd) + 3, wannPosEnd = zeile.find('.', wannPosBegin);
			
			const auto id = stringViewToInt({zeile.data() + idPosBegin, idPosEnd - idPosBegin});
			auto& positionen(infos.Produkte[produktMap[id]].Positionen);
			positionen.emplace_back(ProduktPosition{ProduktPosition::Abgelegt,
			                                        zeile.substr(woPosBegin, woPosEnd - woPosBegin),
			                                        stringViewToInt({zeile.data() + wannPosBegin,
			                                                         wannPosEnd - wannPosBegin})});
		} //else if ( inZeile("drops product") )
		else if ( inZeile("destroies") ) {
			//Zeile: I 16:37:32.037678 ASP-Planer-Thread: RS1 destroies product #1 at 389.
			const auto woPosBegin   = zeile.find("Thread:")      + 8, woPosEnd   = zeile.find(' ', woPosBegin);
			const auto idPosBegin   = zeile.find('#', woPosEnd)  + 1, idPosEnd   = zeile.find(' ', idPosBegin);
			const auto wannPosBegin = zeile.find("at", idPosEnd) + 3, wannPosEnd = zeile.find('.', wannPosBegin);
			
			const auto id = stringViewToInt({zeile.data() + idPosBegin, idPosEnd - idPosBegin});
			auto& positionen(infos.Produkte[produktMap[id]].Positionen);
			positionen.emplace_back(ProduktPosition{ProduktPosition::Zerstoert,
			                                        zeile.substr(woPosBegin, woPosEnd - woPosBegin),
			                                        stringViewToInt({zeile.data() + wannPosBegin,
			                                                         wannPosEnd - wannPosBegin})});
			loescheProdukt(id);
		} //else if ( inZeile("destroies") )
		else if ( inZeile("mounted a") ) {
			//Zeile: I 16:40:11.841326 ASP-Planer-Thread: CS1 mounted a GREY cap on product #0 at 579.
			const auto woPosBegin    = zeile.find("Thread:")        + 8, woPosEnd    = zeile.find(' ', woPosBegin);
			const auto colorPosBegin = zeile.find("a", woPosEnd)    + 2, colorPosEnd = zeile.find(' ', colorPosBegin);
			const auto idPosBegin    = zeile.find('#', colorPosEnd) + 1, idPosEnd    = zeile.find(' ', idPosBegin);
			
			const auto id = stringViewToInt({zeile.data() + idPosBegin, idPosEnd - idPosBegin});
			infos.Produkte[produktMap[id]].Cap = zeile.substr(colorPosBegin, colorPosEnd - colorPosBegin);
		} //else if ( inZeile("mounted a") )
		else if ( inZeile("mounted the") ) {
			//Zeile: I 16:40:40.314128 ASP-Planer-Thread: RS1 mounted the 1. ring with color BLUE on product #1 at 613. The new fillstate is 2.
			const auto woPosBegin    = zeile.find("Thread:")           + 8, woPosEnd    = zeile.find(' ', woPosBegin);
			const auto ringPosBegin  = zeile.find("the", woPosEnd)     + 4, ringPosEnd  = zeile.find('.', ringPosBegin);
			const auto colorPosBegin = zeile.find("color", ringPosEnd) + 6, colorPosEnd = zeile.find(' ', colorPosBegin);
			const auto idPosBegin    = zeile.find('#', colorPosEnd)    + 1, idPosEnd    = zeile.find(' ', idPosBegin);
			
			const auto id   = stringViewToInt({zeile.data() + idPosBegin,   idPosEnd   - idPosBegin});
			const auto ring = stringViewToInt({zeile.data() + ringPosBegin, ringPosEnd - ringPosBegin});
			infos.Produkte[produktMap[id]].Ringe[ring - 1] = zeile.substr(colorPosBegin, colorPosEnd - colorPosBegin);
		} //else if ( inZeile("mounted the") )
		else if ( inZeile("infeasible") ) {
			//Zeile: E 16:45:48.644589 ASP-Planer-Thread: The input is infeasible! #1
			auto& solve(infos.Solves.back());
			solve.Infeasible = true;
			solve.CancelGrund = "Infeasible";
		} //else if ( inZeile("infeasible") )
		else {
			schreibeZeile = false;
		} //else
		
		if ( schreibeZeile ) {
			ausgabeLog<<zeile<<'\n';
		} //if ( schreibeZeile )
	} //while ( !datei.eof() )
	
	std::vector<Double> zeitSolving, zeitCancel;
	
	std::map<int, std::vector<Double>> zeitBisModel, zeitNachModel;
	int uebersprungen = 0, planModelle = 0;
	
	std::cout<<infos.Solves.size()<<" Solver Iterationen.\n";
	int id = 0;
	for ( const auto& solve : infos.Solves ) {
		Double solving = solve.End - solve.Start;
		Double cancel  = istNull(solve.CancelZeit) ? Zeit{} : solve.End - solve.CancelZeit;
		int modellCount = solve.Models.size();
		Double modelZeit = (istNull(solve.CancelZeit) ? solve.End : solve.CancelZeit) - solve.Start;
		
		std::cout<<"Iter #"<<std::right<<std::setw(2)<<++id<<" Solving: "<<solving<<" Wait for cancel: "<<cancel
		         <<" Models: "<<std::setw(2)<<modellCount<<" ("<<(modellCount / modelZeit)<<") Grund: "<<std::left
		         <<std::setw(29)<<solve.CancelGrund<<" Cancel level: "<<solve.CancelLevel<<'\n';
		zeitSolving.push_back(solving);
		zeitCancel.push_back(cancel);
		infos.AnzahlModelle.Sequenz.push_back(modellCount);
		++infos.CancelGrund[solve.CancelGrund];
		++infos.CancelLevel[solve.CancelLevel];
	} //for ( const auto& solve : infos.Solves )
	std::cout<<'\n';
	
	id = 0;
	for ( auto& solve : infos.Solves ) {
		++id;
		
		if ( solve.Models.empty() ) {
			continue;
		} //if ( solve.Models.empty() )
		
		std::cout<<"Iter #"<<std::right<<std::setw(2)<<id<<'\n';
		infos.AnzahlModelleOhneCancel.Sequenz.push_back(solve.Models.size());
		
		int modelId = 0;
		for ( auto& model : solve.Models ) {
			const Double gefunden = model.Gefunden - solve.Start;
			const Double seitLetztem = modelId ? gefunden - zeitBisModel[modelId].back() : 0.;
			std::cout<<"Model #"<<std::right<<std::setw(2)<<++modelId<<" Gefunden nach: "<<gefunden
			         <<" Zeit seit letztem: "<<seitLetztem<<" Benutzt: "<<std::left<<std::setw(12)<<model.Benutzt;
			zeitBisModel[modelId].push_back(gefunden);
			if ( modelId != 1 ) {
				zeitNachModel[modelId].push_back(seitLetztem);
			} //if ( modelId != 1 )
			
			switch ( model.Benutzt ) {
				case Modell::Geparst       : break;
				case Modell::Uebersprungen : ++uebersprungen; break;
				case Modell::PlanGebaut    : ++planModelle;   break;
			} //switch ( model.Benutzt )
			
			if ( model.Benutzt != Modell::Uebersprungen ) {
				int idle[3] = {0, 0, 0}, *idleP = idle, tasks = 0;
				int *maxP = model.MaxTime.PerRobot;
				std::cout<<std::right;
				
				for ( const auto& robot : robots ) {
					int now = model.PlanZeit;
					
					for ( const auto& task : model.Plan.at(robot) ) {
						if ( task.Begin > now ) {
							*idleP += task.Begin - now;
						} //if ( task.Begin > now )
						now = task.End;
						*maxP = std::max(task.End - model.PlanZeit, *maxP);
					} //for ( const auto& task : model.Plan.at(robot) )
					
					tasks += model.Plan.at(robot).size();
					std::cout<<' '<<std::setw(3)<<*idleP;
					
					model.MaxTime.Max = std::max(*maxP, model.MaxTime.Max);
					++idleP;
					++maxP;
				} //for ( const auto& robot : robots )
				
				model.Idle.Summe   = std::accumulate(std::begin(idle), std::end(idle), 0);
				model.Idle.Avg     = static_cast<double>(model.Idle.Summe) / 3.;
				model.Idle.PerTask = static_cast<double>(model.Idle.Summe) / tasks;
				
				model.MaxTime.Avg = static_cast<double>(std::accumulate(std::begin(model.MaxTime.PerRobot),
				                                                        std::end(model.MaxTime.PerRobot), 0)) / 3.;
				
				std::cout<<" | "<<std::setw(3)<<model.Idle.Summe<<' '<<std::setw(10)<<model.Idle.Avg<<' '<<std::setw(10)
				         <<model.Idle.PerTask<<" | "<<std::setw(3)<<model.MaxTime.Max<<' '<<std::setw(10)
				         <<model.MaxTime.Avg;
			} //if ( model.Benutzt != Modell::Uebersprungen )
			
			std::cout<<'\n';
		} //for ( auto& model : solve.Models )
	} //for ( auto& solve : infos.Solves )
	std::cout<<'\n';
	
	infos.GroundBase = Double{baseGrounding};
	infos.GroundTeam = Double{teamGrounding};
	
	std::copy(zeitSolving.cbegin(), zeitSolving.cend(), std::back_inserter(infos.SolveZeit.Sequenz));
	std::copy(zeitCancel.cbegin(),  zeitCancel.cend(),  std::back_inserter(infos.CancelZeit.Sequenz));
	
	for ( const auto& paar : zeitBisModel ) {
		std::copy(paar.second.begin(), paar.second.end(), std::back_inserter(infos.ZeitBisModell[paar.first].Sequenz));
		infos.ZeitBisModell[paar.first].calc();
	} //for ( const auto& paar : zeitBisModel )
	
	for ( const auto& paar : zeitNachModel ) {
		std::copy(paar.second.begin(), paar.second.end(), std::back_inserter(infos.ZeitSeitModell[paar.first].Sequenz));
		infos.ZeitSeitModell[paar.first].calc();
	} //for ( const auto& paar : zeitNachModel )
	
	infos.AnzahlModelle.calc();
	infos.AnzahlModelleOhneCancel.calc();
	infos.SolveZeit.calc();
	infos.CancelZeit.calc();
	
	std::cout<<"Mit Cancel:  Min Modelle: "<<infos.AnzahlModelle.Min<<" Max Modelle: "<<infos.AnzahlModelle.Max
	         <<" Avg: "<<infos.AnzahlModelle.Avg<<" StdAbw: "<<infos.AnzahlModelle.StdAbw<<" Quartile: "
	         <<infos.AnzahlModelle.ErstesQuartil<<' '<<infos.AnzahlModelle.ZweitesQuartil<<' '
	         <<infos.AnzahlModelle.DrittesQuartil<<'\n';
	
	std::cout<<"Ohne Cancel: Min Modelle: "<<infos.AnzahlModelleOhneCancel.Min<<" Max Modelle: "
	         <<infos.AnzahlModelleOhneCancel.Max<<" Avg: "<<infos.AnzahlModelleOhneCancel.Avg<<" StdAbw: "
	         <<infos.AnzahlModelleOhneCancel.StdAbw<<" Quartile: "<<infos.AnzahlModelleOhneCancel.ErstesQuartil<<' '
	         <<infos.AnzahlModelleOhneCancel.ZweitesQuartil<<' '<<infos.AnzahlModelleOhneCancel.DrittesQuartil<<"\n\n";
	
	const Double totalSolving = std::accumulate(zeitSolving.begin(), zeitSolving.end(), 0.);
	const Double totalCancel  = std::accumulate(zeitCancel.begin(), zeitCancel.end(), 0.);
	
	std::cout<<"Grounding base: "<<static_cast<Double>(infos.GroundBase)<<" Grounding team:  "
	         <<static_cast<Double>(infos.GroundTeam)<<" Start-up-Zeit: "<<static_cast<Double>(infos.StartUp)<<'\n';
	std::cout<<"Total: Solving: "<<std::right<<totalSolving<<" Wait for cancel: "<<totalCancel<<'\n';
	std::cout<<'\n';
	
	for ( const auto& grund : infos.CancelGrund ) {
		std::cout<<"Cancel Grund "<<std::setw(29)<<std::left<<grund.first<<' '<<std::setw(2)<<std::right<<grund.second
		         <<'\n';
	} //for ( const auto& grund : infos.CancelGrund  )
	std::cout<<'\n';
	
	for ( const auto& level : infos.CancelLevel ) {
		std::cout<<"Cancel Level "<<std::setw(11)<<std::left<<level.first<<' '<<std::setw(2)<<std::right<<level.second
		         <<'\n';
	} //for ( const auto& level : infos.CancelLevel )
	std::cout<<'\n';
	
	const auto& anzahlModelle = infos.AnzahlModelle.Summe;
	const auto inkonsistent   = anzahlModelle - uebersprungen - planModelle;
	std::cout<<anzahlModelle<<" Modelle gefunden:\n"
	         <<std::setw(3)<<uebersprungen<<" ("
	         <<(static_cast<double>(uebersprungen) * 100. / anzahlModelle)<<"%) nicht geparst\n"
	         <<std::setw(3)<<inkonsistent<<" ("
	         <<(static_cast<double>(inkonsistent) * 100. / anzahlModelle)<<"%) widersprachen der Realität\n"
	         <<std::setw(3)<<planModelle<<" ("
	         <<(static_cast<double>(planModelle) * 100. / anzahlModelle)<<"%) zu Plänen gemacht\n";
	std::cout.flush();
	
	ausgabeInfo<<infos;
	return 0;
}
