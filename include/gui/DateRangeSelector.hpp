#pragma once
#include <QWidget>
#include <QDateTimeEdit>
#include <QComboBox>
#include <QPushButton>
#include <QHBoxLayout>
#include <QDateTime>

/**
 * @brief Widget umożliwiający wybór zakresu czasowego danych na wykresie.
 *
 * Udostępnia predefiniowane okresy (ostatnie 24h, 48h, 7 dni) oraz
 * możliwość ręcznego ustawienia daty „od" i „do".
 * Po zmianie zakresu emituje sygnał rangeChanged().
 */
class DateRangeSelector : public QWidget {
    Q_OBJECT

public:
    explicit DateRangeSelector(QWidget* parent = nullptr);

    /**
     * @brief Zwraca wybrany początek zakresu.
     * @return Data i czas początku okresu.
     */
    QDateTime from() const;
    /** @brief Wymusza ponowne wyemitowanie rangeChanged z aktualnym zakresem. */
    void forceEmit();

    /**
     * @brief Zwraca wybrany koniec zakresu.
     * @return Data i czas końca okresu (zazwyczaj teraz).
     */
    QDateTime to() const;

signals:
    /**
     * @brief Emitowany gdy użytkownik zmieni zakres dat.
     * @param from Nowy początek zakresu.
     * @param to Nowy koniec zakresu.
     */
    void rangeChanged(const QDateTime& from, const QDateTime& to);

private slots:
    void onPresetChanged(int index);
    void onCustomDateChanged();

private:
    void setupUi();

    QComboBox*      m_presetCombo;   ///< Wybór predefiniowanego okresu
    QDateTimeEdit*  m_fromEdit;      ///< Ręczne ustawienie daty „od"
    QDateTimeEdit*  m_toEdit;        ///< Ręczne ustawienie daty „do"
    QPushButton*    m_applyButton;
    bool            m_initialized = false; ///< true po zakończeniu konstruktora
};
