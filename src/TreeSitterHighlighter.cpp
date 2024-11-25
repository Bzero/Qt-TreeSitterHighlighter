#include <QDebug>
#include <QtCore>
#include <QtGui>
#include <QtWidgets>

#include <cstddef>
#include <cstdint>
#include <cstring>

#include <tree_sitter/api.h>

#include "TreeSitterHighlighter.h"


// Minimal initisliser.
TreeSitterHighlighter::TreeSitterHighlighter(const TSLanguage* language, QTextDocument* document = nullptr): QObject(document) {
    // Set the language
    this->language = language;

    // Create a parser and set its language
    this->parser = ts_parser_new();
    bool set_language_success = ts_parser_set_language(this->parser, this->language);
    if (!set_language_success) {
        qDebug() << "Setting the parser language was not successful.\nCheck the language version compatibility.";
        // TODO: set a dummy language
    }

    // Create an empty query
    uint32_t error_offset;
    TSQueryError error_type;
    this->query = ts_query_new(this->language, "", 0, &error_offset, &error_type);

    // Create emty tree
    this->tree = ts_parser_parse_string_encoding(this->parser, NULL, "", 0, TSInputEncodingUTF16LE);

    // Create emtpty format map
    this->format_map = FormatMap();

    // Create empty lookup table
    this->format_lookup_table = new QTextCharFormat[0];

    // Set the document
    this->set_document(document);
}

// Full initialiser.
TreeSitterHighlighter::TreeSitterHighlighter(const TSLanguage* language,
                                             std::string query_string = "",
                                             FormatMap format_map = FormatMap(),
                                             QTextDocument* document = nullptr):
    TreeSitterHighlighter(language, document) {
    // Save the format map
    this->format_map = format_map;

    // Create query
    this->set_query(query_string);

    // Highlight the document
    this->rehighlight();
}

// Destructor.
TreeSitterHighlighter::~TreeSitterHighlighter() {
    ts_parser_delete(this->parser);
    ts_tree_delete(this->tree);
    ts_query_delete(this->query);
    delete[] this->format_lookup_table;
    this->document = nullptr;
}

// Set the highlight query.
// The return value indicates success, if not successful an empty query is created instead.
bool TreeSitterHighlighter::set_query(std::string query_string) {
    ts_query_delete(this->query);

    uint32_t error_offset;
    TSQueryError error_type;
    this->query = ts_query_new(this->language, query_string.c_str(), query_string.length(), &error_offset, &error_type);
    if (!this->query) {
        qDebug() << "The highlight query could not be created due to error" << error_type << " at char " << error_offset << ".";
        this->query = ts_query_new(this->language, "", 0, &error_offset, &error_type);
        return false;
    }

    generate_format_table();
    return true;
}

// Set the format map.
void TreeSitterHighlighter::set_format_map(FormatMap format_map) {
    this->format_map = format_map;
    generate_format_table();
}

// Return the format map.
const FormatMap TreeSitterHighlighter::get_format_map() {
    return this->format_map;
}

// Set the QTextDocument on which the syntax hihglighter is installed.
void TreeSitterHighlighter::set_document(QTextDocument* document) {
    if (this->document) {
        disconnect(this->document, SIGNAL(contentsChange(int, int, int)), this, SLOT(source_changed(int, int, int)));
        this->clear();
    }
    this->document = document;
    if (this->document) {
        this->reparse();
        connect(this->document, SIGNAL(contentsChange(int, int, int)), this, SLOT(source_changed(int, int, int)));
    }
}

// Return the QTextDocument on which this syntax highlighter is installed.
const QTextDocument* TreeSitterHighlighter::get_document() {
    return this->document;
}

// Handle a changed source.
void TreeSitterHighlighter::source_changed(int position, int charsRemoved, int charsAdded) {
    // reparse
    BlockRange changed_range = this->reparse_range(position, position + charsRemoved, position + charsAdded);

    // rehighlight
    this->rehighlight_range(changed_range);
}

// Reparse the selected block range.
BlockRange TreeSitterHighlighter::reparse_range(int start, int old_end, int new_end) {
    if (!this->document) {
        return BlockRange(0, 0);
    }

    // Create and edit the old tree
    TSTree* old_tree = ts_tree_copy(this->tree);
    QTextBlock start_block = this->document->findBlock(start);
    QTextBlock old_end_block = this->document->findBlock(old_end);
    QTextBlock new_end_block = this->document->findBlock(new_end);
    struct TSInputEdit edit = {
        .start_byte = (uint32_t)start * 2,
        .old_end_byte = (uint32_t)old_end * 2,
        .new_end_byte = (uint32_t)new_end * 2,
        .start_point = (TSPoint) {(uint32_t)start_block.blockNumber(), (uint32_t)(start - start_block.position()) * 2},
        .old_end_point = (TSPoint) {(uint32_t)old_end_block.blockNumber(), (uint32_t)(start - old_end_block.position()) * 2},
        .new_end_point = (TSPoint) {(uint32_t)new_end_block.blockNumber(), (uint32_t)(start - new_end_block.position()) * 2},
    };
    ts_tree_edit(old_tree, &edit);

    // reparse
    QString s = this->document->toPlainText();
    ts_tree_delete(this->tree);
    this->tree = ts_parser_parse_string_encoding(this->parser, old_tree, (char*)s.utf16(), s.length() * 2, TSInputEncodingUTF16LE);

    // Compute changed ranges
    int changed_ranges_start_pos = start;
    int changed_ranges_end_pos = std::max(old_end, new_end);
    uint32_t changed_ranges_length;
    TSRange* changed_ranges = ts_tree_get_changed_ranges(old_tree, this->tree, &changed_ranges_length);
    for (uint32_t i = 0; i != changed_ranges_length; i++) {
        TSRange range = changed_ranges[i];
        changed_ranges_start_pos = std::min(changed_ranges_start_pos / 2, (int)range.start_byte / 2);
        changed_ranges_end_pos = std::max(changed_ranges_end_pos / 2, (int)range.end_byte / 2);
    }
    BlockRange changed_range = BlockRange(this->document->findBlock(changed_ranges_start_pos).blockNumber(),
                                          this->document->findBlock(changed_ranges_end_pos).blockNumber());

    // cleanup
    free(changed_ranges);
    ts_tree_delete(old_tree);

    return changed_range;
}

