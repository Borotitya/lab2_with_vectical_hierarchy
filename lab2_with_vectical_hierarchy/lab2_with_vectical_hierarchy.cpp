#include <Windows.h>
#include <iostream>

using namespace std;

#define KEY_DOWN(vk_code) ((GetAsyncKeyState(vk_code) & 0x8000) ? 1 : 0)

void show_body(bool visible, int x, int y, int width, int height);
void show_roof(bool visible, int x, int y, int width, int height);
void show_door(bool visible, int x, int y, int width, int height);
void show_blade(bool visible, int x, int y, int width, int height);
void show_human(bool visible, int x, int y);
void show_ram(bool visible, int x, int y, int width, int height, int wheelRadius);
void show_broken_body(bool visible, int x, int y, int width, int height);
void show_repaired_tower(bool visible, int x, int y, int width, int height);

void hide_body(bool visible, int x, int y, int width, int height);
void hide_roof(bool visible, int x, int y, int width, int height);
void hide_door(bool visible, int x, int y, int width, int height);
void hide_blade(bool visible, int x, int y, int width, int height);
void hide_human(bool visible, int x, int y);
void hide_ram(bool visible, int x, int y, int width, int height, int wheelRadius);
void hide_broken_body(bool visible, int x, int y, int width, int height);
void hide_repaired_tower(bool visible, int x, int y, int width, int height);

HDC hdc;

HWND GetConsoleWindow() {
    WCHAR str[128];
    WCHAR title[] = L"Console Window";

    GetConsoleTitle(str, sizeof(str) / sizeof(str[0])); // Получить текущий заголовок окна
    SetConsoleTitle(title); // Установить новый заголовок окна
    Sleep(100); // Ждем изменения заголовка окна (100 мс)

    HWND hwnd = FindWindow(NULL, title); // Определяем дескриптор окна

    SetConsoleTitle(str); // Возвращаем прежний заголовок

    return hwnd; // Вернуть дескриптор окна
}

class Location {
protected:
    int x;
    int y;
public:
    Location(int init_x, int init_y) {
        x = init_x;
        y = init_y;
    }
    virtual ~Location() {}
    int get_x() const { return x; }
    int get_y() const { return y; }
    void set_x(int X) { x = X; }
    void set_y(int Y) { y = Y; }
};

class Point : public Location {
protected:
    bool visible;
public:
    Point(int init_x, int init_y) : Location(init_x, init_y) {
        visible = false;
    }
    virtual ~Point() {}
    virtual void show() {
        visible = true;
        HPEN Pen = CreatePen(PS_SOLID, 2, RGB(0, 0, 0));
        SelectObject(hdc, Pen);
        SetPixel(hdc, x, y, RGB(255, 0, 0)); // черный
        SetPixel(hdc, x + 1, y, RGB(255, 0, 0)); // 4 точки для удобства
        SetPixel(hdc, x, y + 1, RGB(255, 0, 0));
        SetPixel(hdc, x + 1, y + 1, RGB(255, 0, 0));
        DeleteObject(Pen);
    }
    virtual void hide() {
        visible = false;
        SetPixel(hdc, x, y, RGB(255, 255, 255)); // белый
        SetPixel(hdc, x + 1, y, RGB(255, 255, 255)); // 4 точки для удобства
        SetPixel(hdc, x, y + 1, RGB(255, 255, 255));
        SetPixel(hdc, x + 1, y + 1, RGB(255, 255, 255));
    }
    bool is_visible() const { return visible; }
    void move_to(int new_x, int new_y) {
        hide();
        x = new_x;
        y = new_y;
        show();
    }
};

