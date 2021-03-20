/*! \file DSPclocks.cpp
 * This is DSP engine clocks management module main file.
 *
 * \author Marek Blok
 */

#include <sstream>

#include <DSP_clocks.h>
#include <DSP_lib.h>

#ifdef WIN32
  #include <windef.h>
  #include <stdarg.h>
  #include <winbase.h>
//#else
//  #error NO WIN32
#endif

//***************************************************//
//  DSP::Clock
//***************************************************//
DSP::Clock_ptr *DSP::Clock::First=NULL;
unsigned int  *DSP::Clock::NoOfClocks=NULL;

unsigned int   DSP::Clock::NoOfMasterClocks=0;
DSP::Clock_ptr *DSP::Clock::MasterClocks=NULL;

unsigned long *DSP::Clock::global_cycle_lengths=NULL;
unsigned long *DSP::Clock::current_discrete_times=NULL;
unsigned long *DSP::Clock::global_discrete_times_to_next_step=NULL;
DSP::Clock_ptr* *DSP::Clock::ActiveClocksList=NULL;
int *DSP::Clock::ActiveClocksListLength=NULL; //0;
DSP::Clock_ptr* *DSP::Clock::SignalActivatedClocksList=NULL;
unsigned long * *DSP::Clock::SignalActivatedClocks_cycles_List = NULL;
int *DSP::Clock::SignalActivatedClocksListLength=NULL; //0;
volatile bool *DSP::Clock::Terminated=NULL; //false;
volatile bool *DSP::Clock::InputNeedsMoreTime=NULL; //false;


/*! \bug <b>2006.07.25</b> If there is free slot in MasterClocks
 *    list (NULL) it should be used instead of always adding
 *    new entry at the end.
 */
DSP::Clock_ptr DSP::Clock::CreateMasterClock(void)
{
  DSP::Clock_ptr NewClock;
  unsigned long *gcl;
  unsigned long *cdt;
  unsigned long *gdt2ns;
  DSP::Clock_ptr *MC;
  DSP::Clock_ptr *F;
  unsigned int *NOC;
  DSP::Clock_ptr **ACLs;
  int *ACLL;
  DSP::Clock_ptr **SACL;
  unsigned long int **SACcL;
  int *SACLL;
  volatile bool *Term;
  volatile bool *NeedTime;
  unsigned int ind;

  unsigned int new_master_clock_index;

  // check if there is MasterClock slot available
  new_master_clock_index = NoOfMasterClocks;
  for (ind=0; ind < NoOfMasterClocks; ind++)
    if (MasterClocks[ind] == NULL)
    {
      new_master_clock_index = ind;
      break;
    }

  if (new_master_clock_index == NoOfMasterClocks)
  { // new slot for master clock needed
    gcl=global_cycle_lengths;
    global_cycle_lengths = new unsigned long[NoOfMasterClocks+1];
    cdt=current_discrete_times;
    current_discrete_times = new unsigned long[NoOfMasterClocks+1];
    gdt2ns = global_discrete_times_to_next_step;
    global_discrete_times_to_next_step = new unsigned long[NoOfMasterClocks+1];
    MC=MasterClocks;
    MasterClocks = new DSP::Clock_ptr[NoOfMasterClocks+1];
    F=First;
    First = new DSP::Clock_ptr[NoOfMasterClocks+1];
    NOC=NoOfClocks;
    NoOfClocks = new unsigned int[NoOfMasterClocks+1];
    ACLs = ActiveClocksList;
    ActiveClocksList = new DSP::Clock_ptr *[NoOfMasterClocks+1];
    ACLL = ActiveClocksListLength;
    ActiveClocksListLength=new int[NoOfMasterClocks+1];
    SACL = SignalActivatedClocksList;
    SignalActivatedClocksList = new DSP::Clock_ptr *[NoOfMasterClocks+1];
    SACcL = SignalActivatedClocks_cycles_List;
    SignalActivatedClocks_cycles_List = new unsigned long *[NoOfMasterClocks+1];
    SACLL = SignalActivatedClocksListLength;
    SignalActivatedClocksListLength=new int[NoOfMasterClocks+1];
    Term=Terminated;
    Terminated=new bool[NoOfMasterClocks+1];
    NeedTime=InputNeedsMoreTime;
    InputNeedsMoreTime=new bool[NoOfMasterClocks+1];

    for (ind=0; ind<NoOfMasterClocks; ind++)
    {
      global_cycle_lengths[ind]=gcl[ind];
      current_discrete_times[ind]=cdt[ind];
      global_discrete_times_to_next_step[ind] = gdt2ns[ind];
      MasterClocks[ind]=MC[ind];
      First[ind]=F[ind];
      NoOfClocks[ind]=NOC[ind];
      ActiveClocksList[ind]=ACLs[ind];
      ActiveClocksListLength[ind]=ACLL[ind];
      SignalActivatedClocksList[ind]=SACL[ind];
      SignalActivatedClocks_cycles_List[ind] = SACcL[ind];
      SignalActivatedClocksListLength[ind]=SACLL[ind];
      Terminated[ind]=Term[ind];
      InputNeedsMoreTime[ind]=NeedTime[ind];
    }
    global_cycle_lengths[NoOfMasterClocks]=1;
    current_discrete_times[NoOfMasterClocks]=0;
    global_discrete_times_to_next_step[NoOfMasterClocks] = 0;
    MasterClocks[NoOfMasterClocks]=NULL;
    First[NoOfMasterClocks]=NULL;
    NoOfClocks[ind]=0;
    ActiveClocksList[NoOfMasterClocks]=NULL;
    ActiveClocksListLength[NoOfMasterClocks]=0;

  //  SignalActivatedClocksList[NoOfMasterClocks]= NULL;
    SignalActivatedClocksList[NoOfMasterClocks]= new DSP::Clock_ptr [MAX_NO_OF_SIGNAL_ACTIVATED_CLOCKS];
  //  SignalActivatedClocks_cycles_List[NoOfMasterClocks]=0;
    SignalActivatedClocks_cycles_List[NoOfMasterClocks]= new unsigned long [MAX_NO_OF_SIGNAL_ACTIVATED_CLOCKS];
    SignalActivatedClocksListLength[NoOfMasterClocks]=0;

    Terminated[NoOfMasterClocks]=true; // mark last cycle in execute as terminated
    InputNeedsMoreTime[NoOfMasterClocks]=false;

    NoOfMasterClocks++;
    NewClock = new DSP::Clock;
    NewClock->cycle_length=1;
  //  MasterClocks[NoOfMasterClocks-1]=NewClock;
  //  MasterClocks[NoOfMasterClocks-1]->MasterClockIndex=NoOfMasterClocks; //point to itself

    if (gcl != NULL)
      delete [] gcl;
    if (cdt != NULL)
      delete [] cdt;
    if (gdt2ns != NULL)
      delete [] gdt2ns;
    if (MC != NULL)
      delete [] MC;
    if (F != NULL)
      delete [] F;
    if (NOC != NULL)
      delete [] NOC;
    if (ACLs != NULL)
      delete [] ACLs;
    if (ACLL != NULL)
      delete [] ACLL;
    if (SACL != NULL)
      delete [] SACL;
    if (SACcL != NULL)
    	delete [] SACcL;
    if (SACLL != NULL)
    if (Term != NULL)
      delete [] Term;
    if (NeedTime != NULL)
      delete [] NeedTime;
  }
  else
  { // reuse existing slot for master clock
    global_cycle_lengths[new_master_clock_index]=1;
    current_discrete_times[new_master_clock_index]=0;
    global_discrete_times_to_next_step[new_master_clock_index] = 0;
    MasterClocks[new_master_clock_index]=NULL;
    First[new_master_clock_index]=NULL;
    NoOfClocks[new_master_clock_index]=0;
    ActiveClocksList[new_master_clock_index]=NULL;
    ActiveClocksListLength[new_master_clock_index]=0;

  //  SignalActivatedClocksList[NoOfMasterClocks]= NULL;
    SignalActivatedClocksList[new_master_clock_index]= new DSP::Clock_ptr [MAX_NO_OF_SIGNAL_ACTIVATED_CLOCKS];
  //  SignalActivatedClocks_cycles_List[NoOfMasterClocks]=0;
    SignalActivatedClocks_cycles_List[new_master_clock_index]= new unsigned long [MAX_NO_OF_SIGNAL_ACTIVATED_CLOCKS];
    SignalActivatedClocksListLength[new_master_clock_index]=0;

    Terminated[new_master_clock_index]=true; // mark last cycle in execute as terminated
    InputNeedsMoreTime[new_master_clock_index]=false;

    NewClock = new DSP::Clock(new_master_clock_index);
    NewClock->cycle_length=1;
  }

  return NewClock;
}

