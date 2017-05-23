#include "encodingfenster.hpp"

#include "datenwidget.hpp"
#include "qcustomplot/qcustomplot.h"

#include <algorithm>

#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFont>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollArea>
#include <QScrollBar>
#include <QSpinBox>
#include <QTimer>
#include <QVBoxLayout>

void EncodingFenster::leseDaten(const QString& pfad) {
	const auto horizontal = ScrollWidget->horizontalScrollBar()->sliderPosition();
	const auto vertikal   = ScrollWidget->verticalScrollBar()->sliderPosition();
	
	clear();
	
	QDir verzeichnis(pfad);
	auto dateien(verzeichnis.entryList({"*.parsed"}));
	
	std::sort(dateien.begin(), dateien.end(),
	          [](const QString& a, const QString& b) noexcept {
	          		return a.leftRef(a.indexOf('.')).toInt() < b.leftRef(b.indexOf('.')).toInt();
	          	});
	
	const auto reserve = dateien.size();
	Daten.reserve(reserve);
	GegnerDaten.reserve(reserve);
	Punkte.Sequenz.reserve(reserve);
	PlanerPunkte.Sequenz.reserve(reserve);
	PlanerPunkteNachSpiel.Sequenz.reserve(reserve);
	OutlierPunkte.first.Sequenz.reserve(reserve);
	OutlierPlanerPunkte.first.Sequenz.reserve(reserve);
	OutlierPlanerPunkteNachSpiel.first.Sequenz.reserve(reserve);
	
	Idle.Sequenz.reserve(reserve);
	OutlierIdle.first.Sequenz.reserve(reserve);
	
	StartUp.Sequenz.reserve(reserve);
	
	auto widget = new QWidget(ScrollWidget);
	auto layout = new QGridLayout(widget);
	int spalte = 0, zeile = 1;
	constexpr int maxSpalten = 10;
	
	int spiel = 0;
	for ( const auto& dateiPfad : dateien ) {
		QFile datei(verzeichnis.filePath(dateiPfad));
		
		AnnotatedInfos info, gegnerInfo;
		gegnerInfo.Spiel = info.Spiel = ++spiel;
		
		if ( datei.open(QIODevice::ReadOnly) ) {
			QDataStream stream(&datei);
			stream.setVersion(QDataStream::Qt_5_6);
			stream>>info;
			info.zaehleTasks();
			info.berechnePunkte(0);
		} //else -> if ( datei.open(QIODevice::ReadOnly) )
		
		datei.close();
		datei.setFileName(verzeichnis.filePath(QString::number(spiel) + "-refbox.log"));
		
		if ( datei.open(QIODevice::ReadOnly | QIODevice::Text) ) {
			bool gegner = false;
			//I 15:05:16.816897 C: OVERALL TOTAL POINTS: 162
			while ( !datei.atEnd() ) {
				QByteArray zeile(datei.readLine());
				if ( zeile.contains("OVERALL TOTAL") ) {
					const auto pos = zeile.lastIndexOf(' ') + 1;
					if ( gegner ) {
						gegnerInfo.Punkte = zeile.mid(pos, zeile.size() - pos - 1).toInt();
						break;
					} //if ( gegner )
					else {
						info.Punkte = zeile.mid(pos, zeile.size() - pos - 1).toInt();
						info.berechneIstOutlier();
						gegnerInfo.IstOutlier = info.IstOutlier;
						gegner = true;
					} //else -> if ( gegner )
				} //if ( zeile.contains("OVERALL TOTAL") )
			} //while ( !datei.atEnd() )
		} //if ( datei.open(QIODevice::ReadOnly | QIODevice::Text) )
		
		info.Memory = QPixmap(verzeichnis.filePath(QString::number(spiel) + "-Memory.png"));
		
		Daten.emplace_back(std::move(info));
		GegnerDaten.emplace_back(std::move(gegnerInfo));
		
		auto& daten(Daten.back());
		auto& gegnerDaten(GegnerDaten.back());
		
		auto widget = new DatenWidget(daten);
		connect(widget, &DatenWidget::graphDoppelklick, this, &EncodingFenster::graphDoppelklick);
		Widgets.push_back(widget);
		layout->addWidget(widget, zeile, spalte);
		
		if ( ++spalte == maxSpalten ) {
			++zeile;
			spalte = 0;
		} //if ( ++spalte == maxSpalten )
		
		connect(widget, &DatenWidget::loescheEintrag, this, &EncodingFenster::loescheDaten);
		connect(widget, &DatenWidget::zeigeLog,       this, &EncodingFenster::zeigeLog);
		
		Punkte.Sequenz.push_back(daten.Punkte);
		GegnerPunkte.Sequenz.push_back(gegnerDaten.Punkte);
		PlanerPunkte.Sequenz.push_back(daten.PlanerPunkteInGame);
		PlanerPunkteNachSpiel.Sequenz.push_back(daten.PlanerPunkteComplete);
		
		Idle.Sequenz.push_back(daten.Idle);
		GegnerIdle.Sequenz.push_back(gegnerDaten.Idle);
		
		StartUp.Sequenz.push_back(daten.StartUp);
		
		if ( daten.IstOutlier ) {
			OutlierPunkte.second.push_back(daten.Punkte);
			GegnerOutlierPunkte.second.push_back(gegnerDaten.Punkte);
			OutlierPlanerPunkte.second.push_back(daten.PlanerPunkteInGame);
			OutlierPlanerPunkteNachSpiel.second.push_back(daten.PlanerPunkteComplete);
			
			OutlierIdle.second.push_back(daten.Idle);
			GegnerOutlierIdle.second.push_back(gegnerDaten.Idle);
		} //if ( daten.IstOutlier )
		else {
			OutlierPunkte.first.Sequenz.push_back(daten.Punkte);
			GegnerOutlierPunkte.first.Sequenz.push_back(gegnerDaten.Punkte);
			OutlierPlanerPunkte.first.Sequenz.push_back(daten.PlanerPunkteInGame);
			OutlierPlanerPunkteNachSpiel.first.Sequenz.push_back(daten.PlanerPunkteComplete);
			
			OutlierIdle.first.Sequenz.push_back(daten.Idle);
			GegnerOutlierIdle.first.Sequenz.push_back(gegnerDaten.Idle);
		} //else -> if ( daten.IstOutlier )
	} //for ( const auto& dateiPfad : dateien )
	
	Punkte.calc();
	GegnerPunkte.calc();
	PlanerPunkte.calc();
	PlanerPunkteNachSpiel.calc();
	
	OutlierPunkte.first.calc();
	GegnerOutlierPunkte.first.calc();
	OutlierPlanerPunkte.first.calc();
	OutlierPlanerPunkteNachSpiel.first.calc();
	
	Idle.calc();
	GegnerIdle.calc();
	OutlierIdle.first.calc();
	GegnerOutlierIdle.first.calc();
	
	StartUp.calc();
	
	auto zusammenFassung = new QGridLayout;
	
	QFont font;
	font.setBold(true);
	
	spalte = 0;
	for ( const auto str : {"Anz", "Sum", "Min", "Max", "Avg", "Abw", "1.Q", "2.Q", "3.Q"} ) {
		auto label = new QLabel(str, widget);
		label->setFont(font);
		zusammenFassung->addWidget(label, 0, ++spalte, Qt::AlignHCenter);
	} //for ( const auto str : {"Anz", "Sum", "Min", "Max", "Avg", "Abw", "1.Q", "2.Q", "3.Q"} )
	
	zeile = 1;
	auto fuegeZeileHinzu = [&zeile,&zusammenFassung,&font,&widget](const char *str, const AvgSequenz<double>& seq) {
			auto label = new QLabel(str, widget);
			label->setFont(font);
			zusammenFassung->addWidget(label, zeile, 0);
			
			int spalte = 0;
			zusammenFassung->addWidget(new QLabel(QString::number(seq.Sequenz.size()),              widget), zeile, ++spalte, Qt::AlignRight);
			zusammenFassung->addWidget(new QLabel(QString("%1").arg(seq.Summe,          0, 'f', 3), widget), zeile, ++spalte, Qt::AlignRight);
			zusammenFassung->addWidget(new QLabel(QString("%1").arg(seq.Min,            0, 'f', 3), widget), zeile, ++spalte, Qt::AlignRight);
			zusammenFassung->addWidget(new QLabel(QString("%1").arg(seq.Max,            0, 'f', 3), widget), zeile, ++spalte, Qt::AlignRight);
			zusammenFassung->addWidget(new QLabel(QString("%1").arg(seq.Avg,            0, 'f', 3), widget), zeile, ++spalte, Qt::AlignRight);
			zusammenFassung->addWidget(new QLabel(QString("%1").arg(seq.StdAbw,         0, 'f', 3), widget), zeile, ++spalte, Qt::AlignRight);
			zusammenFassung->addWidget(new QLabel(QString("%1").arg(seq.ErstesQuartil,  0, 'f', 3), widget), zeile, ++spalte, Qt::AlignRight);
			zusammenFassung->addWidget(new QLabel(QString("%1").arg(seq.ZweitesQuartil, 0, 'f', 3), widget), zeile, ++spalte, Qt::AlignRight);
			zusammenFassung->addWidget(new QLabel(QString("%1").arg(seq.DrittesQuartil, 0, 'f', 3), widget), zeile, ++spalte, Qt::AlignRight);
			
			++zeile;
			return;
		};
	
	fuegeZeileHinzu("Punkte:",              Punkte);
	fuegeZeileHinzu("Pl. Punkte:",          PlanerPunkte);
	fuegeZeileHinzu("Pl. Punkte Gesamt:",   PlanerPunkteNachSpiel);
	fuegeZeileHinzu("Idle:",                Idle);
	fuegeZeileHinzu("O Punkte:",            OutlierPunkte.first);
	fuegeZeileHinzu("O Pl. Punkte:",        OutlierPlanerPunkte.first);
	fuegeZeileHinzu("O Pl. Punkte Gesamt:", OutlierPlanerPunkteNachSpiel.first);
	fuegeZeileHinzu("O Idle:",              OutlierIdle.first);
	fuegeZeileHinzu("StartUp",              StartUp);
	if ( GegnerPunkte.Max ) {
		zusammenFassung->addWidget(new QLabel(widget), ++zeile, 0); //Leerzeile
		fuegeZeileHinzu("G. Punkte: ",      GegnerPunkte);
		fuegeZeileHinzu("G. Idle: ",        GegnerIdle);
		fuegeZeileHinzu("G. O Punkte: ",    GegnerOutlierPunkte.first);
		fuegeZeileHinzu("G. O Idle: ",      GegnerOutlierIdle.first);
	} //if ( GegnerPunkte.Max )
	
	auto findMin = new QPushButton("Finde Min", widget);
	connect(findMin, &QPushButton::clicked, this, &EncodingFenster::findeMin);
	
	auto findAvg = new QPushButton("Finde Avg", widget);
	connect(findAvg, &QPushButton::clicked, this, &EncodingFenster::findeAvg);
	
	auto findMax = new QPushButton("Finde Max", widget);
	connect(findMax, &QPushButton::clicked, this, &EncodingFenster::findeMax);
	
	WertSpin = new QSpinBox(widget);
	WertSpin->setRange(Punkte.Min, Punkte.Max);
	
	auto findWert = new QPushButton("Finde Wert", widget);
	connect(findWert, &QPushButton::clicked, this, &EncodingFenster::findeWert);
	
	auto knopfLayout = new QVBoxLayout;
	knopfLayout->addWidget(findMin);
	knopfLayout->addWidget(findAvg);
	knopfLayout->addWidget(findMax);
	knopfLayout->addWidget(WertSpin);
	knopfLayout->addWidget(findWert);
	knopfLayout->addStretch();
	
	zusammenFassung->addLayout(knopfLayout, 0, ++spalte, -1, 1);
	zusammenFassung->setColumnStretch(++spalte, 1);
	
	layout->addLayout(zusammenFassung, 0, 0, 1, -1);
	ScrollWidget->setWidget(widget);
	widget->show();
	
	ScrollWidget->horizontalScrollBar()->setSliderPosition(horizontal);
	ScrollWidget->verticalScrollBar()->setSliderPosition(vertikal);
	return;
}

