/*! \file DSP_logstream.cpp
 * logging and console output support cpp file
 *
 * \author Marek Blok
 */

#include <cassert>

#include <cstring>
#include <iostream>
using namespace std;

#include "DSP_logstream.h"

#include <DSP_misc.h> // DSP::f::ErrorMessage and DSP::f::InfoMessage

namespace DSP
{

    // ***************************************************** //
    // ***************************************************** //
    // ***************************************************** //

    //! main DSP::log for general use
    logstream log;

    //! Class for the object storing LogStatus
    /*! Default LOG MODE is to print onto console
     *  but in case of GUI application this must be changed.
     *
     *
     * Add relaying messages to user defined function,
     * for example to be used in wxWidgets for displaying messages
     * in dialog window
     *
     *
     * \todo_later LOG file should be opened in share read mode
     */
    class LogStatus
    {
      friend void DSP::logstream::SetLogState(const DSP::e::LogState &);
      friend void DSP::logstream::SetLogFileName(const string &);
      friend void DSP::logstream::SetLogFunctionPtr(DSP::Message_callback_ptr);
      friend long int DSP::logstream::NoOfErrors(bool Reset);
      friend DSP::logstream& ::operator<< (DSP::logstream& os, const DSP::e::LogMode& log_mode);
      friend class DSP::logbuf;

      private:
        // LOG actions mode
        DSP::e::LogState Mode;
        long int ErrorsCounter;

        //! LOG file name (possibly together with path)
        string file_name;
        std::ofstream *plik; // bool IsFileOpened;

        //! User defined Message processing function
        Message_callback_ptr function_ptr;

        /*! File is created when first LOG message is issued
         *  with ErrorMessage, InfoMessage or Message
         *
         *  Must take care of
         *   - LS_file=2,
         *   - LS_file_append=6, // = (LS_file | 4)
         */
        void WriteMessage2LOGfile(const string &Message);

        /*! \warning Should be used only in WriteMessage2LOGfile
         */
        void OpenLOGfile(void);
        /*! \warning To be used only in LogStatus
         */
        void CloseLOGfile(void);

      public:
        //! LogStatus constructor
        LogStatus(void);
        /*! In the destructor LOG file if it is opened will be closed
         */
        ~LogStatus(void);
    };


// http://wordaligned.org/articles/cpp-streambufs

     /*! Wysłanie znaku z wybranego wątku blokuje mutex, aż do otrzymania endl.
       * W ten sposób w obrębie jednej linii do tego samego bufora
       * inny wątek może pisać dopiero gdy zakończona jest bieżąca linia.
       */
     class logbuf: public std::streambuf
     {
        friend class DSP::logstream;
        friend DSP::logstream& ::operator<< (DSP::logstream& os, const DSP::e::LogMode& log_mode);

      public:
        logbuf(void);
        ~logbuf(void);

      private:
        // if true new line begins
        bool start_new_line;
        std::shared_ptr<std::recursive_mutex> mtx = nullptr;

        string First_string; //! Main part of the collected message
        string Second_string; //! Second_string part of the collected message
        DSP::e::LogMode mode; //! stream mode (Error/Info)
        DSP::e::LogMode msg_part; //! message part (first/second)
        bool pause_after_message; //! if true wiat for key press in console mode

        LogStatus LogStatus_object; //DSPo_LibraryLogStatus;

        void init_logbuf(void);

        // This log buffer has no buffer. So every character "overflows"
        // and can be put directly into the output buffers.
        virtual int overflow(int c);
        // Sync buffers.
        virtual int sync();

        //! Error message generation
        /*! Waits for user reaction.
         */
        void ErrorMessage(const string &source, const string &message = "");
        //! Information message generation
        /* -# source == "" && message == "" => "\n"
         * -# source != "" && message == "" => "source\n"
         * -# source == "" && message != "" => "Info: message\n"
         * -# source != "" && message != "" => "Info(source): message\n"
         * .
         */
        void InfoMessage(const string &source = "", const string &message = "");
        // //! returns size of text buffer required for the given message
        //int DSP::f::GetMessageLength(bool IsError, const string &source, const string &message = NULL);

