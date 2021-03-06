/* Generated by dbus-binding-tool; do not edit! */

#include <glib.h>
#include <dbus/dbus-glib.h>

G_BEGIN_DECLS

#ifndef _DBUS_GLIB_ASYNC_DATA_FREE
#define _DBUS_GLIB_ASYNC_DATA_FREE
static
#ifdef G_HAVE_INLINE
inline
#endif
void
_dbus_glib_async_data_free (gpointer stuff)
{
	g_slice_free (DBusGAsyncData, stuff);
}
#endif

#ifndef DBUS_GLIB_CLIENT_WRAPPERS_org_freedesktop_DBus_Introspectable
#define DBUS_GLIB_CLIENT_WRAPPERS_org_freedesktop_DBus_Introspectable

static
#ifdef G_HAVE_INLINE
inline
#endif
gboolean
org_freedesktop_DBus_Introspectable_introspect (DBusGProxy *proxy, char ** OUT_arg0, GError **error)

{
  return dbus_g_proxy_call (proxy, "Introspect", error, G_TYPE_INVALID, G_TYPE_STRING, OUT_arg0, G_TYPE_INVALID);
}

typedef void (*org_freedesktop_DBus_Introspectable_introspect_reply) (DBusGProxy *proxy, char * OUT_arg0, GError *error, gpointer userdata);

static void
org_freedesktop_DBus_Introspectable_introspect_async_callback (DBusGProxy *proxy, DBusGProxyCall *call, void *user_data)
{
  DBusGAsyncData *data = (DBusGAsyncData*) user_data;
  GError *error = NULL;
  char * OUT_arg0;
  dbus_g_proxy_end_call (proxy, call, &error, G_TYPE_STRING, &OUT_arg0, G_TYPE_INVALID);
  (*(org_freedesktop_DBus_Introspectable_introspect_reply)data->cb) (proxy, OUT_arg0, error, data->userdata);
  return;
}

static
#ifdef G_HAVE_INLINE
inline
#endif
DBusGProxyCall*
org_freedesktop_DBus_Introspectable_introspect_async (DBusGProxy *proxy, org_freedesktop_DBus_Introspectable_introspect_reply callback, gpointer userdata)

{
  DBusGAsyncData *stuff;
  stuff = g_slice_new (DBusGAsyncData);
  stuff->cb = G_CALLBACK (callback);
  stuff->userdata = userdata;
  return dbus_g_proxy_begin_call (proxy, "Introspect", org_freedesktop_DBus_Introspectable_introspect_async_callback, stuff, _dbus_glib_async_data_free, G_TYPE_INVALID);
}
#endif /* defined DBUS_GLIB_CLIENT_WRAPPERS_org_freedesktop_DBus_Introspectable */

#ifndef DBUS_GLIB_CLIENT_WRAPPERS_org_ofono_VoiceCall
#define DBUS_GLIB_CLIENT_WRAPPERS_org_ofono_VoiceCall

static
#ifdef G_HAVE_INLINE
inline
#endif
gboolean
org_ofono_VoiceCall_get_properties (DBusGProxy *proxy, GHashTable** OUT_arg0, GError **error)

{
  return dbus_g_proxy_call (proxy, "GetProperties", error, G_TYPE_INVALID, dbus_g_type_get_map ("GHashTable", G_TYPE_STRING, G_TYPE_VALUE), OUT_arg0, G_TYPE_INVALID);
}

typedef void (*org_ofono_VoiceCall_get_properties_reply) (DBusGProxy *proxy, GHashTable *OUT_arg0, GError *error, gpointer userdata);

static void
org_ofono_VoiceCall_get_properties_async_callback (DBusGProxy *proxy, DBusGProxyCall *call, void *user_data)
{
  DBusGAsyncData *data = (DBusGAsyncData*) user_data;
  GError *error = NULL;
  GHashTable* OUT_arg0;
  dbus_g_proxy_end_call (proxy, call, &error, dbus_g_type_get_map ("GHashTable", G_TYPE_STRING, G_TYPE_VALUE), &OUT_arg0, G_TYPE_INVALID);
  (*(org_ofono_VoiceCall_get_properties_reply)data->cb) (proxy, OUT_arg0, error, data->userdata);
  return;
}

