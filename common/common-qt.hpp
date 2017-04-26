#ifndef COMMONQT_HPP
#define COMMONQT_HPP

#include "common.hpp"

#include <algorithm>
#include <cmath>
#include <string>
#include <unordered_map>
#include <vector>

#include <QDataStream>

static inline QDataStream& operator<<(QDataStream& out, const Zeit& zeit) noexcept {
	out<<std::get<0>(zeit)<<std::get<1>(zeit)<<std::get<2>(zeit)<<std::get<3>(zeit);
	return out;
}

static inline QDataStream& operator>>(QDataStream& in, Zeit& zeit) noexcept {
	in>>std::get<0>(zeit)>>std::get<1>(zeit)>>std::get<2>(zeit)>>std::get<3>(zeit);
	return in;
}

static inline QDataStream& operator<<(QDataStream& out, const std::string& string) noexcept {
	out<<string.c_str();
	return out;
}

static inline QDataStream& operator>>(QDataStream& in, std::string& string) {
	char *str;
	in>>str;
	string = str;
	delete[] str;
	return in;
}

template<typename T>
static inline QDataStream& operator<<(QDataStream& out, const std::vector<T>& vec) noexcept {
	out<<static_cast<qint32>(vec.size());
	for ( const auto& t : vec ) {
		out<<t;
	} //for ( const auto& t : vec )
	return out;
}

template<typename T>
static inline QDataStream& operator>>(QDataStream& in, std::vector<T>& vec) {
	qint32 size;
	in>>size;
	
	vec.reserve(size);
	
	T temp;
	for ( ; size > 0; --size ) {
		in>>temp;
		vec.emplace_back(std::move(temp));
	} //for ( ; size > 0; --size )
	return in;
}

template<typename Key, typename T>
static inline QDataStream& operator<<(QDataStream& out, const std::unordered_map<Key, T>& map) noexcept {
	out<<static_cast<qint32>(map.size());
	for ( const auto& t : map ) {
		out<<t;
	} //for ( const auto& t : map )
	return out;
}

template<typename Key, typename T>
static inline QDataStream& operator>>(QDataStream& in, std::unordered_map<Key, T>& map) {
	qint32 size;
	in>>size;
	
	map.reserve(size);
	
	std::pair<Key, T> temp;
	for ( ; size > 0; --size ) {
		in>>temp;
		map.emplace(std::move(temp));
	} //for ( ; size > 0; --size )
	return in;
}

template<typename Key, typename T>
static inline QDataStream& operator<<(QDataStream& out, const std::map<Key, T>& map) noexcept {
	out<<static_cast<qint32>(map.size());
	for ( const auto& t : map ) {
		out<<t;
	} //for ( const auto& t : map )
	return out;
}

template<typename Key, typename T>
static inline QDataStream& operator>>(QDataStream& in, std::map<Key, T>& map) {
	qint32 size;
	in>>size;
	
	std::pair<Key, T> temp;
	for ( ; size > 0; --size ) {
		in>>temp;
		map.emplace(std::move(temp));
	} //for ( ; size > 0; --size )
	return in;
}

template<typename T1, typename T2>
static inline QDataStream& operator<<(QDataStream& out, const std::pair<T1, T2>& paar) noexcept {
	out<<paar.first<<paar.second;
	return out;
}

template<typename T1, typename T2>
static inline QDataStream& operator>>(QDataStream& in, std::pair<T1, T2>& paar) {
	in>>paar.first>>paar.second;
	return in;
}

struct Task {
	std::string Name;
	int Begin = 0;
	int End = 0;
	bool Failed = false;
	
	std::string location(void) const {
		const auto begin = Name.find("m("), end = Name.find(')', begin) + 1;
		return Name.substr(begin, end - begin);
	}
};

static inline QDataStream& operator<<(QDataStream& out, const Task& task) noexcept {
	out<<task.Name<<task.Begin<<task.End<<task.Failed;
	return out;
}

static inline QDataStream& operator>>(QDataStream& in, Task& task) {
	in>>task.Name>>task.Begin>>task.End>>task.Failed;
	return in;
}

using TaskList = std::vector<Task>;

struct PlanIdle {
	int Summe = 0;
	double Avg = 0.;
	double PerTask = 0.;
};

static inline QDataStream& operator<<(QDataStream& out, const PlanIdle& idle) noexcept {
	out<<idle.Summe<<idle.Avg<<idle.PerTask;
	return out;
}

static inline QDataStream& operator>>(QDataStream& in, PlanIdle& idle) noexcept {
	in>>idle.Summe>>idle.Avg>>idle.PerTask;
	return in;
}

