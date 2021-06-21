#pragma once

#include "app.hpp"
#include "base/clicker.hpp"
#include "base/matcher.hpp"
#include "definitions.hpp"
#include "driver_control.hpp"
#include "opencv2/core.hpp"
extern "C" {
#include "lauxlib.h"
#include "lua.h"
#include "lualib.h"
}
#include "opencv2/core/cvstd.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/imgproc.hpp"
#include "utils/logger.hpp"
#include "windows/screen.hpp"
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <list>
#include <map>
#include <optional>
#include <stdexcept>
#include <stdio.h>
#include <string>
#include <thread>
#include <vector>

#define REGLUAFUN(x, y)                                                        \
    lua_register(luaState, x, y);                                              \
    logd("Lua function: %s", x);

using namespace std::chrono_literals;

class ScriptTimeout : public std::exception {
    std::string name;

  public:
    ScriptTimeout(std::string name) { this->name = "Timeout " + name; }
    const char *what() const override { return name.c_str(); }
};

class App {
    std::vector<std::filesystem::path> pathList{};
    std::map<std::string, cv::Mat> cacheMat;
    std::string hospital_name;
    std::string picsDir;
    ScreenImpl screen;
    Matcher matcher;
    Clicker clicker;
    cv::Mat screenMat;
    std::vector<std::string> extentsArgument;
    driver_control dr;
    lua_State *luaState;

  public:
    static App *_app;

    App(int argc, char **argv)
        : screen(1920, 1080), dr("\\\\.\\norsefire"),
          clicker(&dr, screen.get_screenWidth(), screen.get_screenHeight()) {

        _app = this;

        for (int i = 0; i < argc; i++) {
            char *x = argv[i];
            logd("%d %s", i, x);
        }

        if (argc <= 4) {
            printf("Arguments error");
            exit(0);
        }

        // load driver
        if (!dr.ready()) {
            logd("Driver load error");
            exit(-4);
        } else {
            logd("Driver load successful");
        }

        for (int i = 5; i < argc; i++) {
            extentsArgument.push_back(argv[i]);
        }

        int x = std::atoi(argv[1]);
        int y = std::atoi(argv[2]);

        clicker.setsx(x);
        clicker.setsy(y);

        screen.set_screenHeight(y);
        screen.set_screenWidth(x);

        logd("Device Height %d", y);
        logd("Device Width %d", x);

        using namespace std::filesystem;

        hospital_name = argv[3];
        picsDir = argv[4];
        logd("Script Path: %s", hospital_name.c_str());
        logd("Pictures Path: %s", picsDir.c_str());

        logd("Init lua..");
        luaState = luaL_newstate();
        luaL_openlibs(luaState);
        registerLuaRuntime();
    }

    void doFile(std::string filename) {
        if (filename.find(".lua") == std::string::npos) {
            filename += ".lua";
        }
        logd("Load script %s", filename.c_str());
        if (0 != luaL_dofile(luaState, filename.c_str())) {
            const char *err = lua_tostring(luaState, -1);
            lua_pop(luaState, 1);
            throw std::runtime_error(err);
        } else {
            logd("Script done");
        }
    }

