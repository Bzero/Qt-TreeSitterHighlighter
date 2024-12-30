#include <QtGui>
#include <QtWidgets>

#include <string>

#include <TreeSitterHighlighter.h>

// Declare the 'tree_sitter_cpp' function provided by the 'tree-sitter-cpp' parser library.
extern "C" {
const TSLanguage* tree_sitter_cpp(void);
}

// A minimal highlights query for C++
// See https://tree-sitter.github.io/tree-sitter/using-parsers/queries/1-syntax.html for query syntax.
std::string query = R"(
["extern" "catch" "class" "delete" "namespace" "new" "private" "protected" "public"
 "throw" "try" "requires" "virtual" "break" "case" "const" "continue" "do" "else"
 "enum" "for" "if"  "inline" "return" "static" "struct" "switch" "typedef" "while" ] @keyword

["#define" "#elif" "#else" "#endif" "#if" "#ifdef" "#ifndef" "#include" ] @preproc
(preproc_directive) @preproc

(number_literal) @number

(true) @bool
(false) @bool

(string_literal) @string.std
(raw_string_literal) @string.raw

(type_identifier) @type
(primitive_type) @type
(sized_type_specifier) @type

(field_identifier) @variable
(identifier) @variable

(call_expression
    function: (identifier) @function)
(call_expression
    function: (field_expression
    field: (field_identifier) @function))
(function_declarator
    declarator: (identifier) @function)

(comment) @comment
)";

// A format map from capture names to the appropriate QTextCharFormat format
FormatMap format_map() {
    FormatMap map;

    map[""].setForeground(QColor("black"));
    map["keyword"].setForeground(QColor("#b055cc"));
    map["preproc"].setForeground(QColor("purple"));
    map["number"].setFontUnderline(true);
    map["bool"].setFontWeight(QFont::Bold);
    map["string"].setForeground(QColor("green"));
    map["string.raw"].setForeground(QColor("dark green"));
    map["type"].setForeground(QColor("#c08000"));
    map["variable"].setForeground(QColor("#ad1d4d"));
    map["function"].setForeground(QColor("blue"));
    map["comment"].setForeground(QColor("gray"));
    map["comment"].setFontItalic(true);

    return map;
}

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    QMainWindow window = QMainWindow();

    // Create the editor
    QPlainTextEdit* editor = new QPlainTextEdit();

    // Create the highlighter and use it on the editors document
    TreeSitterHighlighter* highlighter = new TreeSitterHighlighter(tree_sitter_cpp(), query, format_map(), editor->document());

    // Set font
    QFont font;
    font.setFamily("Monospace");
    font.setFixedPitch(true);
    font.setPointSize(12);
    editor->setFont(font);

    // Open this file in in the QPlainTextEdit as an example
    QFile file(__FILE__);
    if (file.open(QFile::ReadOnly | QFile::Text)) {
        editor->setPlainText(file.readAll());
    }

    // Setup the main window
    window.setCentralWidget(editor);
    window.setWindowTitle("Tree-Sitter Syntax Hihglighter");
    window.resize(800, 600);
    window.show();
    return app.exec();
}
