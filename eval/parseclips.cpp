#include "parseclips.hpp"

#include <QIODevice>

#include "../common/common-qt.hpp"
#include "structs.hpp"

#include <cmath>
#include <unordered_map>

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
	
	Q_ASSERT(std >= 0);
	Q_ASSERT(min >= 0 && min < 60);
	Q_ASSERT(sek >= 0 && sek < 60);
	Q_ASSERT(mic >= 0 && mic < 1'000'000);
	return lhs;
}

static inline Zeit operator-(Zeit lhs, const Zeit& rhs) noexcept {
	return lhs -= rhs;
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

namespace std {
template<>
struct hash<QByteArray> {
	int operator()(const QByteArray& array) const {
		return qHash(array);
	}
};
}

struct ClipsTask {
	TaskTyp Typ;
	QByteArray Machine;
	bool Input = false;
};

static std::string machineName(const ClipsTask& task) {
	return std::string("m(\"M\",\"") + task.Machine.data() + "\",\"" + (task.Input ? "I" : "O") + "\")";
}

static std::string taskName(ClipsTask& task) {
	std::string ret;
	switch ( task.Typ ) {
		case TaskTyp::PrepareCS : {
			task.Input = true;
			ret = "prepareCS(" + machineName(task) + ")";
			break;
		} //case TaskTyp::PrepareCS
		case TaskTyp::GetProduct : {
			task.Input = false;
			ret = "getProduct(" + machineName(task) + ")";
			break;
		} //case TaskTyp::GetProduct
		case TaskTyp::FillRS : {
			task.Input = true;
			ret = "feedRS(" + machineName(task) + ")";
			break;
		} //case TaskTyp::FillRS
		case TaskTyp::Deliver : {
			task.Input = true;
			ret = "deliver(" + machineName(task) + ",1,1)";
			break;
		} //case TaskTyp::Deliver
		case TaskTyp::MountCap : {
			task.Input = true;
			ret = "mountCap(" + machineName(task) + "1,1)";
			break;
		} //case TaskTyp::MountCap
		case TaskTyp::MountRing  :
		case TaskTyp::MountRing1 :
		case TaskTyp::MountRing2 :
		case TaskTyp::MountRing3 : {
			task.Input = true;
			ret = "mountRing(" + machineName(task) + "1,1,1)";
			break;
		} //case TaskTyp::MountRing(1|2|3)?
		case TaskTyp::GetBase : {
			task.Input = true;
			ret = "getBase(" + machineName(task) + ",\"RED\")";
			break;
		} //case TaskTyp::GetBase
		default : Q_UNREACHABLE(); break;
	} //switch ( task.Typ )
	return ret;
}

int parseClips(TaskList& tasks, QIODevice& eingabe) {
	int idle = 0;
	
	const std::unordered_map<QByteArray, TaskTyp> typMap = {{"fill-cap",            TaskTyp::PrepareCS},
	                                                        {"clear-bs",            TaskTyp::GetProduct},
	                                                        {"clear-rs",            TaskTyp::GetProduct},
	                                                        {"clear-cs",            TaskTyp::GetProduct},
	                                                        {"fill-rs",             TaskTyp::FillRS},
	                                                        {"deliver",             TaskTyp::Deliver},
	                                                        {"produce-c0",          TaskTyp::MountCap},
	                                                        {"produce-cx",          TaskTyp::MountCap},
	                                                        {"add-first-ring",      TaskTyp::MountRing1},
	                                                        {"add-additional-ring", TaskTyp::MountRing2},
	                                                        {"discard-unknown",     TaskTyp::Idle},
	                                                       };
	constexpr const int offset = 240;
	
	std::unordered_map<int, ClipsTask> taskMap;
	std::unordered_map<int, int> stepToTaskMap;
	
	int aktuelleZeit = 0, letzteZeit = 0;
	int letzterTask = 0;
	
	enum {
		Idle,
		Running,
		NonTask
	} state = Idle;
	
	while ( !eingabe.atEnd() ) {
		QByteArray zeile = eingabe.readLine();
		
		auto inZeile = [&zeile](std::initializer_list<const char*> args) {
				for ( auto&& arg : args ) {
					if ( !zeile.contains(arg) ) {
						return false;
					} //if ( !zeile.contains(arg) )
				}
				return true;
			};
		
		auto intAusZeile = [&zeile](const int begin, const int end) {
				const auto zb = zeile.begin();
				return stringViewToInt({zb + begin, static_cast<std::size_t>(end - begin)});
			};
		
		if ( inZeile({"<==", "(phase PRODUCTION)"}) ) {
			break;
		} //if ( inZeile({"<==", "(phase PRODUCTION)"})
		else if ( inZeile({"==>", "(game-time"}) ) {
			const auto zeitBegin = zeile.indexOf("time") + 5, zeitEnde = zeile.indexOf(' ', zeitBegin);
			aktuelleZeit = stringViewToInt({zeile.begin() + zeitBegin, static_cast<std::size_t>(zeitEnde - zeitBegin)});
		} //else if ( inZeile({"==>", "(game-time"}) )
		else if ( inZeile({"==>", "(task (id"}) ) {
			//(task (id 744257641) (name fill-cap) (state proposed) (priority 50) (current-step 0) (steps 744257642 744257643))
			const auto idBegin = zeile.indexOf("(id") + 4, idEnd = zeile.indexOf(')', idBegin);
			const int id = intAusZeile(idBegin, idEnd);
			
			if ( !taskMap.count(id) ) {
				const auto nameBegin = zeile.indexOf("(name", idEnd) + 6, nameEnd = zeile.indexOf(')', nameBegin);
				const auto stepsBegin = zeile.indexOf("(steps", nameEnd) + 7, stepsEnd = zeile.indexOf(')', stepsBegin);
				
				taskMap[id].Typ = typMap.at(zeile.mid(nameBegin, nameEnd - nameBegin));
				for ( const auto& step : zeile.mid(stepsBegin, stepsEnd - stepsBegin).split(' ') ) {
					stepToTaskMap.insert({step.toInt(), id});
				} //for ( const auto& step : zeile.mid(stepsBegin, stepsEnd - stepsBegin).split(' ') )
				
				letzterTask = id;
			} //if ( !taskMap.count(id) )
		} //else if ( inZeile({"==>", "(task (id"}) )
		else if ( inZeile({"==>", "(step (id"}) ) {
			//(step (id 744257642) (name get-from-shelf) (state inactive) (task-priority 50) (machine M-CS2) (zone nil) (product-type nil) (machine-feature SHELF) (base BLACK) (ring BLUE) (cs-operation MOUNT_CAP) (gate 1) (product-id 0) (already-at-mps FALSE) (side OUTPUT) (lock NONE))
			const auto idBegin = zeile.indexOf("(id") + 4, idEnd = zeile.indexOf(')', idBegin);
			const auto machineBegin = zeile.indexOf("(machine", idEnd) + 11, machineEnd = zeile.indexOf(')', machineBegin);
			
			const int id = intAusZeile(idBegin, idEnd);
			
			taskMap[stepToTaskMap[id]].Machine = zeile.mid(machineBegin, machineEnd - machineBegin);
		} //else if ( inZeile({"==>", "(step (id"}) )
		else if ( inZeile({"==>", "(state "}) && zeile.count('(') == 2 ) {
			const auto begin = zeile.lastIndexOf(' ') + 1, end = zeile.size() - 2;
			const auto neuerState = zeile.mid(begin, end - begin);
			
			if ( neuerState == "TASK-ORDERED" ) {
				Q_ASSERT(state == Idle);
				state = Running;
				idle += aktuelleZeit - letzteZeit;
				
				auto& task = taskMap[letzterTask];
				if ( task.Typ != TaskTyp::Idle ) {
					tasks.emplace_back(Task{taskName(task), aktuelleZeit + offset, 0, false});
					letzteZeit = aktuelleZeit;
				} //if ( task.Typ != TaskTyp::Idle )
				else {
					state = NonTask;
				} //else -> if ( task.Typ != TaskTyp::Idle )
			} //if ( neuerState == "TASK-ORDERED" )
			else if ( neuerState == "TASK-FINISHED" ) {
				switch ( state ) {
					case Idle : Q_UNREACHABLE(); break;
					case Running : {
						state = Idle;
						tasks.back().End = aktuelleZeit + offset;
						letzteZeit = aktuelleZeit;
						break;
					} //case Running;
					case NonTask : state = Idle; break;
				} //switch ( state )
			} //else if ( neuerState == "TASK-FINISHED" )
			else if ( neuerState == "TASK-FAILED" ) {
				switch ( state ) {
					case Idle : Q_UNREACHABLE(); break;
					case Running : {
						state = Idle;
						auto& task = tasks.back();
						task.End = aktuelleZeit + offset;
						task.Failed = true;
						
						letzteZeit = aktuelleZeit;
						break;
					} //case Running;
					case NonTask : state = Idle; break;
				} //switch ( state )
			} //else if ( neuerState == "TASK-FINISHED" )
		} //else if ( inZeile({"==>", "(state "}) && zeile.count('(') == 2 )
	} //while ( !eingabe.atEnd() )
	
	if ( state == Idle ) {
		idle += 900 - letzteZeit;
	} //if ( state == Idle )
	return idle;
}