// Reparse the whole document.
void TreeSitterHighlighter::reparse() {
    if (!this->document) {
        return;
    }

    QString s = this->document->toPlainText();
    ts_tree_delete(this->tree);
    this->tree = ts_parser_parse_string_encoding(this->parser, NULL, (char*)s.utf16(), s.length() * 2, TSInputEncodingUTF16LE);
}

// Clear the given block range.
inline void TreeSitterHighlighter::clear_range(BlockRange range) {
    if (!this->document) {
        return;
    }

    QTextBlock block = this->document->findBlockByNumber(range.first);
    while (block.isValid() && block.blockNumber() <= range.second) {
        block.layout()->clearFormats();
        block = block.next();
    }
}

// Clear the whole document.
inline void TreeSitterHighlighter::clear() {
    if (!this->document) {
        return;
    }

    BlockRange range = BlockRange(this->document->firstBlock().blockNumber(), this->document->lastBlock().blockNumber());
    this->clear_range(range);
}

// Rehighlight the changed range.
void TreeSitterHighlighter::rehighlight_range(BlockRange changed_range) {
    if (!this->document) {
        return;
    }

    // evaluate query in changed range
    QTextBlock start_block = this->document->findBlockByNumber(changed_range.first);
    QTextBlock stop_block = this->document->findBlockByNumber(changed_range.second);
    TSQueryCursor* query_cursor = ts_query_cursor_new();
    ts_query_cursor_set_byte_range(query_cursor, start_block.position() * 2, (stop_block.position() + stop_block.length()) * 2);
    ts_query_cursor_exec(query_cursor, this->query, ts_tree_root_node(this->tree));

    // Clear changed blocks
    this->clear_range(changed_range);

    // Highlight Captures
    TSQueryMatch match;
    uint32_t capture_index;
    while (ts_query_cursor_next_capture(query_cursor, &match, &capture_index)) {
        TSQueryCapture capture = match.captures[capture_index];

        QTextCharFormat text_char_format = this->format_lookup_table[capture.index];

        if (true) {  // TODO: skip if text_char_format is the standard format.
            // Single line capture
            if (ts_node_start_point(capture.node).row == ts_node_end_point(capture.node).row) {
                QTextBlock block = this->document->findBlockByNumber(ts_node_start_point(capture.node).row);
                this->apply_format(text_char_format,
                                   block,
                                   ts_node_start_point(capture.node).column / 2,
                                   ts_node_end_point(capture.node).column / 2);
            }
            // Multi line capture
            else if (ts_node_start_point(capture.node).row < ts_node_end_point(capture.node).row) {
                QTextBlock block = this->document->findBlockByNumber(ts_node_start_point(capture.node).row);
                this->apply_format(text_char_format, block, ts_node_start_point(capture.node).column / 2, block.length());
                block = block.next();
                while (block.isValid() && block.blockNumber() < ts_node_end_point(capture.node).row) {
                    this->apply_format(text_char_format, block, 0, block.length());
                    block = block.next();
                }
                this->apply_format(text_char_format, block, 0, ts_node_end_point(capture.node).column / 2);
            }
        }
    }

    ts_query_cursor_delete(query_cursor);
}

// Rehighlight the whole document.
void TreeSitterHighlighter::rehighlight() {
    if (!this->document) {
        return;
    }

    BlockRange changed_range = BlockRange(this->document->firstBlock().blockNumber(), this->document->lastBlock().blockNumber());
    this->rehighlight_range(changed_range);
}

// Get the QTextCharFormat from format_map which shares the longes matching (dot separated) prefix with name.
// In case no key with a shared prefix is found, format_map[""] is returned (defaulting to a new QTextCharFormat if not available).
QTextCharFormat TreeSitterHighlighter::get_format_for_capture_name(std::string name) {
    if (this->format_map.count(name)) {
        return this->format_map.at(name);
    } else if (name.find(".") == std::string::npos) {
        return this->format_map[""];
    } else {
        return this->get_format_for_capture_name(name.substr(0, name.rfind(".")));
    }
}

// Generate a lookup table that maps all query match indices to their QTextCharFormats defined in format_map.
void TreeSitterHighlighter::generate_format_table() {
    delete[] this->format_lookup_table;

    uint32_t capture_count = ts_query_capture_count(this->query);
    QTextCharFormat* table = new QTextCharFormat[capture_count];

    for (uint32_t i = 0; i != capture_count; i++) {
        uint32_t length;
        std::string name = std::string(ts_query_capture_name_for_id(this->query, i, &length));
        table[i] = this->get_format_for_capture_name(name);
    }

    this->format_lookup_table = table;
}

// Apply the the format to block from position start to end.
inline void TreeSitterHighlighter::apply_format(QTextCharFormat format, QTextBlock block, int start, int end) {
    QTextLayout::FormatRange r;
    r.start = start;
    r.length = end - start;
    r.format = format;
    // TODO: Skip preedit area
    QList<QTextLayout::FormatRange> ranges = block.layout()->formats();
    ranges << r;
    block.layout()->setFormats(ranges);
    this->document->markContentsDirty(block.position(), block.length());
}
