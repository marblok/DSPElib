path = "c:\Program Files (x86)\Graphviz2.38\bin\";%path%

dot -Tgif macro_DDS.dot -omacro_DDS.gif
dot -Tgif macro_unwraped.dot -omacro_unwraped.gif
dot -Tgif macro_wraped.dot -omacro_wraped.gif
dot -Tgif asynchronous.dot -oasynchronous.gif
dot -Tgif socket_server_2.dot -osocket_server_2.gif
dot -Tgif socket_client_2.dot -osocket_client_2.gif
dot -Tgif callbacks_scheme.dot -ocallbacks_scheme.gif
dot -Tgif hello.dot -ohello.gif
dot -Tgif echo.dot -oecho.gif
dot -Tgif captured_sample.dot -ocaptured_sample.gif
dot -Tgif multirate.dot -omultirate.gif
