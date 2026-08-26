#include "svg.h"

namespace svg {

using namespace std::literals;

void Object::Render(const RenderContext& context) const {
    context.RenderIndent();
    RenderObject(context);
    context.out << std::endl;
}

std::ostream& operator<<(std::ostream& out, const Rgb& rgb) {
    out << "rgb(" << static_cast<int>(rgb.red) << ',' << static_cast<int>(rgb.green) << ','
        << static_cast<int>(rgb.blue) << ')';
    return out;
}

std::ostream& operator<<(std::ostream& out, const Rgba& rgba) {
    out << "rgba(" << static_cast<int>(rgba.red) << ',' << static_cast<int>(rgba.green) << ','
        << static_cast<int>(rgba.blue) << ',' << rgba.opacity << ')';
    return out;
}

namespace {
struct ColorPrinter {
    std::ostream& out;
    void operator()(std::monostate) const {
        out << "none";
    }
    void operator()(const std::string& s) const {
        out << s;
    }
    void operator()(Rgb rgb) const {
        out << rgb;
    }
    void operator()(Rgba rgba) const {
        out << rgba;
    }
};
} // namespace

std::ostream& operator<<(std::ostream& out, const Color& color) {
    std::visit(ColorPrinter{out}, color);
    return out;
}

std::ostream& operator<<(std::ostream& out, StrokeLineCap cap) {
    switch (cap) {
    case StrokeLineCap::BUTT:
        out << "butt"sv;
        break;
    case StrokeLineCap::ROUND:
        out << "round"sv;
        break;
    case StrokeLineCap::SQUARE:
        out << "square"sv;
        break;
    }
    return out;
}

std::ostream& operator<<(std::ostream& out, StrokeLineJoin join) {
    switch (join) {
    case StrokeLineJoin::ARCS:
        out << "arcs"sv;
        break;
    case StrokeLineJoin::BEVEL:
        out << "bevel"sv;
        break;
    case StrokeLineJoin::MITER:
        out << "miter"sv;
        break;
    case StrokeLineJoin::MITER_CLIP:
        out << "miter-clip"sv;
        break;
    case StrokeLineJoin::ROUND:
        out << "round"sv;
        break;
    }
    return out;
}

Circle& Circle::SetCenter(Point center) {
    center_ = center;
    return *this;
}
Circle& Circle::SetRadius(double radius) {
    radius_ = radius;
    return *this;
}
void Circle::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
    out << "r=\""sv << radius_ << "\""sv;
    RenderAttrs(out);
    out << "/>"sv;
}

Polyline& Polyline::AddPoint(Point point) {
    points_.push_back(point);
    return *this;
}
void Polyline::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<polyline points=\""sv;
    bool first = true;
    for (const Point& p : points_) {
        if (!first) {
            out << ' ';
        }
        first = false;
        out << p.x << ',' << p.y;
    }
    out << "\""sv;
    RenderAttrs(out);
    out << "/>"sv;
}

namespace {
std::string EscapeText(const std::string& data) {
    std::string result;
    result.reserve(data.size());
    for (char c : data) {
        switch (c) {
        case '"':
            result += "&quot;"sv;
            break;
        case '\'':
            result += "&apos;"sv;
            break;
        case '<':
            result += "&lt;"sv;
            break;
        case '>':
            result += "&gt;"sv;
            break;
        case '&':
            result += "&amp;"sv;
            break;
        default:
            result += c;
            break;
        }
    }
    return result;
}
} // namespace

Text& Text::SetPosition(Point pos) {
    position_ = pos;
    return *this;
}
Text& Text::SetOffset(Point offset) {
    offset_ = offset;
    return *this;
}
Text& Text::SetFontSize(uint32_t size) {
    font_size_ = size;
    return *this;
}
Text& Text::SetFontFamily(std::string font_family) {
    font_family_ = std::move(font_family);
    return *this;
}
Text& Text::SetFontWeight(std::string font_weight) {
    font_weight_ = std::move(font_weight);
    return *this;
}
Text& Text::SetData(std::string data) {
    data_ = std::move(data);
    return *this;
}

void Text::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<text"sv;
    RenderAttrs(out);
    out << " x=\""sv << position_.x << "\" y=\""sv << position_.y << "\""sv;
    out << " dx=\""sv << offset_.x << "\" dy=\""sv << offset_.y << "\""sv;
    out << " font-size=\""sv << font_size_ << "\""sv;
    if (font_family_) {
        out << " font-family=\""sv << *font_family_ << "\""sv;
    }
    if (font_weight_) {
        out << " font-weight=\""sv << *font_weight_ << "\""sv;
    }
    out << ">"sv << EscapeText(data_) << "</text>"sv;
}

void Document::AddPtr(std::unique_ptr<Object>&& obj) {
    objects_.push_back(std::move(obj));
}

void Document::Render(std::ostream& out) const {
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"sv;
    out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">\n"sv;
    RenderContext ctx(out, 2, 2);
    for (const auto& obj : objects_) {
        obj->Render(ctx);
    }
    out << "</svg>"sv;
}

} // namespace svg
