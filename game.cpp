#include"game.h"
// Rehan
const string BIG_LINE(72, '=');
const string SMALL_LINE(72, '-');

// Removes spaces, tabs and line endings from both sides of a text.
string trimText(const string& text) {
    int start = 0;
    int end = (int)text.length() - 1;
    while (start <= end && isspace((unsigned char)text[start])) start++;
    while (end >= start && isspace((unsigned char)text[end])) end--;
    return text.substr(start, end - start + 1);
}
string toUpperText(string text) {
    for (int i = 0; i < (int)text.length(); i++) text[i] = upperChar(text[i]);
    return text;
}
// Compares two strings without caring about capital/small letters.
bool sameIgnoringCase(const string& first, const string& second) {
    return toUpperText(first) == toUpperText(second);
}
string currentTimestamp() {
    time_t now = time(nullptr);
    char buffer[64];
    strftime(buffer, sizeof(buffer), "%d-%m-%y", localtime(&now));
    return string(buffer);
}
// Prints a title between two lines of '=' (title is centered).
void printTitle(const string& title) {
    int space = (72 - (int)title.length()) / 2;
    if (space < 0) space = 0;
    cout << "\n" << BIG_LINE << "\n" << string(space, ' ') << title << "\n" << BIG_LINE << "\n";
}
// Asks the user for a number between low and high. Keeps asking until the input is valid.
int readNumber(int low, int high, const string& prompt) {
    while (true) {
        cout << prompt;
        string line;
        if (!getline(cin, line)) {
            cout << "\n[Session ended.]\n";
            exit(0);
        }
        line = trimText(line);
        if (line.empty()) {
            cout << "Input cannot be empty. Please enter a number.\n";
            continue;
        }
        char* endPointer = nullptr;
        long value = strtol(line.c_str(), &endPointer, 10);
        if (*endPointer != '\0') {
            cout << "That is not a valid number. Please try again.\n";
            continue;
        }
        if (value < low || value > high) {
            cout << "Out of range! Please enter a number between " << low << " and " << high << ".\n";
            continue;
        }
        return (int)value;
    }
}
void waitForEnter() {
    string ignored;
    if (!getline(cin, ignored)) {
        cout << "\n[Session ended.]\n";
        exit(0);
    }
}
void Victim::display() const {
    cout << "\nVictim: " << name << " - " << role << " (" << nationality << ")\n";
}
void Detective::display() const {
    cout << "\nDetective: " << name << "\n"
         << "Credibility: " << credibility << "/100\n"
         << "Turns used: " << turnsUsed << "/" << maxTurns << "\n";
}
void Suspect::display() const {
    cout << "\nName: " << name << "\n"
         << "Role: " << role << " (" << nationality << ")\n"
         << "Type: " << getCategoryName() << "\n"
         << "Alibi: " << alibi << "\n";
}
void BusinessRelations::display() const {
    Suspect::display();
}
void HouseHelp::display() const {
    Suspect::display();
}
void CloseRelations::display() const {
    Suspect::display();
}
void PA::display() const {
    Suspect::display();
}
int TileRack::countLetter(char letter) const {
    int total = 0;
    for (int i = 0; i < tiles.size(); i++)
        if (tiles[i] == letter) total++;
    return total;
}
void TileRack::addTile(const Tile& tile) {
    if (tiles.size() >= maxTiles)
        throw RackFullException("Cannot add tile '" + string(1, tile.letter) + "': the rack is full.");
    tiles.push_back(tile);
}
bool TileRack::removeLetter(char letter) {
    for (int i = 0; i < tiles.size(); i++) {
        if (tiles[i] == letter) {
            tiles.removeAt(i);
            return true;
        }
    }
    return false;
}
Tile TileRack::removeTileAt(int index) {
    Tile removed = tiles[index];
    tiles.removeAt(index);
    return removed;
}
TileRack TileRack::operator+(const TileRack& other) const {
    TileRack merged(maxTiles + other.maxTiles);
    for (int i = 0; i < tiles.size(); i++) merged.addTile(tiles[i]);
    for (int i = 0; i < other.tiles.size(); i++) merged.addTile(other.tiles[i]);
    return merged;
}
bool TileRack::operator==(const TileRack& other) const {
    if (tiles.size() != other.tiles.size()) return false;
    for (int i = 0; i < tiles.size(); i++)
        if (countLetter(tiles[i].letter) != other.countLetter(tiles[i].letter)) return false;
    return true;
}
ScrabbleBoard::ScrabbleBoard() {
    reset();
}
void ScrabbleBoard::reset() {
    for (int row = 0; row < 2; row++) {
        rows[row].clear();
        for (int slot = 0; slot < getRowLength((WordRow)row); slot++) rows[row].push_back(Tile());
    }
}
int ScrabbleBoard::getRowLength(WordRow row) const {
    return (row == Word_1) ? 7 : 8;
}
void ScrabbleBoard::checkPosition(WordRow row, int position) const {
    int length = getRowLength(row);
    if (position < 0 || position >= length)
        throw InvalidPositionException("Invalid board position " + to_string(position + 1)
                                       + ". Expected a slot between 1 and " + to_string(length));
}
void ScrabbleBoard::placeTile(WordRow row, int position, char letter) {
    checkPosition(row, position);
    rows[row][position] = Tile(upperChar(letter), true);
}
bool ScrabbleBoard::removeTile(WordRow row, int position, char& removedLetter) {
    checkPosition(row, position);
    if (!rows[row][position].filled) return false;
    removedLetter = rows[row][position].letter;
    rows[row][position] = Tile();
    return true;
}
bool ScrabbleBoard::isSlotFilled(WordRow row, int position) const {
    checkPosition(row, position);
    return rows[row][position].filled;
}
bool ScrabbleBoard::isRowFull(WordRow row) const {
    for (int slot = 0; slot < getRowLength(row); slot++)
        if (!rows[row][slot].filled) return false;
    return true;
}
string ScrabbleBoard::getRowText(WordRow row) const {
    string text = "";
    for (int slot = 0; slot < getRowLength(row); slot++)
        text += rows[row][slot].filled ? rows[row][slot].letter : '_';
    return text;
}
void ScrabbleBoard::validateRow(WordRow row, const string& expectedWord) const {
    if (!isRowFull(row)) throw InvalidWordException("Cannot validate: the row still has empty slots.");
    string formed = getRowText(row);
    if (!sameIgnoringCase(formed, expectedWord))
        throw InvalidWordException("Submitted word '" + formed + "' is incorrect! Penalties applied.");
}
// Shows both words. Empty slots are shown as '_'.
void ScrabbleBoard::display() const {
    for (int row = 0; row < 2; row++) {
        cout << "Word " << (row + 1) << ": ";
        for (int slot = 0; slot < getRowLength((WordRow)row); slot++)
            cout << (rows[row][slot].filled ? rows[row][slot].letter : '_') << ' ';
        cout << "\n";
    }
}
