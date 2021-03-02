<map version="0.9.0">
<!-- To view this file, download free mind mapping software FreeMind from http://freemind.sourceforge.net -->
<node CREATED="1223311494140" ID="ID_1615563649" MODIFIED="1223370836281" TEXT="Todo">
<node CREATED="1223311549406" ID="ID_401709503" MODIFIED="1223583641781" POSITION="right" TEXT="socket support">
<node CREATED="1223311593968" ID="ID_893207038" MODIFIED="1223311631312" TEXT="server">
<node CREATED="1223311634265" ID="ID_606282056" MODIFIED="1223311648953" TEXT="single server listen socket">
<node CREATED="1223311658828" ID="ID_1285366183" MODIFIED="1223311663140" TEXT="single port number"/>
<node CREATED="1223361751406" ID="ID_1252960989" MODIFIED="1223842419171" TEXT="only one server listen socket instance">
<icon BUILTIN="button_ok"/>
</node>
</node>
<node CREATED="1223312820265" ID="ID_1150967113" MODIFIED="1223312833796" TEXT="accept multiple connections">
<node CREATED="1223312890359" ID="ID_1770977467" MODIFIED="1223312909031" TEXT="one accepted socket per object"/>
<node CREATED="1223312834515" ID="ID_794896937" MODIFIED="1223312851390" TEXT="AcceptConnections">
<node CREATED="1223313006406" ID="ID_1540366813" MODIFIED="1223313010515" TEXT="run before Execute"/>
</node>
<node CREATED="1223312851906" ID="ID_1937422416" MODIFIED="1223312867593" TEXT="runtime connections acceptance"/>
</node>
<node CREATED="1225741254500" ID="ID_1574222826" MODIFIED="1225741273984" TEXT="wait for connection data untill connection is closed ">
<node CREATED="1225741274687" ID="ID_1419034875" MODIFIED="1225741305296" TEXT="return processing loop during timeout"/>
</node>
</node>
<node CREATED="1223311631843" ID="ID_1196979269" MODIFIED="1223322202656" TEXT="client">
<node CREATED="1223312870640" ID="ID_881265494" MODIFIED="1223842440609" TEXT="each client object has its socket">
<icon BUILTIN="button_ok"/>
</node>
<node CREATED="1223312927750" ID="ID_646993708" MODIFIED="1223312931578" TEXT="connect">
<node CREATED="1223312932343" ID="ID_1232515310" MODIFIED="1223312945781" TEXT="ConnectClients">
<node CREATED="1223313012531" ID="ID_818168126" MODIFIED="1223313019171" TEXT="run before Execute"/>
</node>
<node CREATED="1223312946671" ID="ID_720944799" MODIFIED="1225745094640" TEXT="runtime client connections">
<icon BUILTIN="button_ok"/>
</node>
</node>
<node CREATED="1223322189296" ID="ID_525567026" MODIFIED="1223322205953" TEXT="what if client starts first">
<icon BUILTIN="help"/>
</node>
<node CREATED="1225741064843" ID="ID_286458525" MODIFIED="1225741099593" TEXT="if socket data send failed ==&gt; close socket and TryConnect again"/>
</node>
<node CREATED="1223313025687" ID="ID_1216305003" MODIFIED="1223313050281" TEXT="client server application">
<node CREATED="1223313050859" ID="ID_233363214" MODIFIED="1223313074796">
<richcontent TYPE="NODE"><html>
  <head>
    
  </head>
  <body>
    <p>
      avoid deadlock
    </p>
    <p>
      with accept / connect
    </p>
  </body>
</html></richcontent>
<node CREATED="1223314086640" FOLDED="true" ID="ID_653449199" MODIFIED="1223750862281" TEXT="client">
<icon BUILTIN="button_ok"/>
<node CREATED="1223314092234" ID="ID_1382923988" MODIFIED="1223314121250">
<richcontent TYPE="NODE"><html>
  <head>
    
  </head>
  <body>
    <p>
      InputExcute / OutputExecute
    </p>
    <p>
      if not connected or finished
    </p>
    <p>
      try connect
    </p>
  </body>