        //! Returns error message in format used in DSP::f::ErrorMessage.
        /*!  See also ::DSP::f::GetErrorMessage, ::DSP::f::GetInfoMessage.
         */
        string GetErrorMessage(const string &source, const string &message = "");
        //! Returns informational message in format used in DSP::f::InfoMessage.
        /*!  See also ::GetMessageLength.
         */
        string GetInfoMessage(const string &source, const string &message = "");
        //! Error or Information message generation
        void Message(bool IsError, const string &source, const string &message = "");
     };

    logstream::logstream(void)
    {
       init_logstream();
    // test:
    //  log << "test" << endl;
    //  log << "test2" <<  LogMode::second << "2" << endl;
    }

    logstream::~logstream(void)
    {
      log_buf.reset();
    //  syslog_stream_buf.reset();
    }


    void logstream::init_logstream()
    {
    //  syslog_stream_buf =  log.syslog_stream_buf;

      //log_buf = std::shared_ptr<logbuf>(new logbuf);
      log_buf.reset(new logbuf);
      init(log_buf.get());
    }

    int logbuf::overflow(int c)
    {
      if (c == EOF)
      {
        return !EOF;
      }
      else
      {
        mtx.get()->lock();

        if (start_new_line == true)
        {
          // a first char for new line (message) appeared
          // zablokuj aż pojawi się endl // podwójny lock //
          mtx.get()->lock();

          start_new_line = false;

          /*
          ptime CurrentTime_UTC(microsec_clock::universal_time());
          std::string time = ptime_to_string_ms(CurrentTime_UTC);

          if (sb2 != NULL)
          {
              sb2->sputn(time.c_str(),time.size());
              sb2->sputn(": ", 2);
          }
          else
          {
              sb1->sputn(time.c_str(),time.size());
              sb1->sputn(": ", 2);
          }
          */
        }

        if (c == '\n')
        {
          // send First_string and Second_string using ErrorMessage or InfoMessage
          switch (mode) {
            case DSP::e::LogMode::Error:
              ErrorMessage(First_string, Second_string);
              break;
            case DSP::e::LogMode::Info:
              InfoMessage(First_string, Second_string);
              break;
            default:
              // ignore first and second
              break;
          }

          start_new_line = true;
          First_string = "";
          Second_string = "";
          msg_part = DSP::e::LogMode::first;
          mode = DSP::e::LogMode::Info; // default
          pause_after_message = false;
          msg_part = DSP::e::LogMode::first; // start from first part

          mtx.get()->unlock(); // drugi lock //
        }
        else {
          switch (msg_part) {
            case DSP::e::LogMode::first:
              First_string += (char)c;
              break;
            case DSP::e::LogMode::second:
              Second_string += (char)c;
              break;
            default:
              mtx.get()->unlock();
              assert(!"Undefined DSP::e::LogMode value");
              break;
          }
        }
        mtx.get()->unlock();
        //return r1 == EOF ? EOF : c;
        return c;
      }
    }

    logbuf::logbuf(void)
    {
      //mtx = std::shared_ptr<boost::recursive_mutex>(new boost::recursive_mutex);
      mtx.reset(new std::recursive_mutex);

      init_logbuf();
    }

    void logbuf::init_logbuf(void)
    {
      start_new_line = true;

      First_string = "";
      Second_string = "";
      mode = DSP::e::LogMode::Info;
      pause_after_message = false;
      msg_part = DSP::e::LogMode::first;
    }

    logbuf::~logbuf(void)
    {
      //! \test verify this solution
      mtx.get()->lock(); // ? wait for unloging current lock and block other lock
      mtx.get()->unlock();
      //assert(mtx);
      //delete mtx;
    }

    int logbuf::sync()
    {
      int res = 0;

      // nothing to do

      return res;
    }

