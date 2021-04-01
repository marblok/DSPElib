//---------------------------------------------------------------------------
/*! \file DSP::Clock_ptr.h
 * This is Socket Input/Output DSP module header file.
 *
 * Module blocks providing input from
 * and output to sockets through winsock2.
 *
 * \author Marek Blok
 */
#ifndef DSP_SOCKET_H
#define DSP_SOCKET_H

//#ifdef WIN32
#if defined(WIN32) || defined(WIN64)
  //! \warning required before #include <windows.h> for winsock2 support
  #ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
  #endif

  //#include <windows.h>
#if (_WIN32_WINNT < 0x0501)
  // see http://msdn.microsoft.com/en-us/library/aa383745(VS.85).aspx
  //#error wrong _WIN32_WINNT or WINVER
  #define WINVER 0x501
  #define _WIN32_WINNT 0x501
#endif
  #include <winsock2.h>
  #include <ws2tcpip.h>
  //#include "wspiapi_x.h" // support for freeaddrinfo on Win2000
  //#include <iphlpapi.h>



//  #include <windef.h>
#else
  #error DSP_socket.cpp has been designed to be used with WIN32
#endif

#include <DSP_IO.h>


//---------------------------------------------------------------------------
#include <DSP_setup.h>
//---------------------------------------------------------------------------
#include <DSP_modules.h>
#include <DSP_types.h>
//---------------------------------------------------------------------------
//! \todo detect maximum allowed socket buffer size
#define DSP_socket_buffer_size 1024

enum DSPe_SocketInfoDataType{
  DSP_SID_none = 0x0000,
  DSP_SID_Fp = 0x0001,
  DSP_SID_Offset = 0x0002,
  DSP_SID_UserData = 0x0003,
  DSP_SID_end = 0xffff // no more info data will be send
};

enum DSPe_SocketStatus{
  DSP_socket_none = 0,
  DSP_socket_connected = 1,
  DSP_socket_unconnected_mask = (0xffffff ^ DSP_socket_connected), //xor
  DSP_socket_timeout = 2,
  DSP_socket_timeout_mask = (0xffffff ^ DSP_socket_timeout), //xor
  DSP_socket_server = 4,
  DSP_socket_client_mask = (0xffffff ^ DSP_socket_server), //xor
  DSP_socket_listen_active = 8, // listen socket active
  DSP_socket_listen_active_mask = (0xffffff ^ DSP_socket_listen_active), //xor
  DSP_socket_error = 16,
  DSP_socket_closed = 32
};
DSPe_SocketStatus& operator|= (DSPe_SocketStatus& left,
                               const DSPe_SocketStatus& right);
DSPe_SocketStatus& operator&= (DSPe_SocketStatus& left,
                               const DSPe_SocketStatus& right);

// ***************************************************** //
// ***************************************************** //
//! Basic sockets support
/*!
 * \todo implement non-blocking mechanisms with WSAAsyncSelect or WSAEventSelect functions.
 * \todo implement server port selection algorithm
 * \todo_later implement use of several server instances
 *
 * \warning supports only WINVER >= 501
 */
class DSP_socket
{
  private:
    static bool winsock_initialized;
    static int NoOfSocketObjects;
    static const string DEFAULT_PORT;
    static WSADATA wsaData;

    int iResult;
    struct addrinfo *result;
    struct addrinfo *ptr;
    struct addrinfo  hints;

    //! true is listening socket is ready
    static bool listen_ready;
    static SOCKET ListenSocket;
    //! length of the server_objects_list
    static int no_of_server_objects;
    //! table of DSP_socket created in server mode
    static DSP_socket **server_objects_list;

    string extract_hostname(const string& address_with_port);
    string extract_port(const string& address_with_port, const string& default_port);

  protected:
    //! ID number for the current server object
    /*! This number is used to identify DSP_socket
     *  object to which the given incoming connection
     *  is addressed.
     */
    DWORD ServerObjectID;
    //! true if server or client socket is ready

    bool works_as_server;
    bool socket_ready;
    //! used for server or client connections
    SOCKET ConnectSocket;
    //! used for client connections
    //SOCKET ClientSocket;

