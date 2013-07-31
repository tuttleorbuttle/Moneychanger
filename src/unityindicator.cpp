#include "unityindicator.h"
#include "moneychanger.h"
#include <QApplication>

namespace GUI
{
    void quitIndicator(GtkMenu *, gpointer);

    void ShowUnityIndicator()
    {
        QString desktop;
        bool is_unity;

        desktop = getenv("XDG_CURRENT_DESKTOP");
        is_unity = (desktop.toLower() == "unity");

        if (is_unity) {
          AppIndicator *indicator;
          GtkWidget *menu, *item;

          menu = gtk_menu_new();

          item = gtk_menu_item_new_with_label("Quit");
          gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
          g_signal_connect(item, "activate",
                       G_CALLBACK(quitIndicator), qApp);  // We cannot connect
                       // gtk signal and qt slot so we need to create proxy
                       // function later on, we pass qApp pointer as an argument.
                       // This is useful when we need to call signals on "this"
                       //object so external function can access current object
          gtk_widget_show(item);

          indicator = app_indicator_new(
          "unique-application-name",
              "indicator-messages",
            APP_INDICATOR_CATEGORY_APPLICATION_STATUS
          );

          app_indicator_set_status(indicator, APP_INDICATOR_STATUS_ACTIVE);
          app_indicator_set_menu(indicator, GTK_MENU(menu));
        }
    }

    void quitIndicator(GtkMenu *menu, gpointer data)
    {
        Q_UNUSED(menu);
        QApplication *self = static_cast<QApplication *>(data);

        self->quit();
    }
}