struct Max {
	int PerRobot[3] = {0,0,0};
	int Max = 0;
	double Avg = 0.;
};

static inline QDataStream& operator<<(QDataStream& out, const Max& max) noexcept {
	out<<max.PerRobot[0]<<max.PerRobot[1]<<max.PerRobot[2]<<max.Max<<max.Avg;
	return out;
}

static inline QDataStream& operator>>(QDataStream& in, Max& max) noexcept {
	in>>max.PerRobot[0]>>max.PerRobot[1]>>max.PerRobot[2]>>max.Max>>max.Avg;
	return in;
}

struct Modell {
	using PlanT = std::unordered_map<std::string, TaskList>;
	
	Zeit Gefunden;
	enum {
		Uebersprungen,
		Geparst,
		PlanGebaut
	} Benutzt = Uebersprungen;
	int PlanZeit;
	PlanT Plan;
	PlanIdle Idle;
	Max MaxTime;
	
	Modell(void) = default;
	
	Modell(Zeit&& gefunden, const PlanT& plan) noexcept : Gefunden(std::move(gefunden)), Plan(plan) {
		return;
	}
};

static inline QDataStream& operator<<(QDataStream& out, const Modell& modell) noexcept {
	out<<modell.Gefunden<<modell.PlanZeit<<modell.Plan<<modell.Idle<<modell.MaxTime;
	return out;
}

static inline QDataStream& operator>>(QDataStream& in, Modell& modell) {
	in>>modell.Gefunden>>modell.PlanZeit>>modell.Plan>>modell.Idle>>modell.MaxTime;
	return in;
}

struct Solve {
	Zeit Start;
	Zeit End;
	std::vector<Modell> Models;
	Zeit CancelZeit;
	std::string CancelLevel = "Not";
	std::string CancelGrund = "None";
	bool Infeasible = false;
	
	Solve(void) = default;
	
	Solve(Zeit&& start) noexcept : Start(std::move(start)) {
		return;
	}
};

static inline QDataStream& operator<<(QDataStream& out, const Solve& solve) noexcept {
	out<<solve.Start<<solve.End<<solve.Models<<solve.CancelZeit<<solve.CancelLevel<<solve.CancelGrund<<solve.Infeasible;
	return out;
}

static inline QDataStream& operator>>(QDataStream& in, Solve& solve) {
	in>>solve.Start>>solve.End>>solve.Models>>solve.CancelZeit>>solve.CancelLevel>>solve.CancelGrund>>solve.Infeasible;
	return in;
}

struct ProduktPosition {
	enum WasT : int {
		Generiert = 0,
		Aufgehoben,
		Abgelegt,
		Bearbeitet,
		Fertig,
		Zerstoert
	} Was;
	std::string Wo;
	int Wann;
};

static inline QDataStream& operator<<(QDataStream& out, const ProduktPosition::WasT& was) noexcept {
	out<<static_cast<int>(was);
	return out;
}

static inline QDataStream& operator>>(QDataStream& in, ProduktPosition::WasT& was) {
	int temp;
	in>>temp;
	was = static_cast<ProduktPosition::WasT>(temp);
	return in;
}

static inline QDataStream& operator<<(QDataStream& out, const ProduktPosition& position) noexcept {
	out<<position.Was<<position.Wo<<position.Wann;
	return out;
}

static inline QDataStream& operator>>(QDataStream& in, ProduktPosition& position) {
	in>>position.Was>>position.Wo>>position.Wann;
	return in;
}

struct Produkt {
	std::string Base;
	std::string Ringe[3];
	std::string Cap;
	std::vector<ProduktPosition> Positionen;
	
	Produkt(void) = default;
	Produkt(std::string&& base, std::string&& wo, const int wann) : Base(std::move(base)) {
		Positionen.emplace_back(ProduktPosition{ProduktPosition::Generiert, std::move(wo), wann});
		return;
	}
};

static inline QDataStream& operator<<(QDataStream& out, const Produkt& produkt) noexcept {
	out<<produkt.Base<<produkt.Ringe[0]<<produkt.Ringe[1]<<produkt.Ringe[2]<<produkt.Cap<<produkt.Positionen;
	return out;
}

static inline QDataStream& operator>>(QDataStream& in, Produkt& produkt) {
	in>>produkt.Base>>produkt.Ringe[0]>>produkt.Ringe[1]>>produkt.Ringe[2]>>produkt.Cap>>produkt.Positionen;
	return in;
}