void DSP::Clock::SetAsNewMasterClock(DSP::Clock_ptr new_master)
{
  if (MasterClocks[new_master->MasterClockIndex] != NULL)
  {
    #ifdef __DEBUG__
      DSP::log << DSP::LogMode::Error << "DSP::Clock::SetAsNewMasterClock" << DSP::LogMode::second << "Cannot set new master clock if the old has not been released" << endl;
    #endif
    return;
  }
  if (First[new_master->MasterClockIndex] == NULL)
  {
    #ifdef __DEBUG__
      DSP::log << DSP::LogMode::Error << "DSP::Clock::SetAsNewMasterClock" << DSP::LogMode::second << "There are no clocks on the list for given master clock !!!" << endl;
    #endif
    return;
  }

  DSP::Clock_ptr current;

  global_cycle_lengths[new_master->MasterClockIndex]=1;
  current_discrete_times[new_master->MasterClockIndex]=0;
  global_discrete_times_to_next_step[new_master->MasterClockIndex] = 0;
  new_master->cycle_length=1;

  MasterClocks[new_master->MasterClockIndex]=new_master;
  NoOfClocks[new_master->MasterClockIndex]=0;
  current = First[new_master->MasterClockIndex];
  while (current != NULL)
  {
    NoOfClocks[new_master->MasterClockIndex]++;
    current = current->Next;
  }

  if (ActiveClocksList[new_master->MasterClockIndex] != NULL)
    delete [] ActiveClocksList[new_master->MasterClockIndex];
  ActiveClocksList[new_master->MasterClockIndex]=
    new DSP::Clock_ptr[NoOfClocks[new_master->MasterClockIndex]];
  ActiveClocksListLength[new_master->MasterClockIndex]=0;

  SignalActivatedClocksList[new_master->MasterClockIndex]=
    new DSP::Clock_ptr [MAX_NO_OF_SIGNAL_ACTIVATED_CLOCKS];
  SignalActivatedClocks_cycles_List[new_master->MasterClockIndex]=
    new unsigned long [MAX_NO_OF_SIGNAL_ACTIVATED_CLOCKS];
  SignalActivatedClocksListLength[new_master->MasterClockIndex]=0;

  // update clocks global_cycle_lengths etc.
  UpdateCycleLengths(new_master);
}

/*! \Fixed <b>2005.10.30</b> Added DSP::Clock::ReleaseMasterClock function which frees memory reserved for master clock.
 *    If removed clock was last also global tables storing MasterClocks parameters are freed
 *    and NoOfMasterClocks is reset to zero.
 *
 * \todo_later check whether all alocated memory is freed
 */
void DSP::Clock::ReleaseMasterClock(unsigned int MasterClockIndex)
{
  unsigned int ind, counter;
//  if ((MasterClockIndex < 0) && (MasterClockIndex >= NoOfMasterClocks))
  if (MasterClockIndex >= NoOfMasterClocks)
    return;

  global_cycle_lengths[MasterClockIndex] = 1;
  current_discrete_times[MasterClockIndex] = 0;
  global_discrete_times_to_next_step[MasterClockIndex] = 0;
  MasterClocks[MasterClockIndex] = NULL;
  First[MasterClockIndex] = NULL;
  NoOfClocks[MasterClockIndex] = 0;
  if (ActiveClocksList[MasterClockIndex] != NULL)
  {
    delete [] ActiveClocksList[MasterClockIndex];
    ActiveClocksList[MasterClockIndex] = NULL;
  }
  ActiveClocksListLength[MasterClockIndex] = 0;

  if (SignalActivatedClocksList[MasterClockIndex] != NULL)
  {
    delete [] SignalActivatedClocksList[MasterClockIndex];
    SignalActivatedClocksList[MasterClockIndex] = NULL;
  }
  if (SignalActivatedClocks_cycles_List[MasterClockIndex] != NULL)
  {
    delete [] SignalActivatedClocks_cycles_List[MasterClockIndex];
    SignalActivatedClocks_cycles_List[MasterClockIndex] = NULL;
  }
  SignalActivatedClocksListLength[MasterClockIndex]=0;

  Terminated[MasterClockIndex] = true;
  InputNeedsMoreTime[MasterClockIndex] = false;


  //check if any master clock left if no erase lists itself and reset NoOfMasterClocks counter
  counter = 0;
  for (ind=0; ind < NoOfMasterClocks; ind++)
  {
    if (MasterClocks[ind] != NULL)
      counter++;
  }
  if (counter == 0)
  {
    NoOfMasterClocks = 0;
    if (global_cycle_lengths != NULL)
    {
      delete [] global_cycle_lengths;
      global_cycle_lengths = NULL;
    }
    if (current_discrete_times != NULL)
    {
      delete [] current_discrete_times;
      current_discrete_times = NULL;
    }
    if (global_discrete_times_to_next_step != NULL)
    {
      delete [] global_discrete_times_to_next_step;
      global_discrete_times_to_next_step = NULL;
    }
    if (MasterClocks != NULL)
    {
      delete [] MasterClocks;
      MasterClocks = NULL;
    }
    if (First != NULL)
    {
      delete [] First;
      First = NULL;
    }
    if (NoOfClocks != NULL)
    {
      delete [] NoOfClocks;
      NoOfClocks = NULL;
    }
    if (ActiveClocksList != NULL)
    {
      delete [] ActiveClocksList;
      ActiveClocksList = NULL;
    }
    if (ActiveClocksListLength != NULL)
    {
      delete [] ActiveClocksListLength;
      ActiveClocksListLength = NULL;
    }

    if (SignalActivatedClocksList != NULL)
    {
      delete [] SignalActivatedClocksList;
      SignalActivatedClocksList = NULL;
    }
    if (SignalActivatedClocks_cycles_List != NULL)
    {
      delete [] SignalActivatedClocks_cycles_List;
      SignalActivatedClocks_cycles_List = NULL;
    }
    if (SignalActivatedClocksListLength != NULL)
    {
      delete [] SignalActivatedClocksListLength;
      SignalActivatedClocksListLength = NULL;
    }

    if (Terminated != NULL)
    {
      delete [] Terminated;
      Terminated = NULL;
    }
    if (InputNeedsMoreTime != NULL)
    {
      delete [] InputNeedsMoreTime;
      InputNeedsMoreTime = NULL;
    }
  }
}

/* Main processing loop ver. 2 (if NoOfCyclesToProcess != 0 <- processing
 * only NoOfCyclesToProcess cycles, otherwise runs in infinitive loop)
 *
 * ReferenceClock - clock to which NoOfCyclesToProcess refers
 */
 /*
  * DONE/OK
  *  - StopAtSample condition should be revised
  *  - NoOfCyclesToProcess should be normalized in ReferenceClock cycles
  *  - Reusing ClockList
  *  - check if ClocksList is stored separately for each Masterclock
  *  - global_discrete_time_to_next_step must be stored with MasterClock
  *   so it's independent of ReferenceClock
  *  - Updating clocks' discrete times should be done not by one but by actual
  *    clock step
  *   OK: DSP::Clock::n is in given clock's cycles
  *  - Is it possible that time for ClockList is greater than given StopAtSample
  *   OK: see above
  *  - Updating clocks' discrete times should be done not by one but by actual
  *    clock step
  *   OK: see above
  *
  *  \note If DSP::Clock::Execute finishes with
  *    Terminated[ReferenceClock->MasterClockIndex] set <b>true</b>
  *    Processing have not been completely finished.
  *
  * returns cycles_to_process variable value
  * \note can be reused if Terminated[ReferenceClock->MasterClockIndex] == false
  *
  * \todo FindNextActiveClocks -> zmodyfikowa� tak �eby korzysta� z wcze�niej
  *   przygotowanych list. Przy tworzeniu nowego zegara, dla danego zegara
  *   macierzystego aktualizowa� list� list zegar�w oraz
  *   global_discrete_times_to_next_step.
  *
  */
