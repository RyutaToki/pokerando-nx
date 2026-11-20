#include "ui/widgets.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>

namespace ui {

namespace {
const float kFocusBorder = 3;
void focusBorder(NVGcontext* vg, float x, float y, float w, float h, float r) { strokeRRect(vg, x, y, w, h, r, kFocusBorder, colors().accent); }
}  // namespace

// ---------- Focusable ----------
Focusable::Focusable() {
    setFocusable(true);
    setHideHighlight(true);
    registerClickAction([this](brls::View*) {
        click();
        return true;
    });
    addGestureRecognizer(new brls::TapGestureRecognizer(this));
}

void Focusable::setEnabled(bool e) {
    enabled_ = e;
    setFocusable(e);
    setAlpha(e ? 1.0f : 0.4f);
}

// ---------- Text ----------
Text::Text(std::string s, const char* font, float size, NVGcolor color, float height)
    : text_(std::move(s)), font_(font), size_(size), color_(color) {
    setHeight(height > 0 ? height : std::ceil(size * 1.35f));
}

void Text::draw(NVGcontext* vg, float x, float y, float w, float h, brls::Style, brls::FrameContext*) {
    if (wrap_) {
        textBox(vg, x, y, w, text_, font_, size_, color_);
        return;
    }
    float tx = align_ == Align::LEFT ? x : align_ == Align::CENTER ? x + w / 2 : x + w;
    text(vg, tx, y + h / 2, ellipsize(vg, text_, font_, size_, w), font_, size_, color_, align_, spacing_);
}

Text* sectionLabel(const std::string& s, NVGcolor color) {
    auto* t = new Text(s, font::semibold, 14.5f, color, 18);
    t->setLetterSpacing(14.5f * 0.06f);
    return t;
}

// ---------- Panel ----------
Panel::Panel(NVGcolor bg, float radius) : bg_(bg), radius_(radius) {}

void Panel::draw(NVGcontext* vg, float x, float y, float w, float h, brls::Style s, brls::FrameContext* c) {
    fillRRect(vg, x, y, w, h, radius_, bg_);
    Box::draw(vg, x, y, w, h, s, c);
}

// ---------- MenuButton ----------
MenuButton::MenuButton(Icon icon, std::string label) : icon_(icon), label_(std::move(label)) {
    setWidth(520);
    setHeight(72);
}

void MenuButton::draw(NVGcontext* vg, float x, float y, float w, float h, brls::Style, brls::FrameContext*) {
    bool f = isFocused();
    fillRRect(vg, x, y, w, h, 10, colors().surface);
    if (f) focusBorder(vg, x, y, w, h, 10);
    icon(vg, icon_, x + 24, y + (h - 28) / 2, 28, f ? colors().accent : colors().text);
    text(vg, x + 24 + 28 + 18, y + h / 2, label_, f ? font::semibold : font::medium, 22, colors().text);
}

// ---------- FileRow ----------
FileRow::FileRow(Kind kind, std::string name, std::string right) : kind_(kind), name_(std::move(name)), right_(std::move(right)) {
    setHeight(60);
}

void FileRow::draw(NVGcontext* vg, float x, float y, float w, float h, brls::Style, brls::FrameContext*) {
    bool f = isFocused();
    const auto& c = colors();
    if (f) {
        fillRRect(vg, x, y, w, h, 8, c.surface);
        focusBorder(vg, x, y, w, h, 8);
    }
    NVGcolor ic = kind_ == Kind::FOLDER ? c.warn : kind_ == Kind::FILE ? c.text : c.textMuted;
    if (f) ic = c.accent;
    Icon i = kind_ == Kind::UP ? Icon::ARROW_LEFT : kind_ == Kind::FOLDER ? Icon::FOLDER : Icon::FILE;
    icon(vg, i, x + 20, y + (h - 24) / 2, 24, ic);
    NVGcolor tc = kind_ == Kind::UNSUPPORTED || kind_ == Kind::UP ? c.textMuted : c.text;
    std::string right = kind_ == Kind::UNSUPPORTED && right_.empty() ? "No compatible" : right_;
    float rw = right.empty() ? 0 : textWidth(vg, right, font::regular, 15) + 24;
    float nameX = x + 20 + 24 + 16;
    if (star_) {
        icon(vg, Icon::STAR, nameX, y + (h - 18) / 2, 18, c.warn);
        nameX += 26;
    }
    text(vg, nameX, y + h / 2, ellipsize(vg, name_, f ? font::semibold : font::medium, 19, x + w - rw - nameX - 20), f ? font::semibold : font::medium, 19, tc);
    if (!right.empty()) text(vg, x + w - 20, y + h / 2, right, font::regular, 15, c.textMuted, Align::RIGHT);
}

// ---------- ListRow ----------
ListRow::ListRow(std::string title, std::string subtitle, std::string badge)
    : title_(std::move(title)), subtitle_(std::move(subtitle)), badge_(std::move(badge)) {
    setHeight(76);
}

void ListRow::draw(NVGcontext* vg, float x, float y, float w, float h, brls::Style, brls::FrameContext*) {
    bool f = isFocused();
    const auto& c = colors();
    fillRRect(vg, x, y, w, h, 8, c.surface);
    if (f) focusBorder(vg, x, y, w, h, 8);
    float right = x + w - 20;
    if (!badge_.empty()) {
        float bw = textWidth(vg, badge_, font::bold, 14) + 20;
        badge(vg, right - bw, y + h / 2, badge_, badge_ == "NDS" ? c.warn : c.accent);
        right -= bw + 16;
    }
    text(vg, x + 20, y + 27, ellipsize(vg, title_, f ? font::semibold : font::medium, 19, right - x - 20), f ? font::semibold : font::medium, 19, c.text);
    text(vg, x + 20, y + 52, ellipsize(vg, subtitle_, font::regular, 15, right - x - 20), font::regular, 15, c.textMuted);
}

// ---------- TabItem ----------
TabItem::TabItem(std::string label) : label_(std::move(label)) { setHeight(48); }

void TabItem::draw(NVGcontext* vg, float x, float y, float w, float h, brls::Style, brls::FrameContext*) {
    const auto& c = colors();
    bool f = isFocused();
    if (active_) {
        nvgBeginPath(vg);
        nvgRect(vg, x, y, w, h);
        nvgFillColor(vg, c.surface);
        nvgFill(vg);
        nvgBeginPath(vg);
        nvgRect(vg, x, y, 5, h);
        nvgFillColor(vg, c.accent);
        nvgFill(vg);
    }
    if (f) focusBorder(vg, x + 6, y, w - 12, h, 8);
    text(vg, x + (active_ ? 27 + 5 : 32), y + h / 2, label_, active_ ? font::bold : font::medium, 18, active_ ? c.accent : c.text);
}

// ---------- Segmented ----------
class Segmented::Seg : public Focusable {
public:
    Seg(Segmented* parent, int index, std::string label) : parent_(parent), index_(index), label_(std::move(label)) {
        setHeight(56);
        setGrow(1);
        setShrink(1);
        setWidth(10);  // base pequeña; flex reparte el ancho
        setOnClick([this] { parent_->setSelected(index_); if (parent_->onChange_) parent_->onChange_(index_); });
    }
    void draw(NVGcontext* vg, float x, float y, float w, float h, brls::Style, brls::FrameContext*) override {
        bool sel = parent_->selected_ == index_;
        fillRRect(vg, x, y, w, h, 10, colors().surface);
        if (sel || isFocused()) strokeRRect(vg, x, y, w, h, 10, kFocusBorder, sel ? colors().accent : colors().accentHover);
        if (isFocused() && !sel) strokeRRect(vg, x + 5, y + 5, w - 10, h - 10, 6, 1, colors().textMuted);
        text(vg, x + w / 2, y + h / 2, ellipsize(vg, label_, sel ? font::bold : font::medium, 16, w - 16), sel ? font::bold : font::medium, 16,
             colors().text, Align::CENTER);
    }

private:
    Segmented* parent_;
    int index_;
    std::string label_;
};

Segmented::Segmented(const std::string& label, std::vector<std::string> options, int selected, std::function<void(int)> onChange)
    : selected_(selected), onChange_(std::move(onChange)) {
    setAxis(brls::Axis::COLUMN);
    if (!label.empty()) {
        auto* l = sectionLabel(label);
        l->setMarginBottom(10);
        addView(l);
    }
    auto* row = new brls::Box(brls::Axis::ROW);
    for (size_t i = 0; i < options.size(); i++) {
        auto* s = new Seg(this, static_cast<int>(i), options[i]);
        if (i + 1 < options.size()) s->setMarginRight(10);
        segs_.push_back(s);
        row->addView(s);
    }
    addView(row);
}

void Segmented::setSelected(int i) { selected_ = i; }

// ---------- Toggle ----------
Toggle::Toggle(std::string label, bool on, std::function<void(bool)> onChange)
    : label_(std::move(label)), on_(on), knob_(on ? 1.f : 0.f), onChange_(std::move(onChange)) {
    setHeight(52);
    setOnClick([this] {
        on_ = !on_;
        if (onChange_) onChange_(on_);
    });
}

void Toggle::draw(NVGcontext* vg, float x, float y, float w, float h, brls::Style, brls::FrameContext*) {
    const auto& c = colors();
    knob_ += ((on_ ? 1.f : 0.f) - knob_) * 0.35f;
    if (std::abs(knob_ - (on_ ? 1.f : 0.f)) < 0.01f) knob_ = on_ ? 1.f : 0.f;
    fillRRect(vg, x, y, w, h, 10, c.surface);
    if (isFocused()) focusBorder(vg, x, y, w, h, 10);
    const float tw = 52, th = 28;
    float tx = x + w - 18 - tw, ty = y + (h - th) / 2;
    text(vg, x + 18, y + h / 2, ellipsize(vg, label_, font::medium, 17, tx - x - 30), font::medium, 17, c.text);
    NVGcolor track = nvgLerpRGBA(c.trackOff, c.accent, knob_);
    fillRRect(vg, tx, ty, tw, th, 14, track);
    float kx = tx + 3 + 11 + knob_ * (tw - 6 - 22);
    nvgBeginPath(vg);
    nvgCircle(vg, kx, ty + th / 2, 11);
    nvgFillColor(vg, nvgLerpRGBA(c.text, c.onAccent, knob_));
    nvgFill(vg);
}

// ---------- Slider ----------
Slider::Slider(std::string label, int value, std::function<void(int)> onChange, int min, int max, int step)
    : label_(std::move(label)), value_(value), min_(min), max_(max), step_(step), onChange_(std::move(onChange)) {
    setHeight(64);
    registerAction("", brls::BUTTON_NAV_LEFT, [this](brls::View*) { setValue(value_ - step_); return true; }, true, true);
    registerAction("", brls::BUTTON_NAV_RIGHT, [this](brls::View*) { setValue(value_ + step_); return true; }, true, true);
    addGestureRecognizer(new brls::PanGestureRecognizer(
        [this](brls::PanGestureStatus status, brls::Sound*) {
            if (!enabled_) return;
            brls::Application::giveFocus(this);
            float t = (status.position.x - trackX_) / trackW_;
            t = std::clamp(t, 0.f, 1.f);
            int v = min_ + static_cast<int>(std::round(t * (max_ - min_) / step_)) * step_;
            setValue(v);
        },
        brls::PanAxis::HORIZONTAL));
}

void Slider::setValue(int v) {
    v = std::clamp(v, min_, max_);
    if (v == value_) return;
    value_ = v;
    if (onChange_) onChange_(v);
}

void Slider::draw(NVGcontext* vg, float x, float y, float w, float h, brls::Style, brls::FrameContext*) {
    const auto& c = colors();
    fillRRect(vg, x, y, w, h, 10, c.surface);
    if (isFocused()) focusBorder(vg, x, y, w, h, 10);
    text(vg, x + 20, y + h / 2, ellipsize(vg, label_, font::medium, 17, 220), font::medium, 17, c.text);
    trackX_ = x + 20 + 230;
    trackW_ = w - 20 - 230 - 64 - 20 - 16;
    float t = float(value_ - min_) / float(max_ - min_);
    fillRRect(vg, trackX_, y + h / 2 - 4, trackW_, 8, 4, c.trackOff);
    // relleno desde el centro (0 %) hasta el valor
    float zero = trackX_ + trackW_ * (float(-min_) / float(max_ - min_));
    float pos = trackX_ + trackW_ * t;
    fillRRect(vg, std::min(zero, pos), y + h / 2 - 4, std::abs(pos - zero), 8, 4, c.accent);
    nvgBeginPath(vg);
    nvgCircle(vg, pos, y + h / 2, 14);
    nvgFillColor(vg, c.text);
    nvgFill(vg);
    std::string v = value_ == 0 ? "0 %" : (value_ > 0 ? "+" : "−") + std::to_string(std::abs(value_)) + " %";
    text(vg, x + w - 20, y + h / 2, v, font::bold, 17, c.text, Align::RIGHT);
}

// ---------- Chips ----------
class Chips::Chip : public Focusable {
public:
    Chip(Chips* parent, int index, std::string label, std::optional<NVGcolor> swatch)
        : parent_(parent), index_(index), label_(std::move(label)), swatch_(swatch) {
        setHeight(52);
        setGrow(1);
        setShrink(1);
        setWidth(10);
        setOnClick([this] { parent_->selected_ = index_; if (parent_->onChange_) parent_->onChange_(index_); });
    }
    void draw(NVGcontext* vg, float x, float y, float w, float h, brls::Style, brls::FrameContext*) override {
        const auto& c = colors();
        bool sel = parent_->selected_ == index_;
        if (swatch_) {
            fillRRect(vg, x, y, w, h, 10, c.bg);
            fillRRect(vg, x + 10, y + 10, w - 20, h - 20, 6, *swatch_);
            if (sel) strokeRRect(vg, x, y, w, h, 10, kFocusBorder, c.text);
            if (isFocused()) strokeRRect(vg, x - 4, y - 4, w + 8, h + 8, 12, 2, c.accent);
            return;
        }
        fillRRect(vg, x, y, w, h, 10, sel ? c.accent : c.bg);
        if (isFocused()) strokeRRect(vg, x - 4, y - 4, w + 8, h + 8, 13, kFocusBorder, c.accent);
        text(vg, x + w / 2, y + h / 2, label_, font::bold, 17, sel ? c.onAccent : c.text, Align::CENTER);
    }

private:
    Chips* parent_;
    int index_;
    std::string label_;
    std::optional<NVGcolor> swatch_;
};

Chips::Chips(std::vector<std::string> options, int selected, std::function<void(int)> onChange, std::vector<NVGcolor> swatches)
    : selected_(selected), onChange_(std::move(onChange)) {
    setAxis(brls::Axis::ROW);
    for (size_t i = 0; i < options.size(); i++) {
        auto* ch = new Chip(this, static_cast<int>(i), options[i], i < swatches.size() ? std::optional<NVGcolor>(swatches[i]) : std::nullopt);
        if (i + 1 < options.size()) ch->setMarginRight(10);
        chips_.push_back(ch);
        addView(ch);
    }
}

void Chips::setEnabled(bool e) {
    for (auto* ch : chips_) ch->setEnabled(e);
}

// ---------- Button ----------
Button::Button(std::string label, bool primary, std::function<void()> onClick) : label_(std::move(label)), primary_(primary) {
    setHeight(64);
    setOnClick(std::move(onClick));
}

void Button::draw(NVGcontext* vg, float x, float y, float w, float h, brls::Style, brls::FrameContext*) {
    const auto& c = colors();
    if (primary_) {
        fillRRect(vg, x, y, w, h, 10, isFocused() ? c.accentHover : c.accent);
        if (isFocused()) strokeRRect(vg, x - 5, y - 5, w + 10, h + 10, 14, kFocusBorder, c.accent);
        text(vg, x + w / 2, y + h / 2, label_, font::bold, 19.5f, c.onAccent, Align::CENTER);
    } else {
        strokeRRect(vg, x, y, w, h, 10, 2, c.trackOff);
        if (isFocused()) strokeRRect(vg, x, y, w, h, 10, kFocusBorder, c.accent);
        text(vg, x + w / 2, y + h / 2, label_, font::semibold, 19, c.text, Align::CENTER);
    }
}

// ---------- insignias y logotipo ----------
float badge(NVGcontext* vg, float x, float cy, const std::string& t, NVGcolor bg) {
    float w = textWidth(vg, t, font::bold, 14) + 20;
    fillRRect(vg, x, cy - 12, w, 24, 6, bg);
    text(vg, x + w / 2, cy, t, font::bold, 14, colors().onAccent, Align::CENTER);
    return w;
}

float exclusiveTag(NVGcontext* vg, float x, float cy) {
    const std::string t = "Exclusivo de esta app";
    float w = textWidth(vg, t, font::bold, 14) + 18;
    fillRRect(vg, x, cy - 12, w, 24, 6, colors().warn);
    text(vg, x + w / 2, cy, t, font::bold, 14, colors().onAccent, Align::CENTER);
    return w;
}

float logo(NVGcontext* vg, float x, float cy, bool withName) {
    const auto& c = colors();
    nvgBeginPath(vg);
    nvgCircle(vg, x + 22, cy, 20);
    nvgStrokeColor(vg, c.accent);
    nvgStrokeWidth(vg, 4);
    nvgStroke(vg);
    nvgBeginPath(vg);
    nvgCircle(vg, x + 22, cy, 6);
    nvgFillColor(vg, c.accent);
    nvgFill(vg);
    if (!withName) return 44;
    float w = text(vg, x + 44 + 16, cy, "PokéRando NX", font::title, 30, c.text);
    float v = text(vg, x + 44 + 16 + w + 12, cy + 4, "v0.1", font::regular, 15, c.textMuted);
    return 44 + 16 + w + 12 + v;
}

// ---------- Header / Footer / Frame ----------
Header::Header() {
    setHeight(88);
    setWidthPercentage(100);
}

void Header::draw(NVGcontext* vg, float x, float y, float w, float h, brls::Style, brls::FrameContext*) {
    const auto& c = colors();
    if (drawLeft) drawLeft(vg, x + 48, y, w - 96, h);
    else text(vg, x + 48, y + h / 2, title, font::title, 28, c.text);
    if (drawRight) drawRight(vg, x + 48, y, w - 96, h);
    else if (!rightText.empty()) text(vg, x + w - 48, y + h / 2, rightText, font::regular, 16, c.textMuted, Align::RIGHT);
    nvgBeginPath(vg);
    nvgRect(vg, x, y + h - 1, w, 1);
    nvgFillColor(vg, c.border);
    nvgFill(vg);
}

Footer::Footer() {
    setHeight(64);
    setWidthPercentage(100);
}

static float drawHint(NVGcontext* vg, float x, float cy, const Hint& hnt) {
    // devuelve el ancho total de la ayuda dibujada a partir de x
    const auto& c = colors();
    float kw;
    if (hnt.key.size() <= 1 || hnt.key == "−") {
        kw = 28;
        nvgBeginPath(vg);
        nvgCircle(vg, x + 14, cy, 14);
        nvgFillColor(vg, hnt.accent ? c.accent : c.text);
        nvgFill(vg);
        text(vg, x + 14, cy + 0.5f, hnt.key, font::bold, 15, c.onAccent, Align::CENTER);
    } else {
        kw = textWidth(vg, hnt.key, font::bold, 15) + 20;
        fillRRect(vg, x, cy - 14, kw, 28, 14, hnt.accent ? c.accent : c.text);
        text(vg, x + kw / 2, cy + 0.5f, hnt.key, font::bold, 15, c.onAccent, Align::CENTER);
    }
    float tw = text(vg, x + kw + 10, cy, hnt.text, font::medium, 17, c.text);
    return kw + 10 + tw;
}

void Footer::draw(NVGcontext* vg, float x, float y, float w, float h, brls::Style, brls::FrameContext*) {
    const auto& c = colors();
    nvgBeginPath(vg);
    nvgRect(vg, x, y, w, 1);
    nvgFillColor(vg, c.border);
    nvgFill(vg);
    float cy = y + h / 2;
    float lx = x + 48;
    for (const auto& hn : leftHints) lx += drawHint(vg, lx, cy, hn) + 32;
    // medir de derecha a izquierda
    float rx = x + w - 48;
    for (auto it = hints.rbegin(); it != hints.rend(); ++it) {
        float kw = (it->key.size() <= 1 || it->key == "−") ? 28 : textWidth(vg, it->key, font::bold, 15) + 20;
        float width = kw + 10 + textWidth(vg, it->text, font::medium, 17);
        rx -= width;
        drawHint(vg, rx, cy, *it);
        rx -= 32;
    }
    drawToast(vg);
}

Frame::Frame() {
    setAxis(brls::Axis::COLUMN);
    setWidth(1280);
    setHeight(720);
    header = new Header();
    content = new brls::Box(brls::Axis::COLUMN);
    content->setGrow(1);
    content->setHeight(568);
    footer = new Footer();
    addView(header);
    addView(content);
    addView(footer);
}

void Frame::draw(NVGcontext* vg, float x, float y, float w, float h, brls::Style s, brls::FrameContext* c) {
    nvgBeginPath(vg);
    nvgRect(vg, x, y, w, h);
    nvgFillColor(vg, colors().bg);
    nvgFill(vg);
    Box::draw(vg, x, y, w, h, s, c);
}

// ---------- Toast ----------
namespace {
std::string gToast;
std::chrono::steady_clock::time_point gToastUntil;
}  // namespace

void toast(const std::string& t) {
    gToast = t;
    gToastUntil = std::chrono::steady_clock::now() + std::chrono::milliseconds(2500);
}

void drawToast(NVGcontext* vg) {
    if (gToast.empty() || std::chrono::steady_clock::now() > gToastUntil) return;
    const auto& c = colors();
    float w = textWidth(vg, gToast, font::medium, 17) + 48;
    float x = (1280 - w) / 2, y = 720 - 64 - 24 - 48;
    fillRRect(vg, x, y, w, 48, 10, c.sidebar);
    strokeRRect(vg, x, y, w, 48, 10, 1, c.border);
    text(vg, 640, y + 24, gToast, font::medium, 17, c.text, Align::CENTER);
}

// ---------- Diálogo modal ----------
namespace {
class ModalActivity : public brls::Activity {
public:
    explicit ModalActivity(brls::View* card) : card_(card) {}
    brls::View* createContentView() override {
        auto* root = new Dim();
        root->setWidth(1280);
        root->setHeight(720);
        root->setJustifyContent(brls::JustifyContent::CENTER);
        root->setAlignItems(brls::AlignItems::CENTER);
        root->addView(card_);
        return root;
    }
    bool isTranslucent() override { return true; }

private:
    class Dim : public brls::Box {
    public:
        void draw(NVGcontext* vg, float x, float y, float w, float h, brls::Style s, brls::FrameContext* c) override {
            nvgBeginPath(vg);
            nvgRect(vg, x, y, w, h);
            nvgFillColor(vg, nvgRGBAf(0, 0, 0, 0.6f));
            nvgFill(vg);
            Box::draw(vg, x, y, w, h, s, c);
        }
    };
    brls::View* card_;
};

class DialogTitle : public brls::View {
public:
    DialogTitle(std::string t, bool err) : t_(std::move(t)), err_(err) { setHeight(34); }
    void draw(NVGcontext* vg, float x, float y, float, float h, brls::Style, brls::FrameContext*) override {
        float tx = x;
        if (err_) {
            icon(vg, Icon::WARNING, x, y + (h - 28) / 2, 28, colors().error);
            tx += 40;
        }
        text(vg, tx, y + h / 2, t_, font::bold, 22, colors().text);
    }

private:
    std::string t_;
    bool err_;
};
}  // namespace

void dialog(const std::string& title, const std::string& body, const std::string& okText, std::function<void()> onOk,
            const std::string& cancelText, std::function<void()> onCancel, bool error) {
    auto* card = new Panel(colors().surface, 14);
    card->setAxis(brls::Axis::COLUMN);
    card->setWidth(640);
    card->setPadding(32, 32, 32, 32);
    card->addView(new DialogTitle(title, error));
    if (!body.empty()) {
        auto* b = new Text(body, font::medium, 17, colors().text);
        b->setWrap(true);
        // altura aproximada: 600 px de ancho útil ~ 60 caracteres por línea
        int lines = 1;
        size_t col = 0;
        for (char ch : body) {
            if (ch == '\n' || ++col > 58) {
                lines++;
                col = 0;
            }
        }
        b->setHeight(lines * 23.f);
        b->setMarginTop(14);
        card->addView(b);
    }
    auto* row = new brls::Box(brls::Axis::ROW);
    row->setJustifyContent(brls::JustifyContent::FLEX_END);
    row->setMarginTop(28);
    auto close = [] { brls::Application::popActivity(brls::TransitionAnimation::NONE); };
    Button* cancel = nullptr;
    if (!cancelText.empty()) {
        cancel = new Button(cancelText, false, [close, onCancel] {
            close();
            if (onCancel) onCancel();
        });
        cancel->setWidth(180);
        cancel->setMarginRight(12);
        row->addView(cancel);
    }
    auto* ok = new Button(okText, true, [close, onOk] {
        close();
        if (onOk) onOk();
    });
    ok->setWidth(200);
    row->addView(ok);
    card->addView(row);
    card->registerAction("", brls::BUTTON_B, [close, onCancel, hasCancel = !cancelText.empty()](brls::View*) {
        close();
        if (hasCancel && onCancel) onCancel();
        return true;
    }, true);
    auto* act = new ModalActivity(card);
    brls::Application::pushActivity(act, brls::TransitionAnimation::NONE);
    brls::Application::giveFocus(ok);
}

void menu(const std::string& title, std::vector<std::pair<std::string, std::function<void()>>> items) {
    auto* card = new Panel(colors().surface, 14);
    card->setAxis(brls::Axis::COLUMN);
    card->setWidth(560);
    card->setPadding(28, 28, 28, 28);
    card->addView(new DialogTitle(title, false));
    brls::View* first = nullptr;
    for (auto& [label, fn] : items) {
        auto* row = new FileRow(FileRow::Kind::FILE, label);
        row->setMarginTop(8);
        row->setOnClick([fn = fn] {
            brls::Application::popActivity(brls::TransitionAnimation::NONE);
            if (fn) fn();
        });
        if (!first) first = row;
        card->addView(row);
    }
    card->registerAction("", brls::BUTTON_B, [](brls::View*) {
        brls::Application::popActivity(brls::TransitionAnimation::NONE);
        return true;
    }, true);
    brls::Application::pushActivity(new ModalActivity(card), brls::TransitionAnimation::NONE);
    if (first) brls::Application::giveFocus(first);
}

}  // namespace ui
