#ifndef GRAPHDIALOG_HPP
#define GRAPHDIALOG_HPP

#include <QDialog>

class QCustomPlot;

class QSpinBox;

class LegendenDialog : public QDialog {
	Q_OBJECT
	private:
	QCustomPlot* const Graph;
	
	void speichereGraph(void);
	
	public:
	explicit LegendenDialog(QCustomPlot *graph, QWidget *parent = nullptr);
};

class GraphDialog : public QDialog {
	Q_OBJECT
	private:
	QCustomPlot* const OriginalGraph;
	QSpinBox* const Breite;
	QSpinBox* const Hoehe;
	
	void speichereGraph(void);
	void zeigeLegende(void);
	
	protected:
	QCustomPlot* const Graph;
	
	explicit GraphDialog(QCustomPlot *graph, QWidget *parent = nullptr);
};

class StatistikGraphDialog : public GraphDialog {
	Q_OBJECT
	public:
	explicit StatistikGraphDialog(QCustomPlot *graph, QWidget *parent = nullptr);
};

class BoxGraphDialog : public GraphDialog {
	Q_OBJECT
	public:
	explicit BoxGraphDialog(QCustomPlot *graph, QWidget *parent = nullptr);
};

#endif
