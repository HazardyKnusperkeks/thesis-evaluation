#include "datenwidget.hpp"

#include "produktplot.hpp"
#include "roboterplot.hpp"
#include "structs.hpp"

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

DatenWidget::DatenWidget(const AnnotatedInfos& info, QWidget *parent) : QFrame(parent), Info(info) {
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
	
	auto produktPlot = new ProduktPlot(Info.Produkte, this);
	produktPlot->setMinimumSize(memoryLabel->sizeHint());
	
	auto loeschenKnopf = new QPushButton("Löschen", this);
	
	connect(loeschenKnopf, &QPushButton::clicked, this, [this](void) noexcept { emit loescheEintrag(this); return; });
	
	auto fehlerLabel = new QLabel(checkFehler() ? "Potentiell fehlerhafte Logfiles!" : "", this);
	auto palette = fehlerLabel->palette();
	palette.setColor(QPalette::Foreground, Qt::red);
	fehlerLabel->setPalette(palette);
	
	auto logKnopf = new QPushButton("Log öffnen", this);
	
	connect(logKnopf, &QPushButton::clicked, this, [this](void) noexcept { emit zeigeLog(this); return; });
	
	auto layout = new QGridLayout(this);
	layout->addWidget(spielLabel,                           0, 0, 1, -1, Qt::AlignCenter);
	layout->addWidget(new QLabel("Punkte:", this),          1, 0);
	layout->addWidget(Punkte,                               1, 1);
	layout->addWidget(new QLabel("Pl. Punkte:", this),      2, 0);
	layout->addWidget(PlanerPunkte,                         2, 1);
	layout->addWidget(new QLabel("Pl. Punkte Com.:", this), 3, 0);
	layout->addWidget(PlanerPunkteComplete,                 3, 1);
	layout->addWidget(memoryLabel,                          4, 0, 1, -1);
	layout->addWidget(roboterPlot,                          5, 0, 1, -1);
	layout->addWidget(produktPlot,                          6, 0, 1, -1);
	
	layout->addWidget(loeschenKnopf,                        1, 2);
	layout->addWidget(fehlerLabel,                          2, 2);
	layout->addWidget(logKnopf,                             3, 2);
	
	setFrameShape(QFrame::Box);
	return;
}

void DatenWidget::updatePlanerPunkte(void) {
	PlanerPunkte->setText(QString::number(Info.PlanerPunkteInGame));
	PlanerPunkteComplete->setText(QString::number(Info.PlanerPunkteComplete));
	return;
}