    //! stores current socket state
    /*! \note derived class should update this state variable
     */
    DSPe_SocketStatus current_socket_state;

    bool Init_socket(void);

    bool InitClient(const string &address_with_port);
    bool InitServer_ListenSocket(const string &address_with_port);
    bool InitServer(void); //DWORD ServerObjectID_in);

    //! Attempts to accepts single connections if there is any in the listen queue
    /*! Returns true on success.
     *  \todo (note) The DSP_socket object which will benefit (get connected)
     *      depends on connection ID send through accepted connection.
     *  \warning works only for server objects
     */
    static bool TryAcceptConnection(void);
    //! Attempts to establish connection with the server for the current DSP_socket object
    /*! Returns true on success.
     *   - ServerObjectID == 0x00000000 - connect with any server object
     *   - ServerObjectID != 0x00000000 - connect only with server object which has given ID
     *   .
     *  \warning works only for client objects
     */
    bool TryConnect(DWORD ServerObjectID);
  public:
    //! \note port should be includes in address
    /*!
     * @param address with optional port number after colon 
     * @param run_as_client
     * @param ServerObjectID_in - if run_as_client == false it is current object ID else it is peer object ID
     *
     * \note ServerObjectID_in == 0x00000000 means : accept all IDs
     */
    DSP_socket(const string &address_with_port, bool run_as_client, DWORD ServerObjectID_in);
    ~DSP_socket(void);

    //! Waits until connection with current object is made
    bool WaitForConnection(bool stop_on_fail = false);

    DSPe_SocketStatus GetSocketStatus(void);
};

//! Socket multichannel data input block
/*! Inputs and Outputs names:
 *   - Output:
 *    -# "out" - real, complex or multiple-components
 *    -# "out.re" - first channel (real component)\n
 *       "out.im" - second channel (imag component)
 *    -# "out1", "out2", ... - i-th channel input
 *   - Input: none
 */
class DSPu_SOCKETinput : public DSP::File, public DSP_socket, public DSP::Source
{
  private:
    DSP::e::SampleType SampleType;

    //! size of the output buffer in DSP::Float * NoOfOutputs
    unsigned int BufferSize;
    DSP::Float *Buffer;
    unsigned int BufferIndex;

    unsigned int BytesRead;
    unsigned int SamplingRate;

    //! in bits (all channels together)
    unsigned int InputSampleSize;
    //! in bytes size of the buffer (RawBuffer) used in socket access
    unsigned int inbuffer_size;
    uint8_t *RawBuffer;

    //! number of bytes read in previous socket access
    /*! \warning is initialized with DSP_FILE_READING_NOT_STARTED
     */
    unsigned int LastBytesRead_counter;

    //! stores parent clock in case we must stall it for some time
    DSP::Clock *my_clock;

    void Init(DSP::Clock_ptr ParentClock,
              unsigned int OutputsNo = 1, //just one channel
              DSP::e::SampleType sample_type = DSP::e::SampleType::ST_float);

    static bool OutputExecute(OUTPUT_EXECUTE_ARGS);

    //! true if socket info data has been read
    bool SocketInfoData_received;
    //! sampling rate - received from peer
    long Fp;
    //! number of cycles to skip before first data - received from peer
    long Offset;
    //! reads connection data sent from the DSPu_SOCKEToutput
    /*! \note this must be done right after the connection is established
     *   and before any data are received.
     */
    bool ReadConnectionData(void);

    bool SetSkip(long long Offset_in);

  public:
    //! \note address may include port number after colon
    DSPu_SOCKETinput(DSP::Clock_ptr ParentClock,
                  const string &address_with_port, bool run_as_client,
                  DWORD ServerObjectID,
                  unsigned int NoOfChannels=1,
                  DSP::e::SampleType sample_type=DSP::e::SampleType::ST_float);
    ~DSPu_SOCKETinput(void);

