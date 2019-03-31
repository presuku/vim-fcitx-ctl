#define _GNU_SOURCE
#include <limits.h>
#include <unistd.h>
#include <sys/signal.h>
#include <stdio.h>
#include <string.h>
#include <dlfcn.h>
#include <dbus/dbus.h>
#include <fcitx-utils/utils.h>
#include <fcitx/frontend.h>

#ifdef DEBUG
#define DBG_FPRINTF(x, ...) fprintf(x, __VA_ARGS__)
#else
#define DBG_FPRINTF(x, ...)
#endif

#define FCITX_DBUS_SERVICE "org.fcitx.Fcitx"
#define FCITX_IM_DBUS_PATH "/inputmethod"
#define FCITX_IM_DBUS_INTERFACE "org.fcitx.Fcitx.InputMethod"

enum {
    FCITX_DBUS_ACTIVATE,
    FCITX_DBUS_INACTIVATE,
    FCITX_DBUS_GET_CURRENT_STATE,
    FCITX_DBUS_N
};

static DBusMessage* g_mess[FCITX_DBUS_N];
static DBusConnection* g_conn = NULL;

static void *self = NULL;
static int im_on = -1;
static int inited = 0;

static inline char* get_socket_path(void)
{
    char *addr_file = NULL;
    char *m_id = NULL;
    char* file = NULL;
    int ret;

    m_id = dbus_get_local_machine_id();
    if (!m_id) {
        return NULL;
    }

    ret = asprintf(&addr_file, "%s-%d", m_id, fcitx_utils_get_display_number());
    dbus_free(m_id);
    if (ret == -1) {
        return NULL;
    }

    FcitxXDGGetFileUserWithPrefix("dbus", addr_file, NULL, &file);
    free(addr_file);
    return file;
}

static inline char *get_address_from_env(void)
{
    char *env = NULL;
    env = getenv("FCITX_DBUS_ADDRESS");
    if (!env) {
        return NULL;
    }
    return strdup(env);
}

static inline char *get_address_from_socket(void)
{
    const int BUFSIZE = 1024;
    char buf[BUFSIZE];
    char *spath = NULL;
    FILE *fp = NULL;
    char *p = NULL;
    pid_t *pids;
    size_t sz;

    spath = get_socket_path();
    if (!spath) {
        return NULL;
    }
    fp = fopen(spath, "r");
    free(spath);
    if (!fp) {
        return NULL;
    }

    sz = fread(buf, sizeof(char), BUFSIZE, fp);
    fclose(fp);

    p = memchr(buf, '\0', sz);
    if (!(p && sz == p - buf + (2 * sizeof(pid_t)) + 1)) {
        return NULL;
    }

    /* skip '\0' after string (unix:abstract=/tmp/dbus-*,guid=*). */
    ++p;
    pids = (pid_t*) p;
    if (!fcitx_utils_pid_exists(pids[0])
        || !fcitx_utils_pid_exists(pids[1])) {
        return NULL;
    }
    return strdup(buf);
}

static inline char* get_dbus_address(void)
{
    char *addr = NULL;

    addr = get_address_from_env();
    if (!addr) {
        addr = get_address_from_socket();
    }
    return addr;
}

