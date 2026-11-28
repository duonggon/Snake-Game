#pragma once

// ============================================================================
// File: Core/SfmlCompat.h
// Mô tả: Tự động kiểm tra thư viện SFML bằng chuẩn C++17 (__has_include).
//        - Nếu máy đã cài SFML: Sử dụng trực tiếp <SFML/Graphics.hpp> chính thức.
//        - Nếu máy chưa cài SFML: Cung cấp lớp tương thích (Mock SFML API)
//          giúp biên dịch và kiểm thử độc lập toàn bộ logic OOP ngay lập tức.
// ============================================================================

#if defined(__has_include) && __has_include(<SFML/Graphics.hpp>)
    #include <SFML/Graphics.hpp>
    #define SNAKE_BREAKER_HAS_SFML 1
#else
    #define SNAKE_BREAKER_HAS_SFML 0
    #include <cstdint>

namespace sf {

    // Cấu trúc tọa độ 2 chiều tổng quát
    template <typename T>
    struct Vector2 {
        T x{};
        T y{};

        constexpr Vector2() = default;
        constexpr Vector2(T xVal, T yVal) : x(xVal), y(yVal) {}

        constexpr bool operator==(const Vector2<T>& other) const {
            return x == other.x && y == other.y;
        }
        constexpr bool operator!=(const Vector2<T>& other) const {
            return !(*this == other);
        }
        constexpr Vector2<T> operator+(const Vector2<T>& other) const {
            return Vector2<T>(x + other.x, y + other.y);
        }
        constexpr Vector2<T> operator-(const Vector2<T>& other) const {
            return Vector2<T>(x - other.x, y - other.y);
        }
    };

    using Vector2i = Vector2<int>;
    using Vector2f = Vector2<float>;
    using Vector2u = Vector2<unsigned int>;

    // Cấu trúc màu sắc RGBA tương thích sf::Color
    struct Color {
        std::uint8_t r{0};
        std::uint8_t g{0};
        std::uint8_t b{0};
        std::uint8_t a{255};

        constexpr Color() = default;
        constexpr Color(std::uint8_t red, std::uint8_t green, std::uint8_t blue, std::uint8_t alpha = 255)
            : r(red), g(green), b(blue), a(alpha) {}

        static const Color Black;
        static const Color White;
        static const Color Red;
        static const Color Green;
        static const Color Blue;
        static const Color Yellow;
        static const Color Magenta;
        static const Color Cyan;
        static const Color Transparent;
    };

    inline constexpr Color Color::Black{0, 0, 0, 255};
    inline constexpr Color Color::White{255, 255, 255, 255};
    inline constexpr Color Color::Red{255, 0, 0, 255};
    inline constexpr Color Color::Green{0, 255, 0, 255};
    inline constexpr Color Color::Blue{0, 120, 255, 255};
    inline constexpr Color Color::Yellow{255, 215, 0, 255};
    inline constexpr Color Color::Magenta{255, 0, 255, 255};
    inline constexpr Color Color::Cyan{0, 255, 255, 255};
    inline constexpr Color Color::Transparent{0, 0, 0, 0};

    // Hình chữ nhật dùng để vẽ đốt rắn hoặc ô lưới
    class RectangleShape {
    public:
        RectangleShape() = default;
        explicit RectangleShape(const Vector2f& size) : m_size(size) {}

        void setSize(const Vector2f& size) { m_size = size; }
        const Vector2f& getSize() const { return m_size; }

        void setPosition(const Vector2f& position) { m_position = position; }
        void setPosition(float x, float y) { m_position = Vector2f(x, y); }
        const Vector2f& getPosition() const { return m_position; }

        void setOrigin(const Vector2f& origin) { m_origin = origin; }
        void setOrigin(float x, float y) { m_origin = Vector2f(x, y); }

        void setFillColor(const Color& color) { m_fillColor = color; }
        const Color& getFillColor() const { return m_fillColor; }

        void setOutlineColor(const Color& color) { m_outlineColor = color; }
        void setOutlineThickness(float thickness) { m_outlineThickness = thickness; }

    private:
        Vector2f m_size{0.0f, 0.0f};
        Vector2f m_position{0.0f, 0.0f};
        Vector2f m_origin{0.0f, 0.0f};
        Color m_fillColor{Color::White};
        Color m_outlineColor{Color::Black};
        float m_outlineThickness{0.0f};
    };

    // Hình tròn dùng để vẽ các loại táo hoặc hiệu ứng
    class CircleShape {
    public:
        CircleShape() = default;
        explicit CircleShape(float radius) : m_radius(radius) {}

        void setRadius(float radius) { m_radius = radius; }
        float getRadius() const { return m_radius; }

        void setPosition(const Vector2f& position) { m_position = position; }
        void setPosition(float x, float y) { m_position = Vector2f(x, y); }
        const Vector2f& getPosition() const { return m_position; }

        void setOrigin(const Vector2f& origin) { m_origin = origin; }
        void setOrigin(float x, float y) { m_origin = Vector2f(x, y); }

        void setFillColor(const Color& color) { m_fillColor = color; }
        const Color& getFillColor() const { return m_fillColor; }

        void setOutlineColor(const Color& color) { m_outlineColor = color; }
        void setOutlineThickness(float thickness) { m_outlineThickness = thickness; }

    private:
        float m_radius{0.0f};
        Vector2f m_position{0.0f, 0.0f};
        Vector2f m_origin{0.0f, 0.0f};
        Color m_fillColor{Color::White};
        Color m_outlineColor{Color::Black};
        float m_outlineThickness{0.0f};
    };

    // Bàn phím tương thích sf::Keyboard
    class Keyboard {
    public:
        enum Key {
            Unknown = -1,
            A = 0, B, C, D, E, F, G, H, I, J, K, L, M,
            N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
            Num0, Num1, Num2, Num3, Num4, Num5, Num6, Num7, Num8, Num9,
            Escape, Space, Enter,
            Left, Right, Up, Down
        };

        static bool isKeyPressed(Key) { return false; }
    };

    // Cửa sổ vẽ tương thích sf::RenderWindow
    class RenderWindow {
    public:
        RenderWindow() = default;
        void draw(const RectangleShape&) { ++m_drawCallCount; }
        void draw(const CircleShape&) { ++m_drawCallCount; }
        int getDrawCallCount() const { return m_drawCallCount; }
        void resetDrawCallCount() { m_drawCallCount = 0; }

    private:
        int m_drawCallCount{0};
    };

} // namespace sf
#endif
