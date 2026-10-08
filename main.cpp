#include <iostream>
#include <string>
#include <utility>
#include <vector>
#include <iomanip>

using namespace std;


struct Product {
    int code{};
    string name;
    double price{};
};

struct Node {
    Product data;
    Node *next;

    explicit Node(Product prod, Node *nextNode = nullptr) : data(std::move(prod)), next(nextNode) {
    }
};

class HashTable {
    vector<Node *> buckets;
    size_t capacity;
    size_t count;
    const double MAX_LOAD_FACTOR = 0.75;

    static size_t hashFunction(const int key, const size_t mod) {
        return static_cast<size_t>(key) % mod;
    }

    void rehash() {
        const size_t oldCapacity = capacity;
        const vector<Node *> oldBuckets = buckets;

        capacity = oldCapacity * 2;
        buckets.assign(capacity, nullptr);

        for (size_t i = 0; i < oldCapacity; ++i) {
            Node *current = oldBuckets[i];
            while (current != nullptr) {
                Node *nextNode = current->next;
                const size_t newIndex = hashFunction(current->data.code, capacity);

                current->next = buckets[newIndex];
                buckets[newIndex] = current;

                current = nextNode;
            }
        }
    }

    void clear() {
        for (size_t i = 0; i < capacity; ++i) {
            const Node *current = buckets[i];
            while (current != nullptr) {
                const Node *temp = current;
                current = current->next;
                delete temp;
            }
            buckets[i] = nullptr;
        }
        count = 0;
    }

public:
    explicit HashTable(const size_t initialCapacity = 10) : capacity(initialCapacity), count(0) {
        buckets.assign(capacity, nullptr);
    }

    ~HashTable() {
        clear();
    }

    bool insert(const Product &prod) {
        if (search(prod.code) != nullptr) {
            cout << "Ошибка: Товар с кодом " << prod.code << " уже существует в таблице!\n";
            return false;
        }

        if (static_cast<double>(count + 1) / capacity >= MAX_LOAD_FACTOR) {
            rehash();
        }

        const size_t index = hashFunction(prod.code, capacity);
        const auto newNode = new Node(prod, buckets[index]);
        buckets[index] = newNode;
        count++;

        return true;
    }

    bool remove(const int key) {
        const size_t index = hashFunction(key, capacity);
        Node *current = buckets[index];
        Node *prev = nullptr;

        while (current != nullptr) {
            if (current->data.code == key) {
                if (prev == nullptr) {
                    buckets[index] = current->next;
                } else {
                    prev->next = current->next;
                }
                delete current;
                count--;
                return true;
            }
            prev = current;
            current = current->next;
        }
        return false;
    }

    [[nodiscard]] Product *search(const int key) const {
        const size_t index = hashFunction(key, capacity);
        Node *current = buckets[index];

        while (current != nullptr) {
            if (current->data.code == key) {
                return &(current->data);
            }
            current = current->next;
        }
        return nullptr;
    }

    void display() const {
        cout << "\n================= ТЕКУЩЕЕ СОСТОЯНИЕ ХЕШ-ТАБЛИЦЫ =================" << endl;
        cout << "Вместимость бакетов (m): " << capacity
                << " | Записей всего (n): " << count
                << " | Коэффициент нагрузки (n/m): " << fixed << setprecision(2)
                << (static_cast<double>(count) / capacity) << "\n";
        cout << "-----------------------------------------------------------------" << endl;

        for (size_t i = 0; i < capacity; ++i) {
            cout << "Строка [" << i << "]: ";

            if (const Node *current = buckets[i]; current == nullptr) {
                cout << "[ ПУСТО ]\n";
            } else {
                while (current != nullptr) {
                    cout << "-> (Код: " << current->data.code
                            << ", Название: \"" << current->data.name
                            << "\", Цена: " << fixed << setprecision(2) << current->data.price << " руб.) ";
                    current = current->next;
                }
                cout << endl;
            }
        }
        cout << "=================================================================\n\n";
    }
};

void printMenu() {
    cout << "\n------------------- МЕНЮ -------------------" << endl;
    cout << "1. Отобразить хеш-таблицу" << endl;
    cout << "2. Поиск товара по шестизначному коду" << endl;
    cout << "3. Добавить новую запись" << endl;
    cout << "4. Удалить запись по коду" << endl;
    cout << "0. Выход" << endl;
    cout << "Выберите действие: ";
}

int main() {
    HashTable table(10);

    table.insert({100023, "Мышь игровая", 2490.0}); // Строка 3
    table.insert({100093, "Клавиатура мех.", 5990.0}); // Строка 3 (коллизия)
    table.insert({100150, "Монитор 27\"", 22400.0}); // Строка 0
    table.insert({100207, "Ноутбук", 78900.0}); // Строка 7
    table.insert({100345, "Наушники", 3450.0}); // Строка 5
    table.insert({100411, "Коврик для мыши", 800.0}); // Строка 1
    table.insert({100518, "Микрофон USB", 6500.0}); // Строка 8

    int choice;
    do {
        printMenu();
        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(10000, '\n');
            continue;
        }

        switch (choice) {
            case 1: {
                table.display();
                break;
            }
            case 2: {
                cout << "Введите шестизначный код товара для поиска: ";
                int code;
                cin >> code;
                if (const Product *item = table.search(code)) {
                    cout << ">>> Найдено: [Код: " << item->code
                            << " | Товар: " << item->name
                            << " | Цена: " << fixed << setprecision(2) << item->price << " руб.]\n";
                } else {
                    cout << ">>> Товар с кодом " << code << " не найден.\n";
                }
                break;
            }
            case 3: {
                Product p;
                cout << "Введите шестизначный код товара: ";
                cin >> p.code;
                cout << "Введите наименование товара: ";
                cin.ignore();
                getline(cin, p.name);
                cout << "Введите цену товара: ";
                cin >> p.price;

                if (table.insert(p)) {
                    cout << ">>> Запись успешно добавлена!\n";
                }
                break;
            }
            case 4: {
                cout << "Введите код товара для удаления: ";
                int code;
                cin >> code;
                if (table.remove(code)) {
                    cout << ">>> Товар успешно удален из таблицы!\n";
                } else {
                    cout << ">>> Товар с таким кодом не найден.\n";
                }
                break;
            }
            case 0:
                cout << "Завершение работы программы.\n";
                break;
            default:
                cout << "Неверный пункт меню. Попробуйте еще раз.\n";
        }
    } while (choice != 0);

    return 0;
}