static
#ifdef G_HAVE_INLINE
inline
#endif
DBusGProxyCall*
org_ofono_VoiceCall_get_properties_async (DBusGProxy *proxy, org_ofono_VoiceCall_get_properties_reply callback, gpointer userdata)

{
  DBusGAsyncData *stuff;
  stuff = g_slice_new (DBusGAsyncData);
  stuff->cb = G_CALLBACK (callback);
  stuff->userdata = userdata;
  return dbus_g_proxy_begin_call (proxy, "GetProperties", org_ofono_VoiceCall_get_properties_async_callback, stuff, _dbus_glib_async_data_free, G_TYPE_INVALID);
}
static
#ifdef G_HAVE_INLINE
inline
#endif
gboolean
org_ofono_VoiceCall_deflect (DBusGProxy *proxy, const char * IN_arg0, GError **error)

{
  return dbus_g_proxy_call (proxy, "Deflect", error, G_TYPE_STRING, IN_arg0, G_TYPE_INVALID, G_TYPE_INVALID);
}

typedef void (*org_ofono_VoiceCall_deflect_reply) (DBusGProxy *proxy, GError *error, gpointer userdata);

static void
org_ofono_VoiceCall_deflect_async_callback (DBusGProxy *proxy, DBusGProxyCall *call, void *user_data)
{
  DBusGAsyncData *data = (DBusGAsyncData*) user_data;
  GError *error = NULL;
  dbus_g_proxy_end_call (proxy, call, &error, G_TYPE_INVALID);
  (*(org_ofono_VoiceCall_deflect_reply)data->cb) (proxy, error, data->userdata);
  return;
}

static
#ifdef G_HAVE_INLINE
inline
#endif
DBusGProxyCall*
org_ofono_VoiceCall_deflect_async (DBusGProxy *proxy, const char * IN_arg0, org_ofono_VoiceCall_deflect_reply callback, gpointer userdata)

{
  DBusGAsyncData *stuff;
  stuff = g_slice_new (DBusGAsyncData);
  stuff->cb = G_CALLBACK (callback);
  stuff->userdata = userdata;
  return dbus_g_proxy_begin_call (proxy, "Deflect", org_ofono_VoiceCall_deflect_async_callback, stuff, _dbus_glib_async_data_free, G_TYPE_STRING, IN_arg0, G_TYPE_INVALID);
}
static
#ifdef G_HAVE_INLINE
inline
#endif
gboolean
org_ofono_VoiceCall_hangup (DBusGProxy *proxy, GError **error)

{
  return dbus_g_proxy_call (proxy, "Hangup", error, G_TYPE_INVALID, G_TYPE_INVALID);
}

typedef void (*org_ofono_VoiceCall_hangup_reply) (DBusGProxy *proxy, GError *error, gpointer userdata);

static void
org_ofono_VoiceCall_hangup_async_callback (DBusGProxy *proxy, DBusGProxyCall *call, void *user_data)
{
  DBusGAsyncData *data = (DBusGAsyncData*) user_data;
  GError *error = NULL;
  dbus_g_proxy_end_call (proxy, call, &error, G_TYPE_INVALID);
  (*(org_ofono_VoiceCall_hangup_reply)data->cb) (proxy, error, data->userdata);
  return;
}

static
#ifdef G_HAVE_INLINE
inline
#endif
DBusGProxyCall*
org_ofono_VoiceCall_hangup_async (DBusGProxy *proxy, org_ofono_VoiceCall_hangup_reply callback, gpointer userdata)

