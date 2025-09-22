//
// This code was copied from UDP-CPP:
// https://github.com/UnknownDetectionParty/UDP-CPP
//

#ifndef PHANTOM_MAPPING_H
#define PHANTOM_MAPPING_H
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>

#include "CM.h"
#include "Mem.h"

enum GameVersions {
    CASUAL_1_7_10,
    CASUAL_1_8,
    FORGE_1_7_10,
    FORGE_1_8,
    FEATHER_1_8,
    LUNAR_1_7_10,
    LUNAR_1_8
};
// Map of class names to mapping structures
extern std::map<std::string, std::shared_ptr<CM>> lookup;

class Mapping {
   public:
    Mapping() {
        // Populate the map
        setup();
    }
    static void Initialize(const GameVersions version);
    static std::string Get(const char* mapping, int type = 0);
    static CM* getClass(const char* key);

    static const char* getClassName(const char* key);

    static void setup();

   private:
    static void field(CM* cm, const char* name, const char* desc,
                      bool isStatic);

    static void method(CM* cm, const char* name, const char* desc,
                       bool isStatic);

    static void field(CM* cm, const char* keyName, const char* obName,
                      const char* desc, bool isStatic);

    static void method(CM* cm, const char* keyName, const char* obName,
                       const char* desc, bool isStatic);

    static CM* make(const char* key, const char* name);
};

#endif  // PHANTOM_MAPPING_H