template<typename T>
struct AvgSequenz {
	std::vector<T> Sequenz;
	T Summe;
	T Min;
	T Max;
	double Avg;
	double StdAbw;
	double ErstesQuartil;
	double ZweitesQuartil;
	double DrittesQuartil;
	
	void calc(void) noexcept {
		if ( Sequenz.empty() ) {
			return;
		} //if ( Sequenz.empty() )
		
		const auto square = std::accumulate(Sequenz.cbegin(), Sequenz.cend(), T{0}, [](auto wert, const auto& iter) noexcept { return wert += iter * iter; });
		const auto avgSquare = static_cast<double>(square) / Sequenz.size();
		
		Summe  = std::accumulate(Sequenz.cbegin(), Sequenz.cend(), T{0});
		Min    = std::accumulate(Sequenz.cbegin(), Sequenz.cend(), std::numeric_limits<T>::max(), static_cast<const T&(*)(const T&, const T&)>(std::min));
		Max    = std::accumulate(Sequenz.cbegin(), Sequenz.cend(), std::numeric_limits<T>::min(), static_cast<const T&(*)(const T&, const T&)>(std::max));
		Avg    = static_cast<double>(Summe) / Sequenz.size();
		StdAbw = std::sqrt(avgSquare - Avg * Avg);
		
		std::sort(Sequenz.begin(), Sequenz.end());
		
		auto berechneQuantil = [this](const int quantil) noexcept {
				const auto produkt = quantil * Sequenz.size();
				if ( produkt % 100 == 0 ) {
					const auto index2 = produkt / 100, index1 = index2 - 1;
					return (Sequenz[index1] + Sequenz[index2]) / 2.;
				} //if ( produkt % 100 == 0 )
				const auto index = produkt / 100;
				return static_cast<double>(Sequenz[index]);
			};
		
		ErstesQuartil  = berechneQuantil(25);
		ZweitesQuartil = berechneQuantil(50);
		DrittesQuartil = berechneQuantil(75);
		return;
	}
};

template<typename T>
static inline QDataStream& operator<<(QDataStream& out, const AvgSequenz<T>& sequenz) noexcept {
	out<<sequenz.Sequenz<<sequenz.Summe<<sequenz.Min<<sequenz.Max<<sequenz.Avg<<sequenz.StdAbw<<sequenz.ErstesQuartil
	   <<sequenz.ZweitesQuartil<<sequenz.DrittesQuartil;
	return out;
}

template<typename T>
static inline QDataStream& operator>>(QDataStream& in, AvgSequenz<T>& sequenz) {
	in>>sequenz.Sequenz>>sequenz.Summe>>sequenz.Min>>sequenz.Max>>sequenz.Avg>>sequenz.StdAbw>>sequenz.ErstesQuartil
	  >>sequenz.ZweitesQuartil>>sequenz.DrittesQuartil;
	return in;
}

struct Infos {
	double GroundBase = 0.;
	double GroundTeam = 0.;
	double StartUp = 0.;
	std::vector<Solve> Solves;
	std::unordered_map<std::string, int> CancelLevel;
	std::unordered_map<std::string, int> CancelGrund;
	std::unordered_map<std::string, TaskList> AusgefuehrterPlan;
	std::vector<Produkt> Produkte;
	
	AvgSequenz<int> AnzahlModelle;
	AvgSequenz<int> AnzahlModelleOhneCancel;
	AvgSequenz<double> SolveZeit;
	AvgSequenz<double> CancelZeit;
	
	std::map<int, AvgSequenz<double>> ZeitBisModell;
	std::map<int, AvgSequenz<double>> ZeitSeitModell;
};

static inline QDataStream& operator<<(QDataStream& out, const Infos& infos) noexcept {
	out<<infos.GroundBase<<infos.GroundTeam<<infos.StartUp<<infos.Solves<<infos.CancelLevel<<infos.CancelGrund
	   <<infos.AusgefuehrterPlan<<infos.Produkte<<infos.AnzahlModelle<<infos.AnzahlModelleOhneCancel<<infos.SolveZeit
	   <<infos.CancelZeit<<infos.ZeitBisModell<<infos.ZeitSeitModell;
	return out;
}

static inline QDataStream& operator>>(QDataStream& in, Infos& infos) {
	in>>infos.GroundBase>>infos.GroundTeam>>infos.StartUp>>infos.Solves>>infos.CancelLevel>>infos.CancelGrund
	  >>infos.AusgefuehrterPlan>>infos.Produkte>>infos.AnzahlModelle>>infos.AnzahlModelleOhneCancel>>infos.SolveZeit
	  >>infos.CancelZeit>>infos.ZeitBisModell>>infos.ZeitSeitModell;
	return in;
}

#endif