</html></richcontent>
<node CREATED="1223314193593" ID="ID_664057600" MODIFIED="1223314204515" TEXT="will work only for current object"/>
</node>
</node>
<node CREATED="1223314127250" FOLDED="true" ID="ID_123248065" MODIFIED="1223750858578" TEXT="server">
<icon BUILTIN="button_ok"/>
<node CREATED="1223314137843" ID="ID_133557474" MODIFIED="1223314183375">
<richcontent TYPE="NODE"><html>
  <head>
    
  </head>
  <body>
    <p>
      InputExcute / OutputExecute
    </p>
    <p>
      if not connection not accepted or finished
    </p>
    <p>
      try accept
    </p>
  </body>
</html></richcontent>
<node CREATED="1223314184343" ID="ID_1686581682" MODIFIED="1223314192750" TEXT="will work for all objects"/>
</node>
</node>
<node CREATED="1223314300968" ID="ID_1191685412" MODIFIED="1223321596453" TEXT="WSAAsyncSelect">
<icon BUILTIN="closed"/>
</node>
<node CREATED="1223314312328" FOLDED="true" ID="ID_1995710575" MODIFIED="1223750838484" TEXT="WSAEventSelect ">
<icon BUILTIN="closed"/>
<node CREATED="1223314478750" ID="ID_1270261536" MODIFIED="1223314479734" TEXT="http://msdn.microsoft.com/en-us/library/ms741576(VS.85).aspx"/>
<node CREATED="1223314502218" ID="ID_394221061" MODIFIED="1223314504500" TEXT="WSAEWOULDBLOCK"/>
<node CREATED="1223314628921" ID="ID_704764619" MODIFIED="1223314629812" TEXT="WSAWaitForMultipleEvents "/>
<node CREATED="1223314639296" ID="ID_1421536040" MODIFIED="1223314640218" TEXT="WSAEnumNetworkEvents "/>
<node CREATED="1223314676359" ID="ID_411645100" MODIFIED="1223314677453" TEXT="WSACreateEvent "/>
<node CREATED="1223314699531" ID="ID_1898881136" MODIFIED="1223314704109" TEXT="WSACloseEvent "/>
</node>
<node CREATED="1223363724890" FOLDED="true" ID="ID_480051031" MODIFIED="1223841945281" TEXT="select + DSPf_Sleep">
<icon BUILTIN="button_ok"/>
<node CREATED="1223363737578" ID="ID_146262726" MODIFIED="1223363755000" TEXT="should be compatible with Linux"/>
<node CREATED="1223364902953" ID="ID_1187023791" MODIFIED="1223364904000" TEXT="setsockopt() for setting a timeout for recv(), "/>
<node CREATED="1223364925578" ID="ID_1034347945" MODIFIED="1223364926796" TEXT="ioctlsocket(socket, FIONREAD, count) to get the number of bytes in the buffer">
<node CREATED="1223364948406" ID="ID_1072859902" MODIFIED="1223364963593" TEXT="does not detect socket closure"/>
</node>
<node CREATED="1223363998250" ID="ID_734572828" MODIFIED="1223364002328" TEXT="fd_set"/>
<node CREATED="1223364002781" ID="ID_568387725" MODIFIED="1223364006296" TEXT="listen">
<node CREATED="1223364006968" ID="ID_715246059" MODIFIED="1223364016875" TEXT="readable -&gt; accept should work"/>
</node>
</node>
<node CREATED="1223841949531" ID="ID_423667506" MODIFIED="1223842384625" TEXT="problem with fullduplex">
<arrowlink COLOR="#ff0000" DESTINATION="ID_1045639343" ENDARROW="Default" ENDINCLINATION="24;-28;" ID="Arrow_ID_820822155" STARTARROW="Default" STARTINCLINATION="-17;20;"/>
<icon BUILTIN="yes"/>
<node CREATED="1223841964250" ID="ID_1918666974" MODIFIED="1223842026375">
<richcontent TYPE="NODE"><html>
  <head>
    
  </head>
  <body>
    <p>
      DSPu_SOCKETinput expects
    </p>
    <p>
      full buffer from socket and
    </p>
    <p>
      blocks processing until it gets it
    </p>
  </body>
