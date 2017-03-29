#ifndef HAUPTFENSTER_HPP
#define HAUPTFENSTER_HPP

#include <QHash>
#include <QTabWidget>

#include <vector>

class QHBoxLayout;
class QLineEdit;
class QScrollArea;

class EncodingFenster;

class Hauptfenster : public QTabWidget {
	Q_OBJECT
	private:
	QHash<QString, EncodingFenster*> Encodings;
	
	QWidget *HauptTab;
	QLineEdit *Pfad;
	QHBoxLayout *EncodingLayout;
	QScrollArea *ScrollWidget;
	
	void fuegeHauptTabHinzu(void);
	
	void setzePfad(const QString& pfad);
	
	void fuegeEncodingHinzu(const QString& encoding);
	void entferneEncoding(const QString& encoding);
	
	//slots:
	void neuerTab(void);
	void schliesseTab(const int index);
	
	public:
	explicit Hauptfenster(QWidget *parent = nullptr);
};

#endif