void EncodingFenster::clear(void) {
	delete ScrollWidget->takeWidget();
	Widgets.clear();
	Daten.clear();
	GegnerDaten.clear();
	
	Punkte.Sequenz.clear();
	GegnerPunkte.Sequenz.clear();
	PlanerPunkte.Sequenz.clear();
	PlanerPunkteNachSpiel.Sequenz.clear();
	OutlierPunkte.first.Sequenz.clear();
	GegnerOutlierPunkte.first.Sequenz.clear();
	OutlierPlanerPunkte.first.Sequenz.clear();
	OutlierPlanerPunkteNachSpiel.first.Sequenz.clear();
	
	Idle.Sequenz.clear();
	GegnerIdle.Sequenz.clear();
	OutlierIdle.first.Sequenz.clear();
	GegnerOutlierIdle.first.Sequenz.clear();
	
	StartUp.Sequenz.clear();
	
	WertSpin = nullptr;
	return;
}

void EncodingFenster::loescheDaten(DatenWidget* widget) {
	int spiel = widget->Info.Spiel;
	auto spielString = QString::number(spiel);
	
	constexpr const char *suffixe[] = {".parsed", "-filtered.log", "-Memory.png", "-Memory.rrd", "-roboter1.log",
	                                   "-roboter2.log", "-roboter3.log", "-roboter4.log", "-roboter5.log",
	                                   "-roboter6.log", "-refbox.log", "-planer.log"};
	
	if ( QMessageBox::question(this, "Löschen?",
	                           "Soll Spiel " + spielString + " wirklich gelöscht werden?") == QMessageBox::Yes ) {
		const auto pfad(Pfad->text());
		
		QDir verzeichnis(pfad);
		for ( const auto& suffix : suffixe ) {
			const auto datei(verzeichnis.filePath(spielString + suffix));
			if ( !QFile::remove(datei) ) {
				QMessageBox::warning(this, "Fehler", "Konnte " + datei + " nicht löschen! Manuelles eingreifen nötig!");
			} //if ( !QFile::remove(datei) )
		} //for ( const auto& suffix : suffixe )
		
		bool weiter = true;
		auto freiString = std::move(spielString);
		
		do { //while ( weiter )
			++spiel;
			spielString = QString::number(spiel);
			
			weiter = QFile::exists(verzeichnis.filePath(spielString + suffixe[0]));
			
			if ( weiter ) {
				for ( const auto& suffix : suffixe ) {
					const auto alt(verzeichnis.filePath(spielString + suffix)),
					           neu(verzeichnis.filePath(freiString + suffix));
					
					if ( !QFile::rename(alt, neu) ) {
						QMessageBox::warning(this, "Fehler", "Fehler beim verschieben von " + alt + " nach " + neu);
						return;
					} //if ( !QFile::rename(alt, neu) )
				} //for ( const auto& suffix : suffixe )
				
				freiString = std::move(spielString);
			} //if ( weiter )
		} while ( weiter );
		
		leseDaten(pfad);
	} //if ( QMessageBox::question(this, "Löschen?") == QMessageBox::Yes )
	return;
}

