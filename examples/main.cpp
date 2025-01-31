#include "application.hpp"

int
main( int argc, char ** argv )
{
    Application app;
    app.Initialize();
    app.MainLoop();
}
