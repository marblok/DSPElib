/*! \file DSPclocks.h
 * This is DSP engine clocks management module header file.
 *
 * \author Marek Blok
 */
#ifndef DSPclocksH
#define DSPclocksH

//---------------------------------------------------------------------------
#include <DSP_setup.h>
//---------------------------------------------------------------------------
#include <DSP_types.h>
#include <DSP_modules.h>

#include <iostream>
#include <fstream>
#include <string.h>

namespace DSP {
  class Clock;

  namespace u {
    class AudioInput; // required for friend class ::SocketInput
    class SocketInput; // required for friend class ::SocketInput
  }

  const unsigned long MAX_timeout_counter = 1000;
}


// ***************************************************** //
// ***************************************************** //
//! Class managing DSP algorithm clocks
/*! The main purpose of this class is to manage
 *  multifrequency DSP algorithms.
 *
 * \todo_later Before Starting Execution (?? but once - maybe it would be
 * beter to add just function) check whether all outputs and intputs
 * are connected
 *
 *
  * Class supports independent and signal activated clocks, this allows for
  *  -# implementation of indepentend algorithms
  *  -# implementation of algorithms part working asynchronously, with clocks
  *     activated by signals
  */
class DSP::Clock
{
  friend class DSP::Block;

  public:
    //! Returns Number of existing MasterClocks
    static unsigned long GetNoOfMasterClocks(void)
    {
      return NoOfMasterClocks;
    }
    //! Returns Number of all existing Clocks
    static unsigned long GetNoOfClocks(void)
    {
      unsigned long val;

      val = 0;
      for (unsigned int ind = 0; ind < NoOfMasterClocks; ind++)
        val += NoOfClocks[ind];
      return val;
    }
    //! Returns Number of existing Clocks linked with given Clock
    /*! Returns number of all clocks with the same MasterClock ans given clock.
     *  Given clock and MasterClock are also counted.
     */
    static unsigned long GetNoOfClocks(DSP::Clock_ptr clock)
    {
      return NoOfClocks[clock->GetMasterClockIndex()];
    }

  private:
    //! Length of the clock cycle
    /*! This variable stores number of the clock fundamental
     *  cycles separating consecutive excitations of
     *  the current clock.
     */
    unsigned long cycle_length;

    //! Number of existing MasterClocks
    static unsigned int NoOfMasterClocks;
    //! Table of pointer to existing MasterClocks
    static DSP::Clock_ptr *MasterClocks;

    //! Index to the clock controling this clock work (in MasterClock table)
    /*! should be correct also for the MasterClock
     */
    unsigned int MasterClockIndex; //DSP::Clock *MasterClock;
  public:
    //! returns MasterClockIndex
    inline unsigned int GetMasterClockIndex(void)
    { return MasterClockIndex; };
  private:

    //! ReferenceClock
    DSP::Clock_ptr ParentClock;
    //! ParentClock sampling frequency multiplier
    /*! Sampling frequency of the current clock can be
     *  computed by multiplying by #L and dividing by #M sampling
     *  frequency of the master clock.
     */
    long int L;
    //! ParentClock sampling frequency divider
    /*! Sampling frequency of the current clock can be
     *  computed by multiplying by #L and dividing by #M sampling
     *  frequency of the master clock.
     */
    long int M;

    //! Function reculculating values of #DSP::Clock::cycle_length for all clocks with given MasterClock
    /*! It is executed, for example, after creating or deleting
     *  of DSP::Clock object to update #DSP::Clock::cycle_length in each clock.
     */
    static void UpdateCycleLengths(DSP::Clock_ptr RootParentClock);

    //! Algorithm's global cycle lengths (for each MasterClock)
    static unsigned long *global_cycle_lengths;
    //! Current discrete time of the algorithm (modulo global_cycle_legth) (for each MasterClock)
    static unsigned long *current_discrete_times;
    //! discrete times to next clocks activation in global cycles (for each MasterClock)
    /*! \note table entry == 0 if unknown
     */
    static unsigned long *global_discrete_times_to_next_step;

    //6) funkcja okre�laj�ca w oparciu o current_discrete_time
    //oraz cycle_length ile czasu up�ynie do nast�pnego
    //wywo�ania bie��cego zegara
    unsigned long GetTimeToNextCycle(DSP::Clock_ptr CurrentMasterClock);