    void registerLuaRuntime() {

        REGLUAFUN("getarg", [](lua_State *L) -> int {
            int arg_counts = lua_gettop(L);
            if (arg_counts != 1) {
                luaL_error(L, "getarg arguments error");
            } else {
                auto i = lua_tonumber(L, 1);
                if (App::_app->extentsArgument.size() <= i) {
                    luaL_error(L, "getarg index outrange");
                } else {
                    auto r = App::_app->extentsArgument.at(i);
                    lua_pushstring(L, r.c_str());
                    return 1;
                }
            }
            return 0;
        });

        REGLUAFUN("forceWait", [](lua_State *L) -> int {
            try {
                int arg_counts = lua_gettop(L);

                if (arg_counts == 1) {
                    std::string info;
                    info = lua_tostring(L, 1);
                    App::_app->forceWait(info);
                } else {
                    luaL_error(L, "forceWait arguments error");
                }
            } catch (...) {
                luaL_error(L, "forceWait exception");
            }
            return 0;
        })

        REGLUAFUN("log", [](lua_State *L) -> int {
            try {
                int arg_counts = lua_gettop(L);

                if (arg_counts == 1) {
                    std::string info;
                    info = lua_tostring(L, 1);
                    logd("LUA: %s", info.c_str());
                } else {
                    luaL_error(L, "log arguments error");
                }
            } catch (...) {
                luaL_error(L, "log exception");
            }
            return 0;
        });

        REGLUAFUN("paste", [](lua_State *L) -> int {
            int arg_counts = lua_gettop(L);

            if (arg_counts == 1) {
                std::string info;
                info = lua_tostring(L, 1);
                App::_app->paste(info);
            } else {
                luaL_error(L, "paste arguments error");
            }
            return 0;
        });

        REGLUAFUN("picExists", [](lua_State *L) -> int {
            int arg_counts = lua_gettop(L);
            if (arg_counts != 1) {
                luaL_error(L, "picExists arguments error");
            } else {
                const char *a = lua_tostring(L, 1);
                auto r = App::_app->picExists(a);
                lua_pushboolean(L, r);
                return 1;
            }
            return 0;
        });

        REGLUAFUN("forceClick", [](lua_State *L) -> int {
            try {
                int arg_counts = lua_gettop(L);

                if (arg_counts == 1) {
                    std::string info;
                    info = lua_tostring(L, 1);
                    App::_app->forceClick(info);
                } else if (arg_counts == 2) {
                    std::string info;
                    info = lua_tostring(L, 1);
                    int offx = lua_tonumber(L, 2);
                    App::_app->forceClick(info, offx);
                } else if (arg_counts == 3) {
                    std::string info;
                    info = lua_tostring(L, 1);
                    int offx = lua_tonumber(L, 2);
                    int offy = lua_tonumber(L, 3);
                    App::_app->forceClick(info, offx, offy);
                } else {
                    luaL_error(L, "forceClick arguments error");
                }
            } catch (const std::exception &err) {
                luaL_error(L, "forceClick exception %s", err.what());
            }
            return 0;
        });

        REGLUAFUN("dforceClick", [](lua_State *L) -> int {
            try {
                int arg_counts = lua_gettop(L);

                if (arg_counts == 1) {
                    std::string info;
                    info = lua_tostring(L, 1);
                    App::_app->dforceClick(info);
                } else if (arg_counts == 2) {
                    std::string info;
                    info = lua_tostring(L, 1);
                    int offx = lua_tonumber(L, 2);
                    App::_app->dforceClick(info, offx);
                } else if (arg_counts == 3) {
                    std::string info;
                    info = lua_tostring(L, 1);
                    int offx = lua_tonumber(L, 2);
                    int offy = lua_tonumber(L, 3);
                    App::_app->dforceClick(info, offx, offy);
                } else {
                    luaL_error(L, "dforceClick arguments error");
                }
            } catch (const std::exception &err) {
                luaL_error(L, "dforceClick exception %s", err.what());
            }

            return 0;
        })

        REGLUAFUN("dclick", [](lua_State *L) -> int {
            try {
                int arg_counts = lua_gettop(L);
                if (arg_counts == 1) {
                    auto r = lua_tostring(L, 1);
                    App::_app->dclick(r);
                } else if (arg_counts == 2) {
                    auto r = lua_tostring(L, 1);
                    int offx = lua_tonumber(L, 2);
                    App::_app->dclick(r, offx);
                } else if (arg_counts == 3) {
                    auto r = lua_tostring(L, 1);
                    int offx = lua_tonumber(L, 2);
                    int offy = lua_tonumber(L, 3);
                    App::_app->dclick(r, offx, offy);
                } else {
                    luaL_error(L, "dclick arguments error");
                }
            } catch (const std::exception &err) {
                luaL_error(L, "dclick exception %s", err.what());
            }
            return 0;
        });

        REGLUAFUN("sleep", [](lua_State *L) -> int {
            int arg_counts = lua_gettop(L);
            if (arg_counts != 1) {
                luaL_error(L, "sleep arguments error");
            } else {
                auto r = lua_tonumber(L, 1);
                App::_app->sleep(r);
            }
            return 0;
        });

        REGLUAFUN("click", [](lua_State *L) -> int {
            try {
                int arg_counts = lua_gettop(L);

                if (arg_counts == 1) {
                    std::string info;
                    info = lua_tostring(L, 1);
                    App::_app->click(info);
                } else if (arg_counts == 3) {
                    std::string info;
                    info = lua_tostring(L, 1);
                    int offx = lua_tonumber(L, 2);
                    int offy = lua_tonumber(L, 3);
                    App::_app->click(info, offx, offy);
                } else {
                    luaL_error(L, "click arguments error");
                }
            } catch (const std::exception &err) {
                luaL_error(L, "click exception %s", err.what());
            }
            return 0;
        });

        REGLUAFUN("refreshScreen", [](lua_State *L) -> int {
            int arg_counts = lua_gettop(L);
            if (arg_counts != 0) {
                luaL_error(L, "refreshScreen arguments error");
            }
            App::_app->refreshScreen();
            return 0;
        });
    }

