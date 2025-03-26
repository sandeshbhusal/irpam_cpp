#include "include/CLI11.hpp"

int main(int argc, char** argv)
{
    using namespace CLI;
    App app;
    CLI11_PARSE(app, argc, argv);
    return 0;
}