class WindMill : public Point {
protected:
    int width;
    int height;
public:
    WindMill(int init_x, int init_y, int init_w, int init_h) : Point(init_x, init_y), width(init_w), height(init_h) {}
    virtual ~WindMill() {}
    virtual void show() override {
        show_body(true, x, y, width, height);
        show_roof(true, x, y, width, height);
        show_door(true, x, y, width, height);
        show_blade(true, x, y, width, height);
    }
    virtual void hide() override {
        hide_body(true, x, y, width, height);
        hide_roof(true, x, y, width, height);
        hide_door(true, x, y, width, height);
        hide_blade(true, x, y, width, height);
    }
    bool collide_with_ram(int ram_x, int ram_y, int ram_width, int ram_height) {
        int tower_left = x - width / 2;
        int tower_right = x + width / 2;
        int tower_top = y - height / 2;
        int tower_bottom = y + height / 2;

        int ram_left = ram_x - ram_width / 2;
        int ram_right = ram_x + ram_width / 2;
        int ram_top = ram_y - ram_height / 2;
        int ram_bottom = ram_y + ram_height / 2;

        bool horizontal_overlap = (tower_left < ram_right && tower_right > ram_left);
        bool vertical_overlap = tower_top < ram_bottom && tower_bottom > ram_top;

        return horizontal_overlap && vertical_overlap;
    }

    bool collide_with_human(int human_x, int human_y) {
        int tower_left = x - width / 2;
        int tower_right = x + width / 2;
        int tower_top = y - height / 2;
        int tower_bottom = y + height / 2;

        int human_left = human_x - 10;
        int human_right = human_x + 10;
        int human_top = human_y - 20;
        int human_bottom = human_y + 20;

        return human_left < tower_right && human_right > tower_left && human_top < tower_bottom && human_bottom > tower_top;
    }
    virtual WindMill* transition_to_broken();
    virtual WindMill* transition_to_repaired();
    virtual WindMill* transition_to_windmill();
};

class BrokenWindMill : public WindMill {
public:
    BrokenWindMill(int init_x, int init_y, int init_w, int init_h) : WindMill(init_x, init_y, init_w, init_h) {}
    void show() override {
        show_broken_body(true, x, y, width, height);
        show_roof(true, x, y, width, height);
        show_door(true, x, y, width, height);
        show_blade(true, x, y, width, height);
    }
    void hide() override {
        hide_broken_body(true, x, y, width, height);
        hide_roof(true, x, y, width, height);
        hide_door(true, x, y, width, height);
        hide_blade(true, x, y, width, height);
    }
    WindMill* transition_to_repaired() override;
    WindMill* transition_to_windmill() override;
};

class RepairedWindMill : public BrokenWindMill {
public:
    RepairedWindMill(int init_x, int init_y, int init_w, int init_h) : BrokenWindMill(init_x, init_y, init_w, init_h) {}
    void show() override {
        show_repaired_tower(true, x, y, width, height);
        show_roof(true, x, y, width, height);
        show_door(true, x, y, width, height);
        show_blade(true, x, y, width, height);
    }
    void hide() override {
        hide_repaired_tower(true, x, y, width, height);
        hide_roof(true, x, y, width, height);
        hide_door(true, x, y, width, height);
        hide_blade(true, x, y, width, height);
    }
    WindMill* transition_to_windmill() override;
};

WindMill* WindMill::transition_to_broken() {
    return new BrokenWindMill(x, y, width, height);
}

WindMill* WindMill::transition_to_repaired() {
    return new RepairedWindMill(x, y, width, height);
}

WindMill* WindMill::transition_to_windmill() {
    return new WindMill(x, y, width, height);
}

WindMill* BrokenWindMill::transition_to_repaired() {
    return new RepairedWindMill(x, y, width, height);
}

WindMill* BrokenWindMill::transition_to_windmill() {
    return new WindMill(x, y, width, height);
}

WindMill* RepairedWindMill::transition_to_windmill() {
    return new WindMill(x, y, width, height);
}

class Obstacle : public Point {
public:
    Obstacle(int init_x, int init_y) : Point(init_x, init_y) {
        visible = false;
    }
    ~Obstacle() {}
};

class Human : public Obstacle {
public:
    Human(int init_x, int init_y) : Obstacle(init_x, init_y) {
        visible = false;
    }
    ~Human() {}
    void show() override {
        show_human(visible, x, y);
    }
    void hide() override {
        hide_human(visible, x, y);
    }
};

class Ram : public Obstacle {
protected:
    int width;
    int height;
    int wheel_radius;

public:
    Ram(int init_x, int init_y, int init_width, int init_height, int init_wheel_radius)
        : Obstacle(init_x, init_y), width(init_width), height(init_height), wheel_radius(init_wheel_radius) {
        visible = false;
    }

    ~Ram() {}

    void show() override {
        show_ram(visible, x, y, width, height, wheel_radius);
    }