void EncodingFenster::zeigeLog(DatenWidget* widget) {
	const QDir verzeichnis(Pfad->text());
	constexpr const char *suffixe[] = {"-filtered.log", "-roboter1.log",
	                                   "-roboter2.log", "-roboter3.log", "-refbox.log", "-planer.log"};
	
	for ( const auto& suffix : suffixe ) {
		const QUrl url(QUrl::fromLocalFile(verzeichnis.filePath(QString::number(widget->Info.Spiel) + suffix)));
		QDesktopServices::openUrl(url);
	} //for ( const auto& suffix : suffixe )
	return;
}

bool EncodingFenster::springeZuDaten(std::function<bool(const AnnotatedInfos&)> pred) {
	const auto begin = Daten.begin(), end = Daten.end();
	auto iter = std::find_if(begin, end, pred);
	if ( iter == end ) {
		return false;
	} //if ( iter == end )
	
	ScrollWidget->ensureWidgetVisible(Widgets.at(std::distance(begin, iter)));
	return true;
}

void EncodingFenster::findeMin(void) {
	springeZuDaten([this](const AnnotatedInfos& info) { return !info.IstOutlier && info.Punkte == OutlierPunkte.first.Min; });
	return;
}

void EncodingFenster::findeAvg(void) {
	int delta = 0;
	bool weiter;
	auto lambda = [this,&delta](const AnnotatedInfos& info) { return !info.IstOutlier && !info.HatFailedTask && std::abs(info.Punkte - OutlierPunkte.first.Avg) <= delta; };
	do { //while ( weiter )
		weiter = !springeZuDaten(lambda);
		++delta;
	} while ( weiter );
	return;
}

