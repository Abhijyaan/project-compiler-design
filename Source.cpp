// LL(1) Expression Parser + GUI + Visualizer made by beginner team
// This program lets user type expressions in a window
// It checks validity and also shows the parse tree if successful
// We did our best as a team to make it work!

#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <cctype>
#include <memory>

// Token structure to store type and value
struct Token {
    std::string type; // like PLUS, ID, etc.
    std::string value; // actual string from input
};

// Tree node for parse tree visualization
struct TreeNode {
    std::string label;
    std::vector<std::shared_ptr<TreeNode>> children; // child nodes
};

// Lexer class to convert input into tokens
class Lexer {
    std::string input;
    size_t pos = 0; // current position in input

public:
    Lexer(const std::string& text) : input(text) {}

    std::vector<Token> tokenize() {
        std::vector<Token> tokens;
        while (pos < input.size()) {
            char c = input[pos];
            if (std::isspace(c)) {
                pos++; // skip spaces
            }
            else if (std::isdigit(c)) {
                tokens.push_back(number());
            }
            else if (c == '+') { tokens.push_back({ "PLUS", "+" }); pos++; }
            else if (c == '-') { tokens.push_back({ "MINUS", "-" }); pos++; }
            else if (c == '*') { tokens.push_back({ "MUL", "*" }); pos++; }
            else if (c == '/') { tokens.push_back({ "DIV", "/" }); pos++; }
            else if (c == '(') { tokens.push_back({ "LPAREN", "(" }); pos++; }
            else if (c == ')') { tokens.push_back({ "RPAREN", ")" }); pos++; }
            else {
                throw std::runtime_error("Invalid char in input");
            }
        }
        tokens.push_back({ "EOF", "$" }); // end of input
        return tokens;
    }

private:
    // Reads full number (e.g. 123) as one token
    Token number() {
        size_t start = pos;
        while (pos < input.size() && std::isdigit(input[pos])) pos++;
        return { "ID", input.substr(start, pos - start) };
    }
};

// Parser class to check if input matches grammar and make tree
class Parser {
    std::vector<Token> tokens;
    size_t pos = 0;

public:
    std::shared_ptr<TreeNode> root; // root of parse tree
    Parser(const std::vector<Token>& tokens) : tokens(tokens) {}

    std::string parse() {
        try {
            root = E(); // start from E
            if (tokens[pos].type != "EOF") return "Extra input found!";
            return "Parsing successful! Press R to restart or Esc to exit.";
        }
        catch (const std::exception& e) {
            return std::string("Error: ") + e.what() + ". Press R to retry or Esc to exit.";
        }
    }

private:
    void advance() { if (pos < tokens.size()) pos++; }

    // Match current token with expected one
    void match(const std::string& expected, std::shared_ptr<TreeNode> node) {
        if (tokens[pos].type == expected) {
            node->children.push_back(std::make_shared<TreeNode>(TreeNode{ tokens[pos].value }));
            advance();
        }
        else {
            throw std::runtime_error("Expected " + expected + ", got " + tokens[pos].type);
        }
    }

    // Grammar rule E -> T E'
    std::shared_ptr<TreeNode> E() {
        auto node = std::make_shared<TreeNode>(TreeNode{ "E" });
        node->children.push_back(T());
        auto ep = E_();
        if (ep) node->children.push_back(ep);
        return node;
    }

    // Grammar rule E' -> + T E' | - T E' | ε
    std::shared_ptr<TreeNode> E_() {
        if (tokens[pos].type == "PLUS" || tokens[pos].type == "MINUS") {
            auto node = std::make_shared<TreeNode>(TreeNode{ "E'" });
            match(tokens[pos].type, node);
            node->children.push_back(T());
            auto next = E_();
            if (next) node->children.push_back(next);
            return node;
        }
        return nullptr; // epsilon
    }

    // Grammar rule T -> F T'
    std::shared_ptr<TreeNode> T() {
        auto node = std::make_shared<TreeNode>(TreeNode{ "T" });
        node->children.push_back(F());
        auto tp = T_();
        if (tp) node->children.push_back(tp);
        return node;
    }