    void hide() override {
        hide_ram(visible, x, y, width, height, wheel_radius);
    }
};

void show_broken_body(bool visible, int x, int y, int width, int height) {
    visible = true;
    HPEN Pen = CreatePen(PS_SOLID, 2, RGB(0, 0, 0));
    SelectObject(hdc, Pen);
    int left = x - width / 2; // задание координат
    int top = y - height / 2;
    int right = x + width / 2;
    int bottom = y + height / 2;
    MoveToEx(hdc, x + width / 2, y - height / 2, NULL);
    LineTo(hdc, left, top);
    LineTo(hdc, left, bottom);
    LineTo(hdc, right, bottom);
    MoveToEx(hdc, x + width / 2, y - height / 2, NULL);
    LineTo(hdc, x + width / 4, y + height / 8);
    LineTo(hdc, right, bottom);
    DeleteObject(Pen);
}

void hide_broken_body(bool visible, int x, int y, int width, int height) {
    visible = true;
    HPEN Pen = CreatePen(PS_SOLID, 2, RGB(255, 255, 255));
    SelectObject(hdc, Pen);
    int left = x - width / 2; // задание координат
    int top = y - height / 2;
    int right = x + width / 2;
    int bottom = y + height / 2;
    MoveToEx(hdc, x + width / 2, y - height / 2, NULL);
    LineTo(hdc, left, top);
    LineTo(hdc, left, bottom);
    LineTo(hdc, right, bottom);
    MoveToEx(hdc, x + width / 2, y - height / 2, NULL);
    LineTo(hdc, x + width / 4, y + height / 8);
    LineTo(hdc, right, bottom);
    DeleteObject(Pen);
}

void show_repaired_tower(bool visible, int x, int y, int width, int height) {
    visible = true;
    HPEN Pen = CreatePen(PS_SOLID, 2, RGB(0, 0, 0)); // черный цвет
    SelectObject(hdc, Pen);
    int left = x - width / 2; // задание координат
    int top = y - height / 2;
    int right = x + width / 2;
    int bottom = y + height / 2;
    Rectangle(hdc, left, top, right, bottom); // отрисовка прямоугольника
    Rectangle(hdc, right, top + height / 3, right + height / 3, bottom);
    Rectangle(hdc, right + width / 6, top + height / 2, right + height / 3 - width / 6, bottom - height / 8);
    DeleteObject(Pen);
}

void hide_repaired_tower(bool visible, int x, int y, int width, int height) {
    visible = true;
    HPEN Pen = CreatePen(PS_SOLID, 2, RGB(255, 255, 255)); // белый цвет
    SelectObject(hdc, Pen);
    int left = x - width / 2; // задание координат
    int top = y - height / 2;
    int right = x + width / 2;
    int bottom = y + height / 2;
    Rectangle(hdc, left, top, right, bottom); // отрисовка прямоугольника
    Rectangle(hdc, right, top + height / 3, right + height / 3, bottom);
    Rectangle(hdc, right + width / 6, top + height / 2, right + height / 3 - width / 6, bottom - height / 8);
    DeleteObject(Pen);
}

void show_body(bool visible, int x, int y, int width, int height) {
    visible = true;
    HPEN Pen = CreatePen(PS_SOLID, 2, RGB(0, 0, 0)); // черный цвет
    SelectObject(hdc, Pen);
    int left = x - width / 2; // задание координат
    int top = y - height / 2;
    int right = x + width / 2;
    int bottom = y + height / 2;
    Rectangle(hdc, left, top, right, bottom); // отрисовка прямоугольника
    DeleteObject(Pen);
}

void show_roof(bool visible, int x, int y, int width, int height) {
    visible = true;
    HPEN Pen = CreatePen(PS_SOLID, 2, RGB(0, 0, 0)); // черный цвет
    SelectObject(hdc, Pen);
    POINT vertices[3]; // массив для хранения точек
    vertices[0].x = x + width / 2; // определение координат точек
    vertices[0].y = y - height / 2;
    vertices[1].x = x;
    vertices[1].y = y - height;
    vertices[2].x = x - width / 2;
    vertices[2].y = y - height / 2;
    Polygon(hdc, vertices, 3); // отрисовка крыши башни
    DeleteObject(Pen);
}

