#ifndef ENCODINGFENSTER_HPP
#define ENCODINGFENSTER_HPP

#include "structs.hpp"

#include <functional>
#include <vector>

#include <QWidget>

class DatenWidget;

class QLineEdit;
class QScrollArea;
class QSpinBox;

class QCustomPlot;

class EncodingFenster : public QWidget {
	Q_OBJECT
	private:
	QLineEdit *Pfad;
	QScrollArea *ScrollWidget;
	QSpinBox *WertSpin;
	
	std::vector<AnnotatedInfos> Daten, GegnerDaten;
	std::vector<DatenWidget*> Widgets;
	
	AvgSequenz<double> Punkte, GegnerPunkte;
	AvgSequenz<double> PlanerPunkte;
	AvgSequenz<double> PlanerPunkteNachSpiel;
	
	std::pair<AvgSequenz<double>, QVector<double>> OutlierPunkte, GegnerOutlierPunkte;
	std::pair<AvgSequenz<double>, QVector<double>> OutlierPlanerPunkte;
	std::pair<AvgSequenz<double>, QVector<double>> OutlierPlanerPunkteNachSpiel;
	
	AvgSequenz<double> Idle, GegnerIdle;
	std::pair<AvgSequenz<double>, QVector<double>> OutlierIdle, GegnerOutlierIdle;
	
	AvgSequenz<double> StartUp;
	
	void leseDaten(const QString& pfad);
	void clear(void);
	
	void loescheDaten(DatenWidget *widget);
	void zeigeLog(DatenWidget *widget);
	
	bool springeZuDaten(std::function<bool(const AnnotatedInfos&)> pred);
	
	void findeMin(void);
	void findeAvg(void);
	void findeMax(void);
	void findeWert(void);
	
	void zeigeLetzteDaten(void);
	
	public:
	explicit EncodingFenster(const bool frei, QWidget *parent = nullptr);
	
	void setzePfad(const QString& pfad);
	
	const AvgSequenz<double>& punkte(void) const;
	const AvgSequenz<double>& gegnerPunkte(void) const;
	const AvgSequenz<double>& planerPunkte(void) const;
	const AvgSequenz<double>& planerPunkteNachSpiel(void) const;
	
	const std::pair<AvgSequenz<double>, QVector<double>>& outlierPunkte(void) const;
	const std::pair<AvgSequenz<double>, QVector<double>>& gegnerOutlierPunkte(void) const;
	const std::pair<AvgSequenz<double>, QVector<double>>& outlierPlanerPunkte(void) const;
	const std::pair<AvgSequenz<double>, QVector<double>>& outlierPlanerPunkteNachSpiel(void) const;
	
	const AvgSequenz<double>& idle(void) const;
	const AvgSequenz<double>& gegnerIdle(void) const;
	const std::pair<AvgSequenz<double>, QVector<double>>& outlierIdle(void) const;
	const std::pair<AvgSequenz<double>, QVector<double>>& gegnerOutlierIdle(void) const;
	
	const AvgSequenz<double>& startUp(void) const;
	
	signals:
	void graphDoppelklick(QCustomPlot *graph);
};

#endif
