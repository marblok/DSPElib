/*! \file DSP_sockets.cpp
 * This is Socket Input/Output DSP module file.
 *
 * Module blocks providing input from
 * and output to sockets through winsock2.
 *
 * \author Marek Blok
 */

#include <DSP_clocks.h>
#include <DSP_sockets.h>

// ***************************************************** //
// ***************************************************** //
DSP::e::SocketStatus& DSP::e::operator|= (DSP::e::SocketStatus& left,
                               const DSP::e::SocketStatus& right)
{
  left = DSP::e::SocketStatus(static_cast<std::underlying_type<DSP::e::SocketStatus>::type>(left)
                            | static_cast<std::underlying_type<DSP::e::SocketStatus>::type>(right));
  return left;
}
DSP::e::SocketStatus& DSP::e::operator&= (DSP::e::SocketStatus& left,
                               const DSP::e::SocketStatus& right)
{
  left = DSP::e::SocketStatus(static_cast<std::underlying_type<DSP::e::SocketStatus>::type>(left)
                            & static_cast<std::underlying_type<DSP::e::SocketStatus>::type>(right));
  return left;
}

// ***************************************************** //
// ***************************************************** //
// Basic sockets support
bool DSP::Socket::winsock_initialized = false;
int  DSP::Socket::NoOfSocketObjects = 0;
const string DSP::Socket::DEFAULT_PORT = "27027";
#ifndef  __NO_WINSOCK__
  WSADATA DSP::Socket::wsaData;
#endif

bool DSP::Socket::listen_ready = false;
#ifndef __NO_WINSOCK__
  SOCKET DSP::Socket::ListenSocket = INVALID_SOCKET;
#else
  SOCKET DSP::Socket::ListenSocket = -1;
#endif
int DSP::Socket::no_of_server_objects = 0;
std::vector<DSP::Socket *> DSP::Socket::server_objects_list;

DSP::Socket::Socket(const string &address_with_port, bool run_as_client, uint32_t ServerObjectID_in)
{
  result = NULL;
  ptr = NULL;

  ServerObjectID = ServerObjectID_in; //0x00000000; <== any
  socket_ready = false;
  current_socket_state = DSP::e::SocketStatus::none;

  if (winsock_initialized == false)
  {
    winsock_initialized = Init_socket();
  }
  NoOfSocketObjects++;

#ifndef __NO_WINSOCK__
  ConnectSocket = INVALID_SOCKET;
#else
  ConnectSocket = -1;
#endif

  if (run_as_client == true)
  {
    works_as_server = false;
    InitClient(address_with_port);
    /*
    while (TryConnect(0x00000000) == false)
    {
      DSP::f::Sleep(10);
    }
    */
  }
  else
  {
    works_as_server = true;
    InitServer_ListenSocket(address_with_port);
    InitServer();
    /*
    while (TryAcceptConnection() == false)
    {
      DSP::f::Sleep(10);
    }
    */
  }
}

bool DSP::Socket::is_socket_valid() {
  return is_socket_valid(ListenSocket);
}

bool DSP::Socket::is_socket_valid(const SOCKET &socket_to_check) {
#ifndef __NO_WINSOCK__
  return socket_to_check != INVALID_SOCKET;
#else
  return socket_to_check >= 0;
#endif
}

bool DSP::Socket::is_socket_error(const int &iResult) {
#ifndef __NO_WINSOCK__
  return iResult == SOCKET_ERROR;
#else
  return iResult < 0;
#endif
}

int DSP::Socket::GetLastError() {
#ifndef __NO_WINSOCK__
  return WSAGetLastError();
#else
  return errno;
#endif
}

bool DSP::Socket::InitServer_ListenSocket(const string & address_with_port)
{
  int res;

  if (winsock_initialized == false)
    return false;

  if (listen_ready == true)
  {
    DSP::log << "DSP::Socket::InitServer_ListenSocket" << DSP::e::LogMode::second << "already initialized" << endl;
    return false;
  }

  if (listen_ready == false)
  {
    // socket object initialization code
    // Resolve the local address and port to be used by the server
    memset( &hints, 0x00, sizeof(hints) );
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;
    // Resolve the server address and port

    string hostname = extract_hostname(address_with_port);
    string port = extract_port(address_with_port, DEFAULT_PORT);
    iResult = getaddrinfo(hostname.c_str(), port.c_str(), &hints, &result);
    if ( iResult != 0 )
    {
      //hints.ai_flags |= AI_NUMERICHOST;
      //iResult = getaddrinfo(address, DEFAULT_PORT, &hints, &result);
      //if ( iResult != 0 )
      //{
        DSP::log << DSP::e::LogMode::Error << "DSP::Socket::InitServer" << DSP::e::LogMode::second << "getaddrinfo failed" << endl;
        listen_ready = false;
        return false;
      //}
    }
    listen_ready = true;
  }

  if (listen_ready == true)
  {
    // Create a SOCKET for the server to listen for client connections
    ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (is_socket_valid(ListenSocket) == false)
    {
      res = DSP::Socket::GetLastError();
      DSP::log << DSP::e::LogMode::Error << "DSP::Socket::InitServer" << DSP::e::LogMode::second << "Error at socket(): " << res << endl;
      freeaddrinfo(result);
      result = NULL;
      listen_ready = false;
      return false;
    }

    // set blocking or nonblocking mode
    // set nonblocking mode: on = 1
    unsigned long on; on = 1;
    #ifndef  __NO_WINSOCK__
      iResult = ioctlsocket(ListenSocket, FIONBIO, &on);
    #else
      iResult = ioctl(ListenSocket, FIONBIO, &on);
    #endif
    if (DSP::Socket::is_socket_error(iResult))
    {
      DSP::log << DSP::e::LogMode::Error << "DSP::Socket::InitServer" << DSP::e::LogMode::second << "ioctlsocket failed to set non-blocking mode" << endl;
    }

    // Setup the TCP listening socket
    iResult = ::bind( ListenSocket,
        result->ai_addr, (int)result->ai_addrlen);
    if (DSP::Socket::is_socket_error(iResult))
    {
      DSP::log << DSP::e::LogMode::Error << "DSP::Socket::InitServer" << DSP::e::LogMode::second << "bind failed" << endl;
      //printf("bind failed: %d\n", WSAGetLastError());
      freeaddrinfo(result);
      result = NULL;
      DSP::Socket::close_socket();
      listen_ready = false;
      return false;
    }

    // support multiple outgoing connections
    if ( DSP::Socket::is_socket_error(listen( ListenSocket, SOMAXCONN )) )
    {
      DSP::log << DSP::e::LogMode::Error << "DSP::Socket::InitServer" << DSP::e::LogMode::second << "listen failed" << endl;
      //printf("listen failed: %d\n", WSAGetLastError());
      freeaddrinfo(result);
      result = NULL;
      DSP::Socket::close_socket();
      listen_ready = false;
      return false;
    }
  }

  current_socket_state |= DSP::e::SocketStatus::listen_active;
  return true;
}