{
  DBusGAsyncData *stuff;
  stuff = g_slice_new (DBusGAsyncData);
  stuff->cb = G_CALLBACK (callback);
  stuff->userdata = userdata;
  return dbus_g_proxy_begin_call (proxy, "Hangup", org_ofono_VoiceCall_hangup_async_callback, stuff, _dbus_glib_async_data_free, G_TYPE_INVALID);
}
static
#ifdef G_HAVE_INLINE
inline
#endif
gboolean
org_ofono_VoiceCall_answer (DBusGProxy *proxy, GError **error)

{
  return dbus_g_proxy_call (proxy, "Answer", error, G_TYPE_INVALID, G_TYPE_INVALID);
}

typedef void (*org_ofono_VoiceCall_answer_reply) (DBusGProxy *proxy, GError *error, gpointer userdata);

static void
org_ofono_VoiceCall_answer_async_callback (DBusGProxy *proxy, DBusGProxyCall *call, void *user_data)
{
  DBusGAsyncData *data = (DBusGAsyncData*) user_data;
  GError *error = NULL;
  dbus_g_proxy_end_call (proxy, call, &error, G_TYPE_INVALID);
  (*(org_ofono_VoiceCall_answer_reply)data->cb) (proxy, error, data->userdata);
  return;
}

static
#ifdef G_HAVE_INLINE
inline
#endif
DBusGProxyCall*
org_ofono_VoiceCall_answer_async (DBusGProxy *proxy, org_ofono_VoiceCall_answer_reply callback, gpointer userdata)

{
  DBusGAsyncData *stuff;
  stuff = g_slice_new (DBusGAsyncData);
  stuff->cb = G_CALLBACK (callback);
  stuff->userdata = userdata;
  return dbus_g_proxy_begin_call (proxy, "Answer", org_ofono_VoiceCall_answer_async_callback, stuff, _dbus_glib_async_data_free, G_TYPE_INVALID);
}
#endif /* defined DBUS_GLIB_CLIENT_WRAPPERS_org_ofono_VoiceCall */

#ifndef DBUS_GLIB_CLIENT_WRAPPERS_org_ofono_CallMeter
#define DBUS_GLIB_CLIENT_WRAPPERS_org_ofono_CallMeter

static
#ifdef G_HAVE_INLINE
inline
#endif
gboolean
org_ofono_CallMeter_get_properties (DBusGProxy *proxy, GHashTable** OUT_arg0, GError **error)

{
  return dbus_g_proxy_call (proxy, "GetProperties", error, G_TYPE_INVALID, dbus_g_type_get_map ("GHashTable", G_TYPE_STRING, G_TYPE_VALUE), OUT_arg0, G_TYPE_INVALID);
}

typedef void (*org_ofono_CallMeter_get_properties_reply) (DBusGProxy *proxy, GHashTable *OUT_arg0, GError *error, gpointer userdata);

static void
org_ofono_CallMeter_get_properties_async_callback (DBusGProxy *proxy, DBusGProxyCall *call, void *user_data)
{
  DBusGAsyncData *data = (DBusGAsyncData*) user_data;
  GError *error = NULL;
  GHashTable* OUT_arg0;
  dbus_g_proxy_end_call (proxy, call, &error, dbus_g_type_get_map ("GHashTable", G_TYPE_STRING, G_TYPE_VALUE), &OUT_arg0, G_TYPE_INVALID);
  (*(org_ofono_CallMeter_get_properties_reply)data->cb) (proxy, OUT_arg0, error, data->userdata);
  return;
}

static
#ifdef G_HAVE_INLINE
inline
#endif
DBusGProxyCall*
org_ofono_CallMeter_get_properties_async (DBusGProxy *proxy, org_ofono_CallMeter_get_properties_reply callback, gpointer userdata)

{
  DBusGAsyncData *stuff;
  stuff = g_slice_new (DBusGAsyncData);
  stuff->cb = G_CALLBACK (callback);
  stuff->userdata = userdata;
  return dbus_g_proxy_begin_call (proxy, "GetProperties", org_ofono_CallMeter_get_properties_async_callback, stuff, _dbus_glib_async_data_free, G_TYPE_INVALID);
}
static
#ifdef G_HAVE_INLINE
inline
#endif
gboolean
org_ofono_CallMeter_set_property (DBusGProxy *proxy, const char * IN_arg0, const GValue* IN_arg1, const char * IN_arg2, GError **error)

