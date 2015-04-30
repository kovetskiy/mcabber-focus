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
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include <mcabber/logprint.h>
#include <mcabber/commands.h>
#include <mcabber/screen.h>
#include <mcabber/settings.h>
#include <mcabber/modules.h>
#include <mcabber/config.h>
#include <mcabber/hooks.h>

// xdo vs std=c99
typedef unsigned int useconds_t;
#include <xdo.h>

static void focus_init   (void);
static void focus_uninit (void);

static unsigned int g_hook;
static xdo_t *xdo;

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
    int ret_get_focus = xdo_get_focused_window_sane(xdo, &window);
    if (ret_get_focus) {
        scr_log_print(
            LPRINT_NORMAL,
            "xdo_get_focused_window_sane reported an error"
        );

        goto finish;
    }

    unsigned char *window_name;
    int window_name_len, window_name_type;

    int ret_get_name = xdo_get_window_name(
        xdo, window,
        &window_name, &window_name_len, &window_name_type
    );

    if (ret_get_name) {
        scr_log_print(
            LPRINT_NORMAL,
            "xdo_get_window_name reported an error"
        );

        goto finish;
    }

    // TODO: move to settings or get this value in initialization step
    if (strcmp((char *)window_name, "mcabber") != 0) {
        cmd_get("chat_disable")->func("");
    }

finish:
    return HOOK_HANDLER_RESULT_ALLOW_MORE_HANDLERS;
}

static void focus_init(void) {
    scr_log_print(LPRINT_NORMAL, "focus: init");

    xdo = xdo_new(NULL);

    g_hook = hk_add_handler(
         focus_process_message,
         HOOK_PRE_MESSAGE_IN,
         G_PRIORITY_DEFAULT,
         NULL
     );
}

static void focus_uninit(void) {
    hk_del_handler(HOOK_PRE_MESSAGE_IN, g_hook);
}
