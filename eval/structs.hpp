#ifndef STRUCTS_HPP
#define STRUCTS_HPP

#include "../common/common-qt.hpp"

#include <map>
#include <unordered_map>

#include <QPixmap>

enum class TaskTyp : int {
	Idle = 0,
	PrepareCS,
	MountCap,
	MountRing,
	MountRing1,
	MountRing2,
	MountRing3,
	FillRS,
	Deliver,
	LateDeliver,
	GetBase,
	GetProduct,
	Goto,
	UselessGoto
};

namespace std {
template<>
struct hash<TaskTyp> {
	auto operator()(const TaskTyp typ) const noexcept {
		return hash<int>{}(static_cast<int>(typ));
	}
};
}

inline TaskTyp stringZuTyp(const std::string& task) {
	static const auto map = [](void) {
			std::unordered_map<std::string, TaskTyp> ret;
			ret.reserve(9);
			ret.insert({"prepareCS",   TaskTyp::PrepareCS});
			ret.insert({"mountCap",    TaskTyp::MountCap});
			ret.insert({"mountRing",   TaskTyp::MountRing});
			ret.insert({"feedRS",      TaskTyp::FillRS});
			ret.insert({"deliver",     TaskTyp::Deliver});
			ret.insert({"lateDeliver", TaskTyp::LateDeliver});
			ret.insert({"getBase",     TaskTyp::GetBase});
			ret.insert({"getProduct",  TaskTyp::GetProduct});
			ret.insert({"goto",        TaskTyp::Goto});
			return ret;
		}();
	
	const auto nameEnde = task.find('(');
	const auto iter = map.find(task.substr(0, nameEnde));
	
	return iter == map.end() ? TaskTyp::Idle : iter->second;
}

inline int bewertungEins(const TaskTyp typ) noexcept {
	switch ( typ ) {
//		case TaskTyp::PrepareCS   : return  3;
		case TaskTyp::MountCap    : return 10;
		case TaskTyp::MountRing1  : return 10;
		case TaskTyp::MountRing2  : return 20;
		case TaskTyp::MountRing3  : return 30;
		case TaskTyp::FillRS      : return  2;
		case TaskTyp::Deliver     : return 20;
		case TaskTyp::LateDeliver : return  5;
		default : break;
	}
	return 0;
}

inline auto erzeugeBewertungen(void) {
	std::vector<decltype(&bewertungEins)> ret;
	ret.push_back(&bewertungEins);
	return ret;
}

struct AnnotatedInfos : public Infos {
	int Spiel = 0;
	int Punkte = 0;
	int PlanerPunkteInGame = 0;
	int PlanerPunkteComplete = 0;
	bool HatFailedTask = false;
	bool IstOutlier = false;
	QPixmap Memory;
	int Idle = 0;
	
	std::map<TaskTyp, int> TasksInGame, TasksNachGame;
	std::map<TaskTyp, int> TaskZeitInGame, TaskZeitNachGame;
	
	void berechneIstOutlier(void) {
		if ( Punkte <= 4 ) {
			IstOutlier = true;
			return;
		} //if ( Punkte <= 4 )
		
		constexpr auto spielEnde = 900 + 240;
		for ( const auto& paar : AusgefuehrterPlan ) {
			auto iter = paar.second.begin();
			const auto begin = iter;
			const auto end = std::find_if(iter, paar.second.end(),
				[](const auto& task) { return task.End >= spielEnde; });
			for ( ; iter != end; ++iter ) {
				if ( iter->Failed ) {
					const auto iter2 = std::find_if(iter, end,
						[iter](const auto& task) { return !task.Failed && stringZuTyp(task.Name) == stringZuTyp(iter->Name); });
					if ( iter2 == end ) {
						IstOutlier = true;
						return;
					} //if ( iter2 == end )
				} //if ( iter->Failed )
				else if ( iter != begin && iter->End - iter->Begin >= 210 ) {
					IstOutlier = true;
					return;
				} //else if ( iter != begin && iter->End - iter->Begin >= 210 )
			} //for ( ; iter != end; ++iter )
		} //for ( const auto& paar : AusgefuehrterPlan )
		return;
	}
	