unsigned long DSP::Clock::Execute(DSP::Clock_ptr ReferenceClock,
                                 unsigned long NoOfCyclesToProcess,
                                 bool ReferenceClock_is_signal_activated)
{
//  unsigned long n;
  int *ListLength, clock_ind;
  int *SignalActivatedClocksListLength_ptr;
  int tempNo, SourcesDoneNo; //Number of processed sources
  int last_unprocessed_clock_index;
  bool AllDone, tempAllDone;
  DSP::Clock_ptr *ClocksList, tmp_clock;
  unsigned long cycles_to_process;
//  bool *NotifyDone;

  long int timeout_counter=0;

  if (ReferenceClock==NULL)
  {
//    ReferenceClock=GetClock();
    #ifdef __DEBUG__
      DSP::log << DSP::LogMode::Error << "DSP::Clock::Execute"  << DSP::LogMode::second << "NULL ReferenceClock" << endl;
    #endif
    return 0;
  }

  ListLength = &(ActiveClocksListLength[ReferenceClock->MasterClockIndex]);
  ClocksList = ActiveClocksList[ReferenceClock->MasterClockIndex];
  SignalActivatedClocksListLength_ptr =
     &(SignalActivatedClocksListLength[ReferenceClock->MasterClockIndex]);


  if (Terminated[ReferenceClock->MasterClockIndex] == true)
  { // clocks processing initialization
    Terminated[ReferenceClock->MasterClockIndex]=false;

    DSP::Clock::FindNextActiveClocks(
        MasterClocks[ReferenceClock->MasterClockIndex],
        global_discrete_times_to_next_step[ReferenceClock->MasterClockIndex]);

    //reset clocks counter on the list so all the sources
    //we be processed again
    for (clock_ind=0; clock_ind<*ListLength; clock_ind++)
    {
      ClocksList[clock_ind]->CurrentSource=0;
      ClocksList[clock_ind]->NotifyComponents();
    }

    if (NoOfCyclesToProcess==0)
      cycles_to_process = 0;
    else
    {
      cycles_to_process = (ReferenceClock->cycle_length) * NoOfCyclesToProcess;
    }
  }
  else
    cycles_to_process = NoOfCyclesToProcess;

  while (!Terminated[ReferenceClock->MasterClockIndex])
  {
   	// ************************************************** //
   	// * Processing Active Clocks from the current list * //
   	// ************************************************** //

    // tempNo - returned number of processed Sources in current
    //          Execution of ProcessSources
    AllDone=false;
    while (!AllDone)
    {
      AllDone=true; SourcesDoneNo=0;
      if (*ListLength > 0)
      {
        // start processing form the back of the list
        clock_ind = (*ListLength) - 1;
        // process all clocks except of the first on the list
        last_unprocessed_clock_index = 0;
        while (clock_ind > last_unprocessed_clock_index)
        {
          tempAllDone=ClocksList[clock_ind]->ProcessSources(tempNo);
          SourcesDoneNo+=tempNo;
          if (tempAllDone == true)
          { // decrease clocks list length
            //Updating clock's discrete time
            ClocksList[clock_ind]->n++;
            (*ListLength)--;
            clock_ind--; // process next clock
          }
          else
          { // swap this clock  with the first unprocessed_clock clock on the list
            tmp_clock = ClocksList[last_unprocessed_clock_index];
            ClocksList[last_unprocessed_clock_index] = ClocksList[clock_ind];
            ClocksList[clock_ind] = tmp_clock;
            // do not change clock index
            // but remember where to finish
            last_unprocessed_clock_index++;
          }
        }
        // Process last_unprocessed_clock
        tempAllDone = ClocksList[last_unprocessed_clock_index]->ProcessSources(tempNo);
        SourcesDoneNo+=tempNo;
        if (tempAllDone == true)
        { // decrease clocks list length
          //Updating clock's discrete time
          ClocksList[last_unprocessed_clock_index]->n++;
          (*ListLength)--;
          last_unprocessed_clock_index--;
        }

        if ((*ListLength) > 0)
        {
          AllDone = false;

          if (SourcesDoneNo == 0)
          { // problem: not all sources processed
            // but no source ready to be processed
            if (InputNeedsMoreTime[ReferenceClock->MasterClockIndex] == true)
            {
              timeout_counter++;
              if (timeout_counter > MAX_timeout_counter)
                break;

              /*! \todo_later in the future use DSP::f::Sleep(0)
               *  and real time measurement for timeout detection
               */
              DSP::f::Sleep(1); //wait just a bit
              //AllDone=false;
              //and one more go

              #ifdef __DEBUG__
                #ifdef AUDIO_DEBUG_MESSAGES_ON
                  DSP::log << "DSP::Clock::Execute", "Taking a nap and trying again");
                #endif
              #endif
            }
            else
            {
              // ??? there might be signal activated clocks ready to be processed
              if (*SignalActivatedClocksListLength_ptr == 0)
              { // break processing loop - nothing will result from it
                if (ReferenceClock_is_signal_activated == false)
                {
                  #ifdef __DEBUG__
                    DSP::log << "DSP::Clock::Execute" << DSP::LogMode::second << "Not all sources processed but no source is ready" << endl;
                    DSP::log << "DSP::Clock::Execute" << DSP::LogMode::second << ">> Check if there is the feedback loop without DSPu_LoopDelay." << endl;
                    DSP::log << "DSP::Clock::Execute  >>" << DSP::LogMode::second
                      << "Number of signal activated clocks still on the list:" << *SignalActivatedClocksListLength_ptr << endl;
                    for (clock_ind=0; clock_ind<(*ListLength); clock_ind++)
                    {
                      for (unsigned int component_ind=0; component_ind<ClocksList[clock_ind]->NoOfComponents; component_ind++)
                      {
                        DSP::log << "DSP::Clock::Execute  >>" << DSP::LogMode::second
                          << clock_ind << ":" << component_ind << ") NOTIFY: " << ClocksList[clock_ind]->ComponentsNotifications_Table[component_ind]->GetName()
                          << endl;
                      }

                      for (unsigned int sources_ind=0; sources_ind<ClocksList[clock_ind]->NoOfSources; sources_ind++)
                      {
                        DSP::log << "DSP::Clock::Execute  >>" << DSP::LogMode::second
                          << clock_ind << ":" << sources_ind << ") PROCESS: " << ClocksList[clock_ind]->SourcesTable[sources_ind]->GetName()
                          << endl;
                      }
                    }
                  #endif
                }
                return cycles_to_process;
              }
            }
          }
          InputNeedsMoreTime[ReferenceClock->MasterClockIndex]=false;
        } // if ((*ListLength) > 0)

      } // if ((*ListLength) > 0)

      // **************************************************** //
      // * Processing Signal Activated Clocks from the list * //
      // **************************************************** //
      DSP::Clock_ptr *ActivatedClocksList;
      unsigned long *Clocks_cycles_List;
      ActivatedClocksList = SignalActivatedClocksList[ReferenceClock->MasterClockIndex];
      Clocks_cycles_List = SignalActivatedClocks_cycles_List[ReferenceClock->MasterClockIndex];

      clock_ind = (*SignalActivatedClocksListLength_ptr) - 1;
      while (clock_ind >= 0)
      {
        Clocks_cycles_List[clock_ind] = DSP::Clock::Execute(ActivatedClocksList[clock_ind],
                                                           Clocks_cycles_List[clock_ind],
                                                           true);
        if (Terminated[ActivatedClocksList[clock_ind]->MasterClockIndex] == true)
        { // remove from list
          (*SignalActivatedClocksListLength_ptr)--;
          if (clock_ind < (*SignalActivatedClocksListLength_ptr))
          {
            // last clock move here
            ActivatedClocksList[clock_ind] = ActivatedClocksList[*SignalActivatedClocksListLength_ptr];
            Clocks_cycles_List[clock_ind] = Clocks_cycles_List[*SignalActivatedClocksListLength_ptr];
          }
        }

        clock_ind--;
      }
      if ((*SignalActivatedClocksListLength_ptr) > 0)
        AllDone = false;

    } //  while (!AllDone)

    if (AllDone == true)
    {
      // ************************************************** //
      // * Prepare for the next cycle                     * //
      // ************************************************** //

      //detecting loop end
      if (NoOfCyclesToProcess != 0)
      {
        if (cycles_to_process > global_discrete_times_to_next_step[ReferenceClock->MasterClockIndex]) //! \Fixed <b>2008.03.25</b>
        { // still something to do
          cycles_to_process -= global_discrete_times_to_next_step[ReferenceClock->MasterClockIndex];
          //global_discrete_times_to_next_step[ReferenceClock->MasterClockIndex] = 0;

          DSP::Clock::FindNextActiveClocks(
              MasterClocks[ReferenceClock->MasterClockIndex],
              global_discrete_times_to_next_step[ReferenceClock->MasterClockIndex]);

          //reset clocks counter on the list so all the sources
          //we be processed again
          for (clock_ind=0; clock_ind<*ListLength; clock_ind++)
          {
            ClocksList[clock_ind]->CurrentSource=0;
            ClocksList[clock_ind]->NotifyComponents();
          }
        }
        else
        { // OK: everything has been processed
          //global_discrete_times_to_next_step[ReferenceClock->MasterClockIndex] -= cycles_to_process;
          cycles_to_process = 0; //! \Fixed <b>2008.03.25</b>
          Terminated[ReferenceClock->MasterClockIndex]=true;
        }
      }
    } // if (AllDone == true)

  }
  return cycles_to_process;
}

