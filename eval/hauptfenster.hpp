#ifndef HAUPTFENSTER_HPP
#define HAUPTFENSTER_HPP

#include <QTabWidget>

#include <vector>

class QGridLayout;
class QLineEdit;
class QScrollArea;
class QTimer;

class QCustomPlot;

class EncodingFenster;

class Hauptfenster : public QTabWidget {
	Q_OBJECT
	private:
	std::vector<std::pair<QString, EncodingFenster*>> Encodings;
	
	QWidget *HauptTab;
	QLineEdit *Pfad;
	QGridLayout *EncodingLayout;
	QScrollArea *ScrollWidget;
	
	QTimer *UpdateTimer;
	
	void fuegeHauptTabHinzu(void);
	
	void setzePfad(const QString& pfad);
	
	void fuegeEncodingHinzu(const QString& encoding);
	void entferneEncoding(const QString& encoding);
	
	//slots:
	void update(void);
	
	void neuerTab(void);
	void schliesseTab(const int index);
	
	void graphDoppelKlick(QCustomPlot *graph);
	
	public:
	explicit Hauptfenster(QWidget *parent = nullptr);
};

#endif
