#ifndef IRPAM_H
#define IRPAM_H

#include <security/pam_modules.h>
#include "capture/include/cameramanager.hpp"

// Exports for pam auth.
extern "C" {
    int pam_sm_authenticate(pam_handle_t *pamh, int flags, int argc, const char **argv);
    int pam_sm_setcred(pam_handle_t *pamh, int flags, int argc, const char **argv);
    int pam_sm_acct_mgmt(pam_handle_t *pamh, int flags, int argc, const char **argv);
    int pam_sm_chauthtok(pam_handle_t *pamh, int flags, int argc, const char **argv);
}

#endif