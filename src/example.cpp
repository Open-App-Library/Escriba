/*
 * Example Usage.
 * This is meant as basic documentation
 * to get you up and running with
 * Escriba.
 */

#include <QApplication>
#include <QMainWindow>
#include "escriba.h"

// Short and Sweet Example
int main(int argc, char *argv[])
{
	QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
	QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
	QApplication a(argc, argv);

	Escriba *editor = new Escriba();
	editor->show();

	return a.exec();
}


// Longer but