//Przetwarza �r�d�a danego zegara: SourcesTable
// (zwraca false je�eli nie wszystkie mog�y
//  zosta� przetworzone)
bool DSP::Clock::ProcessSources(int &NoOfProcessedSources)
{
  bool SourceReady;
  unsigned int current;
  DSP::Source_ptr tempBlock;

  NoOfProcessedSources=0;
  if (NoOfSources == 0)
    return true; //Nothing to be done

  current=CurrentSource;

  SourceReady=true;
  // attempt to process all sources
  while (current<NoOfSources)
  {
    //if (SourcesTable[current]->EnableProcessing == false)
    if (SourcesTable[current]->OUTPUT_EXECUTE_PTR(SourcesTable[current], this))
    { //source processed
      NoOfProcessedSources++;
      if (current != CurrentSource)
      { // swap processed with last unprocessed
        tempBlock=SourcesTable[current];
        SourcesTable[current]=SourcesTable[CurrentSource];
        SourcesTable[CurrentSource]=tempBlock;
      }
      current++;
      CurrentSource++; //CurrentSource points at first not processed on the list
    }
    else
    {
      SourceReady=false;
      current++;
    }
  }

  if (CurrentSource == NoOfSources)
  {
    return true;
  }
  return SourceReady;
}

/* clock without its MasterClock (will be added as MasterClock
 * this construtor should be called only from CreateMasterClock
 */
DSP::Clock::Clock(void)
{
  ClockInit(NULL, 0, 0);
}

DSP::Clock::Clock(unsigned int new_MasterClockIndex)
{
  ClockInit(NULL, 0, 0, new_MasterClockIndex);
}

DSP::Clock::Clock(DSP::Clock_ptr ReferenceClock)
{
  ClockInit(ReferenceClock, 1, 1);
}

DSP::Clock::Clock(DSP::Clock_ptr ReferenceClock, long int L_in, long int M_in)
{
  ClockInit(ReferenceClock, L_in, M_in);
}

void DSP::Clock::ClockInit(DSP::Clock_ptr ReferenceClock, long int L_in, long int M_in, unsigned int new_MasterClockIndex)
{
  DSP::Clock *current;

  CurrentSource=0;
  n=0; //reseting clock's discrete time

  GlobalSamplingRate = -1.0; // not yet defined
  SamplingRate = -1.0;
  if (ReferenceClock == NULL)
  {
    if (new_MasterClockIndex == UINT_MAX)
      new_MasterClockIndex = NoOfMasterClocks-1;
    MasterClockIndex=new_MasterClockIndex;
    //MasterClockIndex=NoOfMasterClocks-1;  //updated here instead in CreateMasterClock
    MasterClocks[MasterClockIndex]=this;
  }
  else
  {
    MasterClockIndex=ReferenceClock->MasterClockIndex;
  }
  Next=NULL;

  ParentClock = ReferenceClock;
  L=L_in; M=M_in;


  if (First[MasterClockIndex]==NULL)
    First[MasterClockIndex]=this;
  else
  {
    //Find last on the list
    current=First[MasterClockIndex];
    while (current->Next!=NULL)
      current=current->Next;
    current->Next=this;
  }

  NoOfClocks[MasterClockIndex]++;

  if (ActiveClocksList == NULL)
  {
    #ifdef __DEBUG__
      DSP::log << DSP::LogMode::Error << "DSP::Clock::ClockInit" << DSP::LogMode::second << "ActiveClockList not yet defined" << endl;
    #endif
    return;
  }
  if (ActiveClocksList[MasterClockIndex] != NULL)
    delete [] ActiveClocksList[MasterClockIndex];
  ActiveClocksList[MasterClockIndex]=new DSP::Clock_ptr[NoOfClocks[MasterClockIndex]];
  ActiveClocksListLength[MasterClockIndex]=0;

  SourcesTable=NULL;
  NoOfSources=0;
  ComponentsNotifications_Table = NULL;
  NoOfComponents=0;

  if (ReferenceClock != NULL)
    UpdateCycleLengths(MasterClocks[MasterClockIndex]);
  MasterClocks[MasterClockIndex]->UpdateSamplingRates();
}

void DSP::Clock::UpdateGlobalSamplingRate(void)
{
  DSP::Clock_ptr current;
  long double clocks_GlobalSamplingRate = -1;

  current=First[MasterClockIndex];
  while (current != NULL)
  {
    if (current->SamplingRate > 0) {
      current->GlobalSamplingRate = current->cycle_length * current->SamplingRate;
      if (clocks_GlobalSamplingRate < 0) {
        clocks_GlobalSamplingRate = current->GlobalSamplingRate;
      }
      else {
        if (clocks_GlobalSamplingRate != current->GlobalSamplingRate) {
          DSP::log << DSP::LogMode::Error
           << "DSP::Clock::UpdateGlobalSamplingRate: clocks_GlobalSamplingRate = " << clocks_GlobalSamplingRate
           << " but current->GlobalSamplingRate = " << current->GlobalSamplingRate << endl;
        }
      }
    }
    current=current->Next;
  }
}

void DSP::Clock::UpdateSamplingRates(void)
{
  long double clocks_GlobalSamplingRate;
  DSP::Clock_ptr current;

  // if GlobalSamplingRate is set for this clock use it for all clocks in group
  clocks_GlobalSamplingRate = GlobalSamplingRate;
  if (clocks_GlobalSamplingRate <= 0) {
    //! search for global GlobalSamplingRate

    current=First[MasterClockIndex];
    while (current != NULL)
    {
      if (current != this)
        if (current->GlobalSamplingRate > 0) {
          clocks_GlobalSamplingRate = current->GlobalSamplingRate;
          break;
        }
      current=current->Next;
    }
  }

  // GlobalSamplingRate = cycle_length * SamplingRate;
  //! Update sampling rates of all clocks in group
  current=First[MasterClockIndex];
  while (current != NULL)
  {
    if (clocks_GlobalSamplingRate > 0)
      current->SamplingRate = clocks_GlobalSamplingRate / current->cycle_length;
    else
      current->SamplingRate = -1.0;
    // update GlobalSamplingRate in all clocks in group
    current->GlobalSamplingRate = clocks_GlobalSamplingRate;

    current=current->Next;
  }

}

