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
	
	void leseDaten(const QString& pfad);
	void clear(void);
	
	void loescheDaten(DatenWidget *widget);
	void zeigeLog(DatenWidget *widget);
	
	public:
	explicit EncodingFenster(const bool frei, QWidget *parent = nullptr);
	
	void setzePfad(const QString& pfad);
};

#endif
