#pragma once
#include <QString>
#include <QDebug>
#include <QDateTime>

/**
 * @brief Klasa narzędziowa do logowania zdarzeń aplikacji.
 *
 * Loguje komunikaty z poziomem ważności (INFO, WARNING, ERROR)
 * do konsoli debugowej Qt. Możliwe rozszerzenie o zapis do pliku.
 */
class Logger {
public:
    enum class Level { INFO, WARNING, ERROR };

    /**
     * @brief Loguje wiadomość z podanym poziomem.
     * @param level Poziom ważności komunikatu.
     * @param message Treść komunikatu.
     */
    static void log(Level level, const QString& message) {
        QString prefix;
        QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
        switch (level) {
            case Level::INFO:    prefix = "[INFO]";    break;
            case Level::WARNING: prefix = "[WARN]";    break;
            case Level::ERROR:   prefix = "[ERROR]";   break;
        }
        qDebug().noquote() << timestamp << prefix << message;
    }

    static void info(const QString& msg)    { log(Level::INFO, msg);    }
    static void warning(const QString& msg) { log(Level::WARNING, msg); }
    static void error(const QString& msg)   { log(Level::ERROR, msg);   }
};
