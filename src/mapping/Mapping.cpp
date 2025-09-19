//
// Some of this code was copied from UDP-CPP:
// https://github.com/UnknownDetectionParty/UDP-CPP
//

#include "Mapping.h"
#include <mutex>
#include <memory>

// Use shared_ptr for safer memory management
std::map<std::string, std::shared_ptr<CM>> lookup;
std::mutex lookup_mutex;

CM* Mapping::getClass(const char* key) {
    if (!key) {
        std::cerr << "[Mapping ERROR] Null key provided to getClass" << std::endl;
        return nullptr;
    }
    
    std::lock_guard<std::mutex> lock(lookup_mutex);
    std::string k(key);
    
    // Use safe find instead of at()
    auto it = lookup.find(k);
    if (it == lookup.end()) {
        std::cerr << "[Mapping ERROR] Class not found in lookup: " << k << std::endl;
        return nullptr;
    }
    return it->second.get(); // Return raw pointer from shared_ptr
}

const char* Mapping::getClassName(const char* key) {
    CM* cm = getClass(key);
    return cm ? cm->name : nullptr;
}

void Mapping::setup() {
    std::lock_guard<std::mutex> lock(lookup_mutex);
    
    static bool initialized = false;
    if (initialized) {
        std::cout << "Mapping already initialized, skipping setup" << std::endl;
        return;
    }
    
    // Clear existing mappings - shared_ptr handles cleanup automatically
    lookup.clear();

    try {
        // Base JVM classes
        auto m = make("List", "java.util.List");
        if (m) {
            method(m, "get", "(I)Ljava/lang/Object;", false);
            method(m, "toArray", "()[Ljava/lang/Object;", false);
            method(m, "size", "()I", false);
        }

        m = make("Set", "java.util.Set");
        if (m) {
            method(m, "toArray", "()[Ljava/lang/Object;", false);
            method(m, "size", "()I", false);
        }

        m = make("System", "java.lang.System");
        if (m) {
            field(m, "out", "Ljava/io/PrintStream;", true);
        }

        m = make("PrintStream", "java.io.PrintStream");
        if (m) {
            method(m, "println", "(Ljava/lang/String;)V", false);
        }

        m = make("Mouse", "org.lwjgl.input.Mouse");
        if (m) {
            method(m, "isButtonDown", "(I)Z", true);
        }

        // Minecraft classes
        m = make("Entity", "net.minecraft.entity.Entity");
        if (m) {
            field(m, "x", "field_70165_t", "D", false);
            field(m, "lastTickPosX", "field_70142_S", "D", false);
            field(m, "y", "field_70163_u", "D", false);
            field(m, "z", "field_70161_v", "D", false);
            field(m, "lastTickPosZ", "field_70136_U", "D", false);
            field(m, "ridingEntity", "field_70154_o", "Lnet/minecraft/entity/Entity;", false);
            method(m, "getEyeHeight", "func_70047_e", "()F", false);
            method(m, "getID", "func_145782_y", "()I", false);
            method(m, "getName", "func_70005_c_", "()Ljava/lang/String;", false);
            method(m, "rayTrace", "func_174822_a", "(FD)Lnet/minecraft/util/math/RayTraceResult;", false);
            method(m, "getPositionEyes", "func_174824_e", "(F)Lnet/minecraft/util/Vec3;", false);
            method(m, "getLook", "func_70676_i", "(F)Lnet/minecraft/util/Vec3;", false);
            method(m, "getEntityBoundingBox", "func_174813_aQ", "()Lnet/minecraft/util/AxisAlignedBB;", false);
            method(m, "getCollisionBorderSize", "func_70111_Y", "()F", false);
            method(m, "canBeCollidedWith", "func_70067_L", "()Z", false);
            method(m, "canRiderInteract", "canRiderInteract", "()Z", false);
        }

        m = make("WorldClient", "net.minecraft.client.multiplayer.WorldClient");
        if (m) {
            field(m, "entities", "field_72996_f", "Ljava/util/List;", false);
            field(m, "players", "field_73010_i", "Ljava/util/List;", false);
            method(m, "setTime", "func_72877_b", "(J)V", false);
            method(m, "getEntitiesWithinAABBExcluding", "func_72839_b",
                   "(Lnet/minecraft/entity/Entity;Lnet/minecraft/util/AxisAlignedBB;)Ljava/util/List;", false);
        }

        m = make("PlayerControllerMP", "net.minecraft.client.multiplayer.PlayerControllerMP");

        m = make("PlayerSP", "net.minecraft.client.entity.EntityPlayerSP");
        if (m) {
            field(m, "x", "field_70165_t", "D", false);
            field(m, "y", "field_70163_u", "D", false);
            field(m, "z", "field_70161_v", "D", false);
            field(m, "yaw", "field_70177_z", "F", false);
            field(m, "pitch", "field_70125_A", "F", false);
            field(m, "width", "field_70130_N", "F", false);
            field(m, "height", "field_70131_O", "F", false);
            field(m, "motionX", "field_70159_w", "D", false);
            field(m, "motionY", "field_70181_x", "D", false);
            field(m, "motionZ", "field_70179_y", "D", false);
            field(m, "hurtTime", "field_70737_aN", "I", false);
            field(m, "maxHurtTime", "field_70738_aO", "I", false);
            method(m, "setPosition", "func_70107_b", "(DDD)V", false);
            method(m, "setVelocity", "func_70016_h", "(DDD)V", false);
            method(m, "getPositionVector", "func_174791_d", "()Lnet/minecraft/util/Vec3;", false);
            method(m, "getLook", "func_70676_i", "(F)Lnet/minecraft/util/Vec3;", false);
            method(m, "getID", "func_145782_y", "()I", false);
            method(m, "getEyeHeight", "func_70047_e", "()F", false);
            method(m, "getName", "func_70005_c_", "()Ljava/lang/String;", false);
            method(m, "setSprint", "func_70031_b", "(Z)V", false);
            method(m, "getDisplayName", "func_145748_c_", "()Lnet/minecraft/util/IChatComponent;", false);
        }

        m = make("KeyBinding", "net.minecraft.client.settings.KeyBinding");
        if (m) {
            method(m, "setKeyBindState", "func_74510_a", "(IZ)V", true);
            method(m, "onTick", "func_74507_a", "(I)V", true);
            method(m, "getKeyCode", "func_151463_i", "()I", false);
        }

        m = make("GameSettings", "net.minecraft.client.settings.GameSettings");
        if (m) {
            field(m, "keyBindAttack", "field_74312_F", "Lnet/minecraft/client/settings/KeyBinding;", false);
            field(m, "keyBindBack", "field_74368_y", "Lnet/minecraft/client/settings/KeyBinding;", false);
        }

        m = make("Minecraft", "net.minecraft.client.Minecraft");
        if (m) {
            field(m, "player", "field_71439_g", "Lnet/minecraft/client/entity/EntityPlayerSP;", false);
            field(m, "world", "field_71441_e", "Lnet/minecraft/client/multiplayer/WorldClient;", false);
            field(m, "playerController", "field_71442_b", "Lnet/minecraft/client/multiplayer/PlayerControllerMP;", false);
            field(m, "gameSettings", "field_71474_y", "Lnet/minecraft/client/settings/GameSettings;", false);
            field(m, "inGameHasFocus", "field_71415_G", "Z", false);
            field(m, "pointedEntity", "field_147125_j", "Lnet/minecraft/entity/Entity;", false);
            field(m, "objectMouseOver", "field_71476_x", "Lnet/minecraft/util/math/RayTraceResult;", false);
            field(m, "timer", "field_71428_T", "Lnet/minecraft/util/Timer;", false);
            field(m, "rightClickDelayTimer", "field_71467_ac", "I", false);
            field(m, "entityRenderer", "field_71460_t", "Lnet/minecraft/client/renderer/EntityRenderer;", false);
            field(m, "leftClickMouse", "field_71429_W", "I", false);
            method(m, "getRenderViewEntity", "func_175606_aa", "()Lnet/minecraft/entity/Entity;", false);
            method(m, "getMinecraft", "func_71410_x", "()Lnet/minecraft/client/Minecraft;", true);
        }

        m = make("EntityLivingBase", "net.minecraft.entity.EntityLivingBase");
        if (m) {
            field(m, "x", "field_70165_t", "D", false);
            field(m, "lastTickPosX", "field_70142_S", "D", false);
            field(m, "y", "field_70163_u", "D", false);
            field(m, "z", "field_70161_v", "D", false);
            field(m, "lastTickPosZ", "field_70136_U", "D", false);
            method(m, "getEyeHeight", "func_70047_e", "()F", false);
            method(m, "getID", "func_145782_y", "()I", false);
            method(m, "getName", "func_70005_c_", "()Ljava/lang/String;", false);
        }

        m = make("EntityPlayer", "net.minecraft.entity.player.EntityPlayer");
        if (m) {
            field(m, "x", "field_70165_t", "D", false);
            field(m, "lastTickPosX", "field_70142_S", "D", false);
            field(m, "y", "field_70163_u", "D", false);
            field(m, "z", "field_70161_v", "D", false);
            field(m, "lastTickPosZ", "field_70136_U", "D", false);
            field(m, "yaw", "field_70177_z", "F", false);
            field(m, "pitch", "field_70125_A", "F", false);
            method(m, "getEyeHeight", "func_70047_e", "()F", false);
            method(m, "getID", "func_145782_y", "()I", false);
            method(m, "getName", "func_70005_c_", "()Ljava/lang/String;", false);
            method(m, "setSprint", "func_70031_b", "(Z)V", false);
            method(m, "getDisplayName", "func_145748_c_", "()Lnet/minecraft/util/IChatComponent;", false);
            method(m, "getEntityBoundingBox", "func_174813_aQ", "()Lnet/minecraft/util/AxisAlignedBB;", false);
        }

        m = make("Vec3", "net.minecraft.util.Vec3");
        if (m) {
            field(m, "xCoord", "field_72450_a", "D", false);
            field(m, "yCoord", "field_72448_b", "D", false);
            field(m, "zCoord", "field_72449_c", "D", false);
            method(m, "addVector", "func_72441_c", "(DDD)Lnet/minecraft/util/Vec3;", false);
            method(m, "distanceTo", "func_72438_d", "(Lnet/minecraft/util/Vec3;)D", false);
        }

        m = make("AxisAlignedBB", "net.minecraft.util.AxisAlignedBB");
        if (m) {
            method(m, "addCoord", "func_72321_a", "(DDD)Lnet/minecraft/util/AxisAlignedBB;", false);
            method(m, "expand", "func_72314_b", "(DDD)Lnet/minecraft/util/AxisAlignedBB;", false);
            method(m, "calculateIntercept", "func_72327_a",
                   "(Lnet/minecraft/util/Vec3;Lnet/minecraft/util/Vec3;)Lnet/minecraft/util/MovingObjectPosition;", false);
            method(m, "isVecInside", "func_72318_a", "(Lnet/minecraft/util/Vec3;)Z", false);
            field(m, "maxX", "field_72336_d", "D", false);
            field(m, "maxY", "field_72337_e", "D", false);
            field(m, "maxZ", "field_72334_f", "D", false);
            field(m, "minX", "field_72340_a", "D", false);
            field(m, "minY", "field_72338_b", "D", false);
            field(m, "minZ", "field_72339_c", "D", false);
        }

        m = make("MovingObjectPosition", "net.minecraft.util.MovingObjectPosition");
        if (m) {
            field(m, "hitVec", "field_72307_f", "Lnet/minecraft/util/Vec3;", false);
            field(m, "typeOfHit", "field_72313_a",
                  "Lnet/minecraft/util/MovingObjectPosition$MovingObjectType;", false);
            field(m, "blockPos", "field_178783_e", "Lnet/minecraft/util/BlockPos;", false);
            field(m, "sideHit", "field_178784_b", "Lnet/minecraft/util/EnumFacing;", false);
            field(m, "entityHit", "field_72308_g", "Lnet/minecraft/entity/Entity;", false);
        }

        m = make("Timer", "net.minecraft.util.Timer");
        if (m) {
            field(m, "partialTicks", "field_74281_c", "F", false);
        }

        m = make("EntityRenderer", "net.minecraft.client.renderer.EntityRenderer");
        if (m) {
            field(m, "pointedEntity", "field_78528_u", "Lnet/minecraft/entity/Entity;", false);
        }

        m = make("InventoryPlayer", "net.minecraft.entity.player.InventoryPlayer");
        if (m) {
            field(m, "currentItem", "field_70461_c", "I", false);
            field(m, "mainInventory", "field_70462_a", "[Lnet/minecraft/item/ItemStack;", false);
            method(m, "getCurrentItem", "func_70448_g", "()Lnet/minecraft/item/ItemStack;", false);
        }

        m = make("ItemStack", "net.minecraft.item.ItemStack");
        if (m) {
            field(m, "stackSize", "field_77994_a", "I", false);
            field(m, "item", "field_151002_e", "Lnet/minecraft/item/Item;", false);
            method(m, "getDisplayName", "func_82833_r", "()Ljava/lang/String;", false);
        }

        m = make("Item", "net.minecraft.item.Item");
        if (m) {
            method(m, "getUnlocalizedName", "func_77658_a", "()Ljava/lang/String;", false);
        }
        
        std::cout << "=== Loaded Classes ===" << std::endl;
        for (const auto &entry : lookup) {
            std::cout << " - " << entry.first << std::endl;
        }

        for (const auto &entry : lookup) {
            std::cout << "[Mapping] " << entry.first << std::endl;
        }
        
        initialized = true;
        std::cout << "Mapping setup completed successfully" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "[Mapping FATAL ERROR] Exception during setup: " << e.what() << std::endl;
        lookup.clear();
        initialized = false;
        throw;
    }
}