void EncodingFenster::findeMax(void) {
	springeZuDaten([this](const AnnotatedInfos& info) { return !info.IstOutlier && info.Punkte == OutlierPunkte.first.Max; });
	return;
}

void EncodingFenster::findeWert(void) {
	springeZuDaten([this](const AnnotatedInfos& info) { return !info.IstOutlier && info.Punkte == WertSpin->value(); });
	return;
}

EncodingFenster::EncodingFenster(const bool frei, QWidget *parent) : QWidget(parent), WertSpin(nullptr) {
	Pfad = new QLineEdit(this);
	Pfad->setReadOnly(true);
	
	auto obenLayout = new QHBoxLayout;
	obenLayout->addWidget(Pfad);
	
	if ( frei ) {
		auto knopf = new QPushButton("Durchsuchen", this);
		connect(knopf, &QPushButton::clicked, this, [this](void) {
				QString pfad = QFileDialog::getExistingDirectory(this, "Wähle Pfad", Pfad->text());
				if ( !pfad.isEmpty() ) {
					setzePfad(pfad);
				} //if ( !pfad.isEmpty() )
				return;
			});
		
		obenLayout->addWidget(knopf);
		
		QTimer::singleShot(1, knopf, &QPushButton::click);
	} //if ( frei )
	
	ScrollWidget = new QScrollArea(this);
	
	auto layout = new QVBoxLayout(this);
	layout->addLayout(obenLayout);
	layout->addWidget(ScrollWidget);
	return;
}