void show_blade(bool visible, int x, int y, int width, int height) {
    visible = true;
    HPEN hPen = CreatePen(PS_SOLID, 15, RGB(0, 0, 0)); // Черная ручка
    SelectObject(hdc, hPen);

    // Координаты верхнего центра крыши
    double roofTopCenterX = x + width / 16.6;
    double roofTopCenterY = y - height / 1.5;

    // Длина лопасти от верхнего центра до конца
    int bladeLength = height / 2;

    // Рисуем лопасти в четырех направлениях от верхнего центра крыши
    for (int i = 0; i < 4; i++) {
        double angle = 3.14159265358979323846 * i / 2; // Угол в радианах для каждой лопасти
        int bladeEndX = roofTopCenterX + cos(angle) * bladeLength; // Координаты конца лопасти
        int bladeEndY = roofTopCenterY - sin(angle) * bladeLength; // Координаты конца лопасти
        MoveToEx(hdc, roofTopCenterX, roofTopCenterY, NULL); // Перемещаем ручку в начало лопасти
        LineTo(hdc, bladeEndX, bladeEndY); // Рисуем лопасть
    }

    DeleteObject(hPen); // Удаляем ручку
}

void hide_blade(bool visible, int x, int y, int width, int height) {
    visible = false;
    HPEN hPen = CreatePen(PS_SOLID, 15, GetSysColor(COLOR_WINDOW));
    SelectObject(hdc, hPen);
    SelectObject(hdc, hPen);
    HBRUSH hBrush = CreateSolidBrush(GetSysColor(COLOR_WINDOW));
    SelectObject(hdc, hBrush);

    // Координаты верхнего центра крыши
    double roofTopCenterX = x + width / 16.6;
    double roofTopCenterY = y - height / 1.5;

    // Длина лопасти от верхнего центра до конца
    int bladeLength = height;

    // Рисуем лопасти в четырех направлениях от верхнего центра крыши
    for (int i = 0; i < 4; i++) {
        double angle = 3.14159265358979323846 * i / 2; // Угол в радианах для каждой лопасти
        int bladeEndX = roofTopCenterX + cos(angle) * bladeLength;
        int bladeEndY = roofTopCenterY - sin(angle) * bladeLength;
        MoveToEx(hdc, roofTopCenterX, roofTopCenterY, NULL);
        LineTo(hdc, bladeEndX, bladeEndY);
    }

    DeleteObject(hPen); // Удаляем ручку
    DeleteObject(hBrush);
}

void show_door(bool visible, int x, int y, int width, int height) {
    visible = true;
    HPEN Pen = CreatePen(PS_SOLID, 2, RGB(0, 0, 0)); // черный цвет
    SelectObject(hdc, Pen);
    int left = x - width / 4; // координаты двери
    int top = y - height / 16 + height / 8;
    int right = x + width / 4;
    int bottom = y + height / 2;
    Rectangle(hdc, left, top, right, bottom); // отрисовка двери
    MoveToEx(hdc, x + width / 6, (top + bottom) / 2, NULL); // двигаемся в координаты ручки
    LineTo(hdc, x + width / 12, (top + bottom) / 2); // отрисовка ручки двери
    DeleteObject(Pen);
}

void hide_body(bool visible, int x, int y, int width, int height) {
    visible = false;
    HPEN Pen = CreatePen(PS_SOLID, 2, RGB(255, 255, 255)); // черный цвет
    SelectObject(hdc, Pen);
    int left = x - width / 2; // задание координат
    int top = y - height / 2;
    int right = x + width / 2;
    int bottom = y + height / 2;
    Rectangle(hdc, left, top, right, bottom); // отрисовка прямоугольника
    DeleteObject(Pen);
}

void hide_window(bool visible, int x, int y, int width, int height) {
    visible = false;
    HPEN Pen = CreatePen(PS_SOLID, 2, RGB(255, 255, 255)); // черный цвет
    SelectObject(hdc, Pen);
    int left = x - width / 6;
    int top = y - height / 8 - height / 4;
    int right = x + width / 6;
    int bottom = y + height / 8 - height / 4;
    Rectangle(hdc, left, top, right, bottom); // отрисовка прямоугольника
    DeleteObject(Pen);
}

