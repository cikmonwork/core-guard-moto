#ifndef FLAT_REGISTRY_H
#define FLAT_REGISTRY_H

#include <Arduino.h>
#include <mutex>

/**
 * Улучшенное хранилище Ключ-Значение.
 * Оптимизировано для ESP32: потокобезопасно, экономно к RAM.
 */
template <typename T>
class FlatRegistry
{
private:
    struct Entry
    {
        String key;
        T value;
    };

    Entry *_entries;
    int _capacity;
    int _count;
    mutable std::recursive_mutex _mtx;

public:
    FlatRegistry(int capacity) : _capacity(capacity), _count(0)
    {
        _entries = new Entry[capacity];
    }

    ~FlatRegistry()
    {
        delete[] _entries;
    }

    // Добавить или обновить (возвращает индекс)
    int put(const String &key, const T &value)
    {
        std::lock_guard<std::recursive_mutex> lock(_mtx);

        // 1. Поиск существующего
        for (int i = 0; i < _count; i++)
        {
            if (_entries[i].key == key)
            {
                _entries[i].value = value;
                return i;
            }
        }

        // 2. Добавление нового
        if (_count < _capacity)
        {
            _entries[_count].key = key;
            _entries[_count].value = value;
            return _count++;
        }

        return -1; // Место закончилось
    }

    // Быстрое получение значения
    T get(const String &key, T defaultValue = T()) const
    {
        std::lock_guard<std::recursive_mutex> lock(_mtx);
        for (int i = 0; i < _count; i++)
        {
            if (_entries[i].key == key)
                return _entries[i].value;
        }
        return defaultValue;
    }

    int getIndex(const String &key) const
    {
        std::lock_guard<std::recursive_mutex> lock(_mtx);
        for (int i = 0; i < _count; i++)
        {
            if (_entries[i].key == key)
                return i;
        }
        return -1; // Не найдено
    }

    // Удаление элемента (схлопывание массива для экономии места)
    bool remove(const String &key)
    {
        std::lock_guard<std::recursive_mutex> lock(_mtx);
        for (int i = 0; i < _count; i++)
        {
            if (_entries[i].key == key)
            {
                // Заменяем удаляемый элемент самым последним
                _entries[i] = _entries[_count - 1];
                // Очищаем последний
                _entries[_count - 1].key = "";
                _entries[_count - 1].value = T();
                _count--;
                return true;
            }
        }
        return false;
    }

    // Полезно для циклов сохранения (например, в NVS или JSON)
    int size() const { return _count; }

    // Возвращает строку от "0" до "9", если больше 9 — вернет "9" (для UI)
    String getSizeSmall() const {
        std::lock_guard<std::recursive_mutex> lock(_mtx);
        if (size > 9) return "9";
        return String(size);
    }

    int capacity() const { return _capacity; }

    String getKeyAt(int index) const
    {
        std::lock_guard<std::recursive_mutex> lock(_mtx);
        return (index >= 0 && index < _count) ? _entries[index].key : "";
    }

    T getValueAt(int index) const
    {
        std::lock_guard<std::recursive_mutex> lock(_mtx);
        return (index >= 0 && index < _count) ? _entries[index].value : T();
    }

    void clear()
    {
        std::lock_guard<std::recursive_mutex> lock(_mtx);
        for (int i = 0; i < _count; i++)
        {
            _entries[i].key = "";
            _entries[i].value = T();
        }
        _count = 0;
    }
};

#endif