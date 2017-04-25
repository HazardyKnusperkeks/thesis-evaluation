#ifndef ENCODINGFENSTER_HPP
#define ENCODINGFENSTER_HPP

#include "structs.hpp"

#include <vector>

#include <QWidget>

class DatenWidget;

class QLineEdit;
class QScrollArea;

class EncodingFenster : public QWidget {
	Q_OBJECT
	private:
	QLineEdit *Pfad;
	QScrollArea *ScrollWidget;
	
	std::vector<AnnotatedInfos> Daten;
	std::vector<DatenWidget*> Widgets;
	
	AvgSequenz<double> Punkte;
	AvgSequenz<double> PlanerPunkte;
	AvgSequenz<double> PlanerPunkteNachSpiel;
	
	std::pair<AvgSequenz<double>, QVector<double>> OutlierPunkte;
	std::pair<AvgSequenz<double>, QVector<double>> OutlierPlanerPunkte;
	std::pair<AvgSequenz<double>, QVector<double>> OutlierPlanerPunkteNachSpiel;
	
	AvgSequenz<double> Idle;
	std::pair<AvgSequenz<double>, QVector<double>> OutlierIdle;
	
	AvgSequenz<double> StartUp;
	
	void leseDaten(const QString& pfad);
	void clear(void);
	
	void loescheDaten(DatenWidget *widget);
	void zeigeLog(DatenWidget *widget);
	
	public:
	explicit EncodingFenster(const bool frei, QWidget *parent = nullptr);
	
	void setzePfad(const QString& pfad);
	
	const AvgSequenz<double>& punkte(void) const;
	const AvgSequenz<double>& planerPunkte(void) const;
	const AvgSequenz<double>& planerPunkteNachSpiel(void) const;
	
	const std::pair<AvgSequenz<double>, QVector<double>>& outlierPunkte(void) const;
	const std::pair<AvgSequenz<double>, QVector<double>>& outlierPlanerPunkte(void) const;
	const std::pair<AvgSequenz<double>, QVector<double>>& outlierPlanerPunkteNachSpiel(void) const;
	
	const AvgSequenz<double>& idle(void) const;
	const std::pair<AvgSequenz<double>, QVector<double>>& outlierIdle(void) const;
	
	const AvgSequenz<double>& startUp(void) const;
};

#endif