	void berechnePunkte(const int bewertung) {
		static const auto bewertungen(erzeugeBewertungen());
		const auto bewertungsFunktion(bewertungen[bewertung]);
		
		const auto berechne = [bewertungsFunktion](const auto& container) {
			return std::accumulate(container.begin(), container.end(), 0,
				[bewertungsFunktion](const int punkte, const auto& paar) {
					return punkte + bewertungsFunktion(paar.first) * paar.second;
				});
			};
		
		PlanerPunkteInGame = berechne(TasksInGame);
		PlanerPunkteComplete = PlanerPunkteInGame + berechne(TasksNachGame);
		return;
	}
	
	void zaehleTasks(void) {
		TasksInGame.clear();
		TasksNachGame.clear();
		
		TaskZeitInGame.clear();
		TaskZeitNachGame.clear();
		
		Idle = 0;
		
		constexpr const auto start = 240, ende = 900 + start;
		
		for ( const auto& paar : AusgefuehrterPlan ) {
			auto *tasks = &TasksInGame;
			auto *taskZeit = &TaskZeitInGame;
			
			auto now = start;
			
			const auto end = paar.second.end();
			for ( auto iter = paar.second.begin(); iter != end; ++iter ) {
				const auto& task(*iter);
				
				if ( task.Failed ) {
					HatFailedTask = true;
				} //if ( task.Failed )
				
				if ( task.Begin >= ende ) {
					tasks = &TasksNachGame;
					taskZeit = &TaskZeitNachGame;
				} //if ( task.Begin >= ende )
				
				auto typ = stringZuTyp(task.Name);
				
				switch ( typ ) {
					case TaskTyp::MountRing : {
						//mountRing(m("C","RS1","I"),5,1,1),
						static_assert('1' - '0' == 1, "");
						const int ring = task.Name[task.Name.size() - 3] - '0';
						switch ( ring ) {
							case 1 : typ = TaskTyp::MountRing1; break;
							case 2 : typ = TaskTyp::MountRing2; break;
							case 3 : typ = TaskTyp::MountRing3; break;
							default : Q_UNREACHABLE(); break;
						} //switch ( ring )
						break;
					} //case TaskTyp::MountRing
					case TaskTyp::Goto : {
						const auto next = iter + 1;
						if ( next == end ) {
							typ = TaskTyp::UselessGoto;
							break;
						} //if ( next == end )
						
						const auto& nextTask(*next);
						if ( stringZuTyp(nextTask.Name) == TaskTyp::Goto || task.location() != nextTask.location() ) {
							typ = TaskTyp::UselessGoto;
							break;
						} //if ( stringZuTyp(nextTask.Name) == TaskTyp::Goto || task.location() != nextTask.location() )
						//fall-trough
					} //case TaskTyp::Goto
					default : break;
				} //switch ( typ )
				
				auto zeit = task.End - task.Begin;
				if ( task.End > ende && taskZeit != &TaskZeitNachGame ) {
					const auto zeitNachEnde = task.End - ende;
					zeit -= zeitNachEnde;
					TaskZeitNachGame[typ] += zeitNachEnde;
				} //if ( task.End > ende && taskZeit != &TaskZeitNachGame )
				
				++(*tasks)[typ];
				(*taskZeit)[typ] += zeit;
				
				if ( task.Begin <= ende ) {
					// != anstelle von > weil bei den ersten paar alten Spielen noch nicht alle Zeiten von der Exec übernommen wurden
					// Dies führt zu ein paar Sekunden "idle" zwischen den Tasks, aber z.T. auch "negative idle", dies sollte das ausgleichen
					if ( task.Begin != now ) {
						Idle += task.Begin - now;
					} //if ( task.Begin != now )
					now = task.End;
				} //if ( task.Begin <= ende )
			} //for ( auto iter = paar.second.begin(); iter != end; ++iter )
			
			if ( now < ende ) {
				Idle += ende - now;
			} //if ( now < ende )
		} //for ( const auto& paar : AusgefuehrterPlan )
		return;
	}
};

#endif
