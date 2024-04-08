#include <GUI.hpp>
#include <iostream>
#include <filesystem>

int main(){
    GUI *app = new GUI("RUNS Client", 0, 0);
    app->run();

    delete app;

    return 0;
}