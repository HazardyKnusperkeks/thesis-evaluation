#include "datenwidget.hpp"

#include "parseclips.hpp"
#include "produktplot.hpp"
#include "roboterplot.hpp"
#include "structs.hpp"
#include "zeitbismodellplot.hpp"

#include <QDir>
#include <QFile>
#include <QFont>
#include <QGridLayout>
#include <QImage>
#include <QLabel>
#include <QPalette>
#include <QPushButton>

bool DatenWidget::checkFehler(void) const noexcept {
	if ( Info.Punkte == 0 || Info.PlanerPunkteInGame == 0 ) {
		return true;
	} //if ( Info.Punkte == 0 || Info.PlanerPunkteInGame == 0 )
	return false;
}

DatenWidget::DatenWidget(const AnnotatedInfos& info, AnnotatedInfos& gegnerInfo, const QString& pfad, QWidget *parent) : QFrame(parent), Info(info), GegnerInfo(gegnerInfo) {
	auto spielLabel = new QLabel("Spiel #" + QString::number(info.Spiel), this);
	auto font = spielLabel->font();
	font.setBold(true);
	font.setPointSize(font.pointSize() + 2);
	spielLabel->setFont(font);
	
	Punkte               = new QLabel(QString::number(info.Punkte), this);
	PlanerPunkte         = new QLabel(this);
	PlanerPunkteComplete = new QLabel(this);
	
	updatePlanerPunkte();
	
	auto memoryLabel = new QLabel(this);
	memoryLabel->setPixmap(info.Memory);
	
	auto roboterPlot = new RoboterPlot(Info.AusgefuehrterPlan, this);
	roboterPlot->setMinimumSize(memoryLabel->sizeHint());
	connect(roboterPlot, &RoboterPlot::mouseDoubleClick, this, [this,roboterPlot](void) { emit graphDoppelklick(roboterPlot); return; });
	
	auto produktPlot = new ProduktPlot(Info.Produkte, this);
	produktPlot->setMinimumSize(memoryLabel->sizeHint());
	connect(produktPlot, &RoboterPlot::mouseDoubleClick, this, [this,produktPlot](void) { emit graphDoppelklick(produktPlot); return; });
	
	auto modellPlot = new ZeitBisModellPlot(Info.ZeitBisModell, Info.ZeitSeitModell, this);
	modellPlot->setMinimumSize(memoryLabel->sizeHint());
	connect(modellPlot, &RoboterPlot::mouseDoubleClick, this, [this,modellPlot](void) { emit graphDoppelklick(modellPlot); return; });
	
	auto loeschenKnopf = new QPushButton("Löschen", this);
	
	connect(loeschenKnopf, &QPushButton::clicked, this, [this](void) noexcept { emit loescheEintrag(this); return; });
	
	auto fehlerLabel = new QLabel(checkFehler() ? "Potentiell fehlerhafte Logfiles!" : "", this);
	auto palette = fehlerLabel->palette();
	palette.setColor(QPalette::Foreground, Qt::red);
	fehlerLabel->setPalette(palette);
	
	auto logKnopf = new QPushButton("Log öffnen", this);
	
	connect(logKnopf, &QPushButton::clicked, this, [this](void) noexcept { emit zeigeLog(this); return; });
	
	auto outlierLabel = new QLabel(Info.IstOutlier ? "Ist ein Outlier" : "", this);
	outlierLabel->setPalette(palette);
	
	auto failLabel = new QLabel(Info.HatFailedTask ? "Min 1 Task failed" : "", this);
	failLabel->setPalette(palette);
	
	auto layout = new QGridLayout(this);
	layout->addWidget(spielLabel,                           0, 0, 1, -1, Qt::AlignCenter);
	layout->addWidget(new QLabel("Punkte:", this),          1, 0);
	layout->addWidget(Punkte,                               1, 1);
	layout->addWidget(new QLabel("Pl. Punkte:", this),      2, 0);
	layout->addWidget(PlanerPunkte,                         2, 1);
	layout->addWidget(outlierLabel,                         2, 2);
	layout->addWidget(new QLabel("Pl. Punkte Com.:", this), 3, 0);
	layout->addWidget(PlanerPunkteComplete,                 3, 1);
	layout->addWidget(failLabel,                            3, 2);
	layout->addWidget(new QLabel("Idle:", this),            4, 0);
	layout->addWidget(new QLabel(QString::number(Info.Idle), this), 4, 1);
	layout->addWidget(memoryLabel,                          5, 0, 1, 4);
	layout->addWidget(roboterPlot,                          6, 0, 1, 4);
	layout->addWidget(produktPlot,                          7, 0, 1, 4);
	layout->addWidget(modellPlot,                           8, 0, 1, 4);
	
	layout->addWidget(loeschenKnopf,                        1, 3);
	layout->addWidget(fehlerLabel,                          2, 3);
	layout->addWidget(logKnopf,                             3, 3);
	
	constexpr const char* roboNamen[] = {"R-1", "R-2", "R-3"};
	constexpr const char* dateiNamen[] = {"-roboter4.log", "-roboter5.log", "-roboter6.log"};
	
	QDir verzeichnis(pfad);
	bool mitGegner = false;
	
	for ( int i = 0; i < 3; ++i ) {
		TaskList& tasks = GegnerInfo.AusgefuehrterPlan[roboNamen[i]];
		
		QFile datei(verzeichnis.filePath(QString::number(GegnerInfo.Spiel) + dateiNamen[i]));
		
		if ( datei.open(QIODevice::ReadOnly | QIODevice::Text) ) {
			mitGegner = true;
			GegnerInfo.Idle += parseClips(tasks, datei);
		} //if ( datei.open(QIODevice::ReadOnly | QIODevice::Text) )
		else {
			break;
		} //else -> if ( datei.open(QIODevice::ReadOnly | QIODevice::Text) )
	} //for ( int i = 0; i < 3; ++i )
	
	if ( mitGegner ) {
		auto roboterPlot = new RoboterPlot(GegnerInfo.AusgefuehrterPlan, this);
		roboterPlot->setMinimumSize(memoryLabel->sizeHint());
		connect(roboterPlot, &RoboterPlot::mouseDoubleClick, this, [this,roboterPlot](void) { emit graphDoppelklick(roboterPlot); return; });
		
		layout->addWidget(new QLabel(QString::number(GegnerInfo.Punkte), this), 1, 2);
		layout->addWidget(new QLabel(QString::number(GegnerInfo.Idle),   this), 4, 2);
		layout->addWidget(roboterPlot,                                          6, 4);
	} //if ( mitGegner )
	
	setFrameShape(QFrame::Box);
	return;
}

void DatenWidget::updatePlanerPunkte(void) {
	PlanerPunkte->setText(QString::number(Info.PlanerPunkteInGame));
	PlanerPunkteComplete->setText(QString::number(Info.PlanerPunkteComplete));
	return;
}