bool DSP::Socket::InitServer(void) //uint32_t ServerObjectID_in)
{
  socket_ready = false;

  // 1. add this object to the server_objects_list
  server_objects_list.resize(no_of_server_objects+1);
  server_objects_list[no_of_server_objects] = this;
  no_of_server_objects++;

  // 2. update socket ID and other socket data
  //ServerObjectID = ServerObjectID_in;

  current_socket_state |= DSP::e::SocketStatus::server;

  return socket_ready;
}

// Accept first incoming connections
bool DSP::Socket::TryAcceptConnection(void)
{
  fd_set readfds;
  struct timeval timeout;
  int ind, res;
  unsigned long in_counter;
  uint32_t temp_ServerObjectID;
  SOCKET temp_socket;
  string text;

  if (DSP::Socket::is_socket_valid(ListenSocket) == false)
    return false;

  if (no_of_server_objects <= 0)
    return false;

  //! \bug support for asynchronous connections and accept connections during processing loop
  timeout.tv_sec = 0; timeout.tv_usec = 0;
  FD_ZERO(&readfds);
  FD_SET(ListenSocket, &readfds);
  res = select(int(ListenSocket)+1, &readfds, NULL, NULL, &timeout);
  if (res == 0)
  { //! timeout : no connections awaits for acceptance
    //current_socket_state |= DSP::e::SocketStatus::timeout; // no object
    return false;
  }
  //current_socket_state &= DSP::e::SocketStatus::timeout_mask; // no object
  if (DSP::Socket::is_socket_error(res) == true)
  {
    DSP::log << "DSP::Socket::TryAcceptConnection" << DSP::e::LogMode::second << "SOCKET_ERROR" << endl;
    DSP::Socket::close_socket();
    // current_socket_state |= DSP::e::SocketStatus::error; no object
    return false;
  }

  // Accept a client socket
  temp_socket = accept(ListenSocket, NULL, NULL);
  if (DSP::Socket::is_socket_valid(temp_socket) == false)
  {
    DSP::log << DSP::e::LogMode::Error << "DSP::Socket::TryAcceptConnection" << DSP::e::LogMode::second << "accept failed" << endl;
    //printf("accept failed: %d\n", WSAGetLastError());
    DSP::Socket::close_socket();
    listen_ready = false;
    //current_socket_state &= DSP::e::SocketStatus::listen_active_mask; // no object
    return false;
  }

  // read ServerObjectID from accepted socket
  timeout.tv_sec = 1; timeout.tv_usec = 0;
  FD_ZERO(&readfds);
  FD_SET(temp_socket, &readfds);
  res = select(int(temp_socket)+1, &readfds, NULL, NULL, &timeout);
  if (res == 0)
  { // timeout
    /*! \bug decrease timeout
     *  timeout.tv_sec = 0; timeout.tv_usec = 500;
     *  or even
     *  timeout.tv_sec = 0; timeout.tv_usec = 0;
     *
     *  But on time out do not close connection.
     *  Return to processing loop
     *  and try to read connection data next time.
     *  Close socket only when peer closes it.
     *  ?!? this might block other connections ?!?
     */
    DSP::Socket::close_socket(temp_socket);
    DSP::log << "DSP::Socket::TryAcceptConnection" << DSP::e::LogMode::second << "socket ID data has not been received (timeout - connection closed)" << endl;
    return false;
  }
  if (DSP::Socket::is_socket_error( res ) == true)
  {
    DSP::log << "DSP::Socket::TryAcceptConnection" << DSP::e::LogMode::second << "error reading socket ID data" << endl;
    // current_socket_state |= DSP::e::SocketStatus::error; // no object
    return false;
  }

  #ifndef  __NO_WINSOCK__
    res = ioctlsocket(temp_socket, FIONREAD, &in_counter);
  #else
    res = ioctl(temp_socket, FIONREAD, &in_counter);
  #endif
  if (in_counter == 0)
  {
    // connection has been closed
    DSP::log << "DSP::Socket::TryAcceptConnection" << DSP::e::LogMode::second << "connection has been closed" << endl;
    return false;
  }
  if (in_counter < sizeof(uint32_t))
  {
    DSP::log << "DSP::Socket::TryAcceptConnection" << DSP::e::LogMode::second << "error reading socket ID data (not enough data)" << endl;
    // current_socket_state |= DSP::e::SocketStatus::error; // no object
    return false;
  }
  in_counter = recv(temp_socket, (char *)(&temp_ServerObjectID), sizeof(uint32_t), 0);

  DSP::log << "DSP::Socket::TryAcceptConnection" << DSP::e::LogMode::second << "ServerObjectID = " << (int)temp_ServerObjectID << endl;

  // update ConnectSocket in according socket object
  for (ind = 0; ind < no_of_server_objects; ind++)
  {
    if (server_objects_list[ind] != NULL)
    {
      if (   (server_objects_list[ind]->ServerObjectID == 0x00000000)
          || (server_objects_list[ind]->ServerObjectID == temp_ServerObjectID))
      {
        if (DSP::Socket::is_socket_valid(server_objects_list[ind]->ConnectSocket) == false)
        {
          DSP::log << "DSP::Socket::TryAcceptConnection" << DSP::e::LogMode::second
            << "server_objects_list[" << ind
            << "]ServerObjectID = " << (unsigned int)server_objects_list[ind]->ServerObjectID << endl;

          server_objects_list[ind]->ConnectSocket = temp_socket;
          server_objects_list[ind]->socket_ready = true;

          server_objects_list[ind]->current_socket_state |= DSP::e::SocketStatus::connected;
          return true;
        }
      }
    }
  }
  // current_socket_state |= DSP::e::SocketStatus::error; // no object
  DSP::log << DSP::e::LogMode::Error << "DSP::Socket::TryAcceptConnection" << DSP::e::LogMode::second << "Unexpected incoming connection" << endl;
  return false;
}

string DSP::Socket::extract_hostname(const string &address_with_port) {
  string hostname;
  size_t found = address_with_port.find_first_of(":");
  if (found != string::npos) {
    hostname = address_with_port.substr(0, found);
  }
  else {
    hostname = address_with_port;
  }
  return hostname;
}

string DSP::Socket::extract_port(const string& address_with_port, const string &default_port) {
  string port;
  size_t found = address_with_port.find_first_of(":");
  if (found != string::npos) {
    port = address_with_port.substr(found+1);
  }
  else {
    port = default_port;
  }
  return port;
}

