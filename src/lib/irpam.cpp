#include "recognition.hpp"
#include "include/irpam.hpp"

extern "C" int pam_sm_authenticate(pam_handle_t *pamh, int flags, int argc, const char **argv)
{
    cv::Mat sample;
    if (are_similar(sample, sample))
    {
        return PAM_SUCCESS;
    }
    else
    {
        return PAM_AUTH_ERR;
    }
}
extern "C" int pam_sm_setcred(pam_handle_t *pamh, int flags, int argc, const char **argv)
{
    return PAM_SUCCESS;
}
extern "C" int pam_sm_acct_mgmt(pam_handle_t *pamh, int flags, int argc, const char **argv)
{
    return PAM_SUCCESS;
}
extern "C" int pam_sm_chauthtok(pam_handle_t *pamh, int flags, int argc, const char **argv)
{
    return PAM_SUCCESS;
}