</html></richcontent>
</node>
<node CREATED="1223842029468" ID="ID_1751559352" MODIFIED="1223842301750">
<richcontent TYPE="NODE"><html>
  <head>
    
  </head>
  <body>
    <p>
      DSPu_SOCKEToutput cannot send
    </p>
    <p>
      data until they are generated, but
    </p>
    <p>
      DSPu_SOCKET_input blocks processing
    </p>
  </body>
</html></richcontent>
</node>
<node CREATED="1223843018484" ID="ID_1464043411" MODIFIED="1223843021156" TEXT="solution">
<node CREATED="1223843023187" ID="ID_1759761661" MODIFIED="1223843104296">
<richcontent TYPE="NODE"><html>
  <head>
    
  </head>
  <body>
    <p>
      output sends some zero data
    </p>
    <p>
      on connection start
    </p>
  </body>
</html></richcontent>
</node>
<node CREATED="1223843072468" ID="ID_978078108" MODIFIED="1223843099796">
<richcontent TYPE="NODE"><html>
  <head>
    
  </head>
  <body>
    <p>
      input simulates that it has
    </p>
    <p>
      received some zero data
    </p>
    <p>
      on connection start
    </p>
  </body>
</html></richcontent>
</node>
</node>
</node>
</node>
</node>
<node CREATED="1223361632718" ID="ID_457945349" MODIFIED="1223361639078" TEXT="update documentation">
<node CREATED="1223373258796" ID="ID_1045639343" MODIFIED="1223842384625">
<richcontent TYPE="NODE"><html>
  <head>
    
  </head>
  <body>
    <p>
      blocks client if server stops sending data&#160;
    </p>
    <p>
      but connection is not closed
    </p>
  </body>
</html></richcontent>
<node CREATED="1223373290812" ID="ID_703644541" MODIFIED="1223373297218" TEXT="OK waits for data"/>
<node CREATED="1223373297687" ID="ID_361740704" MODIFIED="1223373331093">
<richcontent TYPE="NODE"><html>
  <head>
    
  </head>
  <body>
    <p>
      server must close connection
    </p>
    <p>
      if&#160;no more data will be send
    </p>
  </body>
</html></richcontent>
</node>
<node CREATED="1223373336859" ID="ID_1624680180" MODIFIED="1223373358718">
<richcontent TYPE="NODE"><html>
  <head>
    
  </head>
  <body>
    <p>
      ?? some form of timeout
    </p>
    <p>
      should be implemented
    </p>
  </body>
</html></richcontent>
</node>
</node>
<node CREATED="1223375281218" ID="ID_1210585109" MODIFIED="1223375282453" TEXT="DSP_FILE_READING_NOT_STARTED"/>
<node CREATED="1224707993296" ID="ID_618289538" MODIFIED="1224707999218" TEXT="linking">
<node CREATED="1224707999953" ID="ID_756377077" MODIFIED="1224708027093">
<richcontent TYPE="NODE"><html>
  <head>
    
  </head>
  <body>
    <p>
      DSPsupport library
    </p>
  </body>
</html></richcontent>
<node CREATED="1224708028984" ID="ID_205784402" MODIFIED="1224708052125">
<richcontent TYPE="NODE"><html>
  <head>
    
  </head>
  <body>
    <p>
      wersj&#281; debug nale&#380;y&#160;linkowa&#263;
    </p>
    <p>
      jedynie z wesj&#261; debug DSPlib
    </p>
  </body>
</html></richcontent>
<node CREATED="1224708053453" ID="ID_1561064565" MODIFIED="1224708085140">
<richcontent TYPE="NODE"><html>
  <head>
    
  </head>
  <body>
    <p>
      polega na dost&#281;pno&#347;ci funkcji i zmiennych
    </p>
    <p>
      wyst&#281;puj&#261;cych tylko we wersji debug DSPlib
    </p>
  </body>
