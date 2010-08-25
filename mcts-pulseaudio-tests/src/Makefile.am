#AUTOMAKE_OPTIONS=foreign
LDFLAGS = -l pulse
audio_PROGRAMS = pa_query_control
audio_SCRIPTS = PA_daemon_status_001.sh PA_loaded_sink_query_001.sh PA_module_load_unload_001.sh PA_set_default_sink_001.sh PA_statistics_001.sh PA_default_setting_001.sh PA_loaded_source_query_001.sh PA_sample_001.sh PA_set_default_source_001.sh PA_loaded_client_query_001.sh PA_mem_statistics_001.sh PA_server_info_query_001.sh PA_sink_input_query_001.sh env.sh
audiodir = /opt/$(PACKAGE)
EXTRA_DIST = $(audio_SCRIPTS) $(audio_PROGRAMS)