void Mapping::field(CM *cm, const char *name, const char *desc, bool isStatic) {
    field(cm, name, name, desc, isStatic);
}

void Mapping::method(CM *cm, const char *name, const char *desc, bool isStatic) {
    method(cm, name, name, desc, isStatic);
}

void Mapping::field(CM *cm, const char *keyName, const char *obName, const char *desc, bool isStatic) {
    if (!cm || !keyName || !obName || !desc) {
        std::cerr << "[Mapping ERROR] Null parameter when adding field" << std::endl;
        return;
    }
    
    std::cout << "  Mapping " << obName << " to " << keyName << std::endl;
    
    // Create Mem object directly in the map - avoid temporary objects
    cm->fields.emplace(std::string(keyName), Mem(obName, desc, isStatic));
}

void Mapping::method(CM *cm, const char *keyName, const char *obName, const char *desc, bool isStatic) {
    if (!cm || !keyName || !obName || !desc) {
        std::cerr << "[Mapping ERROR] Null parameter when adding method" << std::endl;
        return;
    }
    
    std::cout << "  Mapping " << obName << desc << " to " << keyName << std::endl;
    
    // Create Mem object directly in the map - avoid temporary objects
    cm->methods.emplace(std::string(keyName), Mem(obName, desc, isStatic));
}

CM *Mapping::make(const char *key, const char *name) {
    if (!key || !name) {
        std::cerr << "[Mapping ERROR] Null parameters to make()" << std::endl;
        return nullptr;
    }
    
    std::string keyStr(key);
    
    // Check if key already exists
    auto it = lookup.find(keyStr);
    if (it != lookup.end()) {
        std::cerr << "[Mapping WARNING] Class " << key << " already exists, returning existing..." << std::endl;
        return it->second.get();
    }
    
    try {
        auto cm = std::make_shared<CM>(name);
        std::cout << "Mapping " << name << " to " << key << std::endl;
        
        auto result = lookup.emplace(keyStr, cm);
        if (result.second) {
            return cm.get();
        } else {
            std::cerr << "[Mapping ERROR] Failed to insert " << key << " into lookup" << std::endl;
            return nullptr;
        }
    } catch (const std::exception& e) {
        std::cerr << "[Mapping ERROR] Failed to create CM for " << key << ": " << e.what() << std::endl;
        return nullptr;
    }
}
