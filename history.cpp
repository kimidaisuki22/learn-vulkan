#include "history.h"
#include <string>
History::History() : db("me.db") {
  db << "create table if not exists history("
        "id integer primary key autoincrement not null,"
        "path text unique"
        ");";

  db << R"**(
CREATE TABLE IF NOT EXISTS latest_file (
    id INTEGER PRIMARY KEY,
    filename TEXT NOT NULL,
    timestamp DATETIME DEFAULT CURRENT_TIMESTAMP
);)**";

  db << R"**(
CREATE TRIGGER IF NOT EXISTS limit_rows AFTER INSERT ON latest_file
BEGIN
    DELETE FROM latest_file WHERE id NOT IN (SELECT id FROM latest_file ORDER BY timestamp DESC LIMIT 1000);
END;
    )**";

    // key pair
    db << "CREATE TABLE IF NOT EXISTS key_value(key TEXT PRIMARY KEY,value TEXT);";

  for (auto h : get()) {
    drawer.elems_.push_back(h);
  }
  drawer.title_ = "History";
}
std::vector<std::string> History::get() {
  std::vector<std::string> results;
  db << "select path from history;" >>
      [&](std::string path) { results.push_back(std::move(path)); };
  return results;
}
void History::add(std::string path) {
  try {

    db << "INSERT INTO latest_file (filename) VALUES (?)" << path;
    db << "insert into history (path) values (?);" << path;

    drawer.elems_.push_back(path);
  } catch (const sqlite::sqlite_exception &e) {
    using namespace std;
    cerr << "Error inserting row into history table: " << e.what() << endl;
  }
}
void History::draw() { drawer.draw(); }
std::optional<std::string> History::last_loaded() {
  try {
    std::string last;
    db << "SELECT filename FROM latest_file ORDER BY timestamp DESC LIMIT 1;" >>
        last;
    return last;
  } catch (const sqlite::sqlite_exception &e) {
    using namespace std;
    cerr << "Error retrive last loaded file." << e.what() << endl;
  }
  return {};
}
void History::insert(const std::string &key, const std::string &value) {
  db << "INSERT INTO key_value (key ,value)"
     "VALUES(?,?)"
     "ON CONFLICT(key)"
     "DO UPDATE SET value=excluded.value;"
     << key << value;
}
std::optional<std::string> History::query(const std::string &key) {
  std::string value;
  bool success = false;
  db << "SELECT value FROM key_value WHERE key = (?);"<< key >> [&](std::string result){
    success = true;
    value = result;
  };
  if(success){
    return value;
  }
  return {};
}