</html></richcontent>
</node>
</node>
</node>
</node>
<node CREATED="1224935939500" ID="ID_1474065963" MODIFIED="1224935948843" TEXT="DOTplot support"/>
<node CREATED="1224935951531" ID="ID_1918262626" MODIFIED="1224935960531" TEXT="helper classes">
<node CREATED="1224935961453" ID="ID_1011862532" MODIFIED="1224935964468" TEXT="DSP_rand"/>
</node>
<node CREATED="1224935999109" ID="ID_1243745496" MODIFIED="1224936002218" TEXT="socket examples"/>
<node CREATED="1225029976500" ID="ID_841287825" MODIFIED="1225029985953" TEXT="Opis ob&#x142;ugi Makr"/>
</node>
<node CREATED="1223361792234" ID="ID_296646138" MODIFIED="1223584388906" TEXT="agenda">
<node CREATED="1223364104531" ID="ID_986057454" MODIFIED="1223843418031" TEXT="misc">
<node CREATED="1223364064078" ID="ID_1793941236" MODIFIED="1223367721125" TEXT="use ioctlsocket instead of recv for checking buffer data length">
<icon BUILTIN="button_ok"/>
</node>
<node CREATED="1223367743046" ID="ID_1535537116" MODIFIED="1223373391484" TEXT="if no data ready, check if socket is still open">
<icon BUILTIN="button_ok"/>
</node>
<node CREATED="1223372529843" ID="ID_1118466504" MODIFIED="1223375543203" TEXT="implement GetBytesRead">
<icon BUILTIN="button_ok"/>
</node>
<node CREATED="1223843420015" ID="ID_1989620383" MODIFIED="1223843448828" TEXT="Determine maximum packet data length">
<node CREATED="1223843450640" ID="ID_418078990" MODIFIED="1223843457281" TEXT="used 1024"/>
<node CREATED="1223843462890" ID="ID_1490546953" MODIFIED="1223843497437">
<richcontent TYPE="NODE"><html>
  <head>
    
  </head>
  <body>
    <p>
      allow user to define packet data length
    </p>
    <p>
      (for each socket separately)
    </p>
  </body>
</html></richcontent>
</node>
</node>
</node>
<node CREATED="1223361796562" ID="ID_1275324894" MODIFIED="1223363315609" TEXT="user decides if client or server is used">
<icon BUILTIN="button_ok"/>
</node>
<node CREATED="1223363525468" ID="ID_1041351699" MODIFIED="1223583644890" TEXT="single server listen socket instance">
<icon BUILTIN="button_ok"/>
</node>
<node CREATED="1223363324546" ID="ID_1306490426" MODIFIED="1223585894343" TEXT="send connection ID and user data">
<icon BUILTIN="button_ok"/>
<node CREATED="1223363374171" FOLDED="true" ID="ID_1018913776" MODIFIED="1223586056312" TEXT="design header data">
<icon BUILTIN="button_ok"/>
<node CREATED="1223585914875" ID="ID_889572659" MODIFIED="1223585922046" TEXT="just server object ID"/>
</node>
<node CREATED="1223585943968" ID="ID_1114603357" MODIFIED="1223586054671" TEXT="read user data on connection initialization">
<icon BUILTIN="yes"/>
<node CREATED="1223585998187" ID="ID_790313204" MODIFIED="1223586023406" TEXT="call function of the connection server object"/>
<node CREATED="1223585979343" ID="ID_1669397267" MODIFIED="1223585996750" TEXT="DSP_socket::ReadUserData"/>
<node CREATED="1223586028468" ID="ID_324396052" MODIFIED="1223586043484" TEXT="DSP_socket object manages user data format"/>
</node>
<node CREATED="1223363396093" ID="ID_1176033354" MODIFIED="1223585902406" TEXT="read header data on connection accept">
<icon BUILTIN="button_ok"/>
</node>
<node CREATED="1223363423203" ID="ID_1943651743" MODIFIED="1223585908265" TEXT="assign socket to objects based on header data">
<icon BUILTIN="button_ok"/>
</node>
</node>
<node CREATED="1223361871843" FOLDED="true" ID="ID_1462742783" MODIFIED="1223841923968" TEXT="connection accept outside of constructor">
<icon BUILTIN="button_ok"/>
<node CREATED="1223361908328" ID="ID_690661851" MODIFIED="1223583469421" TEXT="list of server objects ">
<icon BUILTIN="button_ok"/>
</node>
<node CREATED="1223578335031" ID="ID_515779890" MODIFIED="1223583620718" TEXT="increase listen queue">
<icon BUILTIN="button_ok"/>
</node>
</node>
<node CREATED="1223361988343" ID="ID_958737342" MODIFIED="1223585889734" TEXT="establish client connection outside of constructor">
<icon BUILTIN="button_ok"/>
<node CREATED="1223670170937" ID="ID_675889362" MODIFIED="1223715158734">
<richcontent TYPE="NODE"><html>
  <head>
    
  </head>
  <body>
    <p>
      TryConnect fails if server
    </p>
    <p>
      wasn't started first
    </p>
  </body>