void DSP::Socket::close_socket() {
  DSP::Socket::close_socket(ListenSocket);
}

void DSP::Socket::close_socket(SOCKET &socket_to_close) {
  #ifndef __NO_WINSOCK__
    closesocket(socket_to_close);
    socket_to_close = INVALID_SOCKET;
  #else
    close(socket_to_close);
    socket_to_close = -1;
  #endif
}

bool DSP::Socket::InitClient(const string & address_with_port)
{
  bool ready;

  if (winsock_initialized == false)
  {
    current_socket_state |= DSP::e::SocketStatus::error;
    return false;
  }

  if (socket_ready == true)
    return false;

  // socket object initialization code
  memset( &hints, 0x00, sizeof(hints) );
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP;
  // Resolve the server address and port

  ready = true;
  string hostname = extract_hostname(address_with_port);
  string port = extract_port(address_with_port, DEFAULT_PORT);
  iResult = getaddrinfo(hostname.c_str(), port.c_str(), &hints, &result);
  if ( iResult != 0 )
  {
    DSP::log << DSP::e::LogMode::Error << "DSP::Socket::InitClient" << DSP::e::LogMode::second << "getaddrinfo failed" << endl;
    current_socket_state |= DSP::e::SocketStatus::error;
    return false;
  }

  if (ready == true)
  {
    // Attempt to connect to the first address returned by
    // the call to getaddrinfo
    ptr=result;
    // Create a SOCKET for connecting to server
    ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
    if (DSP::Socket::is_socket_valid(ConnectSocket) == false)
    {
      DSP::log << DSP::e::LogMode::Error << "DSP::Socket::InitClient" << DSP::e::LogMode::second << "Error at socket()" << endl;
      //printf("Error at socket(): %ld\n", WSAGetLastError());
      freeaddrinfo(result);
      result = NULL;
      current_socket_state |= DSP::e::SocketStatus::error;
      current_socket_state &= DSP::e::SocketStatus::unconnected_mask;
      return false;
    }
    unsigned long on; on = 1;
    #ifndef  __NO_WINSOCK__
      iResult = ioctlsocket(ConnectSocket, FIONBIO, &on);
    #else
      iResult = ioctl(ConnectSocket, FIONBIO, &on);
    #endif
    if (DSP::Socket::is_socket_error(iResult) == true)
    {
      DSP::log << DSP::e::LogMode::Error << "DSP::Socket::InitClient" << DSP::e::LogMode::second << "ioctlsocket failed to set non-blocking mode" << endl;
      current_socket_state |= DSP::e::SocketStatus::error;
    }
  }

  current_socket_state &= DSP::e::SocketStatus::client_mask;
  return true;
}