void hide_roof(bool visible, int x, int y, int width, int height) {
    visible = false;
    HPEN Pen = CreatePen(PS_SOLID, 2, RGB(255, 255, 255)); // черный цвет
    SelectObject(hdc, Pen);
    POINT vertices[3]; // массив для хранения точек
    vertices[0].x = x + width / 2; // определение координат точек
    vertices[0].y = y - height / 2;
    vertices[1].x = x;
    vertices[1].y = y - height;
    vertices[2].x = x - width / 2;
    vertices[2].y = y - height / 2;
    Polygon(hdc, vertices, 3); // отрисовка крыши башни
    DeleteObject(Pen);
}

void hide_door(bool visible, int x, int y, int width, int height) {
    visible = false;
    HPEN Pen = CreatePen(PS_SOLID, 2, RGB(255, 255, 255)); // черный цвет
    SelectObject(hdc, Pen);
    int left = x - width / 4; // координаты двери
    int top = y - height / 16 + height / 8;
    int right = x + width / 4;
    int bottom = y + height / 2;
    Rectangle(hdc, left, top, right, bottom); // отрисовка двери
    MoveToEx(hdc, x + width / 6, (top + bottom) / 2, NULL); // двигаемся в координаты ручки
    LineTo(hdc, x + width / 12, (top + bottom) / 2); // отрисовка ручки двери
    DeleteObject(Pen);
}

void show_human(bool visible, int x, int y) {
    int head_radius = 15;
    visible = true;
    HPEN Pen = CreatePen(PS_SOLID, 2, RGB(0, 0, 0));
    SelectObject(hdc, Pen);
    MoveToEx(hdc, x, y + 5, NULL);
    LineTo(hdc, x, y + 30);

    MoveToEx(hdc, x, y + 5, NULL);
    LineTo(hdc, x - 15, y + 20);

    MoveToEx(hdc, x, y + 5, NULL);
    LineTo(hdc, x + 15, y + 20);

    MoveToEx(hdc, x, y + 30, NULL);
    LineTo(hdc, x - 10, y + 40);

    MoveToEx(hdc, x, y + 30, NULL);
    LineTo(hdc, x + 10, y + 40);

    Ellipse(hdc, x + head_radius, y + head_radius - 10, x - head_radius, y - head_radius - 10);

    MoveToEx(hdc, x - head_radius / 3, y - head_radius, NULL);
    LineTo(hdc, x - head_radius / 4, y - head_radius - 2);

    MoveToEx(hdc, x + head_radius / 3, y - head_radius, NULL);
    LineTo(hdc, x + head_radius / 4, y - head_radius - 2);

    MoveToEx(hdc, x - head_radius / 4, y - 3, NULL);
    LineTo(hdc, x + head_radius / 4, y - 3);

    DeleteObject(Pen);
}

void hide_human(bool visible, int x, int y) {
    int head_radius = 15;
    visible = true;
    HPEN Pen = CreatePen(PS_SOLID, 2, RGB(255, 255, 255));
    SelectObject(hdc, Pen);
    MoveToEx(hdc, x, y + 5, NULL);
    LineTo(hdc, x, y + 30);

    MoveToEx(hdc, x, y + 5, NULL);
    LineTo(hdc, x - 15, y + 20);

    MoveToEx(hdc, x, y + 5, NULL);
    LineTo(hdc, x + 15, y + 20);

    MoveToEx(hdc, x, y + 30, NULL);
    LineTo(hdc, x - 10, y + 40);

    MoveToEx(hdc, x, y + 30, NULL);
    LineTo(hdc, x + 10, y + 40);

    Ellipse(hdc, x + head_radius, y + head_radius - 10, x - head_radius, y - head_radius - 10);

    MoveToEx(hdc, x - head_radius / 3, y - head_radius, NULL);
    LineTo(hdc, x - head_radius / 4, y - head_radius - 2);

    MoveToEx(hdc, x + head_radius / 3, y - head_radius, NULL);
    LineTo(hdc, x + head_radius / 4, y - head_radius - 2);

    MoveToEx(hdc, x - head_radius / 4, y - 3, NULL);
    LineTo(hdc, x + head_radius / 4, y - 3);

    DeleteObject(Pen);
}