    /*
    template <class charT, class Traits>
    inline basic_ostream<charT,Traits>&
    sfmt(basic_ostream<charT,Traits>& ostr, const char* f)        \\1
    {
      try {                                                        \\2
        odatstream<charT,Traits>* p = dynamic_cast<odatstream<charT,Traits>*>(&ostr);
      }
      catch (bad_cast)                                              \\3
      {
        return ostr;
      }

      p->fmt(f);                                                   \\4
      return ostr;
    }
    */



    // ************************************************ //
    // ************************************************ //
    // ************************************************ //
    LogStatus::LogStatus(void)
    {
      Mode = DSP::e::LogState::console;
      ErrorsCounter=0;
      plik = NULL;
      file_name = "";

      function_ptr=NULL;
    }

    LogStatus::~LogStatus(void)
    {

      CloseLOGfile();

      // just to be sure
      if (plik != NULL)
      {
        delete plik;
        plik=NULL;
      }
      file_name = "";
    }

    void logstream::SetLogState(const DSP::e::LogState &Mode)
    {
      //Problem with file option if it is swithed off
      //LOG file should ne closed
      if (((log_buf->LogStatus_object.Mode & DSP::e::LogState::file) == DSP::e::LogState::file) &&
          ((Mode & DSP::e::LogState::file) != DSP::e::LogState::file))
      {
        log_buf->LogStatus_object.CloseLOGfile();
      }

      log_buf->LogStatus_object.Mode = Mode;
    }

    void logstream::SetLogFileName(const string &file_name)
    {
      log_buf->LogStatus_object.file_name = file_name;

      log_buf->LogStatus_object.CloseLOGfile();
    }

    void logstream::SetLogFunctionPtr(Message_callback_ptr function_ptr)
    {
      log_buf->LogStatus_object.function_ptr = function_ptr;
    }

    void LogStatus::WriteMessage2LOGfile(const string &Message)
    {
      if (plik == NULL)
      {
        OpenLOGfile();
        if (plik == NULL)
        { // sorry NO file output
          return;
        }
      }
      if (plik->is_open() == false)
        OpenLOGfile();
      if (plik == NULL)
      { // sorry NO file output
        return;
      }

      (*plik) << Message;
      plik->flush();
    }

    void LogStatus::OpenLOGfile(void)
    {
      CloseLOGfile();

      if ((Mode & DSP::e::LogState::file) == DSP::e::LogState::file)
      {
        if ((Mode & DSP::e::LogState::append) == DSP::e::LogState::append)
        {
          plik = new std::ofstream(file_name, std::ofstream::app | std::ofstream::out);
        }
        else
        {
          plik = new std::ofstream(file_name, std::ofstream::trunc | std::ofstream::out);
        }
        if (plik != NULL)
          if (plik -> is_open() == false)
          { // failed to open file
            delete plik;
            plik = NULL;
          }
      }

    }

    void LogStatus::CloseLOGfile(void)
    {
      if (plik != NULL)
      {
        if (plik->is_open() == true)
          plik->close();

        delete plik;
        plik=NULL;
      }
    }

    long int logstream::NoOfErrors(bool Reset)
    {
      long int temp;

      temp = log_buf->LogStatus_object.ErrorsCounter;

      if (Reset == true)
        log_buf->LogStatus_object.ErrorsCounter = 0;
      return temp;
    }

    // Error message generation
    void logbuf::ErrorMessage(const string &source, const string &message)
    {
      string MessageText;

      // ******************************************* //
      LogStatus_object.ErrorsCounter++;
      // ******************************************* //
      if (LogStatus_object.Mode == DSP::e::LogState::off)
        return;
//      if (source.length() == 0) // pause in console even if the message is empty
//        return;

      if ((LogStatus_object.Mode & DSP::e::LogState::user_function) == DSP::e::LogState::user_function)
      { // console
        if (LogStatus_object.function_ptr != NULL)
          if ((*LogStatus_object.function_ptr)(source, message, true) == true)
            return;
      }

      // ******************************************* //
      MessageText = GetErrorMessage(source, message);

      // ******************************************* //

      if ((LogStatus_object.Mode & DSP::e::LogState::console) == DSP::e::LogState::console)
      { // console
        //printf(MessageText.c_str());
        cout << MessageText << flush;
      }
      if ((LogStatus_object.Mode & DSP::e::LogState::file) == DSP::e::LogState::file)
      { // file & file_append
        LogStatus_object.WriteMessage2LOGfile(MessageText);
      }

      // ******************************************* //
      if (((LogStatus_object.Mode & DSP::e::LogState::console) == DSP::e::LogState::console) && (pause_after_message == true))
      { //Wait for ENTER only if console mode is ON
        pause_after_message = false;
        printf("Press ENTER");
        getchar();
      }
    }