long double DSP::Clock::GetSamplingRate(void)
{
  return SamplingRate;
}

void DSP::Clock::SetSamplingRate(long double SR)
{
	SamplingRate = SR;
	GlobalSamplingRate = cycle_length * SamplingRate;
	UpdateSamplingRates();
}
/**************************************************************/
/**************************************************************/
/**************************************************************/
DSP::Clock::~Clock()
{
  DSP::Clock_ptr current;

  //! Modify ParentClock if necessary
  current=First[MasterClockIndex];
  while (current != NULL) {
    if (current->ParentClock == this) {
      current->ParentClock = this->ParentClock;
      current->L *= this->L;
      current->M *= this->M;
    }
    current=current->Next;
  }

  if (First[MasterClockIndex]==this)
  {
    First[MasterClockIndex]=this->Next;
  }
  else
  {
    //Find clock which points to this one
    current=First[MasterClockIndex];
    while (current->Next!=this)
      current=current->Next;
    current->Next=this->Next;
  }

  if (SourcesTable != NULL)
  {
    delete [] SourcesTable;
    SourcesTable=NULL;
    NoOfSources=0;
  }
  if (ComponentsNotifications_Table != NULL)
  {
    delete [] ComponentsNotifications_Table;
    ComponentsNotifications_Table=NULL;
    NoOfComponents = 0;
  }

  NoOfClocks[MasterClockIndex]--;

  if (MasterClocks[MasterClockIndex] == this)
  { // one of MasterClocks is just being removed
    ReleaseMasterClock(MasterClockIndex);
//    MasterClocks[MasterClockIndex]=NULL;

    // new MasterClock must be elected if necessary
    if (First != NULL)
      if (First[MasterClockIndex] != NULL)
        SetAsNewMasterClock(First[MasterClockIndex]);

  }

  // All master clocks could be deleted above
  if (MasterClocks != NULL) // if NoOfMasterClocks > 0
  {
    if (MasterClocks[MasterClockIndex] != NULL)
      UpdateCycleLengths(MasterClocks[MasterClockIndex]);
  }
}

/*! \Fixed <b>2005.10.30</b> Here clocks are just deleted.
 *  All clocks' data structures are deleted in  their destructors.
 */
void DSP::Clock::FreeClocks(void)
{
  unsigned long ind;

  // Erase objects
  DSP::Clock_ptr temp_master, current, next;
  for (ind=0; ind<NoOfMasterClocks; ind++)
  {
    temp_master = MasterClocks[ind];
    /*
    while (First[ind] != NULL)
      delete First[ind];
    solution present below is better
    because we avoid unnecessary MasterClock reelection
    */
    current = First[ind];
    while (current != NULL)
    {
      next = current->Next;
      if (current != temp_master)
        delete current;
      current = next;
    }
    if (temp_master != NULL)
       delete temp_master;
  }
}

/*  Here clocks are just deleted.
 *  All clocks' data structures are deleted in their destructors.
 */
void DSP::Clock::FreeClocks(DSP::Clock_ptr ReferenceClock)
{
  unsigned long ind;
  DSP::Clock_ptr temp_master, current, next;
  bool OK;

  if (ReferenceClock == NULL)
  {
    #ifdef __DEBUG__
      DSP::log << "DSP::Clock::FreeClocks"  << DSP::LogMode::second << "Reference clock is NULL" << endl;
    #endif
    return;
  }
  ind = ReferenceClock->MasterClockIndex;
  temp_master = MasterClocks[ind];
  if (temp_master == NULL)
  {
    #ifdef __DEBUG__
      DSP::log << "DSP::Clock::FreeClocks"  << DSP::LogMode::second << "Reference clock refers to NULL Master Clock" << endl;
    #endif
    // ??? Master Clock was not reelected
    //return;
    // trying to free clocks anyway
  }

  // check if given clock is still on the list
  OK = false;
  current = First[ind];
  while (current != NULL)
  {
    next = current->Next;
    if (current == ReferenceClock)
    {
      OK = true;
      break;
    }
    current = next;
  }
  if (OK == false)
  {
    #ifdef __DEBUG__
      DSP::log << DSP::LogMode::Error << "DSP::Clock::FreeClocks" << DSP::LogMode::second
        << "Reference clock refers to Master Clock which it is not related to (it's possible that Reference clock is invalid or was deleted before)" << endl;
    #endif
    return;
  }

  // Erase objects
  current = First[ind];
  while (current != NULL)
  {
    next = current->Next;
    if (current != temp_master)
      delete current;
    current = next;
  }
  // MasterClock deleted last to avoid unnecessary MasterClock reelections
  delete temp_master;
}

//Register given Source block to this clock
/* Appends given source to the list in DSP::Clock::SourcesTable,
 * but first checks whether source isn't already registered
 */
bool DSP::Clock::RegisterSource(DSP::Source_ptr Source)
{
  DSP::Source_ptr *temp;
  unsigned int ind;


  //Source->ProtectOutputClock = true;

  CurrentSource=0; //reset CurrentSource

  //check whether it is already on the list
  for (ind=0; ind<NoOfSources; ind++)
    if (SourcesTable[ind]==Source)
      return false; //source already on the list

  temp = SourcesTable;
  SourcesTable = new DSP::Source_ptr [NoOfSources+1];

  if ((NoOfSources > 0) && (SourcesTable != NULL))
    memcpy(SourcesTable, temp, sizeof(DSP::Source_ptr)*NoOfSources);

  if (temp != NULL)
    delete [] temp;


  if (SourcesTable != NULL)
  {
    SourcesTable[NoOfSources]=Source;
    NoOfSources++;
  }
  else
  {
    NoOfSources=0;
  }

  //returns true if successful
  return (NoOfSources != 0);
}

bool DSP::Clock::RegisterNotification(DSP::Component_ptr Component)
{
  DSP::Component_ptr *temp;
  unsigned int ind;

  //check whether it is already on the list
  for (ind=0; ind<NoOfComponents; ind++)
    if (ComponentsNotifications_Table[ind]==Component)
      return false; //component already on the list

  temp = ComponentsNotifications_Table;
  ComponentsNotifications_Table = new DSP::Component_ptr [NoOfComponents+1];

  if ((NoOfComponents > 0) && (ComponentsNotifications_Table != NULL))
    memcpy(ComponentsNotifications_Table, temp, sizeof(DSP::Source_ptr)*NoOfComponents);

  if (temp != NULL)
    delete [] temp;


  if (ComponentsNotifications_Table != NULL)
  {
    ComponentsNotifications_Table[NoOfComponents]=Component;
    NoOfComponents++;
  }
  else
  {
    NoOfComponents=0;
  }

  //returns true if successful
  return (NoOfComponents != 0);
}