    // sleep and then refresh screen
    void sleep(int ms) {
        std::this_thread::sleep_for(1ms * ms);
        refreshScreen();
    }

    void loadCache(const std::string &hospital_name) {
        logd("Loading pictures from %s..", hospital_name.c_str());
        using namespace std::filesystem;
        for (auto r : recursive_directory_iterator(hospital_name)) {
            pathList.push_back(r.path());
        }
        if (pathList.size() == 0) {
            printf("No pictures loaded");
            std::terminate();
        }

        for (auto r : pathList) {
            cv::Mat mat = cv::imread(r.string(), cv::IMREAD_COLOR);
            cacheMat[r.u8string()] = mat;
            printf("Load %s type %d\n", r.u8string().c_str(), mat.type());
        }

        std::printf("Load done, pictures count %d\n", cacheMat.size());
    }

    const HWND hwndFromClass(const std::string &classname) {
        HWND wnd = FindWindowExA(NULL, NULL, classname.c_str(), NULL);
        if (wnd != NULL) {
            return wnd;
        }
        return NULL;
    }

    // Wait util visible
    bool forceWait(const std::string &name) {
        for (int i = 0; i < 10; i++) {
            std::this_thread::sleep_for(1s);
            refreshScreen();
            if (picExists(name)) {
                return true;
            }
        }
        throw ScriptTimeout(name);
    }

    bool picExists(const std::string &name) {
        const cv::Mat &mat = fetchMat(name);
        auto r = matcher.matchTemplate(name, screenMat, mat);
        if (r != std::nullopt) {
            return true;
        }
        return false;
    }

    bool click(const std::string &name, int offx = 0, int offy = 0) {
        using namespace std::chrono_literals;
        auto r = matcher.matchTemplate(name, screenMat, fetchMat(name));
        if (r == std::nullopt) {
            printf("click fail %s\n", name.c_str());
            return false;
        }
        clicker.click(r->x + offx, r->y + offy);
        refreshScreen();
        return true;
    }

    const cv::Mat &fetchMat(const std::string &name) {
        auto r = normalizeName(name);
        if (cacheMat.find(r) == cacheMat.end()) {
            throw std::runtime_error("Pic not exists " + r);
        }
        return cacheMat[r];
    }

    std::string normalizeName(std::string name) {
        name = picsDir + "\\" + name + ".png";
        return name;
    }

    // Wait target visible and click, timeout 15s
    void forceClick(const std::string &name, int offx = 0, int offy = 0) {
        for (int i = 0; i < 15; i++) {
            if (auto r =
                    matcher.matchTemplate(name, screenMat, fetchMat(name))) {
                clicker.click(r->x + offx, r->y + offy);
                refreshScreen();
                return;
            }
            std::this_thread::sleep_for(1s);
            refreshScreen();
        }
        throw ScriptTimeout(name);
    }

    void refreshScreen() {
        auto screenMatv = screen.fetch();
        if (screenMatv == std::nullopt) {
            throw std::runtime_error("Screen null\n");
        }

        screenMat = screenMatv.value();
    }

    int run() {
        try {
            loadCache(picsDir);
            doFile(hospital_name);
        } catch (std::filesystem::filesystem_error &err) {
            printf("Load pictures error %s\n", hospital_name.c_str());
            return -1;
        } catch (std::exception &err) {
            printf("Exception %s\n", err.what());
            return -2;
        } catch (...) {
            printf("Unhandled exception\n");
            return -3;
        }
        logd("Automation exit");
        return 0;
    }

    void paste(const std::string &name) {
        logd("Paste %s", name.c_str());
        clicker.paste(name);
        std::this_thread::sleep_for(UI_WAITTIME);
        refreshScreen();
    }

    void test() {
        // std::this_thread::sleep_for(3s);
        // paste("11115555");
        // paste("中文也可以");
        // paste("149mfqwefkmasdf");
        // paste("91fw0%$@!#$@%+_");
        doFile("example/" + hospital_name);
    }

    void dforceClick(std::string name, int offx = 0, int offy = 0) {
        for (int i = 0; i < 15; i++) {
            if (auto r =
                    matcher.matchTemplate(name, screenMat, fetchMat(name))) {
                clicker.dclick(r->x + offx, r->y + offy);
                refreshScreen();
                return;
            }
            std::this_thread::sleep_for(1s);
            refreshScreen();
        }
        throw ScriptTimeout(name);
    }

    void dclick(std::string name, int offx = 0, int offy = 0) {
        auto r = matcher.matchTemplate(name, screenMat, fetchMat(name));
        if (r == std::nullopt) {
            throw std::runtime_error("pic not exists");
        }
        clicker.dclick(r->x + offx, r->y + offy);
        refreshScreen();
    }
};