{
  return dbus_g_proxy_call (proxy, "SetProperty", error, G_TYPE_STRING, IN_arg0, G_TYPE_VALUE, IN_arg1, G_TYPE_STRING, IN_arg2, G_TYPE_INVALID, G_TYPE_INVALID);
}

typedef void (*org_ofono_CallMeter_set_property_reply) (DBusGProxy *proxy, GError *error, gpointer userdata);

static void
org_ofono_CallMeter_set_property_async_callback (DBusGProxy *proxy, DBusGProxyCall *call, void *user_data)
{
  DBusGAsyncData *data = (DBusGAsyncData*) user_data;
  GError *error = NULL;
  dbus_g_proxy_end_call (proxy, call, &error, G_TYPE_INVALID);
  (*(org_ofono_CallMeter_set_property_reply)data->cb) (proxy, error, data->userdata);
  return;
}

static
#ifdef G_HAVE_INLINE
inline
#endif
DBusGProxyCall*
org_ofono_CallMeter_set_property_async (DBusGProxy *proxy, const char * IN_arg0, const GValue* IN_arg1, const char * IN_arg2, org_ofono_CallMeter_set_property_reply callback, gpointer userdata)

{
  DBusGAsyncData *stuff;
  stuff = g_slice_new (DBusGAsyncData);
  stuff->cb = G_CALLBACK (callback);
  stuff->userdata = userdata;
  return dbus_g_proxy_begin_call (proxy, "SetProperty", org_ofono_CallMeter_set_property_async_callback, stuff, _dbus_glib_async_data_free, G_TYPE_STRING, IN_arg0, G_TYPE_VALUE, IN_arg1, G_TYPE_STRING, IN_arg2, G_TYPE_INVALID);
}
static
#ifdef G_HAVE_INLINE
inline
#endif
gboolean
org_ofono_CallMeter_reset (DBusGProxy *proxy, const char * IN_arg0, GError **error)

{
  return dbus_g_proxy_call (proxy, "Reset", error, G_TYPE_STRING, IN_arg0, G_TYPE_INVALID, G_TYPE_INVALID);
}

typedef void (*org_ofono_CallMeter_reset_reply) (DBusGProxy *proxy, GError *error, gpointer userdata);

static void
org_ofono_CallMeter_reset_async_callback (DBusGProxy *proxy, DBusGProxyCall *call, void *user_data)
{
  DBusGAsyncData *data = (DBusGAsyncData*) user_data;
  GError *error = NULL;
  dbus_g_proxy_end_call (proxy, call, &error, G_TYPE_INVALID);
  (*(org_ofono_CallMeter_reset_reply)data->cb) (proxy, error, data->userdata);
  return;
}

static
#ifdef G_HAVE_INLINE
inline
#endif
DBusGProxyCall*
org_ofono_CallMeter_reset_async (DBusGProxy *proxy, const char * IN_arg0, org_ofono_CallMeter_reset_reply callback, gpointer userdata)

{
  DBusGAsyncData *stuff;
  stuff = g_slice_new (DBusGAsyncData);
  stuff->cb = G_CALLBACK (callback);
  stuff->userdata = userdata;
  return dbus_g_proxy_begin_call (proxy, "Reset", org_ofono_CallMeter_reset_async_callback, stuff, _dbus_glib_async_data_free, G_TYPE_STRING, IN_arg0, G_TYPE_INVALID);
}
#endif /* defined DBUS_GLIB_CLIENT_WRAPPERS_org_ofono_CallMeter */

#ifndef DBUS_GLIB_CLIENT_WRAPPERS_org_ofono_CallVolume
#define DBUS_GLIB_CLIENT_WRAPPERS_org_ofono_CallVolume