//Unregister given Source block to this clock
bool DSP::Clock::UnregisterSource(DSP::Clock_ptr SourceClock, DSP::Source_ptr Source)
{
  DSP::Source_ptr *temp;
  unsigned int ind, tempNo;
  bool SourceClockExists;
  DSP::Clock_ptr current;

  if (First == NULL)
  { // clocks already destroyed
    return false;
  }

  if (SourceClock->MasterClockIndex >= NoOfMasterClocks)
  { // no-sense SourceClock data
    return false;
  }

  //check wether the SourceClock already exists
  SourceClockExists = false;
  current=First[SourceClock->MasterClockIndex];
  for (ind=0; ind<NoOfClocks[SourceClock->MasterClockIndex]; ind++)
  {
    if (current == SourceClock)
    {
      SourceClockExists = true;
      break;
    }
    current=current->Next;
  }
  if (!SourceClockExists)
  { //SourceClock not on the list
    return false;
  }

  //check whether Source is on the list of SourceClock
  tempNo=SourceClock->NoOfSources;
  for (ind=0; ind<SourceClock->NoOfSources; ind++)
    if (SourceClock->SourcesTable[ind]==Source)
    {
      tempNo=ind;
      break;
    }
  if (tempNo == SourceClock->NoOfSources)
  { //Source not on the list
    return false;
  }

  temp = SourceClock->SourcesTable;
  if (SourceClock->NoOfSources>1)
  {
    SourceClock->SourcesTable = new DSP::Source_ptr [SourceClock->NoOfSources-1];
    if (SourceClock->SourcesTable == NULL)
    { //memory allocation failure
      SourceClock->NoOfSources=0;
      return false;
    }
    SourceClock->NoOfSources--;
    if (tempNo==0) //Source in the begining
    {
      memcpy(SourceClock->SourcesTable, temp+1, sizeof(DSP::Source_ptr)*SourceClock->NoOfSources);
    }
    else
      if (tempNo==SourceClock->NoOfSources) //Source at the end
      {
        memcpy(SourceClock->SourcesTable, temp, sizeof(DSP::Source_ptr)*SourceClock->NoOfSources);
      }
      else
      {
        memcpy(SourceClock->SourcesTable, temp, sizeof(DSP::Source_ptr)*tempNo);
        memcpy(SourceClock->SourcesTable+tempNo, temp+tempNo+1,
               sizeof(DSP::Source_ptr)*(SourceClock->NoOfSources-tempNo));
      }
  }
  else
  {
    SourceClock->SourcesTable = NULL;
    SourceClock->NoOfSources=0;
  }
  delete [] temp;

  //returns true if successful
  return true;
}
bool DSP::Clock::UnregisterNotification(DSP::Clock_ptr NotificationClock, DSP::Component_ptr Component)
{
  DSP::Component_ptr *temp;
  unsigned int ind, tempNo;
  bool NotificationClockExists;
  DSP::Clock_ptr current;

  if (First == NULL)
  { // clocks already destroyed
    return false;
  }

  if (NotificationClock->MasterClockIndex >= NoOfMasterClocks)
  { // nonsense SourceClock data
    return false;
  }

  //check wether the SourceClock already exists
  NotificationClockExists = false;
  current=First[NotificationClock->MasterClockIndex];
  for (ind=0; ind<NoOfClocks[NotificationClock->MasterClockIndex]; ind++)
  {
    if (current == NotificationClock)
    {
      NotificationClockExists = true;
      break;
    }
    current=current->Next;
  }
  if (!NotificationClockExists)
  { //NotificationClock not on the list
    return false;
  }

  //check whether Component is on the list of ComponentsNotifications_Table
  tempNo=NotificationClock->NoOfComponents;
  for (ind=0; ind<NotificationClock->NoOfComponents; ind++)
    if (NotificationClock->ComponentsNotifications_Table[ind]==Component)
    {
      tempNo=ind;
      break;
    }
  if (tempNo == NotificationClock->NoOfComponents)
  { //Component not on the list
    return false;
  }

  temp = NotificationClock->ComponentsNotifications_Table;
  if (NotificationClock->NoOfComponents > 1)
  {
    NotificationClock->ComponentsNotifications_Table =
                      new DSP::Component_ptr [NotificationClock->NoOfComponents-1];
    if (NotificationClock->ComponentsNotifications_Table == NULL)
    { //memory allocation failure
      NotificationClock->NoOfComponents=0;
      return false;
    }
    NotificationClock->NoOfComponents--;
    if (tempNo==0) //Component at the begining of the table
    {
      memcpy(NotificationClock->ComponentsNotifications_Table, temp+1,
             sizeof(DSP::Source_ptr)*NotificationClock->NoOfComponents);
    }
    else
      if (tempNo==NotificationClock->NoOfComponents) //Component at the end
      {
        memcpy(NotificationClock->ComponentsNotifications_Table, temp,
               sizeof(DSP::Source_ptr)*NotificationClock->NoOfComponents);
      }
      else
      {
        memcpy(NotificationClock->ComponentsNotifications_Table, temp, sizeof(DSP::Source_ptr)*tempNo);
        memcpy(NotificationClock->ComponentsNotifications_Table+tempNo, temp+tempNo+1,
               sizeof(DSP::Source_ptr)*(NotificationClock->NoOfComponents-tempNo));
      }
  }
  else
  {
    NotificationClock->ComponentsNotifications_Table = NULL;
    NotificationClock->NoOfComponents=0;
  }
  delete [] temp;

  //returns true if successful
  return true;
}

//4) funkcja statyczna przeliczajca warto�ci cycle_length
//dla wszystkich zegar�w. Wywo�ana po stworzeniu ew.
//usuni�ciu obiektu DSP::Clock
void DSP::Clock::UpdateCycleLengths(DSP::Clock_ptr RootParentClock)
{
  unsigned int ind, temp_in_clock, ClocksNo;
  DSP::Clock_ptr temp_clock;
  DSP::Clock_ptr CurrentClock;
  unsigned long global_multiplier;
  //Przetwarzanie dla ka�dego obiektu DSP::Clock na li�cie po kolei

  //Mamy
  //(1) lista obiekt�w DSP::Clock do przetwarzania DSP::Clock_in;
  DSP::Clock_ptr *clocks_in;
  unsigned int clocks_in_counter;
  //(2) lista obiekt�w DSP::Clock przetworzonych DSP::Clock_out;
  DSP::Clock_ptr *clocks_out;
  unsigned int clocks_out_counter;
//  //(3) bie��cy (przetwarzany) zegar steruj�cy CurrentMasterClock;
  DSP::Clock_ptr CurrentParentClock;
  // numer zegara na li�cie DSP::Clock_out b�d�cego CurrentMasterClock
  unsigned int current_out_clock;

  //*********************************************//
  //Inicjacja
  //(a) CurrentMasterClock=zegar g��wny (ten dla kt�rego
  //MasterClock==NULL)
//  CurrentMasterClock=MasterClock; NULL;
//  temp_clock=First;
//  while (temp_clock != NULL)
//    if (temp_clock->MasterClock==NULL)
//    {
//      CurrentMasterClock=temp_clock;
//      break;
//    }
//    else
//      temp_clock=temp_clock->Next;
  if (RootParentClock==NULL)
  {
    #ifdef __DEBUG__
      DSP::log << DSP::LogMode::Error << "DSP::Clock::UpdateCycleLengths"  << DSP::LogMode::second << "RootParentClock undefined" << endl;
    #endif
    return; //there's nothing to do
  }

  ClocksNo = NoOfClocks[RootParentClock->MasterClockIndex];
  // przygotowanie list zegarw   (with one extra NULL at the end)
  clocks_in=  new DSP::Clock_ptr[ClocksNo+1];
  clocks_out= new DSP::Clock_ptr[ClocksNo+1];
  for (ind=0; ind<ClocksNo+1; ind++)
  {
    clocks_in[ind]=NULL;
    clocks_out[ind]=NULL;
  }

  //(b) DSP::Clock_in lista wszystkich zegar�w poza zegarem
  //g��wnym
  temp_clock=First[RootParentClock->MasterClockIndex];
  clocks_in_counter=0;
  for (ind=0; ind<ClocksNo; ind++)
  {
    if (temp_clock!=RootParentClock)
    {
      clocks_in[clocks_in_counter]=temp_clock;
      clocks_in_counter++;
    }
    temp_clock=temp_clock->Next;
  }

  //(c) DSP::Clock_out zegar gwny / current_out_clock = 0;
  clocks_out_counter=0;
  //"Przetwarzamy" MasterClock:
  // resetujemy jego cycle_length
  // i przepisujemy do listy clocks_out
  global_cycle_lengths[RootParentClock->MasterClockIndex]=1;
  clocks_out[clocks_out_counter]=RootParentClock;
  clocks_out[clocks_out_counter]->cycle_length=1;
  clocks_out_counter++;
  current_out_clock = 0;

  CurrentParentClock = RootParentClock;
  //*********************************************//
  //Robimy
  while (CurrentParentClock != NULL)
  {
    //(4) Szukamy na licie DSP::Clock_in obiekt�w, dla kt�rych
    //ReferenceClock == CurrentParentClock
    CurrentClock=NULL;
    for (temp_in_clock=0; temp_in_clock<ClocksNo; temp_in_clock++)
    {
      if (clocks_in[temp_in_clock] != NULL)
        if (clocks_in[temp_in_clock]->ParentClock ==
            CurrentParentClock)
        {
          CurrentClock=clocks_in[temp_in_clock];

          //usuni�cie z listy clocks_in
          clocks_in[temp_in_clock]=NULL;
          clocks_in_counter--;

          //Przetwarzamy CurrentClock
          //uaktualniamy cycle_length
          CurrentClock->cycle_length=CalculateNewCycle(
              //CurrentParentClock,
              CurrentParentClock->cycle_length,
              CurrentClock->L, CurrentClock->M,
              global_multiplier);
//          global_cycle_length*=global_multiplier;
          global_cycle_lengths[CurrentClock->MasterClockIndex]=CurrentClock->cycle_length;
          //uaktualniamy cycle_length dla zegar�w z listy clocks_out
          for (ind=0; ind<clocks_out_counter; ind++)
          {
            clocks_out[ind]->cycle_length*=global_multiplier;
            //jeeli clocks_out[ind]->cycle_length nie jest
            // podwielokrotno?ci? global_cycle_length
            // uaktualnij global_cycle_length
            if ((global_cycle_lengths[CurrentClock->MasterClockIndex] % clocks_out[ind]->cycle_length) != 0)
            {
              global_cycle_lengths[CurrentClock->MasterClockIndex]*=(clocks_out[ind]->cycle_length /
                DSP::f::gcd(global_cycle_lengths[CurrentClock->MasterClockIndex], clocks_out[ind]->cycle_length));
            }
          }

          //Dodajemy CurrentClock do listy clocks_out
          clocks_out[clocks_out_counter]=CurrentClock;
          clocks_out_counter++;
        }

    }

    //Wybierz kolejny CurrentMasterClock
    current_out_clock++;
    CurrentParentClock=clocks_out[current_out_clock];
  }

  //Sprawd�my czy wszystkie zegary z DSP::Clock_in
  //zosta�y przetworzone, je�eli nie to b��d przetwarzania

  delete [] clocks_in;
  delete [] clocks_out;

  RootParentClock->UpdateGlobalSamplingRate();
}

