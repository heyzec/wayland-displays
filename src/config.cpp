#include <string>
#include <vector>

using namespace std;

class MonitorSettings {
  int position_x;
  int position_y;

  int size_x;
  int size_y;
};

class Profile {
  string name;
};

class Config {
  vector<Profile> profiles;
};

class ConfigApplier {
  virtual void apply(Config) = 0;
};

class ConfigSaver {
  virtual double save() = 0;
};
