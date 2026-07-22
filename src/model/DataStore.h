#pragma once
#include <Arduino.h>
#include <mutex>

template <typename T>
class DataStore {
private:
    T *items;
    int capacity;
    int head;   // Индекс самого старого элемента
    int tail;   // Индекс для новой записи
    int size;
    T lastItem; // Тот самый "последний след", выживает после clear()
    mutable std::recursive_mutex mtx;

public:
    DataStore(int _capacity) : capacity(_capacity), head(0), tail(0), size(0) {
        items = new T[capacity](); 
        lastItem = T(); // Инициализация нулями (через конструктор по умолчанию)
    }

    ~DataStore() {
        delete[] items;
    }

    // Добавить элемент в очередь
    void enqueue(T item) {
        std::lock_guard<std::recursive_mutex> lock(mtx);
        
        lastItem = item; // Обновляем "последний след"

        if (size == capacity) {
            // Если переполнено, сдвигаем голову (затираем самое старое)
            head = (head + 1) % capacity;
            size--;
        }

        items[tail] = item;
        tail = (tail + 1) % capacity;
        size++;
    }

    // Извлечь самый старый элемент (с удалением)
    bool pop(T &output) {
        std::lock_guard<std::recursive_mutex> lock(mtx);
        if (size == 0) return false;

        output = items[head];
        head = (head + 1) % capacity;
        size--;
        return true;
    }

    // Быстро удалить count элементов из головы очереди (без лишнего копирования)
    void drop(int count) {
        std::lock_guard<std::recursive_mutex> lock(mtx);
        if (count <= 0 || size == 0) return;
        
        // Защита: если просят удалить больше, чем есть — просто очищаем всё
        if (count >= size) {
            clear();
            return;
        }

        head = (head + count) % capacity;
        size -= count;
    }

    // Получить "последний след" без удаления из основной очереди
    T getLast() const {
        std::lock_guard<std::recursive_mutex> lock(mtx);
        return lastItem;
    }

    // Текущее количество элементов
    int getSize() const {
        std::lock_guard<std::recursive_mutex> lock(mtx);
        return size;
    }

    // Возвращает строку от "0" до "9", если больше 9 — вернет "9" (для UI)
    String getSizeSmall() const {
        std::lock_guard<std::recursive_mutex> lock(mtx);
        if (size > 9) return "9";
        return String(size);
    }
    
    // Очистить очередь (последний след при этом сохраняется)
    void clear() {
        std::lock_guard<std::recursive_mutex> lock(mtx);
        head = 0;
        tail = 0;
        size = 0;
    }

    // Посмотреть элемент по индексу без удаления (0 - самый старый)
    bool peek(int index, T &output) const {
        std::lock_guard<std::recursive_mutex> lock(mtx);
        if (index < 0 || index >= size) return false;
        int realIdx = (head + index) % capacity;
        output = items[realIdx];
        return true;
    }
};