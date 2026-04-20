#pragma once
#include <stdexcept>
#include <string>

/**
 * @brief Bazowy wyjątek aplikacji AirQuality.
 */
class AppException : public std::runtime_error {
public:
    explicit AppException(const std::string& msg) : std::runtime_error(msg) {}
};

/**
 * @brief Wyjątek błędu sieci (np. brak połączenia z API GIOŚ).
 */
class NetworkException : public AppException {
public:
    explicit NetworkException(const std::string& msg)
        : AppException("[Network] " + msg) {}
};

/**
 * @brief Wyjątek błędu pliku (np. brak pliku bazy danych).
 */
class FileException : public AppException {
public:
    explicit FileException(const std::string& msg)
        : AppException("[File] " + msg) {}
};

/**
 * @brief Wyjątek błędu danych (np. nieprawidłowy JSON, null).
 */
class DataException : public AppException {
public:
    explicit DataException(const std::string& msg)
        : AppException("[Data] " + msg) {}
};

/**
 * @brief Wyjątek błędu parsowania JSON.
 */
class ParseException : public DataException {
public:
    explicit ParseException(const std::string& msg)
        : DataException("[Parse] " + msg) {}
};
