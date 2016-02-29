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
#include <signal.h>

#include <mcabber/logprint.h>
#include <mcabber/commands.h>
#include <mcabber/screen.h>
#include <mcabber/settings.h>
#include <mcabber/modules.h>
#include <mcabber/config.h>
#include <mcabber/hooks.h>

#include <xdo.h>

#define UNUSED(x) (x = x)

static void focus_init   (void);
static void focus_uninit (void);

static unsigned int g_message_hook;
static unsigned int g_unread_hook;

static xdo_t *g_xdo;

static Window g_mcabber_window;

static unsigned int g_unread_count;

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

static unsigned int focus_process_message(
    const gchar *hook, hk_arg_t *args, gpointer data
) {
    UNUSED(hook);
    UNUSED(data);

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

static unsigned int focus_unread_handler(
    const gchar *hook, hk_arg_t *args, gpointer data
) {
    UNUSED(hook);
    UNUSED(data);

    unsigned int total_unread = 0;

    for (; args->name; args++) {
        if (!g_strcmp0(args->name, "unread")) {
            total_unread = atoi(args->value);
        }
    }

    if (g_unread_count > total_unread) {
        int ret_get_focus = xdo_get_focused_window_sane(
            g_xdo, &g_mcabber_window
        );
        if (ret_get_focus) {
            scr_log_print(
                LPRINT_NORMAL,
                "xdo_get_focused_window_sane reported an error"
            );
        }
    }

    g_unread_count = total_unread;

    return HOOK_HANDLER_RESULT_ALLOW_MORE_HANDLERS;
}


static void focus_init(void) {
    scr_log_print(LPRINT_NORMAL, "focus: init");

    g_xdo = xdo_new(NULL);
    if (g_xdo == NULL) {
        scr_log_print(LPRINT_NORMAL, "focus: xdo_new failed.");
        return;
    }

    int ret_get_focus = xdo_get_focused_window_sane(g_xdo, &g_mcabber_window);
    if (ret_get_focus) {
        scr_log_print(
            LPRINT_NORMAL,
            "xdo_get_focused_window_sane reported an error"
        );
    }

    g_message_hook = hk_add_handler(
         focus_process_message,
         HOOK_PRE_MESSAGE_IN,
         G_PRIORITY_DEFAULT,
         NULL
    );

    g_unread_hook = hk_add_handler(
         focus_unread_handler,
         HOOK_UNREAD_LIST_CHANGE,
         G_PRIORITY_DEFAULT,
         NULL
    );

    g_unread_count = 0;
}

static void focus_uninit(void) {
    hk_del_handler(HOOK_PRE_MESSAGE_IN, g_message_hook);
    hk_del_handler(HOOK_UNREAD_LIST_CHANGE, g_unread_hook);
}