bool DSP::Socket::TryConnect(uint32_t SerwerObjectID)
{
  unsigned long out_counter;
#ifdef __DEBUG__
  string text;
#endif
  int err;

  //http://msdn.microsoft.com/en-us/library/ms737625(VS.85).aspx

  //DSP::log << "sssocket");
  current_socket_state &= DSP::e::SocketStatus::unconnected_mask;
  // connect on nonblocking socket
  // Connect to server.
  iResult = connect( ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
  if (DSP::Socket::is_socket_error(iResult) == true)
  {
    err = DSP::Socket::GetLastError();

    switch (err)
    {
      case WSAEWOULDBLOCK: // EINPROGRESS
        // OK ==> wait for finalized connection
      case WSAEALREADY: // EALREADY
      case WSAEINVAL: // EINVAL
        // connection not ready yet
        current_socket_state &= DSP::e::SocketStatus::timeout_mask;
        DSP::f::Sleep(0);
        return false;
        break;

      case WSAEISCONN: // EISCONN
        // connection completed
        current_socket_state &= DSP::e::SocketStatus::timeout_mask;
        socket_ready = true;
        #ifdef __DEBUG__
          DSP::log << "DSP::Socket::TryConnect" << DSP::e::LogMode::second << "connection established (" << err << ")" << endl;
        #endif
        break;

      //case SOCKET_ERROR:
      default:
        // error or unexpected situation
        current_socket_state &= DSP::e::SocketStatus::timeout_mask;
        err = DSP::Socket::GetLastError();

        freeaddrinfo(result);
        result = NULL;
        close_socket(ConnectSocket);
        // b u g: must recreate socket before another connect attempt

        current_socket_state |= DSP::e::SocketStatus::closed;
        current_socket_state |= DSP::e::SocketStatus::error;
        current_socket_state &= DSP::e::SocketStatus::unconnected_mask;

        return false;
        break;
    }
  }
  socket_ready = true;

  struct timeval timeout;
  fd_set write_fs;
  timeout.tv_sec = 0;  timeout.tv_usec = 10;
  FD_ZERO(&write_fs); FD_SET(ConnectSocket, &write_fs);
  #ifdef __DEBUG__
    int res = 
  #endif
  select(int(ConnectSocket)+1, NULL, &write_fs, NULL, &timeout);
  #ifdef __DEBUG__
    if (res == 0)
    {
      DSP::log << DSP::e::LogMode::Error << "DSP::Socket::TryConnect" << DSP::e::LogMode::second << "connected but not ready to write (" << res << ")" << endl;
    }
    else
    {
      DSP::log << "DSP::Socket::TryConnect" << DSP::e::LogMode::second << "connected and ready to write (" << res << ")" << endl;
    }
  #endif

  current_socket_state |= DSP::e::SocketStatus::connected;

  // send expected server object ID
  out_counter = send(ConnectSocket, (char *)(&SerwerObjectID), sizeof(uint32_t), 0);
  if (out_counter < sizeof(uint32_t))
  {
    #ifdef __DEBUG__
      DSP::log << DSP::e::LogMode::Error << "DSP::Socket::TryConnect" << DSP::e::LogMode::second << "failed to send expected server object data" << endl;
    #endif
    current_socket_state |= DSP::e::SocketStatus::error;
    return false;
  }
  #ifdef __DEBUG__
    DSP::log << "DSP::Socket::TryConnect" << DSP::e::LogMode::second << "SerwerObjectID has been sent" << endl;
  #endif
  return socket_ready;
}

bool DSP::Socket::Init_socket(void)
{
#ifndef __NO_WINSOCK__
  // Initialize Winsock
  iResult = WSAStartup(MAKEWORD(2,2), &wsaData);

  if (iResult != 0)
  {
    #ifdef __DEBUG__
      DSP::log << DSP::e::LogMode::Error << "DSP::Socket::Init_socket" << DSP::e::LogMode::second << "WSAStartup failed" << endl;
    #endif
    current_socket_state |= DSP::e::SocketStatus::error;
    return false;
  }
#endif
  return true;
}

bool DSP::Socket::WaitForConnection(bool stop_on_fail)
{
  bool rs;

  rs = false;
  while (socket_ready == false)
  { // no socket ready
    if (works_as_server)
    {
      rs = TryAcceptConnection();
    }
    else
    { // Use proper destination ServerID
      rs = TryConnect(ServerObjectID);
    }

    if (stop_on_fail == true)
      break;

    DSP::f::Sleep(10);
  }
  return rs;
}

DSP::e::SocketStatus DSP::Socket::GetSocketStatus(void)
{
  if (listen_ready == true)
    current_socket_state |= DSP::e::SocketStatus::listen_active;
  else
    current_socket_state &= DSP::e::SocketStatus::listen_active_mask;

  return current_socket_state;
}

DSP::Socket::~Socket(void)
{
  current_socket_state = DSP::e::SocketStatus::none;

  if (result != NULL)
  {
    freeaddrinfo(result);
    result = NULL;
  }
  if (DSP::Socket::is_socket_valid(ConnectSocket) == true)
  {
    DSP::Socket::close_socket(ConnectSocket);
  }

  if (NoOfSocketObjects > 0)
  {
    NoOfSocketObjects--;
    if ((NoOfSocketObjects == 0) && (winsock_initialized == true))
    {
      // close socket
      if (DSP::Socket::is_socket_valid(ListenSocket) == true)
      {
        listen_ready = false;
        DSP::Socket::close_socket(ListenSocket);
      }

      #ifndef __NO_WINSOCK__
        WSACleanup();
      #endif
      winsock_initialized = false;
    }
  }

  if (NoOfSocketObjects == 0)
  {
    server_objects_list.clear();
  }
}

/* ***************************************** */
/* ***************************************** */
DSP::u::SocketInput::SocketInput(DSP::Clock_ptr ParentClock,
      const string &address_with_port, bool run_as_client,
      uint32_t ServerObjectID_in,
      unsigned int NoOfChannels,
      DSP::e::SampleType sample_type)
  : DSP::Socket(address_with_port, run_as_client, ServerObjectID_in), DSP::Source()
{
  Init(ParentClock, NoOfChannels, sample_type);

  OutputExecute_ptr = &OutputExecute;
}

/* Inputs and Outputs names:
 *   - Output: none
 *   - Input:
 *    -# "in" - real or complex
 *    -# "in.re" - first channel (real component)\n
 *       "in.im" - second channel (imag component if exists)
 *    -# "in1", "in2" - i-th channel input
 */
void DSP::u::SocketInput::Init(DSP::Clock_ptr ParentClock,
                            unsigned int OutputsNo, //just one channel
                            DSP::e::SampleType sample_type)
{
  string temp;

  Fp = -1;
  Offset = 0;
  SocketInfoData_received = false;

  SetNoOfOutputs(OutputsNo);
  SetName("SocketInput", false);

  if (OutputsNo == 1)
  {
    DefineOutput("out", 0);
    DefineOutput("out.re", 0);
  }
  if (OutputsNo == 2)
  {
    DefineOutput("out", 0, 1);
    DefineOutput("out.re", 0);
    DefineOutput("out.im", 1);
  }
  for (unsigned int ind=0; ind<NoOfOutputs; ind++)
  {
    temp = "out" + to_string(ind+1);
    DefineOutput(temp, ind);
  }

  RegisterOutputClock(ParentClock);
  my_clock = ParentClock;

  SampleType = sample_type;
  switch (SampleType)
  {
    case DSP::e::SampleType::ST_uchar:
      InputSampleSize = 8 * NoOfOutputs;
      break;
    case DSP::e::SampleType::ST_short:
      InputSampleSize = 16 * NoOfOutputs;
      break;
    case DSP::e::SampleType::ST_float:
      InputSampleSize = 32 * NoOfOutputs;
      break;
    default:
      SampleType = DSP::e::SampleType::ST_short;
      InputSampleSize = 16 * NoOfOutputs;
      break;
  }

  inbuffer_size = DSP::Socket_buffer_size / (InputSampleSize / 8);
  BufferSize = inbuffer_size;
  inbuffer_size *= (InputSampleSize / 8);

  RawBuffer.resize(inbuffer_size);
  Buffer.resize(NoOfOutputs * BufferSize);
  BufferIndex = BufferSize;

  LastBytesRead_counter = DSP_FILE_READING_NOT_STARTED;
}

DSP::u::SocketInput::~SocketInput(void)
{
  RawBuffer.clear();
  Buffer.clear();
}

bool DSP::u::SocketInput::SetSkip(long long Offset_in)
{
  UNUSED_ARGUMENT(Offset_in);

  #ifdef __DEBUG__
    DSP::log << "DSP::u::SocketInput::SetSkip" << DSP::e::LogMode::second << "Offset setting not yet supported" << endl;
  #endif
  return false;
}

unsigned long DSP::u::SocketInput::GetBytesRead(void)
{
  return LastBytesRead_counter;
}
//! \bug not implemented
unsigned long DSP::u::SocketInput::GetSamplingRate(void)
{
  return 0;
}

#define THIS ((DSP::u::SocketInput *)source)
bool DSP::u::SocketInput::OutputExecute(OUTPUT_EXECUTE_ARGS)
{
  UNUSED_DEBUG_ARGUMENT(clock);

  unsigned int ind, ind2;
  unsigned long in_counter;
  int res;
  uint8_t *in_temp;
  DSP::Float_ptr temp_buffer;
  //string text;

  // this is just a client socket
  if (THIS->socket_ready == false)
  { // no socket opened
    if (THIS->works_as_server == true)
    {
      //DSP::log << "DSP::u::SocketInput::OutputExecute", "THIS->TryAcceptConnection(1)");
      TryAcceptConnection();
      //DSP::log << "DSP::u::SocketInput::OutputExecute", "THIS->TryAcceptConnection(2)");
    }
    else
    { //Use proper destination ServerID
      //DSP::log << "DSP::u::SocketInput::OutputExecute", "THIS->TryConnect(1)");
      THIS->TryConnect(THIS->ServerObjectID);
      //DSP::log << "DSP::u::SocketInput::OutputExecute", "THIS->TryConnect(2)");
    }

    //DSP::log << "DSP::u::SocketInput::OutputExecute", "no socket opened");
    for (ind=0; ind < THIS->NoOfOutputs; ind++)
    {
      THIS->OutputBlocks[ind]->EXECUTE_PTR(
          THIS->OutputBlocks[ind],
          THIS->OutputBlocks_InputNo[ind],
          0.0, source);
    }
    //THIS->BufferIndex++;
    return true;
  }
  if (THIS->SocketInfoData_received == false)
  {
    DSP::log << "DSP::u::SocketInput::OutputExecute" << DSP::e::LogMode::second << "THIS->ReadConnectionData" << endl;
    THIS->SocketInfoData_received = THIS->ReadConnectionData();
  }

  if (THIS->Offset > 0)
  { // wait before reading data from socket
    THIS->Offset--;
    for (ind=0; ind < THIS->NoOfOutputs; ind++)
    {
      THIS->OutputBlocks[ind]->EXECUTE_PTR(
            THIS->OutputBlocks[ind], THIS->OutputBlocks_InputNo[ind], 0.0, source);
    }
    return true;
  }

  // if no input buffer is ready return false (system will later return here)
  if (THIS->BufferIndex == THIS->BufferSize)
  { //no input samples are available
    // so we must wait for some
    // ++++++++++++++++++++++++++++++++++++ //
    // check if socket received data
    fd_set readfds;
    struct timeval timeout;
    timeout.tv_sec = 0; timeout.tv_usec = 0;

    FD_ZERO(&readfds);
    FD_SET(THIS->ConnectSocket, &readfds);
    res = select(int(THIS->ConnectSocket)+1, &readfds, NULL, NULL, &timeout);
    if (res == 0)
    { //! timeout
      //DSP::log << "DSP::u::SocketInput::OutputExecute", "timeout");
      DSP::Clock::InputNeedsMoreTime[THIS->my_clock->MasterClockIndex] = true;
      THIS->current_socket_state |= DSP::e::SocketStatus::timeout;

      /*! \todo 2011.12.02 ??? allow user to turn off waiting for data
       *  - just fill in zeros to the buffer or give single zero sample ones
       */

      // ++++++++++++++++++++++++++++++++ //
      DSP::Clock::InputNeedsMoreTime[THIS->my_clock->MasterClockIndex] = false;
      // output zero insteaf of sample
      for (ind=0; ind < THIS->NoOfOutputs; ind++)
      {
        THIS->OutputBlocks[ind]->EXECUTE_PTR(
            THIS->OutputBlocks[ind], THIS->OutputBlocks_InputNo[ind],
            0.0, source);
      }
      return true;
      // ++++++++++++++++++++++++++++++++ //

      return false;
    }
    THIS->current_socket_state &= DSP::e::SocketStatus::timeout_mask;
    if (DSP::Socket::is_socket_error(res) == true)
    {
      DSP::log << "DSP::u::SocketInput::OutputExecute" << DSP::e::LogMode::second << "SOCKET_ERROR" << endl;
      //  close socket
      DSP::Socket::close_socket(THIS->ConnectSocket);

      THIS->socket_ready = false;
      // force repeated call to this function
      DSP::Clock::InputNeedsMoreTime[THIS->my_clock->MasterClockIndex] = true;
      THIS->LastBytesRead_counter = 0;
      THIS->current_socket_state |= DSP::e::SocketStatus::closed;
      THIS->current_socket_state |= DSP::e::SocketStatus::error;
      return false;
    }

    #ifndef  __NO_WINSOCK__
      res = ioctlsocket(THIS->ConnectSocket, FIONREAD, &in_counter);
    #else
      res = ioctl(THIS->ConnectSocket, FIONREAD, &in_counter);
    #endif

    if (in_counter == 0)
    {
      // connection has been closed
      DSP::log << "DSP::u::SocketInput::OutputExecute" << DSP::e::LogMode::second << "connection has been closed" << endl;
      // close socket
      close_socket(THIS->ConnectSocket);

      THIS->socket_ready = false;
      // force repeated call to this function
      DSP::Clock::InputNeedsMoreTime[THIS->my_clock->MasterClockIndex] = true;
      THIS->LastBytesRead_counter = 0;
      THIS->current_socket_state |= DSP::e::SocketStatus::closed;
      THIS->current_socket_state &= DSP::e::SocketStatus::unconnected_mask;
      return false;
    }
    if (in_counter < THIS->inbuffer_size)
    {
	  #ifdef __DEBUG__
        DSP::log << "DSP::u::SocketInput::OutputExecute" << DSP::e::LogMode::second << "more data expected" << endl;
      #endif // __DEBUG__
      DSP::Clock::InputNeedsMoreTime[THIS->my_clock->MasterClockIndex] = true;

      /*! \todo 2011.12.02 ??? allow user to turn off waiting for data
       *  - just fill in zeros to the buffer or give single zero sample ones
       */

      // ++++++++++++++++++++++++++++++++ //
      DSP::Clock::InputNeedsMoreTime[THIS->my_clock->MasterClockIndex] = false;
      // output zero insteaf of sample
      for (ind=0; ind < THIS->NoOfOutputs; ind++)
      {
        THIS->OutputBlocks[ind]->EXECUTE_PTR(
            THIS->OutputBlocks[ind], THIS->OutputBlocks_InputNo[ind],
            0.0, source);
      }
      return true;
      // ++++++++++++++++++++++++++++++++ //

      return false;
    }
    in_counter = recv(THIS->ConnectSocket, (char *)THIS->RawBuffer.data(), THIS->inbuffer_size, 0);
    THIS->LastBytesRead_counter = in_counter;

    in_temp = THIS->RawBuffer.data();
    temp_buffer = THIS->Buffer.data();
    for (ind = 0; ind < THIS->BufferSize; ind++)
    {
      for (ind2 = 0; ind2 < THIS->NoOfOutputs; ind2++)
      {
        if (ind >= in_counter / (THIS->InputSampleSize / 8))
        {
          *temp_buffer = 0.0;
          temp_buffer++;
        }
        else
        {
          switch(THIS->SampleType)
          {
            case DSP::e::SampleType::ST_short:
              *temp_buffer = *((short *)in_temp);
              *temp_buffer /= 0x8000;
              in_temp += sizeof(short);
              break;
            case DSP::e::SampleType::ST_float:
              *temp_buffer = *((DSP::Float_ptr)in_temp);
              in_temp += sizeof(DSP::Float);
              break;
            case DSP::e::SampleType::ST_uchar:
            default:
              *temp_buffer = *((uint8_t *)in_temp);
              *temp_buffer -= 0x80;
              *temp_buffer /= 0x80;
              in_temp += sizeof(uint8_t);
              break;
          }
          temp_buffer++;
        }
      }
    }
    THIS->BufferIndex = 0;
  }

  // output sample
  for (ind=0; ind < THIS->NoOfOutputs; ind++)
  {
    THIS->OutputBlocks[ind]->EXECUTE_PTR(
        THIS->OutputBlocks[ind], THIS->OutputBlocks_InputNo[ind],
        THIS->Buffer[THIS->NoOfOutputs * (THIS->BufferIndex) + ind], source);
  }
  THIS->BufferIndex++;

  return true; // samples have been generated
}
#undef THIS

bool DSP::u::SocketInput::ReadConnectionData(void)
{
  unsigned short buffer[1024];
  long expected_data_size;
  DSP::e::SocketInfoDataType data_type;
  int state;
  unsigned long in_counter, in_counter_recv;
  int res;

  fd_set readfds;
  struct timeval timeout;
  timeout.tv_sec = 0; timeout.tv_usec = 0;

  in_counter_recv = 0;

  data_type = DSP::e::SocketInfoDataType::none;
  state = 0; // wait for marker and version
  expected_data_size = 4;
  do
  {
    do
    {
      DSP::f::Sleep(0);

      FD_ZERO(&readfds);
      FD_SET(ConnectSocket, &readfds);
      res = select(int(ConnectSocket)+1, &readfds, NULL, NULL, &timeout);
      if (res == 0)
      {
        //DSP::log << "DSP::u::SocketInput::ReadConnectionData", "timeout");
        current_socket_state |= DSP::e::SocketStatus::timeout;
      }
    }
    while (res == 0); // timeout
    current_socket_state &= DSP::e::SocketStatus::timeout_mask;
    if (DSP::Socket::is_socket_error(res) == true)
    {
      #ifdef __DEBUG__
        DSP::log << DSP::e::LogMode::Error << "DSP::u::SocketInput::ReadConnectionData" << DSP::e::LogMode::second << "SOCKET_ERROR" << endl;
      #endif
      // close socket
      socket_ready = false;
      close_socket(ConnectSocket);
      current_socket_state |= DSP::e::SocketStatus::closed;
      current_socket_state |= DSP::e::SocketStatus::error;
      return false;
    }

    #ifndef  __NO_WINSOCK__
      res = ioctlsocket(ConnectSocket, FIONREAD, &in_counter);
    #else
      res = ioctl(ConnectSocket, FIONREAD, &in_counter);
    #endif
    if (in_counter == 0)
    {
      // connection has been closed
      #ifdef __DEBUG__
        DSP::log << "DSP::u::SocketInput::ReadConnectionData" << DSP::e::LogMode::second << "connection has been closed" << endl;
      #endif
      // close socket
      socket_ready = false;
      close_socket(ConnectSocket);
      current_socket_state |= DSP::e::SocketStatus::closed;
      current_socket_state &= DSP::e::SocketStatus::unconnected_mask;
      return false;
    }
    if ((long)in_counter < expected_data_size)
    {
      #ifdef __DEBUG__
      {
        DSP::log << "DSP::u::SocketInput::ReadConnectionData" << DSP::e::LogMode::second
          << "more data expected: in_counter = " << in_counter
          << "; expected_data_size = " << expected_data_size << endl;
      }
      #endif
      continue;
    }

    in_counter_recv += recv(ConnectSocket, (char *)buffer, expected_data_size, 0);
    #ifdef __DEBUG__
    {
      DSP::log << "DSP::u::SocketInput::ReadConnectionData" << DSP::e::LogMode::second
        << "in_counter_recv = " << in_counter_recv << "; state = " << state
        << "; expected_data_size = " << expected_data_size << endl;
    }
    #endif

    switch (state)
    {
      case 0: // waiting for marker
        if (buffer[0] != 0xffff)
        {
          #ifdef __DEBUG__
            DSP::log << DSP::e::LogMode::Error << "DSP::u::SocketInput::ReadConnectionData" << DSP::e::LogMode::second << "unexpected marker" << endl;
          #endif
          data_type = DSP::e::SocketInfoDataType::end;
        }
        //! \todo make use of buffer[1] <== version
        state = 1; // waiting for field type
        expected_data_size = 2;
        break;
      case 1: // waiting for field type
        data_type = (DSP::e::SocketInfoDataType)(buffer[0]);
        state = 2; // wait for field length
        expected_data_size = 2;
        break;

      case 2:
        //! \bug compare read data size with expected data size
        switch (data_type)
        {
          case DSP::e::SocketInfoDataType::Fp:
            #ifdef __DEBUG__
              DSP::log << "DSP::u::SocketInput::ReadConnectionData" << DSP::e::LogMode::second << "DSP::e::SocketInfoDataType::Fp" << endl;
            #endif
            state = 3; // waiting for Fp data
            expected_data_size = sizeof(long);
            break;

          case DSP::e::SocketInfoDataType::Offset:
            #ifdef __DEBUG__
              DSP::log << "DSP::u::SocketInput::ReadConnectionData" << DSP::e::LogMode::second << "DSP::e::SocketInfoDataType::Offset" << endl;
            #endif
            state = 3; // waiting for Offset data
            expected_data_size = sizeof(long);
            break;

          case DSP::e::SocketInfoDataType::UserData:
            #ifdef __DEBUG__
              DSP::log << "DSP::u::SocketInput::ReadConnectionData" << DSP::e::LogMode::second << "DSP::e::SocketInfoDataType::UserData" << endl;
            #endif
            state = 3; // waiting for user data
//            expected_data_size = *((long *)buffer);
            memcpy(&expected_data_size,buffer, sizeof(long));
            break;
          case DSP::e::SocketInfoDataType::end:
            #ifdef __DEBUG__
              DSP::log << "DSP::u::SocketInput::ReadConnectionData" << DSP::e::LogMode::second << "DSP::e::SocketInfoDataType::end" << endl;
            #endif
            // ignore: transmission finished
            break;
          default:
            state = 3; // waiting for unknown data
//            expected_data_size = *((long *)buffer);
            memcpy(&expected_data_size,buffer, sizeof(long));
            #ifdef __DEBUG__
              DSP::log << "DSP::u::SocketInput::ReadConnectionData" << DSP::e::LogMode::second << "unexpected field type" << endl;
            #endif
            current_socket_state |= DSP::e::SocketStatus::error;
            break;
        }
        break;
      case 3:
        //! \bug compare read data size with expected data size
        switch (data_type)
        {
          case DSP::e::SocketInfoDataType::Fp:
//            Fp = *((long *)buffer);
            memcpy(&Fp, buffer, sizeof(long));
            break;
          case DSP::e::SocketInfoDataType::Offset:
//            Offset = *((long *)buffer);
            memcpy(&Offset, buffer, sizeof(long));
            break;
          case DSP::e::SocketInfoDataType::UserData:
            //UserData = *((long *)buffer);
            break;
          default:
            #ifdef __DEBUG__
              DSP::log << DSP::e::LogMode::Error << "DSP::u::SocketInput::ReadConnectionData" << DSP::e::LogMode::second << "unexpected data type" << endl;
            #endif
            current_socket_state |= DSP::e::SocketStatus::error;
            break;
        }
        state = 1; // waiting for field type
        expected_data_size = 2;
        break;
      default:
        data_type = DSP::e::SocketInfoDataType::end;
        #ifdef __DEBUG__
          DSP::log << DSP::e::LogMode::Error << "DSP::u::SocketInput::ReadConnectionData" << DSP::e::LogMode::second << "unexpected state" << endl;
        #endif
        current_socket_state |= DSP::e::SocketStatus::error;
        break;
    }
  }
  while (data_type != DSP::e::SocketInfoDataType::end);

  #ifdef __DEBUG__
  {
    DSP::log << "DSP::u::SocketInput::ReadConnectionData" << DSP::e::LogMode::second
        << "in_counter_recv = " << in_counter_recv << endl;
  }
  #endif
  return true;
}
/* ***************************************** */
/* ***************************************** */
DSP::u::SocketOutput::SocketOutput(
      const string &address_with_port, bool run_as_client,
      uint32_t ServerObjectID_in,
      unsigned int NoOfChannels,
      DSP::e::SampleType sample_type)
  : DSP::Socket(address_with_port, run_as_client, ServerObjectID_in), DSP::Block()
{
  Init(NoOfChannels, sample_type);

  Execute_ptr = &InputExecute;
}

void DSP::u::SocketOutput::Init(unsigned int InputsNo,
                             DSP::e::SampleType sample_type)
{
  string temp;

  SocketInfoData_sent = false;

  SetNoOfInputs(InputsNo, true);
  SetName("SocketOutput", false);

  if (InputsNo == 1)
  {
    DefineInput("in", 0);
    DefineInput("in.re", 0);
  }
  if (InputsNo == 2)
  {
    DefineInput("in", 0, 1);
    DefineInput("in.re", 0);
    DefineInput("in.im", 1);
  }
  for (unsigned int ind=0; ind<NoOfInputs; ind++)
  {
    temp = "in" + to_string(ind+1);
    DefineInput(temp, ind);
  }

  ClockGroups.AddInputs2Group("input", 0, NoOfInputs-1);

  SampleType = sample_type;
  switch (SampleType)
  {
    case DSP::e::SampleType::ST_uchar:
      OutputSampleSize = 8 * NoOfInputs;
      break;
    case DSP::e::SampleType::ST_short:
      OutputSampleSize = 16 * NoOfInputs;
      break;
    case DSP::e::SampleType::ST_float:
      OutputSampleSize = 32 * NoOfInputs;
      break;
    default:
      SampleType = DSP::e::SampleType::ST_short;
      OutputSampleSize = 16 * NoOfInputs;
      break;
  }

  outbuffer_size = DSP::Socket_buffer_size / (OutputSampleSize / 8);
  BufferSize = outbuffer_size;
  outbuffer_size *= (OutputSampleSize / 8);

  RawBuffer.resize(outbuffer_size);
  Buffer.resize(NoOfInputs * BufferSize);

  BufferIndex = 0;
  NoOfInputsProcessed = 0;
}

DSP::u::SocketOutput::~SocketOutput(void)
{
  Buffer.clear();
  RawBuffer.clear();
}

void DSP::u::SocketOutput::FlushBuffer(void)
{
  uint8_t *temp8;
  short *temp16;
  DSP::Float_ptr Sample, temp_float;
  short Znak;
  uint32_t ind;

  // ************************************************** //
  // Send buffer to the socket
  Sample=Buffer.data();
  // ************************************************** //
  // Converts samples format to the one suitable for the audio device
  switch (SampleType)
  {
    case DSP::e::SampleType::ST_uchar:
      temp8=(uint8_t *)(RawBuffer.data());
      for (ind=0; ind<BufferSize * NoOfInputs; ind++)
      {
        if (*Sample < 0)
          Znak=-1;
        else
          Znak=1;

        *Sample*=127;
        if ((*Sample)*Znak > 127)
          *temp8=(unsigned char)(128+Znak*127);
        else
          *temp8=(unsigned char)(128+*Sample+Znak*0.5);

        Sample++;
        temp8++;
      }
      break;
    case DSP::e::SampleType::ST_short:
      temp16=(short *)(RawBuffer.data());
      for (ind=0; ind<BufferSize * NoOfInputs; ind++)
      {
        if (*Sample < 0)
          Znak=-1;
        else
          Znak=1;

        *Sample*=SHRT_MAX;
        if ((*Sample)*Znak > SHRT_MAX)
          *temp16=(short)(Znak*SHRT_MAX);
        else
          *temp16=(short)(*Sample+Znak*0.5);
        Sample++;
        temp16++;
      }
      break;
    case DSP::e::SampleType::ST_float:
    default:
      temp_float=(float *)(RawBuffer.data());
      for (ind=0; ind<BufferSize * NoOfInputs; ind++)
      {
        *temp_float = *Sample;

        Sample++;
        temp_float++;
      }
      break;
  }

  //string text;

  struct timeval timeout;
  fd_set write_fs;
  int res;
  do
  {
    timeout.tv_sec = 0;  timeout.tv_usec = 10;
    FD_ZERO(&write_fs); FD_SET(ConnectSocket, &write_fs);
    res = select(int(ConnectSocket)+1, NULL, &write_fs, NULL, &timeout);
  }
  while (res == 0);

  //! \todo getsockopt with the optname parameter set to SO_MAX_MSG_SIZE
  //! \todo use select to check if data can be written
  // send data to socket
  // int out_counter = send(ConnectSocket, (char *)RawBuffer, outbuffer_size, 0);
  send(ConnectSocket, (char *)RawBuffer.data(), outbuffer_size, 0);

  // reset buffer
  memset(Buffer.data(), 0x00, Buffer.size() * sizeof(DSP::Float));
  BufferIndex = 0;
}

// Sends connection data to the DSP::u::SocketInput
/* \note this must be done right after the connection is established
 *   and before any data are sent.
 */
bool DSP::u::SocketOutput::SendConnectionData(void)
{
  unsigned short buffer[1024];
  unsigned long out_counter;
  #ifdef __DEBUG__
    string text;
  #endif

  fd_set writefds;
  struct timeval timeout;
  int res;
  long long_val;

  timeout.tv_sec = 0; timeout.tv_usec = 0;
  do
  {
    FD_ZERO(&writefds);
    FD_SET(ConnectSocket, &writefds);
    res = select(int(ConnectSocket)+1, NULL, &writefds, NULL, &timeout);
    DSP::f::Sleep(0);
    if (res == 0)
      current_socket_state |= DSP::e::SocketStatus::timeout;
  }
  while (res == 0); // timeout : no connections awaits for acceptance
  current_socket_state &= DSP::e::SocketStatus::timeout_mask;
  if (DSP::Socket::is_socket_error(res) == true)
  {
    #ifdef __DEBUG__
      DSP::log << "DSP::Socket::TryAcceptConnection" << DSP::e::LogMode::second << "SOCKET_ERROR" << endl;
    #endif
    close_socket(ConnectSocket);
    current_socket_state |= DSP::e::SocketStatus::closed;
    current_socket_state |= DSP::e::SocketStatus::error;
    current_socket_state &= DSP::e::SocketStatus::unconnected_mask;
    return false;
  }

  // marker
  buffer[0] = 0xffff;
  out_counter = send(ConnectSocket, (char *)buffer, sizeof(unsigned short), 0);
  #ifdef __DEBUG__
    DSP::log << "DSP::u::SocketOutput::SendConnectionData" << DSP::e::LogMode::second
      << "marker(end) out_counter = " << out_counter << endl;
  #endif
  // version
  buffer[0] = 0x0001;
  out_counter += send(ConnectSocket, (char *)buffer, sizeof(unsigned short), 0);
  #ifdef __DEBUG__
    DSP::log << "DSP::u::SocketOutput::SendConnectionData" << DSP::e::LogMode::second
      << "version(end) out_counter = " << out_counter << endl;

    DSP::log << "DSP::u::SocketOutput::SendConnectionData" << DSP::e::LogMode::second
      << "DSP::e::SocketInfoDataType::Fp(start) out_counter = " << out_counter << endl;
  #endif

  // field type
  buffer[0] = (unsigned short)(DSP::e::get_value(DSP::e::SocketInfoDataType::Fp));
  out_counter += send(ConnectSocket, (char *)buffer, sizeof(unsigned short), 0);
  // field length
  buffer[0] = sizeof(long);
  out_counter += send(ConnectSocket, (char *)buffer, sizeof(unsigned short), 0);
  // field data
//  *((long *)buffer) = -1; // not specified yet
  long_val = -1; memcpy(buffer,&long_val,sizeof(long)); // not specified yet
  out_counter += send(ConnectSocket, (char *)buffer, sizeof(long), 0);
  #ifdef __DEBUG__
    DSP::log << "DSP::u::SocketOutput::SendConnectionData" << DSP::e::LogMode::second
      << "DSP::e::SocketInfoDataType::Fp(end) out_counter = " << out_counter << endl;
    DSP::log << "DSP::u::SocketOutput::SendConnectionData" << DSP::e::LogMode::second
      << "DSP::e::SocketInfoDataType::Offset(start) out_counter = " << out_counter << endl;
  #endif

  // field type
  buffer[0] = (unsigned short)(DSP::e::get_value(DSP::e::SocketInfoDataType::Offset));
  out_counter += send(ConnectSocket, (char *)buffer, sizeof(unsigned short), 0);
  // field length
  buffer[0] = sizeof(long);
  out_counter += send(ConnectSocket, (char *)buffer, sizeof(unsigned short), 0);
  // field data
  //! \todo select proper offset
//  *((long *)buffer) = 2*BufferSize;
  long_val = 2*BufferSize; memcpy(buffer,&long_val,sizeof(long));
  out_counter += send(ConnectSocket, (char *)buffer, sizeof(long), 0);

  #ifdef __DEBUG__
    DSP::log << "DSP::u::SocketOutput::SendConnectionData" << DSP::e::LogMode::second
      << "DSP::e::SocketInfoDataType::Offset(end) out_counter = " << out_counter << endl;
  #endif

  // field type
  buffer[0] = (unsigned short)(DSP::e::get_value(DSP::e::SocketInfoDataType::end));
  out_counter += send(ConnectSocket, (char *)buffer, sizeof(unsigned short), 0);

  //! check if data actually has been sent
  return true;
}

#define THIS ((DSP::u::SocketOutput *)block)
void DSP::u::SocketOutput::InputExecute(INPUT_EXECUTE_ARGS)
{
  UNUSED_DEBUG_ARGUMENT(Caller);

  if (THIS->socket_ready == false)
  { // no socket ready
    if (THIS->works_as_server)
    {
      //DSP::log << "DSP::u::SocketOutput::InputExecute", "THIS->TryAcceptConnection(1)");
      TryAcceptConnection();
      //DSP::log << "DSP::u::SocketOutput::InputExecute", "THIS->TryAcceptConnection(2)");
    }
    else
    { // Use proper destination ServerID
      //DSP::log << "DSP::u::SocketOutput::InputExecute", "THIS->TryConnect(1)");
      THIS->TryConnect(THIS->ServerObjectID);
      //DSP::log << "DSP::u::SocketOutput::InputExecute", "THIS->TryConnect(2)");
    }
    //! \todo Should rather fill input buffer with zeros and const values
    if (THIS->socket_ready == false)
    {
      //DSP::log << "DSP::u::SocketOutput::InputExecute", "still no socket ready");
      return; // still no socket ready
    }
  }
  if (THIS->SocketInfoData_sent == false)
  {
    DSP::log << "DSP::u::SocketOutput::InputExecute" << DSP::e::LogMode::second << "THIS->SendConnectionData" << endl;
    THIS->SocketInfoData_sent = THIS->SendConnectionData();
  }

  THIS->Buffer[THIS->BufferIndex * THIS->NoOfInputs + InputNo]=value;
  THIS->NoOfInputsProcessed++;

  if (THIS->NoOfInputsProcessed == THIS->NoOfInputs)
  {
    THIS->BufferIndex++;
    THIS->BufferIndex %= THIS->BufferSize;

    if (THIS->BufferIndex == 0)
    { // Data must be written to file from buffer
      //First we need to convert data from RawBuffer
      //if SampleType == DSP::e::SampleType::ST_float we don't need to convert
      THIS->FlushBuffer();
    }

    //NoOfInputsProcessed=0;
    if (THIS->IsUsingConstants == true)
    {
      for (unsigned int ind=0; ind < THIS->NoOfInputs; ind++)
        if (THIS->IsConstantInput[ind])
        {
          THIS->Buffer[THIS->BufferIndex * THIS->NoOfInputs + InputNo]=
            THIS->ConstantInputValues[ind];
          THIS->NoOfInputsProcessed++;
        }
    }
    THIS->NoOfInputsProcessed = THIS->InitialNoOfInputsProcessed;
  }
}
#undef THIS