    // Grammar rule T' -> * F T' | / F T' | ε
    std::shared_ptr<TreeNode> T_() {
        if (tokens[pos].type == "MUL" || tokens[pos].type == "DIV") {
            auto node = std::make_shared<TreeNode>(TreeNode{ "T'" });
            match(tokens[pos].type, node);
            node->children.push_back(F());
            auto next = T_();
            if (next) node->children.push_back(next);
            return node;
        }
        return nullptr; // epsilon
    }

    // Grammar rule F -> (E) | ID
    std::shared_ptr<TreeNode> F() {
        auto node = std::make_shared<TreeNode>(TreeNode{ "F" });
        if (tokens[pos].type == "ID") {
            match("ID", node);
        }
        else if (tokens[pos].type == "LPAREN") {
            match("LPAREN", node);
            node->children.push_back(E());
            match("RPAREN", node);
        }
        else {
            throw std::runtime_error("Invalid factor: " + tokens[pos].value);
        }
        return node;
    }
};

// Recursive function to draw parse tree
void drawTree(sf::RenderWindow& window, std::shared_ptr<TreeNode> node, float x, float y, float dx, sf::Font& font) {
    if (!node) return;
    sf::CircleShape circle(20);
    circle.setPosition(x - 20, y - 20);
    circle.setFillColor(sf::Color::Yellow);

    sf::Text label(node->label, font, 16);
    label.setFillColor(sf::Color::Black);
    label.setPosition(x - 15, y - 10);

    window.draw(circle);
    window.draw(label);

    float childX = x - dx * (node->children.size() - 1) / 2;
    for (auto& child : node->children) {
        sf::Vertex line[] = {
            sf::Vertex(sf::Vector2f(x, y)),
            sf::Vertex(sf::Vector2f(childX, y + 70))
        };
        window.draw(line, 2, sf::Lines);
        drawTree(window, child, childX, y + 70, dx / 2, font);
        childX += dx;
    }
}

// Main GUI function
int main() {
    sf::RenderWindow window(sf::VideoMode(900, 600), "LL(1) Parser with Visualizer");
    sf::Font font;
    font.loadFromFile("arial.ttf");

    std::string input;
    sf::Text userText("Input: ", font, 24);
    userText.setFillColor(sf::Color::White);
    userText.setPosition(10, 10);

    sf::Text resultText("Press Enter to Parse", font, 20);
    resultText.setFillColor(sf::Color::Green);
    resultText.setPosition(10, 40);

    std::shared_ptr<TreeNode> root = nullptr;

    float scrollOffset = 0;
    float zoom = 1.0f;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();
            else if (event.type == sf::Event::TextEntered) {
                if (event.text.unicode == 8 && !input.empty()) input.pop_back();
                else if (event.text.unicode < 128 && std::isprint(event.text.unicode))
                    input += static_cast<char>(event.text.unicode);
                userText.setString("Input: " + input);
            }
            else if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Enter) {
                    try {
                        Lexer lexer(input);
                        auto tokens = lexer.tokenize();
                        Parser parser(tokens);
                        resultText.setString(parser.parse());
                        root = parser.root;
                    }
                    catch (const std::exception& e) {
                        resultText.setString("Lexer error: " + std::string(e.what()) + ". Press R to retry or Esc to exit.");
                        root = nullptr;
                    }
                }
                else if (event.key.code == sf::Keyboard::Up) {
                    scrollOffset += 20;
                }
                else if (event.key.code == sf::Keyboard::Down) {
                    scrollOffset -= 20;
                }
                else if (event.key.code == sf::Keyboard::R) {
                    input = "";
                    userText.setString("Input: ");
                    resultText.setString("Press Enter to Parse");
                    root = nullptr;
                }
                else if (event.key.code == sf::Keyboard::Escape) {
                    window.close();
                }
            }
            else if (event.type == sf::Event::MouseWheelScrolled) {
                zoom += event.mouseWheelScroll.delta * 0.1f;
                if (zoom < 0.5f) zoom = 0.5f;
                if (zoom > 2.0f) zoom = 2.0f;
            }
        }

        window.clear(sf::Color(30, 30, 30));
        window.draw(userText);
        window.draw(resultText);
        if (root) drawTree(window, root, 450, 100 + scrollOffset, 150 * zoom, font); // show tree
        window.display();
    }
    return 0;
}