    //! returns number of bytes read during last socket access
    /*!\note return zero if connections has been closed 
     * \warning returns DSP_FILE_READING_NOT_STARTED if reading
     *   not started already
     */
    unsigned int GetBytesRead(void);
    //! returns sampling rate of audio sample
    long int GetSamplingRate(void);

    //! Returns raw sample size in bytes corresponding to given SampleType
    /*! \note For SampleType = DSP::e::SampleType::ST_none returns internal raw sample size
     *   used in DSPu_FILEinput.
     *
     *  \warning Sample size is given in bits and encloses all channels
     */
    unsigned int GetSampleSize(DSP::e::SampleType SampleType = DSP::e::SampleType::ST_none);

    //! Returns raw buffer size in bytes needed for NoOfSamples samples.
    /*! If NoOfSamples == 0 return allocated internal raw buffer size.
     */
    unsigned int GetRawBufferSize(unsigned int NoOfSamples = 0);
    //! Returns DSP::Float buffer size needed for SizeInSamples samples.
    /*! If SizeInSamples == 0 return allocated internal DSP::Float buffer size.
     *
     *  \note Returned value is NoOfSamples * NoOfChannels.
     */
    unsigned int GetFltBufferSize(unsigned int NoOfSamples = 0);

    //! Reads segment for file and stores it in the buffer
    /*! Returns number of read bytes.
     */
    unsigned int ReadSegmentToBuffer(
      //  //! buffer size in samples
      //  unsigned int buffer_size,
      //  //! Raw buffer which will be used internally by the function
      //  /*! \note raw_buffer_size == buffer_size * sample_size / 8.
      //   *  \note Raw sample size can be determined with
      //   *  DSPu_FILEinput::GetSampleSize function
      //   *
      //   * \warning this buffer must be allocated and deleted by the user.
      //   */
      //  char        *raw_buffer,
       //! Buffer where read data will be stored in DSP::Float format
       /*! \note size == buffer_size * no_of_channels
        * \warning this buffer must be allocated and deleted by the user.
        */
       DSP::Float_vector &flt_buffer
       );
};

//! Creates object for sending signals through socket
/*!
 *  InputsNo - number of inputs (one channel per input)
 *
 * Inputs and Outputs names:
 *   - Output: none
 *   - Input:
 *    -# "in" - real or complex
 *    -# "in.re" - first channel (real component)\n
 *       "in.im" - second channel (imag component if exists)
 *    -# "in1", "in2" - i-th channel input
 */
class DSPu_SOCKEToutput : public DSP_socket, public DSP::Block
{
  private:
    unsigned int BufferSize;
    DSP::Float *Buffer;
    unsigned int BufferIndex;

    //! Type of samples send into socket
    DSP::e::SampleType SampleType;
    //! output sample size in bits (all channels)
    unsigned int OutputSampleSize;
    //! size of the buffers used for socket
    DWORD outbuffer_size;
    uint8_t *RawBuffer;


    //! To be used in constructor
    bool Init(void);
    //! Prepares buffers for playing and sends it to the audio device
    /*! saturation logic should be implemented */
    void FlushBuffer(void);

    void Init(unsigned int InputsNo=1, //just one channel
              DSP::e::SampleType sample_type=DSP::e::SampleType::ST_float);

    static void InputExecute(INPUT_EXECUTE_ARGS);

    //! true if socket info data has been sent
    bool SocketInfoData_sent;
    //! Sends connection data to the DSPu_SOCKETinput
    /*! \note this must be done right after the connection is established
     *   and before any data are sent.
     */
    bool SendConnectionData(void);

  public:
    //! \note address should maycontain port number after colon
    /*!
     * @param address_with_port
     * @param run_as_client
     * @param ServerObjectID - if run_as_client == false it is current object ID else it is peer object ID
     * @param NoOfChannels
     * @param sample_type
     */
    DSPu_SOCKEToutput(
        const string & address_with_port, bool run_as_client,
        DWORD ServerObjectID,
        unsigned int NoOfChannels=1,
        DSP::e::SampleType sample_type=DSP::e::SampleType::ST_float);
    ~DSPu_SOCKEToutput(void);
};

#endif // DSP_SOCKET_H

