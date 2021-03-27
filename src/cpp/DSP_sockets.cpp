/*! \file DSPsockets.cpp
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
DSPe_SocketStatus& operator|= (DSPe_SocketStatus& left,
                               const DSPe_SocketStatus& right)
{
  left = DSPe_SocketStatus(left | right);
  return left;
}
DSPe_SocketStatus& operator&= (DSPe_SocketStatus& left,
                               const DSPe_SocketStatus& right)
{
  left = DSPe_SocketStatus(left & right);
  return left;
}

// ***************************************************** //
// ***************************************************** //
// Basic sockets support
bool DSP_socket::winsock_initialized = false;
int  DSP_socket::NoOfSocketObjects = 0;
const string DSP_socket::DEFAULT_PORT = "27027";
WSADATA DSP_socket::wsaData;

bool DSP_socket::listen_ready = false;
SOCKET DSP_socket::ListenSocket = INVALID_SOCKET;
int DSP_socket::no_of_server_objects = 0;
DSP_socket **DSP_socket::server_objects_list = NULL;

DSP_socket::DSP_socket(const string &address_with_port, bool run_as_client, DWORD ServerObjectID_in)
{
  result = NULL;
  ptr = NULL;

  ServerObjectID = ServerObjectID_in; //0x00000000; <== any
  socket_ready = false;
  current_socket_state = DSP_socket_none;

  if (winsock_initialized == false)
  {
    winsock_initialized = Init_socket();
  }
  NoOfSocketObjects++;

  ConnectSocket = INVALID_SOCKET;

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

bool DSP_socket::InitServer_ListenSocket(const string & address_with_port)
{
  int res;

  if (winsock_initialized == false)
    return false;

  if (listen_ready == true)
  {
    DSP::log << "DSP_socket::InitServer_ListenSocket" << DSP::LogMode::second << "already initialized" << endl;
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
        DSP::log << DSP::LogMode::Error << "DSP_socket::InitServer" << DSP::LogMode::second << "getaddrinfo failed" << endl;
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
    if (ListenSocket == INVALID_SOCKET)
    {
      res = WSAGetLastError();
      DSP::log << DSP::LogMode::Error << "DSP_socket::InitServer" << DSP::LogMode::second << "Error at socket(): " << res << endl;
      freeaddrinfo(result);
      result = NULL;
      listen_ready = false;
      return false;
    }

    // set blocking or nonblocking mode
    // set nonblocking mode: on = 1
    unsigned long on; on = 1;
    iResult = ioctlsocket(ListenSocket, FIONBIO, &on);
    if (iResult == SOCKET_ERROR)
    {
      DSP::log << DSP::LogMode::Error << "DSP_socket::InitServer" << DSP::LogMode::second << "ioctlsocket failed to set non-blocking mode" << endl;
    }

    // Setup the TCP listening socket
    iResult = bind( ListenSocket,
        result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR)
    {
      DSP::log << DSP::LogMode::Error << "DSP_socket::InitServer" << DSP::LogMode::second << "bind failed" << endl;
      //printf("bind failed: %d\n", WSAGetLastError());
      freeaddrinfo(result);
      result = NULL;
      closesocket(ListenSocket);
      ListenSocket = INVALID_SOCKET;
      listen_ready = false;
      return false;
    }

    // support multiple outgoing connections
    if ( listen( ListenSocket, SOMAXCONN ) == SOCKET_ERROR )
    {
      DSP::log << DSP::LogMode::Error << "DSP_socket::InitServer" << DSP::LogMode::second << "listen failed" << endl;
      //printf("listen failed: %d\n", WSAGetLastError());
      freeaddrinfo(result);
      result = NULL;
      closesocket(ListenSocket);
      ListenSocket = INVALID_SOCKET;
      listen_ready = false;
      return false;
    }
  }

  current_socket_state |= DSP_socket_listen_active;
  return true;
}

bool DSP_socket::InitServer(void) //DWORD ServerObjectID_in)
{
  DSP_socket **server_objects_list_tmp;

  socket_ready = false;

  // 1. add this object to the server_objects_list
  server_objects_list_tmp = new DSP_socket *[no_of_server_objects+1];
  memcpy(server_objects_list_tmp, server_objects_list, sizeof(DSP_socket *)*no_of_server_objects);
  server_objects_list_tmp[no_of_server_objects] = this;
  delete [] server_objects_list;
  server_objects_list = server_objects_list_tmp;
  no_of_server_objects++;

  // 2. update socket ID and other socket data
  //ServerObjectID = ServerObjectID_in;

  current_socket_state |= DSP_socket_server;

  return socket_ready;
}

// Accept first incoming connections
bool DSP_socket::TryAcceptConnection(void)
{
  fd_set readfds;
  struct timeval timeout;
  int ind, res;
  unsigned long in_counter;
  DWORD temp_ServerObjectID;
  SOCKET temp_socket;
  string text;

  if (ListenSocket == INVALID_SOCKET)
    return false;

  if (no_of_server_objects <= 0)
    return false;

  //! \bug support for asynchronous connections and accept connections during processing loop
  timeout.tv_sec = 0; timeout.tv_usec = 0;
  FD_ZERO(&readfds);
  FD_SET(ListenSocket, &readfds);
  res = select(0, &readfds, NULL, NULL, &timeout);
  if (res == 0)
  { //! timeout : no connections awaits for acceptance
    //current_socket_state |= DSP_socket_timeout; // no object
    return false;
  }
  //current_socket_state &= DSP_socket_timeout_mask; // no object
  if (res == SOCKET_ERROR)
  {
    DSP::log << "DSP_socket::TryAcceptConnection" << DSP::LogMode::second << "SOCKET_ERROR" << endl;
    closesocket(ListenSocket);
    ListenSocket = INVALID_SOCKET;
    // current_socket_state |= DSP_socket_error; no object
    return false;
  }

  // Accept a client socket
  temp_socket = accept(ListenSocket, NULL, NULL);
  if (temp_socket == INVALID_SOCKET)
  {
    DSP::log << DSP::LogMode::Error << "DSP_socket::TryAcceptConnection" << DSP::LogMode::second << "accept failed" << endl;
    //printf("accept failed: %d\n", WSAGetLastError());
    closesocket(ListenSocket);
    ListenSocket = INVALID_SOCKET;
    listen_ready = false;
    //current_socket_state &= DSP_socket_listen_active_mask; // no object
    return false;
  }

  // read ServerObjectID from accepted socket
  timeout.tv_sec = 1; timeout.tv_usec = 0;
  FD_ZERO(&readfds);
  FD_SET(temp_socket, &readfds);
  res = select(0, &readfds, NULL, NULL, &timeout);
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
    closesocket(temp_socket);
    DSP::log << "DSP_socket::TryAcceptConnection" << DSP::LogMode::second << "socket ID data has not been received (timeout - connection closed)" << endl;
    return false;
  }
  if (res == SOCKET_ERROR)
  {
    DSP::log << "DSP_socket::TryAcceptConnection" << DSP::LogMode::second << "error reading socket ID data" << endl;
    // current_socket_state |= DSP_socket_error; // no object
    return false;
  }

  res = ioctlsocket(temp_socket, FIONREAD, &in_counter);
  if (in_counter == 0)
  {
    // connection has been closed
    DSP::log << "DSP_socket::TryAcceptConnection" << DSP::LogMode::second << "connection has been closed" << endl;
    return false;
  }
  if (in_counter < sizeof(DWORD))
  {
    DSP::log << "DSP_socket::TryAcceptConnection" << DSP::LogMode::second << "error reading socket ID data (not enough data)" << endl;
    // current_socket_state |= DSP_socket_error; // no object
    return false;
  }
  in_counter = recv(temp_socket, (char *)(&temp_ServerObjectID), sizeof(DWORD), 0);

  DSP::log << "DSP_socket::TryAcceptConnection" << DSP::LogMode::second << "ServerObjectID = " << (int)temp_ServerObjectID << endl;

  // update ConnectSocket in according socket object
  for (ind = 0; ind < no_of_server_objects; ind++)
  {
    if (server_objects_list[ind] != NULL)
    {
      if (   (server_objects_list[ind]->ServerObjectID == 0x00000000)
          || (server_objects_list[ind]->ServerObjectID == temp_ServerObjectID))
      {
        if (server_objects_list[ind]->ConnectSocket == INVALID_SOCKET)
        {
          DSP::log << "DSP_socket::TryAcceptConnection" << DSP::LogMode::second
            << "server_objects_list[" << ind
            << "]ServerObjectID = " << (unsigned int)server_objects_list[ind]->ServerObjectID << endl;

          server_objects_list[ind]->ConnectSocket = temp_socket;
          server_objects_list[ind]->socket_ready = true;

          server_objects_list[ind]->current_socket_state |= DSP_socket_connected;
          return true;
        }
      }
    }
  }
  // current_socket_state |= DSP_socket_error; // no object
  DSP::log << DSP::LogMode::Error << "DSP_socket::TryAcceptConnection" << DSP::LogMode::second << "Unexpected incoming connection" << endl;
  return false;
}

string DSP_socket::extract_hostname(const string &address_with_port) {
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

string DSP_socket::extract_port(const string& address_with_port, const string &default_port) {
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

bool DSP_socket::InitClient(const string & address_with_port)
{
  bool ready;

  if (winsock_initialized == false)
  {
    current_socket_state |= DSP_socket_error;
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
    DSP::log << DSP::LogMode::Error << "DSP_socket::InitClient" << DSP::LogMode::second << "getaddrinfo failed" << endl;
    current_socket_state |= DSP_socket_error;
    return false;
  }

  if (ready == true)
  {
    // Attempt to connect to the first address returned by
    // the call to getaddrinfo
    ptr=result;
    // Create a SOCKET for connecting to server
    ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
    if (ConnectSocket == INVALID_SOCKET)
    {
      DSP::log << DSP::LogMode::Error << "DSP_socket::InitClient" << DSP::LogMode::second << "Error at socket()" << endl;
      //printf("Error at socket(): %ld\n", WSAGetLastError());
      freeaddrinfo(result);
      result = NULL;
      current_socket_state |= DSP_socket_error;
      current_socket_state &= DSP_socket_unconnected_mask;
      return false;
    }
    unsigned long on; on = 1;
    iResult = ioctlsocket(ConnectSocket, FIONBIO, &on);
    if (iResult == SOCKET_ERROR)
    {
      DSP::log << DSP::LogMode::Error << "DSP_socket::InitClient" << DSP::LogMode::second << "ioctlsocket failed to set non-blocking mode" << endl;
      current_socket_state |= DSP_socket_error;
    }
  }

  current_socket_state &= DSP_socket_client_mask;
  return true;
}

bool DSP_socket::TryConnect(DWORD SerwerObjectID)
{
  unsigned int out_counter;
#ifdef __DEBUG__
  string text;
#endif
  int err;

  //http://msdn.microsoft.com/en-us/library/ms737625(VS.85).aspx

  //DSP::log << "sssocket");
  current_socket_state &= DSP_socket_unconnected_mask;
  // connect on nonblocking socket
  // Connect to server.
  iResult = connect( ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
  if (iResult == SOCKET_ERROR)
  {
    err = WSAGetLastError();

    switch (err)
    {
      case WSAEWOULDBLOCK:
        // OK ==> wait for finalized connection
      case WSAEALREADY: //
      case WSAEINVAL:
        // connection not ready yet
        current_socket_state &= DSP_socket_timeout_mask;
        DSP::f::Sleep(0);
        return false;
        break;

      case WSAEISCONN:
        // connection completed
        current_socket_state &= DSP_socket_timeout_mask;
        socket_ready = true;
        #ifdef __DEBUG__
          DSP::log << "DSP_socket::TryConnect" << DSP::LogMode::second << "connection established (" << err << ")" << endl;
        #endif
        break;

      case SOCKET_ERROR:
      default:
        // error or unexpected situation
        current_socket_state &= DSP_socket_timeout_mask;
        err = WSAGetLastError();

        freeaddrinfo(result);
        result = NULL;
        closesocket(ConnectSocket);
        ConnectSocket = INVALID_SOCKET;
        // b u g: must recreate socket before another connect attempt

        current_socket_state |= DSP_socket_closed;
        current_socket_state |= DSP_socket_error;
        current_socket_state &= DSP_socket_unconnected_mask;

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
  select(0, NULL, &write_fs, NULL, &timeout);
  #ifdef __DEBUG__
    if (res == 0)
    {
      DSP::log << DSP::LogMode::Error << "DSP_socket::TryConnect" << DSP::LogMode::second << "connected but not ready to write (" << res << ")" << endl;
    }
    else
    {
      DSP::log << "DSP_socket::TryConnect" << DSP::LogMode::second << "connected and ready to write (" << res << ")" << endl;
    }
  #endif

  current_socket_state |= DSP_socket_connected;

  // send expected server object ID
  out_counter = send(ConnectSocket, (char *)(&SerwerObjectID), sizeof(DWORD), 0);
  if (out_counter < sizeof(DWORD))
  {
    #ifdef __DEBUG__
      DSP::log << DSP::LogMode::Error << "DSP_socket::TryConnect" << DSP::LogMode::second << "failed to send expected server object data" << endl;
    #endif
    current_socket_state |= DSP_socket_error;
    return false;
  }
  #ifdef __DEBUG__
    DSP::log << "DSP_socket::TryConnect" << DSP::LogMode::second << "SerwerObjectID has been sent" << endl;
  #endif
  return socket_ready;
}

bool DSP_socket::Init_socket(void)
{
  // Initialize Winsock
  iResult = WSAStartup(MAKEWORD(2,2), &wsaData);

  if (iResult != 0)
  {
    #ifdef __DEBUG__
      DSP::log << DSP::LogMode::Error << "DSP_socket::Init_socket" << DSP::LogMode::second << "WSAStartup failed" << endl;
    #endif
    current_socket_state |= DSP_socket_error;
    return false;
  }
  return true;
}

bool DSP_socket::WaitForConnection(bool stop_on_fail)
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

DSPe_SocketStatus DSP_socket::GetSocketStatus(void)
{
  if (listen_ready == true)
    current_socket_state |= DSP_socket_listen_active;
  else
    current_socket_state &= DSP_socket_listen_active_mask;

  return current_socket_state;
}

DSP_socket::~DSP_socket(void)
{
  current_socket_state = DSP_socket_none;

  if (result != NULL)
  {
    freeaddrinfo(result);
    result = NULL;
  }
  if (ConnectSocket != INVALID_SOCKET)
  {
    closesocket(ConnectSocket);
    ConnectSocket = INVALID_SOCKET;
  }

  if (NoOfSocketObjects > 0)
  {
    NoOfSocketObjects--;
    if ((NoOfSocketObjects == 0) && (winsock_initialized == true))
    {
      // close socket
      if (ListenSocket != INVALID_SOCKET)
      {
        listen_ready = false;
        closesocket(ListenSocket);
        ListenSocket = INVALID_SOCKET;
      }

      WSACleanup();
      winsock_initialized = false;
    }
  }

  if (NoOfSocketObjects == 0)
  {
    if (server_objects_list != NULL)
    {
      no_of_server_objects = 0;
      delete [] server_objects_list;
      server_objects_list = NULL;
    }
  }
}

/* ***************************************** */
/* ***************************************** */
DSPu_SOCKETinput::DSPu_SOCKETinput(DSP::Clock_ptr ParentClock,
      const string &address_with_port, bool run_as_client,
      DWORD ServerObjectID_in,
      unsigned int NoOfChannels,
      DSP::e::SampleType sample_type)
  : DSP_socket(address_with_port, run_as_client, ServerObjectID_in), DSP::Source()
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
void DSPu_SOCKETinput::Init(DSP::Clock_ptr ParentClock,
                            unsigned int OutputsNo, //just one channel
                            DSP::e::SampleType sample_type)
{
  string temp;

  Fp = -1;
  Offset = 0;
  SocketInfoData_received = false;

  SetNoOfOutputs(OutputsNo);
  SetName("SOCKETinput", false);

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

  inbuffer_size = DSP_socket_buffer_size / (InputSampleSize / 8);
  BufferSize = inbuffer_size;
  inbuffer_size *= (InputSampleSize / 8);

  RawBuffer = new uint8_t[inbuffer_size];
  Buffer = new DSP::Float[NoOfOutputs * BufferSize];
  BufferIndex = BufferSize;

  LastBytesRead_counter = DSP_FILE_READING_NOT_STARTED;
}