unsigned long DSP::Clock::CalculateNewCycle(//DSP::Clock_ptr MasterClock,
              unsigned long old_cycle,
              long int L, long int M,
              // oraz wyprowadza mno�nik do old_cycle
              // zapewniaj�cy, �e d�ugo�� cyklu wyj�ciowego
              // jest warto�ci� ca�kowit�
              unsigned long &global_multiplier)
{
  unsigned long NewCycleLength;

  NewCycleLength=old_cycle*L;

//  global_multiplier=M/gcd(NewCycleLength,M);
//  NewCycleLength*=global_multiplier;
//  NewCycleLength/=M;

  NewCycleLength=old_cycle*M;

  global_multiplier=L/DSP::f::gcd(NewCycleLength,L);
  NewCycleLength*=global_multiplier;
  NewCycleLength/=L;

  return NewCycleLength;
}

//DSP::Clock_ptr DSP::Clock::GetClock(long L, long M)
//{
//  return GetClock(NULL, L, M);
//}

DSP::Clock_ptr DSP::Clock::GetClock(DSP::Clock_ptr ParentClock, long L, long M)
{
  unsigned long new_cycle_length, global_multiplier;
  DSP::Clock_ptr tempClock, NewClock;

  //if there is no clock new clock should be created
  if (First==NULL)
  {
    NewClock = new DSP::Clock;
    return NewClock;
  }

  //Problem when ParentClock is NULL
  if (ParentClock==NULL)
  {
    #ifdef __DEBUG__
      DSP::log << DSP::LogMode::Error << "GetClock"  << DSP::LogMode::second << "ParrentClock undefined" << endl;
    #endif
    return NULL;
  }

  //check whether already on the list there isn't such a clock
  new_cycle_length=CalculateNewCycle(
              //MasterClocks[ParentClock->MasterClockIndex],
              ParentClock->cycle_length,
              L, M, global_multiplier);

  NewClock=NULL;
  if (global_multiplier == 1) //if not ==1 new clock must be created
  { // check if there is clock with cycle_length == new_cycle_length
    tempClock=First[ParentClock->MasterClockIndex];
    while (tempClock != NULL)
    {
      if (tempClock->cycle_length == new_cycle_length)
      {
        NewClock=tempClock;
      }
      tempClock=tempClock->Next;
    }
  }

  if (NewClock==NULL)
  { //Create new clock
    NewClock = new DSP::Clock(ParentClock, L, M);
  }

  return NewClock;
}

/*******************************************************/
//6) funkcja okrelajca w oparciu o current_discrete_time
//oraz cycle_length ile czasu upynie do nastpnego
//wywoania bie곿cego zegara
unsigned long DSP::Clock::GetTimeToNextCycle(DSP::Clock_ptr CurrentMasterClock)
{
  unsigned long temp;

  temp=current_discrete_times[CurrentMasterClock->MasterClockIndex] % cycle_length;
  if (temp!=0)
    return cycle_length-temp;
  return 0;
}

// Returns all clocks of the algorithm linked with ReferenceClock
/* Generaly it means that returns all the clocks with the same MasterClock as ReferenceClock.
 *
 * ClockList - pointer to list (allocated by user) where clocks' pointers will be stored
 * clocks_number - number of available entries in ClockList
 */
long DSP::Clock::GetAlgorithmClocks(DSP::Clock_ptr ReferenceClock,
                                   DSP::Clock_ptr *ClocksList, unsigned long clocks_number,
                                   bool FindSignalActivatedClocks)
{
  unsigned long index;
  long ind, ind2, ind3, ile;
  DSP::Clock_ptr tempClock;
  DSP::Clock_trigger_ptr tempClockTrigger;
  bool Ignore;

  index = ReferenceClock->MasterClockIndex; // index to MasterClock

  ind = -1;
  tempClock=First[index];
  while (tempClock != NULL)
  {
    ind++;
    if ((unsigned long)ind >= clocks_number)
    {
      #ifdef __DEBUG__
        DSP::log << DSP::LogMode::Error << "DSP::Clock::GetAlgorithmClocks" << DSP::LogMode::second << "Not enough space in ClocksList" << endl;
      #endif
      break;
    }

    ClocksList[ind] = tempClock;

    tempClock=tempClock->Next;
  }
  if (ind == -1)
    return 0; // no clocks have been found

  // look for signal activated clocks
  if (FindSignalActivatedClocks == true)
  {
    // look for any signal activated clocks
    for (ind2 = 0; ind2 < DSP::Component::GetNoOfComponentsInTable(); ind2++)
    {
      tempClockTrigger = DSP::Component::GetComponent(ind2)->Convert2ClockTrigger();
      if (tempClockTrigger != NULL)
      {
        if (tempClockTrigger->MasterClockIndex == index)
        {
          //check if the clock is not already on the list
          Ignore = false;
          for (ind3 = 0; ind3<= ind; ind3++)
            if (ClocksList[ind3] == tempClockTrigger->SignalActivatedClock)
              Ignore = true;
          if (Ignore == false)
          {
            if ((int)clocks_number > (ind+1))
            {
              ile = GetAlgorithmClocks(tempClockTrigger->SignalActivatedClock,
                                 ClocksList+ind+1, clocks_number - (ind+1), true);
              ind += ile;
            }
            #ifdef __DEBUG__
              else
                DSP::log << DSP::LogMode::Error << "DSP::Clock::GetAlgorithmClocks" << DSP::LogMode::second
                  << "Not enough space in ClocksList for possible signal activated clocks" << endl;
            #endif
          }
        }
      }
    }
  }

  return ind+1;
}