int load(const char* dsopath)
{
    char *srv_name = NULL;
    char *addr = NULL;
    DBusMessage* mes = NULL;
    DBusConnection* conn = NULL;

    int ret;
    int i;

    for (i = 0; i < FCITX_DBUS_N; ++i) {
        g_mess[i] = NULL;
    }

    DBG_FPRINTF(stderr, "%s, start\n", __func__);

    self = dlopen(dsopath, RTLD_LAZY);
    if (!self) {
        return 1;
    }

    ret = asprintf(&srv_name, "%s-%d",
            FCITX_DBUS_SERVICE, fcitx_utils_get_display_number());
    if (ret == -1) {
        goto error;
    }

    mes = dbus_message_new_method_call(srv_name, FCITX_IM_DBUS_PATH,
                                   FCITX_IM_DBUS_INTERFACE, "ActivateIM");
    if (mes) {
        g_mess[FCITX_DBUS_ACTIVATE] = mes;
    } else {
        goto error;
    }

    mes = dbus_message_new_method_call(srv_name, FCITX_IM_DBUS_PATH,
                                   FCITX_IM_DBUS_INTERFACE, "InactivateIM");
    if (mes) {
        g_mess[FCITX_DBUS_INACTIVATE] = mes;
    } else {
        goto error;
    }

    mes = dbus_message_new_method_call(srv_name, FCITX_IM_DBUS_PATH,
                                   FCITX_IM_DBUS_INTERFACE, "GetCurrentState");
    if (mes) {
        g_mess[FCITX_DBUS_GET_CURRENT_STATE] = mes;
    } else {
        goto error;
    }
    free(srv_name);

    addr = get_dbus_address();
    if (!addr) {
        goto error;
    }

    conn = dbus_connection_open(addr, NULL);
    fcitx_utils_free(addr);
    if (!conn) {
        goto error;
    }

    if (!dbus_bus_register(conn, NULL)) {
        dbus_connection_unref(conn);
        conn = NULL;
        goto error;
    }

    if (!conn) {
        conn = dbus_bus_get(DBUS_BUS_SESSION, NULL);
        if (!conn) {
            goto error;
        }
        dbus_connection_set_exit_on_disconnect(conn, FALSE);
    }

    g_conn = conn;
    inited = 1;

    DBG_FPRINTF(stderr, "%s, end\n", __func__);

    return 0;
error:
    if (srv_name) {
        free(srv_name);
    }
    for (i = 0; i < FCITX_DBUS_N; ++i) {
        if (g_mess[i]) {
            dbus_message_unref(g_mess[i]);
        }
    }
    if (conn) {
        dbus_connection_unref(conn);
        g_conn = NULL;
    }
    return 1;
}

int unload(int unuse)
{
    int i;

    (void)unuse;

    DBG_FPRINTF(stderr, "%s, start\n", __func__);
    if (self) {
        dlclose(self);
        self = NULL;
    }

    for (i = 0; i < FCITX_DBUS_N; ++i) {
        if (g_mess[i]) {
            dbus_message_unref(g_mess[i]);
        }
    }

    if (g_conn) {
        dbus_connection_unref(g_conn);
        g_conn = NULL;
    }

    DBG_FPRINTF(stderr, "%s, end\n", __func__);
    return 0;
}

int is_im_enable(int unuse)
{
    DBusMessage* reply = NULL;
    int result = 0;

    (void)unuse;

    if (!inited) {
        goto end;
    }

    DBG_FPRINTF(stderr, "%s, start\n", __func__);

    reply = dbus_connection_send_with_reply_and_block(
                  g_conn, g_mess[FCITX_DBUS_GET_CURRENT_STATE], 1000, NULL);
    if (reply
        && dbus_message_get_args(reply, NULL, DBUS_TYPE_INT32,
                                 &result, DBUS_TYPE_INVALID)) {
        im_on = (result == 2) ? 1 : 0;
    }

end:
    DBG_FPRINTF(stderr, "%s, end, reply:%p, result:%d\n", __func__, reply, result);
    return im_on;
}

int im_set(char *active)
{
    DBG_FPRINTF(stderr, "%s, start, active : %s\n", __func__, active);

    if (active[0] == '1') {
        dbus_connection_send(g_conn, g_mess[FCITX_DBUS_ACTIVATE], NULL);
    } else {
        dbus_connection_send(g_conn, g_mess[FCITX_DBUS_INACTIVATE], NULL);
    }
    dbus_connection_flush(g_conn);

    DBG_FPRINTF(stderr, "%s, end\n", __func__);
    return 0;
}

