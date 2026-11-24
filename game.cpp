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
//Haad
DynamicArray<string> FileManager::splitFields(const string& line) {
    DynamicArray<string> fields;
    stringstream stream(line);
    string field;
    while (getline(stream, field, '|')) fields.push_back(field);
    return fields;
}
// Reads the next useful line from a data file and splits it at '|'.
// Empty lines and lines starting with '#' are skipped. Returns false at end of file.
bool FileManager::readDataLine(ifstream& file, DynamicArray<string>& fields) {
    string line;
    while (getline(file, line)) {
        line = trimText(line);
        if (line.empty() || line[0] == '#') continue;
        fields = splitFields(line);
        return true;
    }
    return false;
}
// The columns in the data files are named P1 and P2, but the roles are called "Partner 1" and "Partner 2".
string FileManager::columnToRole(const string& column) {
    if (column == "P1") return "Partner 1";
    if (column == "P2") return "Partner 2";
    return column;
}
// Finds the column of a role in a header line. Returns -1 if it is not found.
int FileManager::findColumn(const DynamicArray<string>& header, int firstColumn, const string& role) {
    for (int i = firstColumn; i < header.size(); i++)
        if (columnToRole(header[i]) == role) return i;
    return -1;
}
// Looks in story.txt for a line that starts with the key (a role or a place name).
bool FileManager::findStory(const string& key, DynamicArray<string>& texts) {
    ifstream file("story.txt");
    string header;
    getline(file, header);
    DynamicArray<string> fields;
    while (readDataLine(file, fields)) {
        if (fields.size() >= 2 && fields[0] == key) {
            texts = fields;
            return true;
        }
    }
    return false;
}
// Adds one line to leader/scoreBoard.txt (Detective|Result|Credibility|Turns_Left|Culprit|Date).
void FileManager::appendScore(const LeaderboardEntry& entry) {
    ofstream file("leader/scoreBoard.txt", ios::app);
    file << entry.playerName << "|" << entry.outcome << "|" << entry.credibility << "|"
         << setw(2) << setfill('0') << entry.turnsLeft << setfill(' ') << "|"
         << entry.culprit << "|" << entry.date << "\n";
}
void FileManager::loadScores(DynamicArray<LeaderboardEntry>& entries) {
    entries.clear();
    ifstream file("leader/scoreBoard.txt");
    string header;
    getline(file, header);
    DynamicArray<string> fields;
    while (readDataLine(file, fields)) {
        if (fields.size() < 6) continue;
        entries.push_back(LeaderboardEntry(fields[0], fields[1], atoi(fields[2].c_str()),
                                           atoi(fields[3].c_str()), fields[4], fields[5]));
    }
}
GameEngine::GameEngine()
    : state(GameState::Menu), detective(nullptr), victim(nullptr),
      word1(""), word2(""), culpritIndex(0),
      word1Solved(false), word2Solved(false), cipherMessageShown(false) {}
