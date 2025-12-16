#include "ui/mainwindow.h"
#include <QApplication>

// Точка входа в приложение
// Создает QApplication и главное окно, запускает цикл обработки событий Qt
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec(); // Запуск главного цикла обработки событий Qt
}