DSPu_SOCKETinput::~DSPu_SOCKETinput(void)
{
  if (RawBuffer != NULL)
  {
    delete [] RawBuffer;
    RawBuffer = NULL;
  }
  if (Buffer != NULL)
  {
    delete [] Buffer;
    Buffer = NULL;
  }
}

bool DSPu_SOCKETinput::SetSkip(long long Offset_in)
{
  UNUSED_ARGUMENT(Offset_in);

  #ifdef __DEBUG__
    DSP::log << "DSPu_SOCKETinput::SetSkip" << DSP::LogMode::second << "Offset setting not yet supported" << endl;
  #endif
  return false;
}

unsigned int DSPu_SOCKETinput::GetBytesRead(void)
{
  return LastBytesRead_counter;
}
//! \bug not implemented
long int DSPu_SOCKETinput::GetSamplingRate(void)
{
  return 0;
}

#define THIS ((DSPu_SOCKETinput *)source)
bool DSPu_SOCKETinput::OutputExecute(OUTPUT_EXECUTE_ARGS)
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
      //DSP::log << "DSPu_SOCKETinput::OutputExecute", "THIS->TryAcceptConnection(1)");
      TryAcceptConnection();
      //DSP::log << "DSPu_SOCKETinput::OutputExecute", "THIS->TryAcceptConnection(2)");
    }
    else
    { //Use proper destination ServerID
      //DSP::log << "DSPu_SOCKETinput::OutputExecute", "THIS->TryConnect(1)");
      THIS->TryConnect(THIS->ServerObjectID);
      //DSP::log << "DSPu_SOCKETinput::OutputExecute", "THIS->TryConnect(2)");
    }

    //DSP::log << "DSPu_SOCKETinput::OutputExecute", "no socket opened");
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
    DSP::log << "DSPu_SOCKETinput::OutputExecute" << DSP::LogMode::second << "THIS->ReadConnectionData" << endl;
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
    res = select(0, &readfds, NULL, NULL, &timeout);
    if (res == 0)
    { //! timeout
      //DSP::log << "DSPu_SOCKETinput::OutputExecute", "timeout");
      DSP::Clock::InputNeedsMoreTime[THIS->my_clock->MasterClockIndex] = true;
      THIS->current_socket_state |= DSP_socket_timeout;

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
    THIS->current_socket_state &= DSP_socket_timeout_mask;
    if (res == SOCKET_ERROR)
    {
      DSP::log << "DSPu_SOCKETinput::OutputExecute" << DSP::LogMode::second << "SOCKET_ERROR" << endl;
      //  close socket
      closesocket(THIS->ConnectSocket);
      THIS->ConnectSocket = INVALID_SOCKET;

      THIS->socket_ready = false;
      // force repeated call to this function
      DSP::Clock::InputNeedsMoreTime[THIS->my_clock->MasterClockIndex] = true;
      THIS->LastBytesRead_counter = 0;
      THIS->current_socket_state |= DSP_socket_closed;
      THIS->current_socket_state |= DSP_socket_error;
      return false;
    }

    res = ioctlsocket(THIS->ConnectSocket, FIONREAD, &in_counter);

    if (in_counter == 0)
    {
      // connection has been closed
      DSP::log << "DSPu_SOCKETinput::OutputExecute" << DSP::LogMode::second << "connection has been closed" << endl;
      // close socket
      closesocket(THIS->ConnectSocket);
      THIS->ConnectSocket = INVALID_SOCKET;

      THIS->socket_ready = false;
      // force repeated call to this function
      DSP::Clock::InputNeedsMoreTime[THIS->my_clock->MasterClockIndex] = true;
      THIS->LastBytesRead_counter = 0;
      THIS->current_socket_state |= DSP_socket_closed;
      THIS->current_socket_state &= DSP_socket_unconnected_mask;
      return false;
    }
    if (in_counter < THIS->inbuffer_size)
    {
	  #ifdef __DEBUG__
        DSP::log << "DSPu_SOCKETinput::OutputExecute" << DSP::LogMode::second << "more data expected" << endl;
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
    in_counter = recv(THIS->ConnectSocket, (char *)THIS->RawBuffer, THIS->inbuffer_size, 0);
    THIS->LastBytesRead_counter = in_counter;

    in_temp = THIS->RawBuffer;
    temp_buffer = THIS->Buffer;
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

bool DSPu_SOCKETinput::ReadConnectionData(void)
{
  unsigned short buffer[1024];
  long expected_data_size;
  DSPe_SocketInfoDataType data_type;
  int state;
  unsigned long in_counter, in_counter_recv;
  int res;

  fd_set readfds;
  struct timeval timeout;
  timeout.tv_sec = 0; timeout.tv_usec = 0;

  in_counter_recv = 0;

  data_type = DSP_SID_none;
  state = 0; // wait for marker and version
  expected_data_size = 4;
  do
  {
    do
    {
      DSP::f::Sleep(0);

      FD_ZERO(&readfds);
      FD_SET(ConnectSocket, &readfds);
      res = select(0, &readfds, NULL, NULL, &timeout);
      if (res == 0)
      {
        //DSP::log << "DSPu_SOCKETinput::ReadConnectionData", "timeout");
        current_socket_state |= DSP_socket_timeout;
      }
    }
    while (res == 0); // timeout
    current_socket_state &= DSP_socket_timeout_mask;
    if (res == SOCKET_ERROR)
    {
      #ifdef __DEBUG__
        DSP::log << DSP::LogMode::Error << "DSPu_SOCKETinput::ReadConnectionData" << DSP::LogMode::second << "SOCKET_ERROR" << endl;
      #endif
      // close socket
      socket_ready = false;
      closesocket(ConnectSocket);
      ConnectSocket = INVALID_SOCKET;
      current_socket_state |= DSP_socket_closed;
      current_socket_state |= DSP_socket_error;
      return false;
    }

    res = ioctlsocket(ConnectSocket, FIONREAD, &in_counter);
    if (in_counter == 0)
    {
      // connection has been closed
      #ifdef __DEBUG__
        DSP::log << "DSPu_SOCKETinput::ReadConnectionData" << DSP::LogMode::second << "connection has been closed" << endl;
      #endif
      // close socket
      socket_ready = false;
      closesocket(ConnectSocket);
      ConnectSocket = INVALID_SOCKET;
      current_socket_state |= DSP_socket_closed;
      current_socket_state &= DSP_socket_unconnected_mask;
      return false;
    }
    if ((long)in_counter < expected_data_size)
    {
      #ifdef __DEBUG__
      {
        DSP::log << "DSPu_SOCKETinput::ReadConnectionData" << DSP::LogMode::second
          << "more data expected: in_counter = " << in_counter
          << "; expected_data_size = " << expected_data_size << endl;
      }
      #endif
      continue;
    }

    in_counter_recv += recv(ConnectSocket, (char *)buffer, expected_data_size, 0);
    #ifdef __DEBUG__
    {
      DSP::log << "DSPu_SOCKETinput::ReadConnectionData" << DSP::LogMode::second
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
            DSP::log << DSP::LogMode::Error << "DSPu_SOCKETinput::ReadConnectionData" << DSP::LogMode::second << "unexpected marker" << endl;
          #endif
          data_type = DSP_SID_end;
        }
        //! \todo make use of buffer[1] <== version
        state = 1; // waiting for field type
        expected_data_size = 2;
        break;
      case 1: // waiting for field type
        data_type = (DSPe_SocketInfoDataType)(buffer[0]);
        state = 2; // wait for field length
        expected_data_size = 2;
        break;

      case 2:
        //! \bug compare read data size with expected data size
        switch (data_type)
        {
          case DSP_SID_Fp:
            #ifdef __DEBUG__
              DSP::log << "DSPu_SOCKETinput::ReadConnectionData" << DSP::LogMode::second << "DSP_SID_Fp" << endl;
            #endif
            state = 3; // waiting for Fp data
            expected_data_size = sizeof(long);
            break;

          case DSP_SID_Offset:
            #ifdef __DEBUG__
              DSP::log << "DSPu_SOCKETinput::ReadConnectionData" << DSP::LogMode::second << "DSP_SID_Offset" << endl;
            #endif
            state = 3; // waiting for Offset data
            expected_data_size = sizeof(long);
            break;

          case DSP_SID_UserData:
            #ifdef __DEBUG__
              DSP::log << "DSPu_SOCKETinput::ReadConnectionData" << DSP::LogMode::second << "DSP_SID_UserData" << endl;
            #endif
            state = 3; // waiting for user data
//            expected_data_size = *((long *)buffer);
            memcpy(&expected_data_size,buffer, sizeof(long));
            break;
          case DSP_SID_end:
            #ifdef __DEBUG__
              DSP::log << "DSPu_SOCKETinput::ReadConnectionData" << DSP::LogMode::second << "DSP_SID_end" << endl;
            #endif
            // ignore: transmission finished
            break;
          default:
            state = 3; // waiting for unknown data
//            expected_data_size = *((long *)buffer);
            memcpy(&expected_data_size,buffer, sizeof(long));
            #ifdef __DEBUG__
              DSP::log << "DSPu_SOCKETinput::ReadConnectionData" << DSP::LogMode::second << "unexpected field type" << endl;
            #endif
            current_socket_state |= DSP_socket_error;
            break;
        }
        break;
      case 3:
        //! \bug compare read data size with expected data size
        switch (data_type)
        {
          case DSP_SID_Fp:
//            Fp = *((long *)buffer);
            memcpy(&Fp, buffer, sizeof(long));
            break;
          case DSP_SID_Offset:
//            Offset = *((long *)buffer);
            memcpy(&Offset, buffer, sizeof(long));
            break;
          case DSP_SID_UserData:
            //UserData = *((long *)buffer);
            break;
          default:
            #ifdef __DEBUG__
              DSP::log << DSP::LogMode::Error << "DSPu_SOCKETinput::ReadConnectionData" << DSP::LogMode::second << "unexpected data type" << endl;
            #endif
            current_socket_state |= DSP_socket_error;
            break;
        }
        state = 1; // waiting for field type
        expected_data_size = 2;
        break;
      default:
        data_type = DSP_SID_end;
        #ifdef __DEBUG__
          DSP::log << DSP::LogMode::Error << "DSPu_SOCKETinput::ReadConnectionData" << DSP::LogMode::second << "unexpected state" << endl;
        #endif
        current_socket_state |= DSP_socket_error;
        break;
    }
  }
  while (data_type != DSP_SID_end);

  #ifdef __DEBUG__
  {
    DSP::log << "DSPu_SOCKETinput::ReadConnectionData" << DSP::LogMode::second
        << "in_counter_recv = " << in_counter_recv << endl;
  }
  #endif
  return true;
}
/* ***************************************** */
/* ***************************************** */
DSPu_SOCKEToutput::DSPu_SOCKEToutput(
      const string &address_with_port, bool run_as_client,
      DWORD ServerObjectID_in,
      unsigned int NoOfChannels,
      DSP::e::SampleType sample_type)
  : DSP_socket(address_with_port, run_as_client, ServerObjectID_in), DSP::Block()
{
  Init(NoOfChannels, sample_type);

  Execute_ptr = &InputExecute;
}

void DSPu_SOCKEToutput::Init(unsigned int InputsNo,
                             DSP::e::SampleType sample_type)
{
  string temp;

  SocketInfoData_sent = false;

  SetNoOfInputs(InputsNo, true);
  SetName("SOCKEToutput", false);

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

  outbuffer_size = DSP_socket_buffer_size / (OutputSampleSize / 8);
  BufferSize = outbuffer_size;
  outbuffer_size *= (OutputSampleSize / 8);

  RawBuffer = new uint8_t[outbuffer_size];
  Buffer = new DSP::Float[NoOfInputs * BufferSize];

  BufferIndex = 0;
  NoOfInputsProcessed = 0;
}

DSPu_SOCKEToutput::~DSPu_SOCKEToutput(void)
{
  if (Buffer != NULL)
  {
    delete [] Buffer;
    Buffer = NULL;
  }
  if (RawBuffer != NULL)
  {
    delete [] RawBuffer;
    RawBuffer = NULL;
  }
}

void DSPu_SOCKEToutput::FlushBuffer(void)
{
  uint8_t *temp8;
  short *temp16;
  DSP::Float_ptr Sample, temp_float;
  short Znak;
  DWORD ind;

  // ************************************************** //
  // Send buffer to the socket
  Sample=Buffer;
  // ************************************************** //
  // Converts samples format to the one suitable for the audio device
  switch (SampleType)
  {
    case DSP::e::SampleType::ST_uchar:
      temp8=(uint8_t *)(RawBuffer);
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
      temp16=(short *)(RawBuffer);
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
      temp_float=(float *)(RawBuffer);
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
    res = select(0, NULL, &write_fs, NULL, &timeout);
  }
  while (res == 0);

  //! \todo getsockopt with the optname parameter set to SO_MAX_MSG_SIZE
  //! \todo use select to check if data can be written
  // send data to socket
  // int out_counter = send(ConnectSocket, (char *)RawBuffer, outbuffer_size, 0);
  send(ConnectSocket, (char *)RawBuffer, outbuffer_size, 0);

  // reset buffer
  memset(Buffer, 0x00, BufferSize * NoOfInputs * sizeof(DSP::Float));
  BufferIndex = 0;
}

// Sends connection data to the DSPu_SOCKETinput
/* \note this must be done right after the connection is established
 *   and before any data are sent.
 */
bool DSPu_SOCKEToutput::SendConnectionData(void)
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
    res = select(0, NULL, &writefds, NULL, &timeout);
    DSP::f::Sleep(0);
    if (res == 0)
      current_socket_state |= DSP_socket_timeout;
  }
  while (res == 0); // timeout : no connections awaits for acceptance
  current_socket_state &= DSP_socket_timeout_mask;
  if (res == SOCKET_ERROR)
  {
    #ifdef __DEBUG__
      DSP::log << "DSP_socket::TryAcceptConnection" << DSP::LogMode::second << "SOCKET_ERROR" << endl;
    #endif
    closesocket(ConnectSocket);
    ConnectSocket = INVALID_SOCKET;
    current_socket_state |= DSP_socket_closed;
    current_socket_state |= DSP_socket_error;
    current_socket_state &= DSP_socket_unconnected_mask;
    return false;
  }

  // marker
  buffer[0] = 0xffff;
  out_counter = send(ConnectSocket, (char *)buffer, sizeof(unsigned short), 0);
  #ifdef __DEBUG__
    DSP::log << "DSPu_SOCKEToutput::SendConnectionData" << DSP::LogMode::second
      << "marker(end) out_counter = " << out_counter << endl;
  #endif
  // version
  buffer[0] = 0x0001;
  out_counter += send(ConnectSocket, (char *)buffer, sizeof(unsigned short), 0);
  #ifdef __DEBUG__
    DSP::log << "DSPu_SOCKEToutput::SendConnectionData" << DSP::LogMode::second
      << "version(end) out_counter = " << out_counter << endl;

    DSP::log << "DSPu_SOCKEToutput::SendConnectionData" << DSP::LogMode::second
      << "DSP_SID_Fp(start) out_counter = " << out_counter << endl;
  #endif

  // field type
  buffer[0] = DSP_SID_Fp;
  out_counter += send(ConnectSocket, (char *)buffer, sizeof(unsigned short), 0);
  // field length
  buffer[0] = sizeof(long);
  out_counter += send(ConnectSocket, (char *)buffer, sizeof(unsigned short), 0);
  // field data