</html></richcontent>
<icon BUILTIN="button_ok"/>
</node>
<node CREATED="1223670220015" FOLDED="true" ID="ID_177037186" MODIFIED="1223715153000" TEXT="connect on non blocking socket">
<icon BUILTIN="button_ok"/>
<node CREATED="1223670229671" ID="ID_675042680" MODIFIED="1223713305875" TEXT="check if it is really non-blocking">
<icon BUILTIN="button_ok"/>
</node>
<node CREATED="1223670462843" ID="ID_1770417725" MODIFIED="1223715135609">
<richcontent TYPE="NODE"><html>
  <head>
    
  </head>
  <body>
    <p>
      WSAGetLastError returns WSAEWOULDBLOCK
    </p>
  </body>
</html></richcontent>
<icon BUILTIN="button_ok"/>
</node>
<node CREATED="1223670549562" FOLDED="true" ID="ID_1937662103" MODIFIED="1223715132421">
<richcontent TYPE="NODE"><html>
  <head>
    
  </head>
  <body>
    <p>
      Until the connection attempt completes on a nonblocking socket,
    </p>
    <p>
      all subsequent calls to connect on the same socket will fail with
    </p>
    <p>
      the error code WSAEALREADY, and WSAEISCONN when the
    </p>
    <p>
      connection completes successfully
    </p>
  </body>
</html></richcontent>
<icon BUILTIN="button_ok"/>
<node CREATED="1223670625078" ID="ID_1757951168" MODIFIED="1223670629406">
<richcontent TYPE="NODE"><html>
  <head>
    
  </head>
  <body>
    <p>
      Due to ambiguities in version 1.1 of the Windows Sockets specification, error codes returned from connect while a connection is already pending may vary among implementations. As a result, it is not recommended that applications use multiple calls to connect to detect connection completion. If they do, they must be prepared to handle WSAEINVAL and WSAEWOULDBLOCK error values the same way that they handle WSAEALREADY, to assure robust operation.
    </p>
  </body>
</html></richcontent>
</node>
<node CREATED="1223670669250" ID="ID_309677090" MODIFIED="1223670672218">
<richcontent TYPE="NODE"><html>
  <head>
    
  </head>
  <body>
    <p>
      If the error code returned indicates the connection attempt failed (that is, WSAECONNREFUSED, WSAENETUNREACH, WSAETIMEDOUT) the application can call connect again for the same socket.
    </p>
  </body>
</html></richcontent>
</node>
</node>
<node CREATED="1223670360296" ID="ID_1727305881" MODIFIED="1223715147093">
<richcontent TYPE="NODE"><html>
  <head>
    
  </head>
  <body>
    <p>
      writefds in select
    </p>
    <p>
      on success
    </p>
  </body>
</html></richcontent>
<icon BUILTIN="button_ok"/>
</node>
<node CREATED="1223670385218" ID="ID_1111170462" MODIFIED="1223715140453">
<richcontent TYPE="NODE"><html>
  <head>
    
  </head>
  <body>
    <p>
      exceptfds in select&#160;
    </p>
    <p>
      if failed
    </p>
  </body>
</html></richcontent>
<icon BUILTIN="button_ok"/>
</node>
</node>
<node CREATED="1223750689953" ID="ID_1197985990" MODIFIED="1223750700343" TEXT="list of client objects">
<node CREATED="1223750708531" ID="ID_1304772902" MODIFIED="1223750743625" TEXT="TryConnectAll for all client objects">
<icon BUILTIN="yes"/>
</node>
</node>
<node CREATED="1223670246437" ID="ID_1829873257" MODIFIED="1223750688687" TEXT="connection fail support">
<node CREATED="1223670252171" ID="ID_630493451" MODIFIED="1223670259671" TEXT="should be socket closed"/>
<node CREATED="1223670260109" ID="ID_1560366765" MODIFIED="1223715181500">
<richcontent TYPE="NODE"><html>
  <head>
    
  </head>
  <body>
    <p>
      socker recreation if the ConnectSocket
    </p>
    <p>
      is the INVALID_SOCKET
    </p>
  </body>
