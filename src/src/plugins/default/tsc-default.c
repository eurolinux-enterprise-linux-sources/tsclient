
#include <glib.h>
#include "tsc-manager.h"
#include "tsc-rdp-provider.h"
#include "tsc-generic-provider.h"
#include "tsc-vnc-provider.h"
#include "tsc-ica-provider.h"
#include "tsc-mainframe-provider.h"
#include "tsc-ssh-provider.h"
#include "tsc-xdmcp-provider.h"
#include "tsc-web-provider.h"
#include "tsc-web-connection.h"

static void
tsc_init_ica (TSCManager *manager)
{
	TSCIcaProvider *provider;

	provider = tsc_ica_provider_new ();
	g_signal_connect (manager, "removed", G_CALLBACK (tsc_ica_provider_appsrv_removed), NULL);
	tsc_manager_register_provider (manager, TSC_PROVIDER (provider));
	tsc_ica_provider_load_connections (provider, manager);
}

void
tsc_init_plugin (TSCManager *manager)
{
	tsc_init_ica (manager);
	tsc_manager_register_provider (manager, TSC_PROVIDER (tsc_rdp_provider_new ()));
	tsc_manager_register_provider (manager, TSC_PROVIDER (tsc_vnc_provider_new ()));
	tsc_manager_register_provider (manager, TSC_PROVIDER (tsc_mainframe_provider_new ()));
	
	g_signal_connect (manager, "removed", G_CALLBACK (tsc_web_connection_removed), NULL);
	tsc_manager_register_provider (manager, TSC_PROVIDER (tsc_web_provider_new ()));

	tsc_manager_register_provider (manager, TSC_PROVIDER (tsc_ssh_provider_new ()));
	tsc_manager_register_provider (manager, TSC_PROVIDER (tsc_xdmcp_provider_new ()));
	tsc_manager_register_provider (manager, TSC_PROVIDER (tsc_generic_provider_new ()));
}
