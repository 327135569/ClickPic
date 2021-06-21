#include "app.hpp"
#include <exception>
#include <stdexcept>

App *App::_app = nullptr;

int main(int argc, char **argv) {
    try {
        App app(argc, argv);
        return app.run();
    } catch (const std::exception &err) {
        logd("Exception %s", err.what());
        return -1;
    }
}