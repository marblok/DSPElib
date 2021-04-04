/*! \file DSP_logstream.h
 * logging and console output support header file
 *
 * \author Marek Blok
 */

#ifndef DSP_TEESTREAM_H
#define DSP_TEESTREAM_H

  #include <ostream>
  #include <memory>
  #include <mutex>

  #include <DSP_types.h>

  namespace DSP
  {
    //! Pointer to the Message callback function
    /*! bool func(const string &source, const string &message, const bool IsError)
     *  used in DSP::logstream::Message, DSP::logstream::ErrorMessage, DSP::logstream::InfoMessage
     *
     *  If function returns true all other message logging
     *  actions will be abandoned. Message will be treated
     *  as local addressed only for callback function.
     */
    typedef bool (*Message_callback_ptr)(const string &, const string &, const bool);

    class logstream;

      enum class LogMode {
         first,  // Main part of message (default)
         second, // Second part of message
         pause, // Force pause after message in Info mode
         pause_off, // Dissable pause after message in Error mode
         Info,  // InfoMessage mode (default)
         Error  // ErrorMessage mode
      };

      logstream& operator<< (logstream& os, const LogMode& log_mode);

      class logbuf;

      typedef std::shared_ptr<logbuf> logbuf_ptr;

      //! LOG actions state enumerations
      /*! Several options may be used together
       */
      enum E_LS_Mode
      {
          LS_off=0,
          LS_console = 1,
          LS_file    = LS_console << 1,
          LS_append  = LS_file << 1, //! only valid with LS_file
          LS_errors_only = LS_append << 1,
          LS_user_function = LS_errors_only << 1,

          LS_file_append= (LS_file | LS_append)
      };
      inline E_LS_Mode operator|(E_LS_Mode __a, E_LS_Mode __b)
      { return E_LS_Mode(static_cast<int>(__a) | static_cast<int>(__b)); }

      class logstream : public std::ostream // std::basic_ostream
      {
      public:
        // Construct an ostream which forwards output to DSP::InfoMessage and DSP::ErrorMessage.
        logstream(void);
        ~logstream(void);

        friend logstream& operator<< (logstream& os, const LogMode& log_mode);

      public:
        //! Sets current LOG actions state
        void SetLogState(E_LS_Mode Mode);
        //! Returns current LOG actions state
        E_LS_Mode GetLogState(void);
        //! Sets/changes current LOG file name
        /*!
         * \warning this function does not change LOG state to LS_file
         *  it must be done manually with SetLogState
         *
         * \warning File is created when first LOG message is issued
         *  with ErrorMessage, InfoMessage or Message.
         *  !!! WriteMessage2File function must be used
         *
         * If file is not opened, the stored file_name is simply changed.
         *
         * If LOG file is open, it is closed and file_name is changed.
         * New file will be created when next LOG message will be issued.
         */
        void SetLogFileName(const string &file_name);
        //! Sets/changes current user LOG Message processing function
        void SetLogFunctionPtr(Message_callback_ptr function_ptr);

        //! Returns number of Errors registered by ErrorMessage function for given DSP::logstream object
        /*! If Reset is true function zeros the counter
         */
        long int NoOfErrors(bool Reset=true);

      private:
        void init_logstream();

        logbuf_ptr log_buf;
      };

    //  template <class charT, class Traits>
    //  inline basic_ostream<charT,Traits>& operator<< (basic_ostream<charT,Traits>& os, const DSP::logMode& log_mode) {
      inline ostream& operator<< (ostream& os, const LogMode& log_mode) {
         logstream *p;
         try {
            p = dynamic_cast<logstream*>(&os);
        }
        catch (const std::bad_cast &)
        {
           return os;
        }

        *p << log_mode;
        return os;
      }


      //! "globalny" log stream
      extern logstream log;

    #ifndef __DEBUG__
       // Wyłączenie komunikatów mtee_dbg jeżeli nie zdefiniowano flagi __DEBUG__

       // zapewnia poprawność składni ale jednocześnie deaktywuje przetwarzanie wszystkich strumieni dla mtee_dbg
       #define log_dbg \
         if (false) log

    #else
      // użycie w trybie __DEBUG__

       #define log_dbg log //! output stream active only in debug mode

    #endif // __DEBUG__



}

#endif // DSP_TEESTREAM_H
