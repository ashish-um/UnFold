#include <QApplication>
#include "app/MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("Unfold");
    app.setOrganizationName("Unfold");
    app.setApplicationVersion("1.0.0");

    // Set dark fusion style
    app.setStyle("Fusion");
    QPalette darkPalette;
    darkPalette.setColor(QPalette::Window, QColor(30, 30, 30));
    darkPalette.setColor(QPalette::WindowText, QColor(220, 220, 220));
    darkPalette.setColor(QPalette::Base, QColor(25, 25, 25));
    darkPalette.setColor(QPalette::AlternateBase, QColor(40, 40, 40));
    darkPalette.setColor(QPalette::ToolTipBase, QColor(50, 50, 50));
    darkPalette.setColor(QPalette::ToolTipText, QColor(220, 220, 220));
    darkPalette.setColor(QPalette::Text, QColor(220, 220, 220));
    darkPalette.setColor(QPalette::Button, QColor(45, 45, 45));
    darkPalette.setColor(QPalette::ButtonText, QColor(220, 220, 220));
    darkPalette.setColor(QPalette::BrightText, Qt::red);
    darkPalette.setColor(QPalette::Link, QColor(100, 160, 230));
    darkPalette.setColor(QPalette::Highlight, QColor(70, 130, 200));
    darkPalette.setColor(QPalette::HighlightedText, Qt::white);
    app.setPalette(darkPalette);

    MainWindow window;
    window.showMaximized();

    return app.exec();
}