void show_ram(bool visible, int x, int y, int width, int height, int wheel_radius) {
    visible = true;
    HPEN Pen = CreatePen(PS_SOLID, 2, RGB(0, 0, 0)); // черный цвет
    SelectObject(hdc, Pen);
    Rectangle(hdc, x - width / 2, y + height / 2, x + width + width / 4, y + height / 4);
    DeleteObject(Pen);
    SelectObject(hdc, Pen);
    Ellipse(hdc, x - wheel_radius, y + height / 2 - wheel_radius, x, y + height / 2 + wheel_radius);
    SelectObject(hdc, Pen);
    Ellipse(hdc, x + width - wheel_radius, y + height / 2 - wheel_radius, x + width, y + height / 2 + wheel_radius);
    DeleteObject(Pen);
}

void hide_ram(bool visible, int x, int y, int width, int height, int wheel_radius) {
    visible = false;
    HPEN Pen = CreatePen(PS_SOLID, 2, RGB(255, 255, 255)); // белый цвет
    SelectObject(hdc, Pen);
    Rectangle(hdc, x - width / 2, y + height / 2, x + width + width / 4, y + height / 4);
    DeleteObject(Pen);
    SelectObject(hdc, Pen);
    Ellipse(hdc, x - wheel_radius, y + height / 2 - wheel_radius, x, y + height / 2 + wheel_radius);
    SelectObject(hdc, Pen);
    Ellipse(hdc, x + width - wheel_radius, y + height / 2 - wheel_radius, x + width, y + height / 2 + wheel_radius);
    DeleteObject(Pen);
}

int main() {
    int x0 = 500; // координаты объекта
    int y0 = 300;
    int width0 = 200; // ширина и высота объекта
    int height0 = 400;
    int wheel0 = 50;
    int collide_ram = 0;
    int collide_human = 0;
    HWND hwnd = GetConsoleWindow(); // получение дескриптора окна

    if (hwnd != NULL) {
        hdc = GetWindowDC(hwnd); // получение контекста
        if (hdc != 0) {
            RECT rect;
            GetClientRect(hwnd, &rect); // получаем размеры окна
            int win_width = rect.right;
            int win_height = rect.bottom;

            WindMill* windmill = new WindMill(x0, y0, width0, height0);
            Point point(x0, y0);
            Human human(x0 + 400, y0 + 200);  // расположено дальше вправо
            Ram ram(x0 - 300, y0 + 200, width0, height0, wheel0);  // расположено в видимой области

            human.show();
            ram.show();

            while (1) {
                if (windmill->collide_with_ram(ram.get_x(), ram.get_y(), width0, height0)) {
                    WindMill* new_windmill = windmill->transition_to_broken();
                    delete windmill;
                    windmill = new_windmill;
                    windmill->show();
                }
                if (windmill->collide_with_human(human.get_x(), human.get_y())) {
                    WindMill* new_windmill = windmill->transition_to_repaired();
                    delete windmill;
                    windmill = new_windmill;
                    windmill->show();
                }
                if (KEY_DOWN(49)) {
                    windmill->show();
                }
                if (KEY_DOWN(50)) {
                    windmill->hide();
                }
                if (KEY_DOWN(82)) {  // кнопка R для возвращения в состояние WindMill
                    WindMill* new_windmill = windmill->transition_to_windmill();
                    delete windmill;
                    windmill = new_windmill;
                    windmill->show();
                }
                if (KEY_DOWN(37)) {
                    windmill->move_to(windmill->get_x() - 1, windmill->get_y());
                    ram.show();
                    human.show();
                }
                if (KEY_DOWN(38)) {
                    windmill->move_to(windmill->get_x(), windmill->get_y() - 1);
                    ram.show();
                    human.show();
                }
                if (KEY_DOWN(39)) {
                    windmill->move_to(windmill->get_x() + 1, windmill->get_y());
                    ram.show();
                    human.show();
                }
                if (KEY_DOWN(40)) {
                    windmill->move_to(windmill->get_x(), windmill->get_y() + 1);
                    ram.show();
                    human.show();
                }
                if (KEY_DOWN(51)) {
                    delete windmill;
                    break;
                }
            }
        }
    }
    ReleaseDC(hwnd, hdc);
    return 0;
}