//  *((long *)buffer) = -1; // not specified yet
  long_val = -1; memcpy(buffer,&long_val,sizeof(long)); // not specified yet
  out_counter += send(ConnectSocket, (char *)buffer, sizeof(long), 0);
  #ifdef __DEBUG__
    DSP::log << "DSPu_SOCKEToutput::SendConnectionData" << DSP::LogMode::second
      << "DSP_SID_Fp(end) out_counter = " << out_counter << endl;
    DSP::log << "DSPu_SOCKEToutput::SendConnectionData" << DSP::LogMode::second
      << "DSP_SID_Offset(start) out_counter = " << out_counter << endl;
  #endif

  // field type
  buffer[0] = DSP_SID_Offset;
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
    DSP::log << "DSPu_SOCKEToutput::SendConnectionData" << DSP::LogMode::second
      << "DSP_SID_Offset(end) out_counter = " << out_counter << endl;
  #endif

  // field type
  buffer[0] = DSP_SID_end;
  out_counter += send(ConnectSocket, (char *)buffer, sizeof(unsigned short), 0);

  //! check if data actually has been sent
  return true;
}

#define THIS ((DSPu_SOCKEToutput *)block)
void DSPu_SOCKEToutput::InputExecute(INPUT_EXECUTE_ARGS)
{
  UNUSED_DEBUG_ARGUMENT(Caller);

  if (THIS->socket_ready == false)
  { // no socket ready
    if (THIS->works_as_server)
    {
      //DSP::log << "DSPu_SOCKEToutput::InputExecute", "THIS->TryAcceptConnection(1)");
      TryAcceptConnection();
      //DSP::log << "DSPu_SOCKEToutput::InputExecute", "THIS->TryAcceptConnection(2)");
    }
    else
    { // Use proper destination ServerID
      //DSP::log << "DSPu_SOCKEToutput::InputExecute", "THIS->TryConnect(1)");
      THIS->TryConnect(THIS->ServerObjectID);
      //DSP::log << "DSPu_SOCKEToutput::InputExecute", "THIS->TryConnect(2)");
    }
    //! \todo Should rather fill input buffer with zeros and const values
    if (THIS->socket_ready == false)
    {
      //DSP::log << "DSPu_SOCKEToutput::InputExecute", "still no socket ready");
      return; // still no socket ready
    }
  }
  if (THIS->SocketInfoData_sent == false)
  {
    DSP::log << "DSPu_SOCKEToutput::InputExecute" << DSP::LogMode::second << "THIS->SendConnectionData" << endl;
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

