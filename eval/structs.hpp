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
	QPixmap Memory;
	
	std::map<TaskTyp, int> TasksInGame, TasksNachGame;
	std::map<TaskTyp, int> TaskZeitInGame, TaskZeitNachGame;
	
	bool istOutlier(void) const {
		return HatFailedTask;
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
		
		constexpr auto ende = 900 + 240;
		
		for ( const auto& paar : AusgefuehrterPlan ) {
			auto *tasks = &TasksInGame;
			auto *taskZeit = &TaskZeitInGame;
			
			const auto end = paar.second.end();
			for ( auto iter = paar.second.begin(); iter != end; ++iter ) {
				const auto& task(*iter);
				
				if ( task.Failed ) {
					HatFailedTask = true;
				} //if ( task.Failed )
				
				if ( task.End >= ende ) {
					tasks = &TasksNachGame;
					taskZeit = &TaskZeitNachGame;
				} //if ( task.End >= ende )
				
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
			} //for ( auto iter = paar.second.begin(); iter != end; ++iter )
		} //for ( const auto& paar : AusgefuehrterPlan )
		return;
	}
};

#endif
