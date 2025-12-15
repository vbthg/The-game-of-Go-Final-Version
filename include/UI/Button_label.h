
#pragma once

#include <SFML/Graphics.hpp>

#include <functional>
#include <string>


/// CẦN THÊM 1 BIẾN BOOL IS_INTERACTIVE ĐỂ XEM LIỆU ĐÂY CÓ PHẢI NÚT BẤM THẬT

namespace UI
{

class Button
{
public:
    /**
     * @brief Hàm khởi tạo dùng Sprite (Texture).
     * @param text Chuỗi văn bản hiển thị trên nút.
     * @param textureRef Tham chiếu hằng đến sf::Texture (ảnh) làm nền cho nút.
     * @param position Vị trí của nút (góc trên bên trái).
     * @param fontRef Tham chiếu hằng đến sf::Font đã được tải.
     * @param shadow Bật (true) hoặc tắt (false) hiệu ứng đổ bóng cho chữ.
     */
    Button(const std::string& text,
           const sf::Texture& textureRef,
           const sf::Vector2f& position,
           const sf::Font& fontRef,
           const unsigned int& charSize);

    /// --- Thiết lập giao diện ---

    /**
     * @brief Đặt lại văn bản và cỡ chữ cho nút.
     * @param text Chuỗi văn bản mới.
     * @param charSize Kích thước chữ (mặc định là 24).
     */
    void setLabel(const std::string& text, const unsigned int &charSize);

    /**
     * @brief Đặt vị trí của nút ngay lập tức.
     * Hàm này sẽ được gọi bởi các class hiệu ứng (Animation) của bạn.
     * @param pos Vị trí mới.
     */
    void setPosition(const sf::Vector2f& pos);

    /**
     * @brief Đặt tỉ lệ phóng/thu của nút.
     * Hàm này sẽ được gọi bởi các class hiệu ứng (Animation) của bạn.
     * @param scale Tỉ lệ mới (ví dụ: 1.0 là gốc).
     */
    void setScale(float scale); // <-- HÀM MỚI (để hiệu ứng phóng to gọi)

    /**
     * @brief Lấy kích thước (rộng, cao) gốc của nút.
     * @return sf::FloatRect chứa width và height.
     */
    sf::FloatRect getLocalBounds() const;

    /**
     * @brief Lấy vị trí "hiện tại" của nút (góc trên bên trái).
     */
    sf::Vector2f getPosition() const;

    /// --- Xử lý sự kiện & Cập nhật ---

    /**
     * @brief Xử lý các sự kiện đầu vào (chủ yếu là chuột) cho nút.
     * @param ev Sự kiện sf::Event từ vòng lặp.
     * @param window Cửa sổ render để lấy vị trí chuột chính xác.
     * @return true nếu nút được click.
     */
    bool handleEvent(const sf::Event& ev, const sf::RenderWindow& window);

    /**
     * @brief Cập nhật trạng thái hover/pressed của nút.
     * Lưu ý: Không còn nhận 'dt' vì logic animation đã bị xóa.
     * @param window Cửa sổ render để lấy vị trí chuột.
     */
    void update(const sf::RenderWindow& window); // <-- ĐÃ BỎ (sf::Time dt)

    /// --- Vẽ ---

    /**
     * @brief Vẽ nút lên một mục tiêu render.
     * @param target Mục tiêu để vẽ.
     */
    void draw(sf::RenderTarget& target) const;

    /// --- Callback ---

    /**
     * @brief Gán một hàm callback khi nút được click.
     * @param cb Hàm kiểu std::function<void()>.
     */
    void setOnClick(std::function<void()> cb);

    /**
     * @brief Lấy tham chiếu đến Sprite (để các class hiệu ứng có thể tác động).
     * Đây là một cách khác để hiệu ứng hoạt động.
     * @return sf::Sprite&
     */
    sf::Sprite& getSprite(); // <-- HÀM MỚI (Tùy chọn)

private:
    sf::Sprite m_sprite;     // Sprite làm nền cho nút
    sf::Text label;          // Văn bản trên nút
    sf::Text ShadowLabel;    // Văn bản dùng làm bóng
//    const sf::Font& font;    // Tham chiếu đến font

    // Các màu sắc cho các trạng thái (phủ màu)
    sf::Color normalColor;
    sf::Color hoverColor;
    sf::Color pressedColor;

    bool hovered = false;    // Chuột đang trỏ vào?
    bool pressed = false;    // Chuột đang nhấn và giữ?
    std::function<void()> onClick = nullptr; // Hàm callback khi click

    // --- Các hàm private ---
    void applyShadow();
    void applyVisualState();

    // --- ĐÃ BỎ ---
    // sf::Vector2f m_targetPosition;
    // float m_easingFactor;
    // bool m_useEasing;
    // sf::Vector2f lerp(...);
};

} // namespace UI
