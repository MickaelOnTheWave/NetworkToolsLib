#ifndef CURLLIBEMAILSENDER_H
#define CURLLIBEMAILSENDER_H

#include <curl/curl.h>
#include <string>
#include <vector>

#include "emailaccountdata.h"
#include "emaildata.h"

class CurlLibEmailSender
{
public:
    CurlLibEmailSender();
    CurlLibEmailSender(const EmailAccountData& _senderData);
    ~CurlLibEmailSender();

    void SetTempFilename(const std::string& name);

    void SetSenderData(const EmailAccountData& _senderData);

    bool Send(
        const EmailData& emailData,
        const std::vector<std::string> &fileList,
        const std::vector<std::pair<std::string,std::string> >& fileBuffers
    );

    bool Send(
        const EmailData& emailData,
        const bool useBuffers,
        std::vector<std::string> &attachmentList
    );

    std::string GetLastError() const;

protected:
    FILE* CreateEmailContent(
        const EmailData& emailData,
        const std::vector<std::string> &attachmentFiles,
        const std::vector<std::pair<std::string, std::string> >& attachmentBuffers
    );

    bool RunCurlSend(const std::wstring& destEmail, FILE* emailFile);

    void SetupCurlApi(CURL* curlApi, struct curl_slist *recipients,
                      const std::wstring& destEmail, FILE* emailFile);

    bool SetupResult(const CURLcode curlResult);

    void FillBufferListFromFileList(std::vector<std::string> &files,
                                    std::vector<std::pair<std::string,std::string> > &buffers);
    std::string GetSmtpUrl();

    std::string displayName;
    EmailAccountData senderData;
    std::string emailFileName;

private:
    std::string GetUtf8EmailAddress() const;

    std::string curlLastErrorMessage;
};

#endif // CURLLIBEMAILSENDER_H