</html></richcontent>
</node>
</node>
</node>
<node CREATED="1223362292265" ID="ID_601961304" MODIFIED="1223362384453" TEXT="establishing connections">
<node CREATED="1223362064968" FOLDED="true" ID="ID_1355030053" MODIFIED="1223584397015" TEXT="turn on listen">
<icon BUILTIN="button_ok"/>
<node CREATED="1223362426937" ID="ID_1454296626" MODIFIED="1223362438906" TEXT="in first server object constructor"/>
</node>
<node CREATED="1223362442031" FOLDED="true" ID="ID_346100201" MODIFIED="1223750758109" TEXT="define all connections">
<icon BUILTIN="button_ok"/>
<node CREATED="1223362457859" ID="ID_1323851264" MODIFIED="1223362468343" TEXT="in sever object constructors"/>
<node CREATED="1223362468953" ID="ID_1266895239" MODIFIED="1223362474687" TEXT="in client object constructors"/>
</node>
<node CREATED="1223362072234" FOLDED="true" ID="ID_1451325575" MODIFIED="1223750795015" TEXT="try connect">
<icon BUILTIN="button_ok"/>
<node CREATED="1223362483937" ID="ID_1295890267" MODIFIED="1223362497781" TEXT="fo client connections"/>
<node CREATED="1223362272531" ID="ID_1856042142" MODIFIED="1223750772234" TEXT="WSAEventSelect ">
<icon BUILTIN="closed"/>
</node>
</node>
<node CREATED="1223362080343" FOLDED="true" ID="ID_1580635218" MODIFIED="1223750784078" TEXT="accept connections">
<icon BUILTIN="button_ok"/>
<node CREATED="1223362502171" ID="ID_1169512668" MODIFIED="1223362509531" TEXT="for server connections"/>
<node CREATED="1223362276828" ID="ID_1388356029" MODIFIED="1223750767890" TEXT="WSAEventSelect ">
<icon BUILTIN="closed"/>
</node>
</node>
<node CREATED="1223362134937" ID="ID_1072744176" MODIFIED="1223750777718" TEXT="use DSPf_Sleep if neccessary">
<icon BUILTIN="button_ok"/>
</node>
<node CREATED="1223362091484" ID="ID_990935044" MODIFIED="1223362532828">
<richcontent TYPE="NODE"><html>
  <head>
    
  </head>
  <body>
    <p>
      repeat all
    </p>
    <p>
      untill all connections are established
    </p>
  </body>
</html></richcontent>
</node>
</node>
<node CREATED="1223374167343" ID="ID_440032120" MODIFIED="1223374177843" TEXT="manage sampling rates">
<node CREATED="1223374156640" ID="ID_1410683545" MODIFIED="1223374163515" TEXT="SetSamplingRate">
<node CREATED="1223374182515" ID="ID_1914319233" MODIFIED="1223374186421" TEXT="outgoing socket"/>
</node>
<node CREATED="1223374187812" ID="ID_477459068" MODIFIED="1223374192640" TEXT="GetSamplingRate">
<node CREATED="1223374192640" ID="ID_1872291660" MODIFIED="1223374198328" TEXT="incoming socket"/>
<node CREATED="1223374198906" ID="ID_1791276189" MODIFIED="1223374203734" TEXT="outgoing socket">
<node CREATED="1223374204984" ID="ID_1525145283" MODIFIED="1223374226875" TEXT="read what has been set"/>
</node>
</node>
</node>
</node>
<node CREATED="1223540542687" ID="ID_822421217" MODIFIED="1223540547562" TEXT="DSPu_SOCKETinput"/>
<node CREATED="1223540517078" ID="ID_297769531" MODIFIED="1223540538015" TEXT="DSPu_SOCKEToutput">
<node CREATED="1223540549343" ID="ID_181568853" MODIFIED="1223540626421">
<richcontent TYPE="NODE"><html>
  <head>
    
  </head>
  <body>
    <p>
      one object should be able to&#160;
    </p>
    <p>
      send data to multiple sockets
    </p>
  </body>
