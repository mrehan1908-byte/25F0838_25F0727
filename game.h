#ifndef GAME_H
#define GAME_H
#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstdlib>
#include <ctime>
#include <cctype>
#include <exception>
#include <iomanip>
using namespace std;
enum class GameState { Menu, Playing, Won, Lost, Exit };
enum WordRow { Word_1, Word_2 };
class GameException : public exception {
protected:
    string message;
public:
    GameException(const string& text) : message(text) {}
    const char* what() const noexcept override { return message.c_str(); }
};
class InvalidWordException : public GameException {
public:
    InvalidWordException(const string& text = "The submitted word does not match the cipher.")
        : GameException("InvalidWordException: " + text) {}
};
class RackFullException : public GameException {
public:
    RackFullException(const string& text = "The tile rack is full.")
        : GameException("RackFullException: " + text) {}
};
class InvalidPositionException : public GameException {
public:
    InvalidPositionException(const string& text = "The position is out of bounds.")
        : GameException("InvalidPositionException: " + text) {}
};
class LowCredibilityException : public GameException {
public:
    LowCredibilityException(const string& text = "Detective credibility is too low.")
        : GameException("LowCredibilityException: " + text) {}
};
class AlreadyVisitedException : public GameException {
public:
    AlreadyVisitedException(const string& text = "This target has already been visited.")
        : GameException("AlreadyVisitedException: " + text) {}
};
inline char upperChar(char letter) {
    return (char)toupper((unsigned char)letter);
}
template <typename T>
T customMin(T first, T second) {
    return (second < first) ? second : first;
}
template <typename T>
void customSwap(T& first, T& second) {
    T temp = first;
    first = second;
    second = temp;
}
string trimText(const string& text);
string toUpperText(string text);
bool sameIgnoringCase(const string& first, const string& second);
string currentTimestamp();
void printTitle(const string& title);
int readNumber(int low, int high, const string& prompt);
void waitForEnter();
template <typename T>
class DynamicArray {
private:
    T* items;
    int capacity;
    int itemCount;
    void checkIndex(int index) const {
        if (index < 0 || index >= itemCount)
            throw InvalidPositionException("Index out of bounds: " + to_string(index));
    }
    void grow() {
        int newCapacity = capacity * 2;
        T* newItems = new T[newCapacity];
        for (int i = 0; i < itemCount; i++) newItems[i] = items[i];
        delete[] items;
        items = newItems;
        capacity = newCapacity;
    }
    void copyFrom(const DynamicArray<T>& other) {
        capacity = other.capacity;
        itemCount = other.itemCount;
        items = new T[capacity];
        for (int i = 0; i < itemCount; i++) items[i] = other.items[i];
    }
public:
    DynamicArray() : capacity(4), itemCount(0) { items = new T[capacity]; }
    DynamicArray(const DynamicArray<T>& other) { copyFrom(other); }
    DynamicArray<T>& operator=(const DynamicArray<T>& other) {
        if (this != &other) {
            delete[] items;
            copyFrom(other);
        }
        return *this;
    }
    ~DynamicArray() { delete[] items; }
    void push_back(const T& item) {
        if (itemCount == capacity) grow();
        items[itemCount++] = item;
    }
    void removeAt(int index) {
        checkIndex(index);
        for (int i = index; i < itemCount - 1; i++) items[i] = items[i + 1];
        itemCount--;
    }
    T& operator[](int index) {
        checkIndex(index);
        return items[index];
    }
    const T& operator[](int index) const {
        checkIndex(index);
        return items[index];
    }
    int size() const { return itemCount; }
    bool isEmpty() const { return itemCount == 0; }
    void clear() { itemCount = 0; }
};
struct Tile {
    char letter;
    bool filled;
    Tile(char newLetter = ' ', bool isFilled = false) : letter(newLetter), filled(isFilled) {}
    bool operator==(const Tile& other) const { return upperChar(letter) == upperChar(other.letter); }
    bool operator==(char otherLetter) const { return upperChar(letter) == upperChar(otherLetter); }
    friend ostream& operator<<(ostream& out, const Tile& tile) {
        if (tile.filled) out << "[" << tile.letter << "]";
        else out << "[ _ ]";
        return out;
    }
};
struct Clue {
    char letter;
    int position;
    WordRow row;
    string source;
    string story;
    Clue(char newLetter = ' ', int newPosition = 0, WordRow newRow = Word_1, const string& newSource = "", const string& newStory = "")
        : letter(newLetter), position(newPosition), row(newRow), source(newSource), story(newStory) {}
    friend ostream& operator<<(ostream& out, const Clue& clue) {
        out << "Letter " << clue.letter << " - Word " << (clue.row + 1) << ", slot "
            << (clue.position + 1) << " - " << clue.source;
        return out;
    }
};
struct Location {
    string name;
    char letter;
    int position;
    bool searched;
    string description;
    Location(const string& newName = "", char newLetter = ' ', int newPosition = 0, const string& newDescription = "")
        : name(newName), letter(newLetter), position(newPosition), searched(false), description(newDescription) {}
};
class Character {
protected:
    string name;
    string role;
    string nationality;
    bool isCulprit;
    friend class GameEngine;
public:
    Character(const string& newName = "", const string& newRole = "", const string& newNationality = "")
        : name(newName), role(newRole), nationality(newNationality), isCulprit(false) {}
    virtual ~Character() {}
    virtual void display() const = 0;
    string getName() const { return name; }
    string getRole() const { return role; }
    string getNationality() const { return nationality; }
    bool isTheCulprit() const { return isCulprit; }
    void setCulprit(bool value) { isCulprit = value; }
    friend ostream& operator<<(ostream& out, const Character& character) {
        out << "[" << character.role << "] " << character.name << " (" << character.nationality << ")";
        return out;
    }
    friend istream& operator>>(istream& in, Character& character) {
        string line;
        if (getline(in, line)) {
            stringstream parts(line);
            string culpritText;
            getline(parts, character.name, '|');
            getline(parts, character.role, '|');
            getline(parts, character.nationality, '|');
            getline(parts, culpritText, '|');
            character.isCulprit = (culpritText == "1" || culpritText == "true" || culpritText == "TRUE");
        }
        return in;
    }
};
class Victim : public Character {
private:
    int requiredCredibility;
    string testimony;
    char clueLetter;
    int boardPosition;
    bool interviewed;
public:
    Victim(const string& newName = "Mr.Leonardo", const string& newRole = "Victim",
           const string& newNationality = "French", int minCredibility = 80,
           const string& words = "My titanium vault Locker #77 was broken into!",
           char letter = ' ', int position = 0)
        : Character(newName, newRole, newNationality), requiredCredibility(minCredibility), testimony(words),
          clueLetter(letter), boardPosition(position), interviewed(false) {}
    int getRequiredCredibility() const { return requiredCredibility; }
    string getTestimony() const { return testimony; }
    char getLetter() const { return clueLetter; }
    int getPosition() const { return boardPosition; }
    bool isInterviewed() const { return interviewed; }
    void markInterviewed() { interviewed = true; }
    void display() const override;
};
class Detective : public Character {
private:
    int credibility;
    int turnsUsed;
    int maxTurns;
public:
    Detective(const string& newName = "Raymond Vance")
        : Character(newName, "Inspector Principal", "French"), credibility(100), turnsUsed(0), maxTurns(25) {}
    int getCredibility() const { return credibility; }
    int getTurnsUsed() const { return turnsUsed; }
    int getMaxTurns() const { return maxTurns; }
    int getTurnsRemaining() const { return maxTurns - turnsUsed; }
    void loseCredibility(int amount) {
        credibility -= amount;
        if (credibility < 0) credibility = 0;
    }
    void gainCredibility(int amount) {
        credibility += amount;
        if (credibility > 100) credibility = 100;
    }
    void useTurns(int amount = 1) { turnsUsed += amount; }
    void display() const override;
};
class Suspect : virtual public Character {
protected:
    char clueLetter;
    int boardPosition;
    bool interviewed;
    string alibi;
    string statement;
    string confession;
public:
    Suspect(char letter, int position, const string& newAlibi, const string& newStatement, const string& newConfession)
        : clueLetter(letter), boardPosition(position), interviewed(false),
          alibi(newAlibi), statement(newStatement), confession(newConfession) {}
    virtual int getRequiredCredibility() const = 0;
    virtual string getCategoryName() const = 0;
    char getLetter() const { return clueLetter; }
    int getPosition() const { return boardPosition; }
    bool isInterviewed() const { return interviewed; }
    void markInterviewed() { interviewed = true; }
    string getStatement() const { return statement; }
    string getConfession() const { return confession; }
    void display() const override;
};
class BusinessRelations : virtual public Suspect {
public:
    BusinessRelations(const string& newName, const string& newRole, const string& newNationality,
                      char letter, int position, const string& newAlibi,
                      const string& newStatement, const string& newConfession)
        : Character(newName, newRole, newNationality),
          Suspect(letter, position, newAlibi, newStatement, newConfession) {}
    int getRequiredCredibility() const override { return 50; }
    string getCategoryName() const override { return "Business Relations"; }
    void display() const override;
};
class HouseHelp : public Suspect {
public:
    HouseHelp(const string& newName, const string& newRole, const string& newNationality,
              char letter, int position, const string& newAlibi,
              const string& newStatement, const string& newConfession)
        : Character(newName, newRole, newNationality),
          Suspect(letter, position, newAlibi, newStatement, newConfession) {}
    int getRequiredCredibility() const override { return 0; }
    string getCategoryName() const override { return "House Help"; }
    void display() const override;
};
class CloseRelations : virtual public Suspect {
public:
    CloseRelations(const string& newName, const string& newRole, const string& newNationality,
                   char letter, int position, const string& newAlibi,
                   const string& newStatement, const string& newConfession)
        : Character(newName, newRole, newNationality),
          Suspect(letter, position, newAlibi, newStatement, newConfession) {}
    int getRequiredCredibility() const override { return 75; }
    string getCategoryName() const override { return "Close Relations"; }
    void display() const override;
};
class PA : public BusinessRelations, public CloseRelations {
public:
    PA(const string& newName, const string& newRole, const string& newNationality,
       char letter, int position, const string& newAlibi,
       const string& newStatement, const string& newConfession)
        : Character(newName, newRole, newNationality),
          Suspect(letter, position, newAlibi, newStatement, newConfession),
          BusinessRelations(newName, newRole, newNationality, letter, position, newAlibi, newStatement, newConfession),
          CloseRelations(newName, newRole, newNationality, letter, position, newAlibi, newStatement, newConfession) {}
    int getRequiredCredibility() const override { return 60; }
    string getCategoryName() const override { return "Personal Assistant (Business & Family Nexus)"; }
    void display() const override;
};
class TileRack {
private:
    DynamicArray<Tile> tiles;
    int maxTiles;
    int countLetter(char letter) const;
public:
    TileRack(int capacity = 20) : maxTiles(capacity) {}
    void addTile(const Tile& tile);
    bool removeLetter(char letter);
    Tile removeTileAt(int index);
    bool hasLetter(char letter) const { return countLetter(letter) > 0; }
    Tile getTile(int index) const { return tiles[index]; }
    int getCount() const { return tiles.size(); }
    bool isEmpty() const { return tiles.isEmpty(); }
    void clear() { tiles.clear(); }
    TileRack operator+(const TileRack& other) const;
    bool operator==(const TileRack& other) const;
    friend ostream& operator<<(ostream& out, const TileRack& rack) {
        if (rack.tiles.isEmpty()) return out << "[ Empty Rack ]";
        for (int i = 0; i < rack.tiles.size(); i++) out << "[" << rack.tiles[i].letter << "] ";
        return out;
    }
};
class ScrabbleBoard {
private:
    DynamicArray<Tile> rows[2];
    void checkPosition(WordRow row, int position) const;
public:
    ScrabbleBoard();
    void reset();
    int getRowLength(WordRow row) const;
    void placeTile(WordRow row, int position, char letter);
    bool removeTile(WordRow row, int position, char& removedLetter);
    bool isSlotFilled(WordRow row, int position) const;
    bool isRowFull(WordRow row) const;
    string getRowText(WordRow row) const;
    void validateRow(WordRow row, const string& expectedWord) const;
    void display() const;
};
struct LeaderboardEntry {
    string playerName;
    string outcome;
    int credibility;
    int turnsLeft;
    string culprit;
    string date;
    LeaderboardEntry() : credibility(0), turnsLeft(0) {}
    LeaderboardEntry(const string& newName, const string& newOutcome, int newCredibility, int newTurnsLeft,
                     const string& newCulprit, const string& newDate)
        : playerName(newName), outcome(newOutcome), credibility(newCredibility),
          turnsLeft(newTurnsLeft), culprit(newCulprit), date(newDate) {}
};
class FileManager {
public:
    static DynamicArray<string> splitFields(const string& line);
    static bool readDataLine(ifstream& file, DynamicArray<string>& fields);
    static string columnToRole(const string& column);
    static int findColumn(const DynamicArray<string>& header, int firstColumn, const string& role);
    static bool findStory(const string& key, DynamicArray<string>& texts);
    static void appendScore(const LeaderboardEntry& entry);
    static void loadScores(DynamicArray<LeaderboardEntry>& entries);
};
class GameEngine {
private:
    GameState state;
    Detective* detective;
    Victim* victim;
    DynamicArray<Suspect*> suspects;
    DynamicArray<Location> locations;
    DynamicArray<Clue> journal;
    TileRack rack;
    ScrabbleBoard board;
    string word1;
    string word2;
    string culpritRole;
    int culpritIndex;
    bool word1Solved;
    bool word2Solved;
    bool cipherMessageShown;
    GameEngine();
    ~GameEngine();
    GameEngine(const GameEngine&) = delete;
    GameEngine& operator=(const GameEngine&) = delete;
    void cleanUp();
    void showMainMenu();
    void showTextFile(const string& filename);
    void showLeaderboard();
    void startNewGame();
    void loadLocations();
    void loadCharacters();
    void chooseCulpritRole();
    void loadWords();
    void pickCulprit();
    void playGame();
    void showDashboard();
    void searchLocation();
    void interrogateSuspect();
    void interviewVictim();
    void interviewSuspect(Suspect* suspect);
    void showClues();
    void placeFoundTile(char letter, int position, WordRow row);
    void checkWords();
    void showRackMerge();
    void accuseCulprit();
    void culpritCaught(Suspect* accused);
    void wrongAccusation(Suspect* accused);
    void showDetectiveFile();
    void forfeitGame();
    void checkGameEnd();
    void finishGame();
    int calculateScore() const;
public:
    static GameEngine& getInstance();
    void run();
};
#endif
