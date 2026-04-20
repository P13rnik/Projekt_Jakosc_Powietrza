#include <QApplication>
#include <QFont>
#include "gui/MainWindow.hpp"

/**
 * @brief Punkt wejścia aplikacji AirQuality.
 *
 * Inicjalizuje aplikację Qt, ustawia globalną czcionkę
 * i uruchamia główne okno.
 */
int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    app.setApplicationName("AirQuality");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("Projekt C++");

    // Globalny styl aplikacji
    app.setStyle("Fusion");
    QFont font("Segoe UI", 10);
    font.setHintingPreference(QFont::PreferFullHinting);
    app.setFont(font);

    app.setStyleSheet(R"(
        QMainWindow { background: #F5F5F5; }
        QGroupBox {
            font-weight: bold;
            border: 1px solid #DEDEDE;
            border-radius: 6px;
            margin-top: 8px;
            background: white;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 4px;
            color: #555;
        }
        QComboBox { padding: 4px 8px; border: 1px solid #ccc; border-radius: 4px; }
        QComboBox:focus { border-color: #1976D2; }
        QStatusBar { font-size: 12px; color: #666; }
    )");

    MainWindow window;
    window.show();

    return app.exec();
}