    //7) statyczna funkcja tworz�ca list� zegar�w, kt�re
    //powinny si� uaktywni� w kolejnym cyklu (czyli tych
    //dla kt�rych n0=GetTimeToNextCycle() zwraca najmniejsz�
    //warto�� (po stworzeniu listy zwi�kszana jest warto��
    //current_discrete_time o n0) (for each MasterClock)
    //! lista wskaza� na aktywne zegary
    /*! rezerwowana na tyle ile jest zegar�w (ju� w konstruktorze DSP::Clock)
     */
    static DSP::Clock_ptr* *ActiveClocksList;
    //! Liczba wa�nych wpis�w w ActiveClocksLists (for each MasterClock)
    static int *ActiveClocksListLength;
    //! Returns pointer to list of clocks active in next cycle
    /*! global_discrete_time_to_next_step - number of global cycles to next step
     *
     * Retuned table entries are only valid until FindNextActiveClocks
     *  is called next time for the same MasterClock.
     * \warning Retuned table MUST not be freed by user, it will be released automaticaly.
     */
    static DSP::Clock_ptr* FindNextActiveClocks(DSP::Clock_ptr CurrentMasterClock,
                               unsigned long &global_discrete_time_to_next_step); //(for each MasterClock)
    //! List of clocks activated by signals in the current cycle (for each MasterClock)
    /*! rezerwowana na tyle ile jest zegar�w (ju� w konstruktorze DSP::Clock)
     */

    static DSP::Clock_ptr* *SignalActivatedClocksList;
    //! lists storing number of cycles of signal activated clock to process
    /*! Values correspond to clocks in SignalActivatedClocksList
     */
    static unsigned long * *SignalActivatedClocks_cycles_List;
    //! Length of currently activated clocks (for each MasterClock)
    static int *SignalActivatedClocksListLength;

  public:
    //! Adds Signal activated clock to the list SignalActivatedClocksList
    /*! MasterClockIndex - index of the MasterClock on list of which add ActivatedClock
     *  cycles - number of cycles of ActivatedClock to execute
     */
    static void AddSignalActivatedClock(unsigned int MasterClockIndex, DSP::Clock_ptr ActivatedClock, unsigned long cycles = 1);

    //! Returns all (in ClocksList) clocks of the algorithm linked with ReferenceClock
    /*! Generaly it means that returns all the clocks with the same MasterClock as ReferenceClock.
     *
     * ClockList - vector with list where clocks' pointers will be stored (appended) \n
     * FindSignalActivatedClocks - if <b>true</b> function will look for
     *   signal activated clock (clocks with different MasterClock).
     *
     *
     * Function returns actual number of stored clocks.
     * In case of error:
     *  returns -1: no clock where found
     */
    static long GetAlgorithmClocks(DSP::Clock_ptr ReferenceClock,
        vector<DSP::Clock_ptr> &ClocksList, bool FindSignalActivatedClocks = false);

  private:
    //8) Jednokierunkowa lista obiekt�w DSP::Clock
    //! table of lists of DSP::Clock objects (for each MasterClock)
    static DSP::Clock_ptr *First;
    //! Number of ALL clocks (for each MasterClock)
    static unsigned int *NoOfClocks;
    //! Next clock but only with the same MasterClock
    DSP::Clock_ptr Next;

    //9) tabela zarejestrowanych �r�de� DSP::Block for this clock
    DSP::Source_ptr* SourcesTable;
    //! number of sources regstered in SourcesTable
    unsigned int NoOfSources;
    //! true if coresponding source is SourcesTable is registered for notificatio0ns not for processing
    DSP::Component_ptr* ComponentsNotifications_Table;
    //! number of components registered in ComponentsNotifications_Table
    unsigned int NoOfComponents;

