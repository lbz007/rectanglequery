
#include "gui_tcl_command.h"
#include "console/tcl_console.h"
#include "qtint/qt_int.h"
#include "util/util.h"
#include "win/main_window.h"

namespace open_edi {
namespace gui {

#define GUI_ON_MODE  "-has_gui"
#define GUI_OFF_MODE "-no_gui"

//set the default startup mode for gui
#define DEFUALT_GUI_DISPLAY false

#if (DEFUALT_GUI_DISPLAY == false)
#define DEFUALT_GUI_STARTUP_MOD GUI_ON_MODE
#else
#define DEFUALT_GUI_STARTUP_MOD GUI_OFF_MODE
#endif

static bool display = DEFUALT_GUI_DISPLAY;

static int showGUI(ClientData cld, Tcl_Interp* itp, int argc, const char* argv[]) {
    MAIN_WINDOW->setTclInterp(itp);
    MAIN_WINDOW->show();

    return TCL_OK;
}

static int hideGUI(ClientData cld, Tcl_Interp* itp, int argc, const char* argv[]) {

    MAIN_WINDOW->hide();

    return TCL_OK;
}

void registerGuiTclCommands(Tcl_Interp* itp) {
    if (!display) return;

    QtNotifier::registerTclNotifier();
    Tcl_CreateCommand(itp, "show_gui", showGUI, nullptr, nullptr);
    Tcl_CreateCommand(itp, "hide_gui", hideGUI, nullptr, nullptr);

    //show gui instantly
    Tcl_Eval(itp, "show_gui");
}

void tclConsoleInit(Tcl_Interp* itp) {
    TclreadlineAppInit(itp, open_edi::util::getInstallPath());
}

/// @brief start application according option(with/without gui).
///
/// @param argc
/// @param argv[]
///
/// @return
QCoreApplication* startQt(int& argc, char* argv[]) {
    for (int i = 1; i < argc; ++i) {
        if (!qstrcmp(argv[i], DEFUALT_GUI_STARTUP_MOD)) {

            //remove this "-no_gui" form argv[] due to tcl can not realize it.
            for (int j = i; j < argc; ++j) {
                if (j + 1 < argc) {
                    argv[j] = argv[j + 1];
                }
            }
            argc -= 1;
#if (DEFUALT_GUI_DISPLAY == false)
            display = true;
            return new QApplication(argc, argv);
#else
            // use raw tcl event loop for efficiency if no gui
            display = false;
            return nullptr;
            // return new QCoreApplication (argc, argv);
#endif
        }
    }
#if (DEFUALT_GUI_DISPLAY == false)
    // use raw tcl event loop for efficiency if no gui
    display = false;
    return nullptr;
#else
    display = true;
    return new QApplication(argc, argv);
#endif
}

} // namespace gui
} // namespace open_edi
