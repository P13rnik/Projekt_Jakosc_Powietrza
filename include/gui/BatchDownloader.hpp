#pragma once
#include <QObject>
#include <QList>
#include "core/Station.hpp"

/**
 * @brief Pobiera dane ze wszystkich stacji do lokalnego cache.
 *
 * Działa w tle — dla każdej stacji pobiera listę czujników,
 * a następnie dane pomiarowe pierwszego czujnika (PM10 lub inny).
 * Postęp raportowany sygnałem progressChanged().
 * Po zakończeniu emituje finished().
 *
 * Użycie: kliknięcie "Pobierz wszystko offline" w menu.
 * Pozwala używać aplikacji bez internetu dla wszystkich pobranych stacji.
 */
class BatchDownloader : public QObject {
    Q_OBJECT

public:
    explicit BatchDownloader(QObject* parent = nullptr);

    /**
     * @brief Uruchamia pobieranie dla listy stacji.
     * @param stations Lista stacji do pobrania.
     */
    void start(const QList<Station>& stations);

    /** @brief Anuluje pobieranie. */
    void cancel();

    bool isRunning() const { return m_running; }

signals:
    /** @brief Postęp 0-100. */
    void progressChanged(int percent);

    /** @brief Informacja o aktualnie pobieranej stacji. */
    void statusChanged(const QString& message);

    /**
     * @brief Emitowany po zakończeniu (lub anulowaniu).
     * @param downloaded Liczba pomyślnie pobranych stacji.
     * @param total      Łączna liczba stacji.
     * @param cancelled  true jeśli przerwano przez cancel().
     */
    void finished(int downloaded, int total, bool cancelled);

private:
    bool m_running   = false;
    bool m_cancelled = false;
};