    //10) Funkcj� do wyznaczania cycle_length
    // u�ywane w  UpdateCycleLengths() do wyznaczenia
    // nowych warto�ci cycle_length
    // oraz w GetClock() w celu weryfikacji czy ju� takiego
    // zegara nie ma (for each MasterClock)
    /*! L i M s� odpowiednio krotno�ci� interpolacji oraz decymacji
     * wzgl�dem zegara o podanej d�ugo�ci cyklu (old_cycle)
     *
     * Funkcja zwraca nowa d�ugo�� cyklu (z uwzglednieniem L i M) oraz
     * warto�� mno�nika przez jaki nale�a�oby przemno�y� wszystkie cycle zegar�w
     * powi�zanych z MasterClock.
     *
     * Je�eli global_multiplier == 1 to napewno nie istnieje jeszcze zegar o
     * takim cyklu o jaki si� zapytujemy
     */
    static unsigned long CalculateNewCycle(//DSP::Clock_ptr MasterClock,
                  unsigned long old_cycle,
                  long int L, long int M,
                  // oraz wyprowadza mno�nik do old_cycle
                  // zapewniaj�cy, �e d�ugo�� cyclu wyj�ciowego
                  // jest warto�ci� ca�kowit�
                  unsigned long &global_multiplier);

   //Funkcja do wykorzystania tylko przez construktor klasy
    void ClockInit(DSP::Clock_ptr ReferenceClock,
                   long int L_in, long int M_in,
                   unsigned int new_MasterClockIndex = UINT_MAX);

    //when true processing loop stops (for each MasterClock)
    volatile static bool *Terminated;
    //! source sets this when it still needs more time for external data to arrive
    /*! e.g. audio card input
     */
    volatile static bool *InputNeedsMoreTime;
    friend class DSP::u::AudioInput;
    friend class DSP::u::SocketInput;

    //! Processes all sources related to given clock: SourcesTable
    /*! (returns false when not all sources could be processed)
     */
    bool ProcessSources(int &NoOfProcessedSources);
    //! Notify all components of this clock (all in ComponentsNotifications_Table) that new processign cycle started
    void NotifyComponents(void);
    //! Index of currently being processed source
    unsigned int CurrentSource;

    //! Discrete time for the current clock
    /*! Number of clock cycles from the beginning.
     * Should be updated in DSP::Clock::Execute.
     */
    uint32_t n;


  private:
    //! This will be MasterClock
    /*! Creates clock without its MasterClock (will be added as MasterClock)
     * this constructor should be called only from CreateMasterClock
    */
    Clock(void);
    //! This will be MasterClock
    /*! This version reuses MasterClocks slot
     */
    Clock(unsigned int new_MasterClockIndex);
  public:
    //DSP::Clock(unsigned int L_in, unsigned int M_in);
    //! This will be descendant clock
    Clock(DSP::Clock_ptr ReferenceClock, long int L_in, long int M_in);
    //! Initiates new clock group (signal activated) synchronous with ReferenceClock
    Clock(DSP::Clock_ptr ReferenceClock);

    //! removes clock and frees its structures
    /*! Also takes care of MasterClocks list if MasterClock is removed.
     */
    ~Clock();


//?? Maybe clock's offset should also be used
    //! Creates new independent MasterClock
    /*! \test Test if it is now possible to create several indepenent
     * algorithms
     */
    static DSP::Clock_ptr CreateMasterClock(void);

  private:
    //! Released alocated memory and all annotations about given MasterClock
    static void ReleaseMasterClock(unsigned int MasterClockIndex);
    //! Selects new clocks as master clock
    /*! Invoked from DSP::Clock::ReleaseMasterClock
     */
    static void SetAsNewMasterClock(DSP::Clock_ptr new_master);

  public:
//    // Return pointer to the clock which is L/M
//    // times faster than MasterClock (if necessary create it)
//    static DSP::Clock_ptr GetClock(long L=1, long M=1);
    // Returns pointer to the clock which is L/M
    // times faster than *ParentClock
    static DSP::Clock_ptr GetClock(DSP::Clock_ptr ParentClock, long L, long M);
    //! Free all clocks
    static void FreeClocks(void);
    //! Free all clocks with the same MasterClock as ReferenceClock
    /*! \note In case of signal activated asynchronous clocks
     *   Reference clock must be aquired manualy using GetClock
     *   and freed separately.
     */
    static void FreeClocks(DSP::Clock_ptr ReferenceClock);

    //! List all registered components
    /*! This is for safety precautions to indicate if any components
     * are still registed when we want to call FreeClocks
     *
     * \warning This function works only in DEBUG mode
     */
    static void ListComponents(void);
    //! Lists all components (if list_outputs == true also lists components outputs)
    static void ListOfAllComponents(bool list_outputs = false);
    //! List all components registered with given MasterClock
    /*! This is for safety precautions to indicate if any components
     * are still registed when we want to call FreeClocks
     */
    static void ListComponents(DSP::Clock_ptr MasterClock);

