#include "ui.h"

// Convert character to raylib key code
int keyname_to_key(char keyname)
{
    // Convert to uppercase for consistency
    if (keyname >= 'a' && keyname <= 'z')
    {
        keyname = keyname - 'a' + 'A';
    }

    // Letters A-Z
    if (keyname >= 'A' && keyname <= 'Z')
    {
        return KEY_A + (keyname - 'A');
    }

    // Numbers 0-9
    if (keyname >= '0' && keyname <= '9')
    {
        return KEY_ZERO + (keyname - '0');
    }

    // Special characters and keys
    switch (keyname)
    {
    case ' ':
        return KEY_SPACE;
    case '\n':
    case '\r':
        return KEY_ENTER;
    case '\t':
        return KEY_TAB;
    case '\b':
        return KEY_BACKSPACE;
    case 27:
        return KEY_ESCAPE; // ESC character
    case '.':
        return KEY_PERIOD;
    case ',':
        return KEY_COMMA;
    case ';':
        return KEY_SEMICOLON;
    case '\'':
        return KEY_APOSTROPHE;
    case '/':
        return KEY_SLASH;
    case '\\':
        return KEY_BACKSLASH;
    case '[':
        return KEY_LEFT_BRACKET;
    case ']':
        return KEY_RIGHT_BRACKET;
    case '=':
        return KEY_EQUAL;
    case '-':
        return KEY_MINUS;
    case '`':
        return KEY_GRAVE;
    default:
        return -1; // Unknown key
    }
}

// Convert raylib key code to character (returns 0 if not convertible)
char key_to_keyname(int key)
{
    // Letters A-Z
    if (key >= KEY_A && key <= KEY_Z)
    {
        return 'A' + (key - KEY_A);
    }

    // Numbers 0-9
    if (key >= KEY_ZERO && key <= KEY_NINE)
    {
        return '0' + (key - KEY_ZERO);
    }

    // Special keys
    switch (key)
    {
    case KEY_SPACE:
        return ' ';
    case KEY_ENTER:
        return '\n';
    case KEY_TAB:
        return '\t';
    case KEY_BACKSPACE:
        return '\b';
    case KEY_ESCAPE:
        return 27; // ESC character
    case KEY_PERIOD:
        return '.';
    case KEY_COMMA:
        return ',';
    case KEY_SEMICOLON:
        return ';';
    case KEY_APOSTROPHE:
        return '\'';
    case KEY_SLASH:
        return '/';
    case KEY_BACKSLASH:
        return '\\';
    case KEY_LEFT_BRACKET:
        return '[';
    case KEY_RIGHT_BRACKET:
        return ']';
    case KEY_EQUAL:
        return '=';
    case KEY_MINUS:
        return '-';
    case KEY_GRAVE:
        return '`';
    default:
        return 0; // Non-printable or unknown key
    }
}

int get_midi_note_from_key(char keyname, int octave)
{
    int base_note = 60 + (octave - 4) * 12;

    // Check lower row (white keys) - C D E F G A B
    for (int i = 0; i < 7; i++)
    {
        if (keyname == piano_keys_lower[i])
        {
            int note_offsets[] = {0, 2, 4, 5, 7, 9, 11}; // C D E F G A B
            return base_note + note_offsets[i];
        }
    }

    // Check upper row (black keys) - C# D# F# G# A#
    for (int i = 0; i < 5; i++)
    {
        if (keyname == piano_keys_upper[i])
        {
            int note_offsets[] = {1, 3, 6, 8, 10}; // C# D# F# G# A#
            return base_note + note_offsets[i];
        }
    }

    return -1;
}