void EncodingFenster::setzePfad(const QString& pfad) {
	Pfad->setText(pfad);
	leseDaten(pfad);
	return;
}

const AvgSequenz<double>& EncodingFenster::punkte(void) const {
	return Punkte;
}

const AvgSequenz<double>& EncodingFenster::planerPunkte(void) const {
	return PlanerPunkte;
}

const AvgSequenz<double>& EncodingFenster::planerPunkteNachSpiel(void) const {
	return PlanerPunkteNachSpiel;
}

const std::pair<AvgSequenz<double>, QVector<double>>& EncodingFenster::outlierPunkte(void) const {
	return OutlierPunkte;
}

const std::pair<AvgSequenz<double>, QVector<double>>& EncodingFenster::outlierPlanerPunkte(void) const {
	return OutlierPlanerPunkte;
}

const std::pair<AvgSequenz<double>, QVector<double>>& EncodingFenster::outlierPlanerPunkteNachSpiel(void) const {
	return OutlierPlanerPunkteNachSpiel;
}

const AvgSequenz<double>& EncodingFenster::idle(void) const {
	return Idle;
}

const std::pair<AvgSequenz<double>, QVector<double>>& EncodingFenster::outlierIdle(void) const {
	return OutlierIdle;
}

const AvgSequenz<double>& EncodingFenster::startUp(void) const {
	return StartUp;
}

