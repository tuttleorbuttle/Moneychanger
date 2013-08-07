#ifndef UNITYINDICATOR_H
#define UNITYINDICATOR_H

// this is a small hack to create an indicator with a "Quit" button in the unity bar so the program can be shut down when started on default ubuntu.
// http://stackoverflow.com/questions/17193307/qt-systray-icon-appears-next-to-launcher-on-ubuntu-instead-of-on-the-panel

    #undef signals // Collides with GTK symbols
    extern "C" {
      #include <libappindicator/app-indicator.h>
      #include <gtk/gtk.h>
    }
    #define signals public

namespace GUI
{
    void ShowUnityIndicator();
}

#endif // UNITYINDICATOR_H