    string logbuf::GetErrorMessage(const string &source, const string &message)
    {
      string text_buffer;
      if (message.length() == 0)
	if (source.length() == 0) {
          text_buffer = "";
	}
        else {
          text_buffer = "Error: " + source + '\n';
        }
      else
        text_buffer = "Error(" + source + "): " + message + '\n';
      return text_buffer;
    }

    // Information message generation
    void logbuf::InfoMessage(const string &source, const string &message)
    {
      string MessageText;

      if (LogStatus_object.Mode == DSP::e::LogState::off)
        return;
      //if (source == NULL)
      //  return;
      if ((LogStatus_object.Mode & DSP::e::LogState::errors_only) == DSP::e::LogState::errors_only)
        return;

      if ((LogStatus_object.Mode & DSP::e::LogState::user_function) == DSP::e::LogState::user_function)
      { // console
        if (LogStatus_object.function_ptr != NULL)
          if ((*LogStatus_object.function_ptr)(source, message, false) == true)
            return;
      }

      // ******************************************* //
      MessageText = GetInfoMessage(source, message);
      // ******************************************* //

      if ((LogStatus_object.Mode & DSP::e::LogState::console) == DSP::e::LogState::console)
      { // console
        //printf(MessageText.c_str());
        cout << MessageText << flush;
      }
      if ((LogStatus_object.Mode & DSP::e::LogState::file) == DSP::e::LogState::file)
      { // file & file_append
        LogStatus_object.WriteMessage2LOGfile(MessageText);
      }

      // ******************************************* //
      if (((LogStatus_object.Mode & DSP::e::LogState::console) == DSP::e::LogState::console) && (pause_after_message == true))
      { //Wait for ENTER only if console mode is ON
        pause_after_message = false;
        printf("Press ENTER");
        getchar();
      }
    }

    string logbuf::GetInfoMessage(const string &source, const string &message)
    {
      string text_buffer;
      if (message.length() == 0)
        if (source.length() == 0)
          text_buffer = "\n";
        else
          text_buffer = source + '\n';
      else
        if (source.length() == 0)
          text_buffer = "Info: " + message + "\n";
        else
          text_buffer = "Info(" + source + "): " + message + "\n";
      return text_buffer;
    }


    // Error or Information message generation
    void logbuf::Message(bool IsError, const string &source, const string &message)
    {
      if (IsError == false)
        InfoMessage(source, message);
      else
        ErrorMessage(source, message);
    }

}


DSP::logstream& operator<< (DSP::logstream& os, const DSP::e::LogMode& log_mode)
{
//  static_cast<Log *>(os.rdbuf())->priority_ = (int)log_priority;
  switch (log_mode) {
    case DSP::e::LogMode::first:
    case DSP::e::LogMode::second:
      os.log_buf.get()->msg_part = log_mode;
      break;
    case DSP::e::LogMode::pause:
      os.log_buf.get()->pause_after_message = true;
      break;
    case DSP::e::LogMode::pause_off:
      os.log_buf.get()->pause_after_message = false;
      break;
    case DSP::e::LogMode::Error:
      os.log_buf.get()->pause_after_message = true;
      os.log_buf.get()->mode = log_mode;
      break;
    case DSP::e::LogMode::Info:
      os.log_buf.get()->pause_after_message = false;
      os.log_buf.get()->mode = log_mode;
      break;
    default:
      assert(!"Unsupported log_mode");
      break;
  }
  return os;
}

