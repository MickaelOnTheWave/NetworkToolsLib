#include "curllibemailsender.h"

#include <time.h>
#include <sstream>
#include <string.h>

#include <filetools.h>
#include <mimetools.h>
#include <stringtools.h>

using namespace std;

static const string defaultEmailFileName = "tmpMail";

size_t payload_source(void *ptr, size_t size, size_t nmemb, void *userp)
{
	FILE* filePtr = (FILE*)(userp);

	if((size == 0) || (nmemb == 0) || ((size*nmemb) < 1)) {
		return 0;
	}

	char data[512];
   if (fgets(data, 512, filePtr) != NULL)
   {
      string tmp(data);

      if(!feof(filePtr)) {
         size_t len = strlen(data);
         memcpy(ptr, data, len);
         return len;
      }
   }

	return 0;
}

long GetFileSize(FILE* filePtr)
{
   fseek(filePtr, 0, SEEK_END);
   const long fileSize = ftell(filePtr);
   fseek(filePtr, 0, SEEK_SET);

   return fileSize;
}

CurlLibEmailSender::CurlLibEmailSender()
    : emailFileName(defaultEmailFileName),
      curlLastErrorMessage("")
{
}

CurlLibEmailSender::CurlLibEmailSender(const EmailAccountData& _senderData)
    : senderData(_senderData),
      emailFileName(defaultEmailFileName),
      curlLastErrorMessage("")
{
}

CurlLibEmailSender::~CurlLibEmailSender()
{
}

void CurlLibEmailSender::SetTempFilename(const string &name)
{
    emailFileName = name;
}

void CurlLibEmailSender::SetSenderData(const EmailAccountData& _senderData)
{
   senderData = _senderData;
}

string CurlLibEmailSender::GetLastError() const
{
   return curlLastErrorMessage;
}

bool CurlLibEmailSender::Send(
   const EmailData& emailData,
   const std::vector<std::string> &fileList,
   const std::vector<std::pair<std::string,std::string> >& fileBuffers
)
{
    FILE* emailFile = CreateEmailContent(emailData, fileList, fileBuffers);
    return RunCurlSend(emailData.GetTo(), emailFile);
}

bool CurlLibEmailSender::Send(
   const EmailData& emailData,
   const bool useBuffers,
   vector<string>& attachmentList
)
{
    FILE* emailFile = NULL;
    if (useBuffers == false)
    {
        vector<pair<string, string> > emptyList;
        emailFile = CreateEmailContent(emailData, attachmentList, emptyList);
    }
    else
    {
        vector<string> emptyList;
        vector<pair<string, string> > attachments;
        FillBufferListFromFileList(attachmentList, attachments);
        emailFile = CreateEmailContent(emailData, emptyList, attachments);
    }

    return RunCurlSend(emailData.GetTo(), emailFile);
}

string CurlLibEmailSender::GetSmtpUrl()
{
	string smtpUrl("smtp://");
   smtpUrl += StringTools::UnicodeToUtf8(senderData.GetSmtpServer()) + ":";
	stringstream s;
   s << senderData.GetSmtpPort();
	smtpUrl += s.str();
   return smtpUrl;
}

string CurlLibEmailSender::GetUtf8EmailAddress() const
{
   return StringTools::UnicodeToUtf8(senderData.GetAddress());
}

FILE *CurlLibEmailSender::CreateEmailContent(
   const EmailData& emailData,
   const vector<string>& attachmentFiles,
   const vector<pair<string, string> >& attachmentBuffers
)
{
   // TODO : check where to fix issue with newline.
   // In HTML it doesn't work, probably not the right tag.
   FILE* filePtr = fopen(emailFileName.c_str(), "w");

   MimeTools mimeCreator;
   const string contents = mimeCreator.CreateEmailContent(
      displayName,
      GetUtf8EmailAddress(),
      emailData,
      attachmentFiles,
      attachmentBuffers
   );
   fwrite(contents.c_str(), sizeof(char), contents.size(), filePtr);
   fclose(filePtr);

   filePtr = fopen(emailFileName.c_str(), "r");
   return filePtr;
}

