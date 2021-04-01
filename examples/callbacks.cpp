/*! Simple Digital Signal Processing Engine usage example.
 * \author Marek Blok
 * \date 2008.05.28
 * \date updated 2021.04.01
 */
#include <memory>

#include <DSP_lib.h>

#define buffer_size 4
DSP::Float_vector read_buffer;

void BufferCallback(unsigned int NoOfInputs, unsigned int NoOfOutputs, DSP::Float_vector &OutputSamples, DSP::void_ptr *UserDataPtr, unsigned int UserDefinedIdentifier, DSP::Component_ptr Caller)
{
  if (NoOfInputs == DSP::c::Callback_Init)
  {
    read_buffer.resize(buffer_size);
    return;
  }
  if (NoOfInputs == DSP::c::Callback_Delete)
  {
    read_buffer.clear();
    return;
  }

  DSPu_OutputBuffer *dsp_buffer;
  int ind, counter;

  dsp_buffer = (DSPu_OutputBuffer *)Caller->Convert2Block();
  counter = dsp_buffer->ReadBuffer(read_buffer.data(),
                                   buffer_size*sizeof(DSP::Float), // read all samples
                                   -2,  // reset only NotificationsStep slots in buffer block
                                   DSP::e::SampleType::ST_float); // write to read_buffer in float format

  for (ind = 0; ind < counter; ind++)
  {
    switch (UserDefinedIdentifier)
    {
      case 0:
        OutputSamples[ind] = read_buffer[ind];
        break;
      default:
        if ((ind % 2) == 0)
          OutputSamples[ind] = read_buffer[ind];
        else
          OutputSamples[ind] = -read_buffer[ind];
        break;
    }
  }
  for (ind = counter; ind < buffer_size; ind++)
    OutputSamples[ind] = 0.0;
}

int main(void)
{
  DSP::Clock_ptr MasterClock, BufferClock, MuxClock, DemuxClock;
  int temp;
  long int Fp;
  int callback_type;

  std::shared_ptr<DSPu_WaveInput>     AudioIn;
  std::shared_ptr<DSPu_OutputBuffer>  OutputBuffer;
  std::shared_ptr<DSPu_Multiplexer>   Multiplexer;
  std::shared_ptr<DSPu_AudioOutput>   AudioOut;
  std::shared_ptr<DSPu_Demultiplexer> Demultiplexer;
  std::shared_ptr<DSPu_Amplifier>     Scale;
  std::shared_ptr<DSPu_Multiplexer>   Multiplexer2;

  DSP::log.SetLogState(DSP::E_LS_Mode::LS_console | DSP::E_LS_Mode::LS_file);
  DSP::log.SetLogFileName("log_file.log");

  DSP::log << DSP_lib_version_string() << endl;

  MasterClock=DSP::Clock::CreateMasterClock();


  AudioIn = std::make_shared<DSPu_WaveInput>(MasterClock, "DSPElib.wav", ".");
  Fp = AudioIn->GetSamplingRate();

  //callback_type = 0; // just copy samples
  callback_type = 1; // inverse spectrum
  OutputBuffer = std::make_shared<DSPu_OutputBuffer>(buffer_size,
                              1,
                              DSP_standard,
                              MasterClock,
                              -1,
                              buffer_size,
                              BufferCallback,
                              callback_type);
  BufferClock = OutputBuffer->GetOutputClock();

  Multiplexer = std::make_shared<DSPu_Multiplexer> (BufferClock, false, buffer_size);
  MuxClock = Multiplexer->GetOutputClock();

  Demultiplexer = std::make_shared<DSPu_Demultiplexer>(false, 2);
  DemuxClock = DSP::Clock::GetClock(MuxClock, 1,2);

  Scale = std::make_shared<DSPu_Amplifier>(-1.0, 1);
  Multiplexer2 = std::make_shared<DSPu_Multiplexer>(DemuxClock, false, 2);

  AudioOut = std::make_shared<DSPu_AudioOutput>(Fp);


  AudioIn->Output("out") >> OutputBuffer->Input("in");
  OutputBuffer->Output("out") >> Multiplexer->Input("in");
  Multiplexer->Output("out") >> Demultiplexer->Input("in");

  Demultiplexer->Output("out1") >> Multiplexer2->Input("in1");
  Demultiplexer->Output("out2") >> Scale->Input("in");
  Scale->Output("out") >> Multiplexer2->Input("in2");

  Multiplexer2->Output("out") >> AudioOut->Input("in");

  DSP::Component::CheckInputsOfAllComponents();
  DSP::Clock::SchemeToDOTfile(MasterClock, "callbacks_scheme.dot");

  temp=1;
  do
  {
    DSP::Clock::Execute(MasterClock, Fp/8);

    DSP::log << "MAIN" << DSP::LogMode::second << temp << endl;
    temp++;
  }
  while (AudioIn->GetBytesRead() != 0);
  // process a bit more so the buffered samples are also sent to output
  DSP::Clock::Execute(MasterClock, Fp/8);

  AudioOut.reset();
  Multiplexer2.reset();
  Scale.reset();
  Demultiplexer.reset();
  OutputBuffer.reset();
  Multiplexer.reset();
  AudioIn.reset();

  DSP::Clock::ListOfAllComponents();
  DSP::Clock::FreeClocks();
  DSP::log << "MAIN" << DSP::LogMode::second << "end" << endl;

  return 0;
}
