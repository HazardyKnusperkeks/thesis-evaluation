#include "hauptfenster.hpp"

#include "encodingfenster.hpp"

#include <QCheckBox>
#include <QDir>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QTabBar>
#include <QTabWidget>
#include <QVBoxLayout>

void Hauptfenster::fuegeHauptTabHinzu(void) {
	addTab(HauptTab, "Zusammenfassung");
	tabBar()->setTabButton(0, QTabBar::RightSide, nullptr);
	return;
}

void Hauptfenster::setzePfad(const QString& pfad) {
	Pfad->setText(pfad);
	
	while ( !Encodings.isEmpty() ) {
		entferneEncoding(*Encodings.keyBegin());
	} //while ( !Encodings.isEmpty() )
	
	while ( EncodingLayout->count() ) {
		auto item = EncodingLayout->takeAt(0);
		Q_ASSERT(item->widget());
		delete item->widget();
		delete item;
	} //while ( EncodingLayout->count() )
	
	QDir verzeichnis(pfad);
	
	for ( const auto& encoding : verzeichnis.entryList(QDir::Dirs | QDir::NoDotAndDotDot) ) {
		auto box = new QCheckBox(encoding, this);
		EncodingLayout->addWidget(box);
		
		connect(box, &QCheckBox::toggled, this, [this,box](const bool aktiv) {
				if ( aktiv ) {
					fuegeEncodingHinzu(box->text());
				} //if ( aktiv )
				else {
					entferneEncoding(box->text());
				} //else -> if ( aktiv )
			});
	} //for ( const auto& encoding : verzeichnis.entryList(QDir::Dirs) )
	
	EncodingLayout->addStretch();
	return;
}

void Hauptfenster::fuegeEncodingHinzu(const QString& encoding) {
	auto fenster = new EncodingFenster(false);
	fenster->setzePfad(Pfad->text() + QDir::separator() + encoding);
	Encodings.insert(encoding, fenster);
	addTab(fenster, encoding);
	return;
}

void Hauptfenster::entferneEncoding(const QString& encoding) {
	auto fenster = Encodings.take(encoding);
	int index = 0;
	for ( ; fenster != widget(index); ++index ) { }
	removeTab(index);
	delete fenster;
	return;
}

void Hauptfenster::neuerTab(void) {
	setCurrentIndex(addTab(new EncodingFenster(true), "Frei"));
	return;
}

void Hauptfenster::schliesseTab(const int index) {
	const auto encoding = tabText(index);
	if ( encoding != "Frei" ) {
		int layoutIndex = 0;
		for ( ; layoutIndex < EncodingLayout->count(); ++layoutIndex ) {
			auto item = EncodingLayout->itemAt(layoutIndex);
			auto box = static_cast<QCheckBox*>(item->widget());
			if ( box->text() == encoding ) {
				box->blockSignals(true);
				box->setChecked(false);
				box->blockSignals(false);
				break;
			} //if ( box->text() == encoding )
		} //for ( ; layoutIndex < EncodingLayout->count(); ++layoutIndex )
	} //if ( encoding != "Frei" )
	
	auto w = widget(index);
	removeTab(index);
	delete w;
	return;
}

Hauptfenster::Hauptfenster(QWidget *parent) : QTabWidget(parent) {
	HauptTab = new QWidget;
	
	Pfad = new QLineEdit(HauptTab);
	Pfad->setReadOnly(true);
	
	auto durchsuchen = new QPushButton("Durchsuchen", HauptTab);
	connect(durchsuchen, &QPushButton::clicked, this, [this](void) {
			QString pfad = QFileDialog::getExistingDirectory(this, "Wähle Pfad", Pfad->text());
			if ( !pfad.isEmpty() ) {
				setzePfad(pfad);
			} //if ( !pfad.isEmpty() )
			return;
		});
	
	auto obenLayout = new QHBoxLayout;
	obenLayout->addWidget(Pfad);
	obenLayout->addWidget(durchsuchen);
	
	EncodingLayout = new QHBoxLayout;
	
	ScrollWidget = new QScrollArea(HauptTab);
	
	auto layout = new QVBoxLayout(HauptTab);
	layout->addLayout(obenLayout);
	layout->addLayout(EncodingLayout);
	layout->addWidget(ScrollWidget);
	
	auto neu = new QPushButton("Neu");
	connect(neu, &QPushButton::clicked, this, &Hauptfenster::neuerTab);
	
	setCornerWidget(neu);
	setTabsClosable(true);
	fuegeHauptTabHinzu();
	
	connect(this, &Hauptfenster::tabCloseRequested, this, &Hauptfenster::schliesseTab);
	
	setWindowTitle("Evaluation");
	setWindowState(Qt::WindowMaximized);
	return;
}
