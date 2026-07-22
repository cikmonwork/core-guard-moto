#ifndef LOCATIONS_STORE_H
#define LOCATIONS_STORE_H

#include <Arduino.h>
#include <model/classes_app.h>
#include <mutex>

/**
 * Класс LocationsStore: Потокобезопасное хранилище координат.
 * Работает по принципу кольцевого буфера.
 */
class LocationsStore {
private:
    Location *locations;
    int capacity;
    int head;   // Индекс самого старого элемента
    int tail;   // Индекс для новой записи
    int size;
    Location lastKnown; // Хранит последнюю добавленную координату навсегда
    mutable std::recursive_mutex mtx; // Защита для работы в многозадачности

public:
    // Конструктор
    LocationsStore(int capacity) : capacity(capacity), head(0), tail(0), size(0) {
        locations = new Location[capacity]();
        lastKnown = Location();
    }

    // Деструктор
    ~LocationsStore() {
        delete[] locations;
    }

    // Добавить новую локацию в очередь
    void enqueue(Location loc) {
        std::lock_guard<std::recursive_mutex> lock(mtx);

        lastKnown = loc;

        if (size == capacity) {
            // Если место закончилось, сдвигаем голову (затираем самое старое)
            head = (head + 1) % capacity;
            size--;
        }

        locations[tail] = loc; 
        tail = (tail + 1) % capacity;
        size++;
    }

    // Забрать самую старую локацию (для отправки)
    bool dequeue(Location &output) {
        std::lock_guard<std::recursive_mutex> lock(mtx);

        if (size == 0) return false;

        output = locations[head];
        // Для простых структур как Location (только числа), очистка памяти не обязательна,
        // но для порядка обнуляем
        locations[head] = Location(); 

        head = (head + 1) % capacity;
        size--;
        return true;
    }

    // Посмотреть локацию по индексу (без удаления из очереди)
    // Полезно, если нужно отправить пачкой, не удаляя по одной сразу
    bool peek(int index, Location &output) const {
        std::lock_guard<std::recursive_mutex> lock(mtx);
        if (index >= size) return false;

        int realIdx = (head + index) % capacity;
        output = locations[realIdx];
        return true;
    }

   // Посмотреть самую новую локацию (последнюю добавленную)
    bool peekLast(Location &output) const {
        std::lock_guard<std::recursive_mutex> lock(mtx);
        
        if (size == 0) return false;

        // Вычисляем индекс последнего добавленного элемента
        int lastIdx = (tail - 1 + capacity) % capacity;
        output = locations[lastIdx];
        
        return true;
    }

    // Получить последнии координаты
    Location getLast() const {
        std::lock_guard<std::recursive_mutex> lock(mtx);
        return lastKnown;
    }

    // Получить индекс последней добавленной записи
    int getLastIndex() const {
        std::lock_guard<std::recursive_mutex> lock(mtx);
        
        if (size == 0) return -1;

        // Последняя запись всегда по адресу (tail - 1)
        // Добавляем capacity, чтобы избежать отрицательных чисел при tail = 0
        return (tail - 1 + capacity) % capacity;
    }

    // Удалить несколько элементов сразу (после успешной отправки пачки)
    void drop(int count) {
        std::lock_guard<std::recursive_mutex> lock(mtx);
        if (count > size) count = size;
        
        for(int i = 0; i < count; i++) {
            locations[head] = Location();
            head = (head + 1) % capacity;
            size--;
        }
    }

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

    bool isEmpty() const {
        std::lock_guard<std::recursive_mutex> lock(mtx);
        return size == 0;
    }

    void clear() {
        std::lock_guard<std::recursive_mutex> lock(mtx);
        head = 0;
        tail = 0;
        size = 0;
    }
};

#endif