static
#ifdef G_HAVE_INLINE
inline
#endif
gboolean
org_ofono_CallVolume_get_properties (DBusGProxy *proxy, GHashTable** OUT_arg0, GError **error)

{
  return dbus_g_proxy_call (proxy, "GetProperties", error, G_TYPE_INVALID, dbus_g_type_get_map ("GHashTable", G_TYPE_STRING, G_TYPE_VALUE), OUT_arg0, G_TYPE_INVALID);
}

typedef void (*org_ofono_CallVolume_get_properties_reply) (DBusGProxy *proxy, GHashTable *OUT_arg0, GError *error, gpointer userdata);

static void
org_ofono_CallVolume_get_properties_async_callback (DBusGProxy *proxy, DBusGProxyCall *call, void *user_data)
{
  DBusGAsyncData *data = (DBusGAsyncData*) user_data;
  GError *error = NULL;
  GHashTable* OUT_arg0;
  dbus_g_proxy_end_call (proxy, call, &error, dbus_g_type_get_map ("GHashTable", G_TYPE_STRING, G_TYPE_VALUE), &OUT_arg0, G_TYPE_INVALID);
  (*(org_ofono_CallVolume_get_properties_reply)data->cb) (proxy, OUT_arg0, error, data->userdata);
  return;
}

static
#ifdef G_HAVE_INLINE
inline
#endif
DBusGProxyCall*
org_ofono_CallVolume_get_properties_async (DBusGProxy *proxy, org_ofono_CallVolume_get_properties_reply callback, gpointer userdata)

{
  DBusGAsyncData *stuff;
  stuff = g_slice_new (DBusGAsyncData);
  stuff->cb = G_CALLBACK (callback);
  stuff->userdata = userdata;
  return dbus_g_proxy_begin_call (proxy, "GetProperties", org_ofono_CallVolume_get_properties_async_callback, stuff, _dbus_glib_async_data_free, G_TYPE_INVALID);
}
static
#ifdef G_HAVE_INLINE
inline
#endif
gboolean
org_ofono_CallVolume_set_property (DBusGProxy *proxy, const char * IN_arg0, const GValue* IN_arg1, GError **error)

{
  return dbus_g_proxy_call (proxy, "SetProperty", error, G_TYPE_STRING, IN_arg0, G_TYPE_VALUE, IN_arg1, G_TYPE_INVALID, G_TYPE_INVALID);
}

typedef void (*org_ofono_CallVolume_set_property_reply) (DBusGProxy *proxy, GError *error, gpointer userdata);

static void
org_ofono_CallVolume_set_property_async_callback (DBusGProxy *proxy, DBusGProxyCall *call, void *user_data)
{
  DBusGAsyncData *data = (DBusGAsyncData*) user_data;
  GError *error = NULL;
  dbus_g_proxy_end_call (proxy, call, &error, G_TYPE_INVALID);
  (*(org_ofono_CallVolume_set_property_reply)data->cb) (proxy, error, data->userdata);
  return;
}

static
#ifdef G_HAVE_INLINE
inline
#endif
DBusGProxyCall*
org_ofono_CallVolume_set_property_async (DBusGProxy *proxy, const char * IN_arg0, const GValue* IN_arg1, org_ofono_CallVolume_set_property_reply callback, gpointer userdata)

{
  DBusGAsyncData *stuff;
  stuff = g_slice_new (DBusGAsyncData);
  stuff->cb = G_CALLBACK (callback);
  stuff->userdata = userdata;
  return dbus_g_proxy_begin_call (proxy, "SetProperty", org_ofono_CallVolume_set_property_async_callback, stuff, _dbus_glib_async_data_free, G_TYPE_STRING, IN_arg0, G_TYPE_VALUE, IN_arg1, G_TYPE_INVALID);
}
#endif /* defined DBUS_GLIB_CLIENT_WRAPPERS_org_ofono_CallVolume */

G_END_DECLS
