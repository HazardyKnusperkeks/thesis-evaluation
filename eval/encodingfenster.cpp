#include "encodingfenster.hpp"

#include "datenwidget.hpp"

#include <algorithm>

#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollArea>
#include <QScrollBar>
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
	Punkte.Sequenz.reserve(reserve);
	PlanerPunkte.Sequenz.reserve(reserve);
	PlanerPunkteNachSpiel.Sequenz.reserve(reserve);
	OutlierPunkte.first.Sequenz.reserve(reserve);
	OutlierPlanerPunkte.first.Sequenz.reserve(reserve);
	OutlierPlanerPunkteNachSpiel.first.Sequenz.reserve(reserve);
	
	auto widget = new QWidget(ScrollWidget);
	auto layout = new QGridLayout(widget);
	int spalte = 0, zeile = 0;
	constexpr int maxSpalten = 10;
	
	int spiel = 0;
	for ( const auto& dateiPfad : dateien ) {
		QFile datei(verzeichnis.filePath(dateiPfad));
		
		AnnotatedInfos info;
		info.Spiel = ++spiel;
		
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
			//I 15:05:16.816897 C: OVERALL TOTAL POINTS: 162
			while ( !datei.atEnd() ) {
				QByteArray zeile(datei.readLine());
				if ( zeile.contains("OVERALL TOTAL") ) {
					const auto pos = zeile.lastIndexOf(' ') + 1;
					info.Punkte = zeile.mid(pos, zeile.size() - pos - 1).toInt();
					break;
				} //if ( zeile.contains("OVERALL TOTAL") )
			} //while ( !datei.atEnd() )
		} //if ( datei.open(QIODevice::ReadOnly | QIODevice::Text) )
		
		info.Memory = QPixmap(verzeichnis.filePath(QString::number(spiel) + "-Memory.png"));
		
		Daten.emplace_back(std::move(info));
		
		auto& daten(Daten.back());
		
		auto widget = new DatenWidget(daten);
		Widgets.push_back(widget);
		layout->addWidget(widget, zeile, spalte);
		
		if ( ++spalte == maxSpalten ) {
			++zeile;
			spalte = 0;
		} //if ( ++spalte == maxSpalten )
		
		connect(widget, &DatenWidget::loescheEintrag, this, &EncodingFenster::loescheDaten);
		connect(widget, &DatenWidget::zeigeLog,       this, &EncodingFenster::zeigeLog);
		
		Punkte.Sequenz.push_back(daten.Punkte);
		PlanerPunkte.Sequenz.push_back(daten.PlanerPunkteInGame);
		PlanerPunkteNachSpiel.Sequenz.push_back(daten.PlanerPunkteComplete);
		
		if ( daten.istOutlier() ) {
			OutlierPunkte.second.push_back(daten.Punkte);
			OutlierPlanerPunkte.second.push_back(daten.PlanerPunkteInGame);
			OutlierPlanerPunkteNachSpiel.second.push_back(daten.PlanerPunkteComplete);
		} //if ( daten.istOutlier() )
		else {
			OutlierPunkte.first.Sequenz.push_back(daten.Punkte);
			OutlierPlanerPunkte.first.Sequenz.push_back(daten.PlanerPunkteInGame);
			OutlierPlanerPunkteNachSpiel.first.Sequenz.push_back(daten.PlanerPunkteComplete);
		} //else -> if ( daten.istOutlier() )
	} //for ( const auto& dateiPfad : dateien )
	
	Punkte.calc();
	PlanerPunkte.calc();
	PlanerPunkteNachSpiel.calc();
	
	OutlierPunkte.first.calc();
	OutlierPlanerPunkte.first.calc();
	OutlierPlanerPunkteNachSpiel.first.calc();
	
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
	return;
}

void EncodingFenster::loescheDaten(DatenWidget* widget) {
	int spiel = widget->Info.Spiel;
	auto spielString = QString::number(spiel);
	
	constexpr const char *suffixe[] = {".parsed", "-filtered.log", "-Memory.png", "-Memory.rrd", "-roboter1.log",
	                                   "-roboter2.log", "-roboter3.log", "-refbox.log", "-planer.log"};
	
	if ( QMessageBox::question(this, "Löschen?",
	                           "Soll Spiel " + spielString + " wirklich gelöscht werden?") == QMessageBox::Yes ) {
		const auto pfad(Pfad->text());
		
		QDir verzeichnis(pfad);
		for ( const auto& suffix : suffixe ) {
			const auto datei(verzeichnis.filePath(spielString + suffix));
			if ( !QFile::remove(datei) ) {
				QMessageBox::warning(this, "Fehler", "Konnte " + datei + " nicht löschen! Manuelles eingreifen nötig!");
				return;
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

EncodingFenster::EncodingFenster(const bool frei, QWidget *parent) : QWidget(parent) {
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