/*******************************************************/
//7) statyczna funkcja tworz�ca list� zegar�w, kt�re
//powinny si� uaktywni� w kolejnym cyklu (czyli tych
//dla kt�rych n0=GetTimeToNextCycle() zwraca najmniejsz�
//warto�� (po stworzeniu listy zwi�kszana jest warto��
//current_discrete_time o n0)
DSP::Clock_ptr* DSP::Clock::FindNextActiveClocks(DSP::Clock_ptr CurrentMasterClock,
                               unsigned long &global_discrete_time_to_next_step)
{
  unsigned long dn;
  DSP::Clock_ptr tempClock;

  if (ActiveClocksList==NULL)
  { //b��d lub brak zegar�w
    #ifdef __DEBUG__
      DSP::log << DSP::LogMode::Error << "DSP::Clock::FindNextActiveClocks" << DSP::LogMode::second << "Error or there are no Clocks at all" << endl;
    #endif
    return NULL;
  }

  if (ActiveClocksList[CurrentMasterClock->MasterClockIndex]==NULL)
  { //b?d lub brak zegarw
    #ifdef __DEBUG__
      DSP::log << DSP::LogMode::Error << "DSP::Clock::FindNextActiveClocks" << DSP::LogMode::second << "Error or there are no Clocks at all (2)" << endl;
    #endif
    return NULL;
  }

  //wypeniamy list
  ActiveClocksListLength[CurrentMasterClock->MasterClockIndex]=0;
  tempClock=First[CurrentMasterClock->MasterClockIndex];
  while (tempClock != NULL)
  {
    if (tempClock->GetTimeToNextCycle(CurrentMasterClock)==0)
    {
      ActiveClocksList[CurrentMasterClock->MasterClockIndex]
        [ActiveClocksListLength[CurrentMasterClock->MasterClockIndex]]=tempClock;
      ActiveClocksListLength[CurrentMasterClock->MasterClockIndex]++;
    }
    tempClock=tempClock->Next;
  }

  //szukamy kiedy nast�pny zegar si� uaktywni
  //testowo zwi�kszamy czas dyskretny tylko jeden
  global_discrete_time_to_next_step = 1;
  current_discrete_times[CurrentMasterClock->MasterClockIndex]++;
  current_discrete_times[CurrentMasterClock->MasterClockIndex]%=global_cycle_lengths[CurrentMasterClock->MasterClockIndex];
  // i sprawdzamy czy nie trzeba jednak o wi�cej
  dn=global_cycle_lengths[CurrentMasterClock->MasterClockIndex];
  tempClock=First[CurrentMasterClock->MasterClockIndex];
  while (tempClock != NULL)
  {
    if (dn>tempClock->GetTimeToNextCycle(CurrentMasterClock))
      dn=tempClock->GetTimeToNextCycle(CurrentMasterClock);
    tempClock=tempClock->Next;
  }

  //Uaktualniamy current_discrete_time
  global_discrete_time_to_next_step += dn;
  current_discrete_times[CurrentMasterClock->MasterClockIndex]+=dn;
  current_discrete_times[CurrentMasterClock->MasterClockIndex]%=global_cycle_lengths[CurrentMasterClock->MasterClockIndex];

  //I koniec
  return ActiveClocksList[CurrentMasterClock->MasterClockIndex];
}

// Notify all components of this clock (all in ComponentsNotifications_Table) that new processing cycle stated
void DSP::Clock::NotifyComponents(void)
{
  if (NoOfComponents == 0)
      return; //Nothing to be done

  unsigned int current;

  current=0;
  // process all registered components
  while (current<NoOfComponents)
  {
    ComponentsNotifications_Table[current]->Notify(this);
    current++;
  }
}


// ********************************************************* //
// ********************************************************* //
void DSP::Clock::ListOfAllComponents(bool list_outputs)
{
  UNUSED_RELEASE_ARGUMENT(list_outputs);

  #ifdef __DEBUG__
    DSP::log << "DSP::Clock::ListOfAllComponents"  << DSP::LogMode::second << "Starting ..." << endl;
    DSP::Component::ListOfAllComponents(list_outputs);
    DSP::log << "DSP::Clock::ListOfAllComponents"  << DSP::LogMode::second << "End" << endl << endl;
  #endif
}

void DSP::Clock::ListComponents(void)
{
  #ifdef __DEBUG__
    DSP::Clock *current;
    string text;

    DSP::log << "DSP::Clock::ListComponents"  << DSP::LogMode::second << "Starting ..." << endl;
    for (unsigned int ind_clocks = 0; ind_clocks < NoOfMasterClocks; ind_clocks++)
    {
      DSP::log << "DSP::Clock::ListComponents" << DSP::LogMode::second
        << "Master clock (index = " << ind_clocks
        << ") / Number of Master clocks " << ((int)NoOfMasterClocks) << endl;


  //    DSP::Component::ListComponents(MasterClocks[ind_clocks]);

      //Find clock which points to this one
      current=First[ind_clocks];
      while (current->Next!=NULL)
      {
        if (current->SourcesTable != NULL)
        {
          for (unsigned int ind = 0; ind < current->NoOfSources; ind++)
          {
            DSP::log << "  >>source" << DSP::LogMode::second << current->SourcesTable[ind]->GetName() << endl;
          }
        }

        DSP::Component::ListComponents(current);
        DSP::log << endl;

        current=current->Next;
      }

      // List Unconnected components
      DSP::log << "DSP::Clock::ListComponents" << DSP::LogMode::second << "Master clock >>NULL<<" << endl;
      DSP::Component::ListComponents(NULL);
    }
    DSP::log << "DSP::Clock::ListComponents" << DSP::LogMode::second << "End" << endl << endl;
  #endif
}

void DSP::Clock::ListComponents(DSP::Clock_ptr MasterClock)
{
  UNUSED_RELEASE_ARGUMENT(MasterClock);

  #ifdef __DEBUG__
    DSP::Clock *current;
    string text;
    bool OK;

    DSP::log << "DSP::Clock::ListComponents" << DSP::LogMode::second << "Starting ..." << endl;

    // checking whether MasterClock still exists
    OK = false;
    for (unsigned int ind = 0; ind < NoOfMasterClocks; ind++)
    {
      if (MasterClocks[ind] == MasterClock)
      {
        OK = true;
        break;
      }
    }
    if (OK == true)
    {
      DSP::log << "DSP::Clock::ListComponents" << DSP::LogMode::second
        << "List for master clock with index: " << (int)MasterClock->MasterClockIndex << endl;

      //Find clock which points to this one
      current=First[MasterClock->MasterClockIndex];
      while (current->Next!=NULL)
      {
        if (current->SourcesTable != NULL)
        {
          for (unsigned int ind = 0; ind < current->NoOfSources; ind++)
          {
            DSP::log << "  >>source" << DSP::LogMode::second << current->SourcesTable[ind]->GetName() << endl;
          }
        }

        DSP::Component::ListComponents(current);

        current=current->Next;
      }
    }
    else
    {
      DSP::log << ">> WARNING" << DSP::LogMode::second << "Given Clock does not exists" << endl;
    }
    DSP::log << "DSP::Clock::ListComponents" << DSP::LogMode::second << "End" << endl << endl;
  #endif
}

// Adds Signal activated clock to the list SignalActivatedClocksList
void DSP::Clock::AddSignalActivatedClock(unsigned int MasterClockIndex,
                                        DSP::Clock_ptr ActivatedClock,
                                        unsigned long cycles)
{
  //[MAX_NO_OF_SIGNAL_ACTIVATED_CLOCKS];
  if (SignalActivatedClocksListLength[MasterClockIndex] < MAX_NO_OF_SIGNAL_ACTIVATED_CLOCKS)
  {
    SignalActivatedClocksList[MasterClockIndex][SignalActivatedClocksListLength[MasterClockIndex]] = ActivatedClock;
    SignalActivatedClocks_cycles_List[MasterClockIndex][SignalActivatedClocksListLength[MasterClockIndex]] = cycles;
    SignalActivatedClocksListLength[MasterClockIndex]++;
  }
#ifdef __DEBUG__
  else
  {
    DSP::log << DSP::LogMode::Error << "DSP::Clock::AddSignalActivatedClock" << DSP::LogMode::second << "Trying too add more than MAX_NO_OF_SIGNAL_ACTIVATED_CLOCKS clocks." << endl;
  }
#endif // __DEBUG__
}
