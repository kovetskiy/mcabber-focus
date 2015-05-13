/*
 * Module "Focus"
 *
 * Copyright 2015 Egor Kovetskiy
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/stat.h>

#include <mcabber/logprint.h>
#include <mcabber/commands.h>
#include <mcabber/screen.h>
#include <mcabber/settings.h>
#include <mcabber/modules.h>
#include <mcabber/config.h>
#include <mcabber/hooks.h>

#include <xdo.h>

static void focus_init   (void);
static void focus_uninit (void);

static unsigned int g_hook;
static xdo_t *g_xdo;
static Window g_mcabber_window;

#include <pthread.h>
static pthread_t ptid_focus;

module_info_t info_focus = {
    .branch          = MCABBER_BRANCH,
    .api             = MCABBER_API_VERSION,
    .version         = "2.2",
    .description     = "focus hokus pocus",
    .requires        = NULL,
    .init            = focus_init,
    .uninit          = focus_uninit,
    .next            = NULL,
};

static guint focus_process_message(const gchar *hook, hk_arg_t *args, gpointer data) {
    const char *message_jid, *buddy_jid;
    int is_delayed, is_error;

    message_jid = (char *)args[0].value;
    is_delayed  = (strlen(args[4].value) > 0);
    is_error    = (strcmp(args[5].value, "true") == 0);

    if (is_delayed || is_error) {
        goto finish;
    }

    buddy_jid = buddy_getjid(BUDDATA(current_buddy));

    if (buddy_jid == NULL
        || strcmp((char *)buddy_jid, (char *)message_jid) != 0
    ) {
        goto finish;
    }

    Window window;
    int ret_get_focus = xdo_get_focused_window_sane(g_xdo, &window);
    if (ret_get_focus) {
        scr_log_print(
            LPRINT_NORMAL,
            "xdo_get_focused_window_sane reported an error"
        );

        goto finish;
    }

    if (window != g_mcabber_window) {
        cmd_get("chat_disable")->func("");
    }

finish:
    return HOOK_HANDLER_RESULT_ALLOW_MORE_HANDLERS;
}

static void* handle_focus(void *arg) {
    while (true) {
        Window window;

        int ret_get_focus = xdo_get_focused_window_sane(g_xdo, &window);
        if (ret_get_focus) {
            scr_log_print(
                LPRINT_NORMAL,
                "xdo_get_focused_window_sane reported an error"
            );
        } else if (window == g_mcabber_window) {
            scr_set_chatmode(TRUE);
            scr_show_buddy_window();
        }

        usleep(100000);
    }

    return NULL;
}

static void focus_init(void) {
    scr_log_print(LPRINT_NORMAL, "focus: init");

    g_xdo = xdo_new(NULL);

    int ret_get_focus = xdo_get_focused_window_sane(g_xdo, &g_mcabber_window);
    if (ret_get_focus) {
        scr_log_print(
            LPRINT_NORMAL,
            "xdo_get_focused_window_sane reported an error"
        );
    }

    g_hook = hk_add_handler(
         focus_process_message,
         HOOK_PRE_MESSAGE_IN,
         G_PRIORITY_DEFAULT,
         NULL
    );

    pthread_create(&ptid_focus, NULL, &handle_focus, NULL);
}

static void focus_uninit(void) {
    hk_del_handler(HOOK_PRE_MESSAGE_IN, g_hook);
}