</html></richcontent>
</node>
<node CREATED="1223540633484" ID="ID_1253787126" MODIFIED="1223540743265" TEXT="realtime socket addition"/>
<node CREATED="1223540752187" ID="ID_894707649" MODIFIED="1223540821296">
<richcontent TYPE="NODE"><html>
  <head>
    
  </head>
  <body>
    <p>
      ? force algorthim sampling rate
    </p>
    <p>
      when no output to synchronized
    </p>
    <p>
      block works
    </p>
  </body>
</html></richcontent>
<node CREATED="1223540822468" ID="ID_108756501" MODIFIED="1223540829953" TEXT="synchronized block">
<node CREATED="1223540831078" ID="ID_354739203" MODIFIED="1223540839468" TEXT="DSPu_AudioInput"/>
<node CREATED="1223540839796" ID="ID_488327903" MODIFIED="1223540844359" TEXT="DSPu_AudioOutput"/>
<node CREATED="1223540844937" ID="ID_1867450105" MODIFIED="1223540852046" TEXT="DSPu_SOCKETinput">
<node CREATED="1223540860093" ID="ID_372809878" MODIFIED="1223540890234">
<richcontent TYPE="NODE"><html>
  <head>
    
  </head>
  <body>
    <p>
      only if peer works
    </p>
    <p>
      with synchronized block
    </p>
  </body>
</html></richcontent>
</node>
</node>
<node CREATED="1223540852765" ID="ID_1301661533" MODIFIED="1223540857203" TEXT="DSPu_SOCKEToutput">
<node CREATED="1223540860093" ID="ID_1153732871" MODIFIED="1223540890234">
<richcontent TYPE="NODE"><html>
  <head>
    
  </head>
  <body>
    <p>
      only if peer works
    </p>
    <p>
      with synchronized block
    </p>
  </body>
</html></richcontent>
</node>
</node>
</node>
</node>
</node>
</node>
<node CREATED="1223311556109" ID="ID_94893781" MODIFIED="1223311573187" POSITION="right" TEXT="separate libraries">
<node CREATED="1223311573718" ID="ID_1326677718" MODIFIED="1223311576625" TEXT="DSP"/>
<node CREATED="1223311586015" ID="ID_1850177595" MODIFIED="1223311590218" TEXT="DSPaudio"/>
<node CREATED="1223311577453" ID="ID_1122794491" MODIFIED="1223321416171" TEXT="DSPsocket">
<icon BUILTIN="button_ok"/>
</node>
</node>
<node CREATED="1223370837437" ID="ID_83146239" MODIFIED="1223370840015" POSITION="left" TEXT="MORSE">
<node CREATED="1223370847250" ID="ID_1495175985" MODIFIED="1223370857703" TEXT="problem chyba z konfiguracj&#x105;">
<node CREATED="1223370859312" ID="ID_1980647390" MODIFIED="1223370940484">
<richcontent TYPE="NODE"><html>
  <head>
    
  </head>
  <body>
    <p>
      bez powodu resetuje do default
    </p>
    <p>
      i wyk&#322;ada si&#281; potem - dla min dot length &lt; default
    </p>
    <p>
      (prawdopodobnie stosuje w
    </p>
    <p>
      przetwarzaniu d&#322;u&#380;sze bufory
    </p>
    <p>
      ni&#380; jest to zaalokowane)
    </p>
  </body>
</html></richcontent>
</node>
<node CREATED="1223370942187" ID="ID_1116194355" MODIFIED="1223370978421">
<richcontent TYPE="NODE"><html>
  <head>
    
  </head>
  <body>
    <p>
      przyda&#322;aby si&#281; fitlacja do pasma
    </p>
    <p>
      poszukiwa&#324; (opcjonalnie)
    </p>
  </body>
</html></richcontent>
<node CREATED="1223370979171" ID="ID_66093080" MODIFIED="1223370996078">
<richcontent TYPE="NODE"><html>
  <head>
    
  </head>
  <body>
    <p>
      przynajmniej usuwanie sk&#322;adowej
    </p>
    <p>
      sta&#322;ej oraz Nyquistowskiej
    </p>
  </body>
</html></richcontent>
</node>
</node>
</node>
</node>
</node>
</map>