GameEngine::~GameEngine() {
    cleanUp();
}
GameEngine& GameEngine::getInstance() {
    static GameEngine instance;
    return instance;
}
void GameEngine::cleanUp() {
    delete detective;
    detective = nullptr;
    delete victim;
    victim = nullptr;
    for (int i = 0; i < suspects.size(); i++) delete suspects[i];
    suspects.clear();
    locations.clear();
    journal.clear();
    rack.clear();
}
// Main loop of the program: shows the main menu until the user exits.
void GameEngine::run() {
    srand((unsigned int)time(nullptr));
   
    while (state != GameState::Exit) {
        showMainMenu();
        int choice = readNumber(1, 5, "Choose (1-5): ");
        if (choice == 1) startNewGame();
        else if (choice == 2) showLeaderboard();
        else if (choice == 3) showTextFile("about.txt");
        else if (choice == 4) showTextFile("rules.txt");
        else {
            state = GameState::Exit;
            cout << "\nGoodbye!\n";
        }
    }
}
void GameEngine::showMainMenu() {
    cout <<"============================================================\n"
        << "                                                            \n"
        << "               LOCKER HEIST-MAINMENU                        \n"
        << "                                                            \n"
        << "============================================================\n"
        << "   1. New Game                                              \n"
        << "   2. Leaderboard                                           \n"
        << "   3. About                                                 \n"
        << "   4. Rules                                                 \n"
        << "   5. Exit                                                  \n"
        << "============================================================\n";
}
void GameEngine::showTextFile(const string& filename) {
    ifstream file(filename);
    string line;
    cout << "\n";
    while (getline(file, line)) cout << line << "\n";
    cout << "Press ENTER to return to Main Menu...";
    waitForEnter();
}
// Win comes first, then higher credibility, then more turns left.
bool isBetterEntry(const LeaderboardEntry& first, const LeaderboardEntry& second) {
    if ((first.outcome == "WIN") != (second.outcome == "WIN")) return first.outcome == "WIN";
    if (first.credibility != second.credibility) return first.credibility > second.credibility;
    return first.turnsLeft > second.turnsLeft;
}
void GameEngine::showLeaderboard() {
    DynamicArray<LeaderboardEntry> entries;
    FileManager::loadScores(entries);
    printTitle("DETECTIVE HALL OF FAME (LEADERBOARD)");
    if (entries.isEmpty()) {
        cout << " No previous case records found. Solve a mystery to register your name!\n";
    } else {
        for (int i = 0; i < entries.size() - 1; i++)
            for (int j = 0; j < entries.size() - i - 1; j++)
                if (isBetterEntry(entries[j + 1], entries[j])) customSwap(entries[j], entries[j + 1]);
        cout << left << setw(6) << "RANK" << setw(20) << "DETECTIVE" << setw(8) << "RESULT"
             << setw(13) << "CREDIBILITY" << setw(12) << "TURNS LEFT" << setw(12) << "CULPRIT"
             << setw(10) << "DATE" << "\n" << SMALL_LINE << "\n";
        int rowsToShow = customMin(entries.size(), 10);
        for (int i = 0; i < rowsToShow; i++) {
            cout << left << setw(6) << (i + 1) << setw(20) << entries[i].playerName
                 << setw(8) << entries[i].outcome << setw(13) << entries[i].credibility
                 << setw(12) << entries[i].turnsLeft << setw(12) << entries[i].culprit
                 << setw(10) << entries[i].date << "\n";
        }
    }
    cout << BIG_LINE << "\nPress ENTER to return to Main Menu...";
    waitForEnter();
}
// Sets up a new game: creates the detective, picks a random culprit, loads the files.
void GameEngine::startNewGame() {
    cleanUp();
    cout << "\nEnter your name (or press ENTER for Raymond Vance): ";
    string playerName;
    getline(cin, playerName);
    playerName = trimText(playerName);
    if (playerName.empty()) playerName = "Raymond Vance";
    detective = new Detective(playerName);
    board.reset();
    word1Solved = false;
    word2Solved = false;
    cipherMessageShown = false;
    chooseCulpritRole();
    loadWords();
    loadLocations();
    loadCharacters();
    pickCulprit();
    state = GameState::Playing;
    playGame();
}
// Picks a random culprit role from the role columns in the header of characters.txt.
void GameEngine::chooseCulpritRole() {
    ifstream file("characters.txt");
    string header;
    getline(file, header);
    DynamicArray<string> columns = FileManager::splitFields(trimText(header));
    culpritRole = "";
    if (columns.size() < 6) return;
    int roleCount = columns.size() - 5;
    culpritRole = FileManager::columnToRole(columns[4 + rand() % roleCount]);
}
// Reads clue.txt (format: Culprit_Role|Word_1|Word_2) and takes the words of the culprit.
void GameEngine::loadWords() {
    word1 = "";
    word2 = "";
    ifstream file("clue.txt");
    string header;
    getline(file, header);
    DynamicArray<string> fields;
    while (FileManager::readDataLine(file, fields)) {
        if (fields.size() >= 3 && fields[0] == culpritRole) {
            word1 = toUpperText(fields[1]);
            word2 = toUpperText(fields[2]);
        }
    }
}
// Reads locations.txt (format: Place|W1Pos|Cook|Guard|...). Takes the letter from the culprit's column.
void GameEngine::loadLocations() {
    ifstream file("locations.txt");
    string header;
    getline(file, header);
    int letterColumn = FileManager::findColumn(FileManager::splitFields(trimText(header)), 2, culpritRole);
    DynamicArray<string> fields;
    while (FileManager::readDataLine(file, fields)) {
        if (letterColumn < 0 || fields.size() <= letterColumn) continue;
        char letter = fields[letterColumn].empty() ? ' ' : fields[letterColumn][0];
        int position = atoi(fields[1].c_str()) - 1;
        DynamicArray<string> story;
        string description = "";
        if (FileManager::findStory(fields[0], story)) description = story[1];
        locations.push_back(Location(fields[0], letter, position, description));
    }
}
// Reads characters.txt (format: Name|Role|Nationality|W2Pos|Cook|Guard|...|isCulprit).
// Takes the letter from the culprit's column and the talk texts from story.txt.
void GameEngine::loadCharacters() {
    ifstream file("characters.txt");
    string header;
    getline(file, header);
    int letterColumn = FileManager::findColumn(FileManager::splitFields(trimText(header)), 4, culpritRole);
    DynamicArray<string> fields;
    while (FileManager::readDataLine(file, fields)) {
        if (letterColumn < 0 || fields.size() <= letterColumn) continue;
        string name = fields[0];
        string role = fields[1];
        string nationality = fields[2];
        int position = atoi(fields[3].c_str()) - 1;
        char letter = fields[letterColumn].empty() ? ' ' : fields[letterColumn][0];
        string alibi = "";
        string statement = "";
        string confession = "";
        DynamicArray<string> story;
        if (FileManager::findStory(role, story)) {
            alibi = story[1];
            if (story.size() > 2) statement = story[2];
            if (story.size() > 3) confession = story[3];
        }
        if (role == "Victim")
            victim = new Victim(name, role, nationality, 80, alibi, letter, position);
        else if (role == "Partner 1" || role == "Partner 2")
            suspects.push_back(new BusinessRelations(name, role, nationality, letter, position, alibi, statement, confession));
        else if (role == "Cook" || role == "Guard" || role == "Servant")
            suspects.push_back(new HouseHelp(name, role, nationality, letter, position, alibi, statement, confession));
        else if (role == "Child")
            suspects.push_back(new CloseRelations(name, role, nationality, letter, position, alibi, statement, confession));
        else if (role == "PA")
            suspects.push_back(new PA(name, role, nationality, letter, position, alibi, statement, confession));
    }
    if (victim == nullptr) victim = new Victim();
}
// Marks the suspect with the culprit role as the real culprit.
void GameEngine::pickCulprit() {
    for (int i = 0; i < suspects.size(); i++) {
        if (suspects[i]->getRole() == culpritRole) {
            culpritIndex = i;
            suspects[i]->setCulprit(true);
        }
    }
}
// Game loop: show the menu, do the chosen action, handle exceptions, check win/lose.
void GameEngine::playGame() {
    cout << "\nCase: Locker #77 was robbed at the Laurent Manor.\n"
         << "Search locations to find the letters of Word 1 (7 letters).\n"
         << "Talk to the victim and suspects to find the letters of Word 2 (8 letters).\n"
         << "Every letter you find is placed on the board automatically.\n";
    while (state == GameState::Playing) {
        showDashboard();
        int action = readNumber(1, 7, "Choose (1-7): ");
        try {
            if (action == 1) searchLocation();
            else if (action == 2) interrogateSuspect();
            else if (action == 3) showClues();
            else if (action == 4) accuseCulprit();
            else if (action == 5) showDetectiveFile();
            else if (action == 6) showRackMerge();
            else forfeitGame();
        }
        catch (const LowCredibilityException& error) {
            cout << "\n" << error.what() << "\n";
            detective->loseCredibility(5);
            detective->useTurns(1);
            cout << "Penalty: -5 credibility and 1 turn lost.\n";
        }
        catch (const AlreadyVisitedException& error) {
            cout << "\n" << error.what() << "\n";
        }
        catch (const GameException& error) {
            cout << "\n" << error.what() << "\n";
        }
        checkGameEnd();
    }
    finishGame();
}
void GameEngine::showDashboard() {
    cout << "\n" << SMALL_LINE << "\n"
         << "Turn: " << detective->getTurnsUsed() << "/" << detective->getMaxTurns()
         << "   Credibility: " << detective->getCredibility() << "/100\n";
    board.display();
    cout << SMALL_LINE << "\n"
         << "1. Search a location\n"
         << "2. Talk to someone\n"
         << "3. See clues\n"
         << "4. Accuse the culprit\n"
         << "5. Detective status\n"
         << "6. Rack merge demo\n"
         << "7. Give up\n";
}
// Searching a location costs 1 turn and gives 1 letter for Word 1.
void GameEngine::searchLocation() {
    cout << "\nLocations:\n";
    for (int i = 0; i < locations.size(); i++)
        cout << i + 1 << ". " << locations[i].name << (locations[i].searched ? " (done)" : "") << "\n";
    cout << "0. Back\n";
    int choice = readNumber(0, locations.size(), "Choose location: ");
    if (choice == 0) return;
    Location& place = locations[choice - 1];
    if (place.searched)
        throw AlreadyVisitedException("You already searched " + place.name + ".");
    detective->useTurns(1);
    place.searched = true;
    cout << "\n" << place.name << ": " << place.description << "\n";
    journal.push_back(Clue(place.letter, place.position, Word_1, place.name, place.description));
    placeFoundTile(place.letter, place.position, Word_1);
}
// Shows everyone in the manor and lets the player pick who to talk to.
void GameEngine::interrogateSuspect() {
    cout << "\nPeople:\n"
         << "0. " << victim->getName() << " (Victim) - needs " << victim->getRequiredCredibility() << " credibility"
         << (victim->isInterviewed() ? " (done)" : "") << "\n";
    for (int i = 0; i < suspects.size(); i++) {
        cout << i + 1 << ". " << suspects[i]->getName() << " (" << suspects[i]->getRole() << ") - needs "
             << suspects[i]->getRequiredCredibility() << " credibility"
             << (suspects[i]->isInterviewed() ? " (done)" : "") << "\n";
    }
    cout << "-1. Back\n";
    int choice = readNumber(-1, suspects.size(), "Choose person: ");
    if (choice == -1) return;
    if (choice == 0) interviewVictim();
    else interviewSuspect(suspects[choice - 1]);
}
// Talking to the victim needs credibility 80 or more. Gives 1 letter for Word 2.
void GameEngine::interviewVictim() {
    if (victim->isInterviewed())
        throw AlreadyVisitedException("You already talked to " + victim->getName() + ".");
    if (detective->getCredibility() < victim->getRequiredCredibility())
        throw LowCredibilityException(victim->getName() + " will only talk if your credibility is "
                                      + to_string(victim->getRequiredCredibility()) + " or more.");
    detective->useTurns(1);
    victim->markInterviewed();
    cout << "\n" << victim->getName() << ": \"" << victim->getTestimony() << "\"\n"
         << victim->getName() << ": \"Check whose alibi has no witness.\"\n";
    journal.push_back(Clue(victim->getLetter(), victim->getPosition(), Word_2,
                           victim->getName(), victim->getTestimony()));
    placeFoundTile(victim->getLetter(), victim->getPosition(), Word_2);
}
// Talking to a suspect costs 1 turn and gives 1 letter for Word 2.
void GameEngine::interviewSuspect(Suspect* suspect) {
    if (suspect->isInterviewed())
        throw AlreadyVisitedException("You already talked to " + suspect->getName() + ".");
    if (detective->getCredibility() < suspect->getRequiredCredibility())
        throw LowCredibilityException(suspect->getName() + " refuses to talk. Needs "
                                      + to_string(suspect->getRequiredCredibility()) + " credibility.");
    detective->useTurns(1);
    suspect->markInterviewed();
    suspect->display();
    cout << "\n" << suspect->getName() << ": \"" << suspect->getStatement() << "\"\n";
    if (suspect->isTheCulprit())
        cout << "(" << suspect->getName() << " looks nervous.)\n";
    else
        cout << "(" << suspect->getName() << " looks calm.)\n";
    journal.push_back(Clue(suspect->getLetter(), suspect->getPosition(), Word_2,
                           suspect->getName(), suspect->getStatement()));
    placeFoundTile(suspect->getLetter(), suspect->getPosition(), Word_2);
}
// Saves the found letter in the rack and puts it on its slot on the board automatically.
void GameEngine::placeFoundTile(char letter, int position, WordRow row) {
    rack.addTile(Tile(letter, true));
    board.placeTile(row, position, letter);
    cout << "Found letter " << letter << "! Placed in Word " << (row + 1) << ", slot " << (position + 1) << ".\n";
    checkWords();
}
// When a row is full, checks it. A correct word gives +10 credibility.
void GameEngine::checkWords() {
    for (int i = 0; i < 2; i++) {
        WordRow row = (WordRow)i;
        bool& solved = (row == Word_1) ? word1Solved : word2Solved;
        string target = (row == Word_1) ? word1 : word2;
        if (solved || !board.isRowFull(row)) continue;
        try {
            board.validateRow(row, target);
            solved = true;
            detective->gainCredibility(10);
            cout << "Word " << (i + 1) << " is complete: " << target << " (+10 credibility)\n";
        }
        catch (const InvalidWordException& error) {
            cout << error.what() << "\n";
        }
    }
}
// Shows the clues found so far and the board.
void GameEngine::showClues() {
    cout << "\nClues:\n";
    if (journal.isEmpty()) cout << "No clues yet.\n";
    for (int i = 0; i < journal.size(); i++) cout << i + 1 << ". " << journal[i] << "\n";
    cout << "\n";
    board.display();
}
// Shows the operator+ demo for the project (adds two racks together).
void GameEngine::showRackMerge() {
    cout << "\nRack merge demo (operator+)\n";
    TileRack extraRack(10);
    extraRack.addTile(Tile('X', true));
    extraRack.addTile(Tile('Y', true));
    extraRack.addTile(Tile('Z', true));
    TileRack mergedRack = rack + extraRack;
    cout << "Your letters:  " << rack << "\n"
         << "Extra letters: " << extraRack << "\n"
         << "Merged:        " << mergedRack << "\n"
         << "Are both racks the same? " << ((rack == mergedRack) ? "Yes" : "No") << "\n";
}
// The player names the culprit. Right = win, wrong = -20 credibility and 2 turns.
void GameEngine::accuseCulprit() {
    cout << "\nWho is the culprit? (wrong answer = -20 credibility and 2 turns)\n";
    for (int i = 0; i < suspects.size(); i++)
        cout << i + 1 << ". " << suspects[i]->getName() << " (" << suspects[i]->getRole() << ")\n";
    cout << "0. Back\n";
    int choice = readNumber(0, suspects.size(), "Choose: ");
    if (choice == 0) return;
    Suspect* accused = suspects[choice - 1];
    cout << "\nYou accuse " << accused->getName() << "!\n";
    if (accused->isTheCulprit()) culpritCaught(accused);
    else wrongAccusation(accused);
}
// Correct accusation: show the confession and win the game.
void GameEngine::culpritCaught(Suspect* accused) {
    cout << "\nCorrect! " << accused->getName() << " confesses:\n"
         << "\"" << accused->getConfession() << "\"\n";
    state = GameState::Won;
}
// Wrong accusation: -20 credibility and 2 turns.
void GameEngine::wrongAccusation(Suspect* accused) {
    cout << "\nWrong! " << accused->getName() << " has a solid alibi.\n"
         << "Penalty: -20 credibility and 2 turns lost.\n";
    detective->loseCredibility(20);
    detective->useTurns(2);
}
void GameEngine::showDetectiveFile() {
    detective->display();
    int searchedCount = 0;
    for (int i = 0; i < locations.size(); i++)
        if (locations[i].searched) searchedCount++;
    int interviewedCount = 0;
    for (int i = 0; i < suspects.size(); i++)
        if (suspects[i]->isInterviewed()) interviewedCount++;
    cout << "Locations searched: " << searchedCount << "/" << locations.size() << "\n"
         << "Suspects talked to: " << interviewedCount << "/" << suspects.size() << "\n";
}
void GameEngine::forfeitGame() {
    cout << "\nDo you really want to give up? (Y/N): ";
    string answer;
    getline(cin, answer);
    answer = toUpperText(trimText(answer));
    if (answer == "Y" || answer == "YES") {
        cout << "\nYou gave up. The case stays unsolved.\n";
        state = GameState::Lost;
    }
}
// Lose if credibility is 0 or turns are over.
void GameEngine::checkGameEnd() {
    if (state != GameState::Playing) return;
    if (detective->getCredibility() <= 0) {
        cout << "\nYour credibility is 0. You are fired!\n";
        state = GameState::Lost;
    } else if (detective->getTurnsUsed() >= detective->getMaxTurns()) {
        cout << "\nNo turns left. The culprit escaped!\n";
        state = GameState::Lost;
    } else if (word1Solved && word2Solved && !cipherMessageShown) {
        cout << "\nBoth words are complete: " << word1 << " and " << word2 << ".\n"
             << "Now accuse the culprit!\n";
        cipherMessageShown = true;
    }
}
// Win gives a big score (credibility, turns left, bonus). Loss gives a small score.
int GameEngine::calculateScore() const {
    if (state == GameState::Won) {
        int turnsLeft = detective->getTurnsRemaining();
        int bonus = (word1Solved ? 100 : 0) + (word2Solved ? 100 : 0);
        return detective->getCredibility() * 10 + turnsLeft * 25 + 500 + bonus;
    }
    return detective->getCredibility() * 2 + (word1Solved ? 50 : 0) + (word2Solved ? 50 : 0);
}
// Shows the result and saves the score to scoreBoard.txt.
void GameEngine::finishGame() {
    bool won = (state == GameState::Won);
    int finalScore = calculateScore();
    cout << "\n=== GAME OVER ===\n"
         << "Result: " << (won ? "YOU WON" : "YOU LOST") << "\n"
         << "Detective: " << detective->getName() << "\n"
         << "Credibility: " << detective->getCredibility() << "/100\n"
         << "Turns used: " << detective->getTurnsUsed() << "/" << detective->getMaxTurns() << "\n"
         << "Score: " << finalScore << "\n";
    if (!won && !suspects.isEmpty()) {
        Suspect* culprit = suspects[culpritIndex];
        cout << "The culprit was " << culprit->getName() << " (" << culprit->getRole() << ").\n";
    }
    string culpritText = culpritRole;
    for (int i = 0; i < (int)culpritText.length(); i++)
        if (culpritText[i] == ' ') culpritText[i] = '_';
    FileManager::appendScore(LeaderboardEntry(detective->getName(), won ? "WIN" : "LOSE", detective->getCredibility(),
                                              detective->getTurnsRemaining(), culpritText, currentTimestamp()));
    cout << "Score saved. Press ENTER to go back to the main menu...";
    waitForEnter();
    state = GameState::Menu;
}
