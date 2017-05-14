#ifndef DATENWIDGET_HPP
#define DATENWIDGET_HPP

#include <QFrame>

struct AnnotatedInfos;

class QLabel;

class QCustomPlot;

class DatenWidget : public QFrame {
	Q_OBJECT
	private:
	QLabel *Punkte;
	QLabel *PlanerPunkte;
	QLabel *PlanerPunkteComplete;
	
	bool checkFehler(void) const noexcept;
	
	public:
	const AnnotatedInfos& Info;
	
	explicit DatenWidget(const AnnotatedInfos& info, QWidget *parent = nullptr);
	
	void updatePlanerPunkte(void);
	
	signals:
	void loescheEintrag(DatenWidget *widget);
	void zeigeLog(DatenWidget *widget);
	
	void graphDoppelklick(QCustomPlot *graph);
};

#endif