  private:
    //Register and Unregister given Source block to this clock
    //!Register given Source block to this clock
    /*! Appends given source to the list in DSP::Clock::SourcesTable,
     * but first checks whether source isn't already registered
     */
    bool RegisterSource(DSP::Source_ptr Source);
    //!Register given Component for notifications to this clock
    bool RegisterNotification(DSP::Component_ptr Component);
    friend void DSP::Source::RegisterOutputClock(DSP::Clock_ptr OutputClock, unsigned int output_index);
    friend void DSP::Component::RegisterForNotification(DSP::Clock_ptr NotifyClock);

    //! Unregister source from the clock list
    static bool UnregisterSource(DSP::Clock_ptr SourceClock, DSP::Source_ptr Source);
    friend void DSP::Source::UnregisterOutputClocks(void);
    //! Unregister component from the clock notification list
    static bool UnregisterNotification(DSP::Clock_ptr NotificationClock, DSP::Component_ptr Component);
    friend void DSP::Component::UnregisterNotifications(void);

  public:
    //!Main processing loop (version 2)
    /*! if NoOfCyclesToProcess != 0 <- processing
     *   only NoOfCyclesToProcess cycles, otherwise runs in infinitive loop.
     *
     * ReferenceClock - clock to which NoOfCyclesToProcess refers
     *
     * Function DSP::Clock::Execute can be called
     * several times in a row to continue processing.
     */
    static unsigned long Execute(DSP::Clock_ptr ReferenceClock,
                                 unsigned long NoOfCyclesToProcess=0,
                                 bool ReferenceClock_is_signal_activated=false);

  public:
    //!Saves scheme information of the algorithm to DOT-file
    /*! ReferenceClock - one of the clocks associated with
     *  the algorithm which we want to store in
     *  DOT-file format.
     *
     * \note Compile file with dot. For example "dot -Tgif filename.dot -ooutput.gif".
     *   See http://www.graphviz.org.
     *
     * \note This function is inactive in release mode.
     *
     */
    static void SchemeToDOTfile(DSP::Clock_ptr ReferenceClock,
                                const string &dot_filename,
                                DSP::Macro_ptr DrawnMacro = NULL);

  #ifdef __DEBUG__
    private:
      //!Saves components information to m-file
      /*! For all components linked with this clock info is stored
       *  in dot-file format. Called from DSP::Clock::SchemeToMfile
       */
      bool ClockComponentsToDOTfile(std::ofstream &m_plik,
              vector<bool> &ComponentDoneTable, long max_components_number,
              vector<bool> &UsedMacrosTable, vector<DSP::Macro_ptr> &MacrosList, 
              vector<bool> &UsedClocksTable, vector<DSP::Clock_ptr> &ClocksList, 
              DSP::Macro_ptr DrawnMacro);
      bool ClockNotificationsToDOTfile(std::ofstream &dot_plik,
              vector<bool> &ComponentDoneTable, long max_components_number);
              //bool *UsedClocksTable, DSP::Clock_ptr *ClocksList, long clocks_number);
  #endif

    private:
    	//! clock's sampling rate
    	/*! Default value for MasterClock is 1.0 [Sa/s]
    	 */
    	long double SamplingRate;
        //! global sampling rate for clock's family (< 0.0 if not set)
    	long double GlobalSamplingRate;
    	/*! Updates sampling rates for all clocks
    	 *  related to this clock, based on
    	 *  this clock SamplingRate
    	 *
    	 * must be called when:
    	 *  - new clock is created
    	 *  - sampling rate was changed
    	 *  .
    	 */
    	void UpdateSamplingRates(void);
        //! Updates global sampling rate stored in clock based on sampling rates for all clocks in group
    	void UpdateGlobalSamplingRate(void);

    public:
    	//! Returns clock's sampling rate
    	/*! \note if sampling rate wasn't set for any of clocks,
    	 *   then result is based on MasterClocks default sampling rate.
    	 */
    	long double GetSamplingRate(void);
    	//! Changes clock's sampling rate
    	/*! \warning: Sampling rate for all clocks
    	 *   related to the same MasterClock will
    	 *   be updated.
    	 */
    	void SetSamplingRate(long double SR);
};

//extern DSP::Clock MasterClock;
/**************************************************/
#endif