bool CurlLibEmailSender::RunCurlSend(const std::wstring& destEmail, FILE *emailFile)
{
   bool returnValue = false;
   CURL *curl;
   CURLcode res = CURLE_OK;
   struct curl_slist *recipients = NULL;

   curl = curl_easy_init();
   if(curl) {
      SetupCurlApi(curl, recipients, destEmail, emailFile);

      res = curl_easy_perform(curl);

      returnValue = SetupResult(res);

      curl_slist_free_all(recipients);

      curl_easy_cleanup(curl);
   }
   else
      curlLastErrorMessage = "Curl initialization error";

   return returnValue;
}

void CurlLibEmailSender::SetupCurlApi(CURL* curlApi, curl_slist* recipients,
                                      const wstring& destEmail, FILE* emailFile)
{
   const string utf8Password = StringTools::UnicodeToUtf8(senderData.GetPassword());

    /* This is the URL for your mailserver */
    curl_easy_setopt(curlApi, CURLOPT_URL, GetSmtpUrl().c_str());
    curl_easy_setopt(curlApi, CURLOPT_USE_SSL, (senderData.GetUseSsl()) ? CURLUSESSL_ALL : CURLUSESSL_NONE);
    curl_easy_setopt(curlApi, CURLOPT_USERNAME, GetUtf8EmailAddress().c_str());
    curl_easy_setopt(curlApi, CURLOPT_PASSWORD, utf8Password.c_str());
    curl_easy_setopt(curlApi, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curlApi, CURLOPT_SSL_VERIFYHOST, 0L);

    /* Note that this option isn't strictly required, omitting it will result in
     * libcurl sending the MAIL FROM command with empty sender data. All
     * autoresponses should have an empty reverse-path, and should be directed
     * to the address in the reverse-path which triggered them. Otherwise, they
     * could cause an endless loop. See RFC 5321 Section 4.5.5 for more details.
     */
    curl_easy_setopt(curlApi, CURLOPT_MAIL_FROM, GetUtf8EmailAddress().c_str());

    const string utf8DestinationEmail = StringTools::UnicodeToUtf8(destEmail);
    recipients = curl_slist_append(recipients, utf8DestinationEmail.c_str());
    curl_easy_setopt(curlApi, CURLOPT_MAIL_RCPT, recipients);

    /* We're using a callback function to specify the payload (the headers and
     * body of the message). You could just use the CURLOPT_READDATA option to
     * specify a FILE pointer to read from. */
    curl_easy_setopt(curlApi, CURLOPT_INFILESIZE, GetFileSize(emailFile));
    curl_easy_setopt(curlApi, CURLOPT_READFUNCTION, payload_source);
    curl_easy_setopt(curlApi, CURLOPT_READDATA, emailFile);
    curl_easy_setopt(curlApi, CURLOPT_UPLOAD, 1L);
    curl_easy_setopt(curlApi, CURLOPT_VERBOSE, 1);
}

bool CurlLibEmailSender::SetupResult(const CURLcode curlResult)
{
   const bool result = (curlResult == CURLE_OK);
   if(result == false)
   {
      curlLastErrorMessage = curl_easy_strerror(curlResult);
      fprintf(stderr, "curl_easy_perform() failed: %s\n", curlLastErrorMessage.c_str());
   }
   return result;
}

void CurlLibEmailSender::FillBufferListFromFileList(vector<string> &files, vector<pair<string, string> > &buffers)
{
    vector<string>::const_iterator it=files.begin();
    for (; it!=files.end(); ++it)
    {
        string name = FileTools::GetFilenameFromUnixPath(*it);

        const wstring unicodeFilename = StringTools::Utf8ToUnicode(*it);
        const wstring unicodeContent = FileTools::GetTextFileContent(unicodeFilename);
        buffers.push_back(make_pair(name, StringTools::UnicodeToUtf8(unicodeContent)));
    }
}
