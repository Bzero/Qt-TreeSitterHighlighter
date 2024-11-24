#ifndef TREESITTERHIGHLIGHTER_H
#define TREESITTERHIGHLIGHTER_H

#include <QtCore>
#include <QtGui>
#include <QtWidgets>

#include <map>
#include <string>
#include <utility>

#include <tree_sitter/api.h>

/// Represents a range(inclusive) of [QTextBlocks](https://doc.qt.io/qt-6/qtextblock.html) in a document.
typedef std::pair<int, int> BlockRange;

/// A mapping of capture names to their syntax highlighting formats.
typedef std::map<std::string, QTextCharFormat> FormatMap;

/// TreeSitterHighlighter is a syntax highlighter for [QTextDocument](https://doc.qt.io/qt-6/qtextdocument.html) based on tree-sitter.
///
/// For functional highlighting it must be provided with the following:
///
/// * A tree-sitter [language](https://tree-sitter.github.io/tree-sitter/using-parsers#the-basic-objects) to parse the source and construct a syntax tree.
/// * A tree-sitter highlighting [query string](https://tree-sitter.github.io/tree-sitter/using-parsers#query-syntax) which extracts the relevant captures from the syntax tree.
/// * A highlighting format map which maps capture names to their desired [QTextCharFormat](https://doc.qt.io/qt-6/qtextcharformat.html) format.
///
/// The highlighter can be applied to any [QTextDocument](https://doc.qt.io/qt-6/qtextdocument.html),
/// in particular to the documents of [QPlainTextEdit](https://doc.qt.io/qt-6/qplaintextedit.html)
/// and [QTextEdit](https://doc.qt.io/qt-6/qtextedit.html) and will rehighlight the document whenever it changes.
class TreeSitterHighlighter: public QObject {
    Q_OBJECT

  public:
    /// Constructor. Constructs a minimal TreeSitterHighlighter.
    /// @param language A tree-sitter language.
    /// @param document A [QTextDocument](https://doc.qt.io/qt-6/qtextdocument.html) to which the highlighter will be connected and which becomes its parent.
    TreeSitterHighlighter(const TSLanguage* language, QTextDocument* document);
    /// Constructor. Constructs a fully functional TreeSitterHighlighter and connects it to document.
    /// @param language A tree-sitter language.
    /// @param query_string A tree-sitter highlighting [query string](https://tree-sitter.github.io/tree-sitter/using-parsers#query-syntax).
    /// @param format_map A map of query capture names to their [QTextCharFormat](https://doc.qt.io/qt-6/qtextcharformat.html) format.
    /// @param document A [QTextDocument](https://doc.qt.io/qt-6/qtextdocument.html) document to which the highlighter will be connected and which becomes its parent.
    TreeSitterHighlighter(const TSLanguage* language, std::string query_str, FormatMap format_map, QTextDocument* document);
    /// Destructor. Disconnects the syntax highlighter from its document.
    ~TreeSitterHighlighter();

  public:
    /// Connects the syntax highlighter to the given [QTextDocument](https://doc.qt.io/qt-6/qtextdocument.html). This will disconnect it from the previous document.
    /// @param document The [QTextDocument](https://doc.qt.io/qt-6/qtextdocument.html) document to connect to.
    void set_document(QTextDocument* document);
    /// Return the [QTextDocument](https://doc.qt.io/qt-6/qtextdocument.html) to which this syntax highlighter is connected.
    /// @return The highlighters document.
    const QTextDocument* get_document();
    /// Set the query from a [query pattern string](https://tree-sitter.github.io/tree-sitter/using-parsers#query-syntax) to use for syntax highlighting.
    /// @param query_string A highlighting query string.
    /// @return Indicates if the query could be created successfully.
    bool set_query(std::string query_string);
    /// Set the format map.
    /// @param format_map A map of capture names to their desired [QTextCharFormat](https://doc.qt.io/qt-6/qtextcharformat.html) format.
    void set_format_map(FormatMap format_map);
    /// Return the highlighters format map.
    /// @return The highlighters frmat map.
    const FormatMap get_format_map();

  public slots:
    /// Reparse the whole document.
    void reparse();
    /// Apply syntax highlighting to the whole document.
    void rehighlight();
    /// Clear the highlighting of the whole document.
    void clear();

  protected:
    /// The highlighters language.
    const TSLanguage* language;
    /// The highlighters parser.
    TSParser* parser;
    /// The current syntax tree of the document.
    TSTree* tree;
    /// The highlight query.
    TSQuery* query;
    /// The highlight format map.
    FormatMap format_map;
    /// The document to which the highlighter is connected to.
    QPointer<QTextDocument> document;

  private:
    // A lookup table mapping query capture indices to syntax highlighting formats.
    const QTextCharFormat* format_lookup_table;

  private:
    // Apply format to block from character start to end.
    void apply_format(QTextCharFormat format, QTextBlock block, int start, int end);
    // Reparse the document from start to old_end(before the edit) and new_end(after the edit).
    BlockRange reparse_range(int start, int old_end, int new_end);
    // Rehighlight the text blocks changed_range.
    void rehighlight_range(BlockRange changed_range);
    // Clear the text blocks range.
    void clear_range(BlockRange range);
    // Generate the format lookup table from the highlighters syle map.
    void generate_format_table();
    // Get the syntax highlighting format for name.
    QTextCharFormat get_format_for_capture_name(std::string name);

  private slots:
    // Notify the syntaxhighlighter of a change in the document at position with charsRemoved characters removed and charsAdded characters added.
    // This slot is meant to be connected to QTextDocument::contentsChange().
    void source_changed(int position, int charsRemoved, int charsAdded);

  private:
    Q_DISABLE_COPY(TreeSitterHighlighter);
};

#endif
