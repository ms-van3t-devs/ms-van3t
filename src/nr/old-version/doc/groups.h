#ifndef GROUPS_H
#define GROUPS_H

/**
  * \defgroup test Test
  *
  * \brief Test module
  *
  * Contains all the test for the NR 5G-LENA module.
  */

/**
  * \defgroup examples Examples
  *
  * \brief Usage examples of the NR module
  *
  * If you are unsure how to use the NR module, we provide some examples that may
  * help you.
  */

/**
  * \defgroup gnb g-Node-B classes
  *
  * \brief Grouping of the gNb-related classes
  */

/**
  * \defgroup ue User Equipment classes
  *
  * \brief Grouping of the ue-related classes.
  */


/**
  * \addtogroup ue-phy User Equipment PHY classes
  * \ingroup ue
  *
  * \brief Grouping of the UE physical layer related classes
  */

/**
  * \addtogroup gnb-phy g-Node-B PHY classes
  * \ingroup gnb
  *
  * \brief Grouping of the gNb physical layer related classes
  */

/**
  * \addtogroup ue-mac User Equipment MAC classes
  * \ingroup ue
  *
  * \brief Grouping of the UE MAC related classes
  */

/**
  * \addtogroup ue-bwp User Equipment BWP management classes
  * \ingroup ue
  *
  * \brief Grouping of the UE BWP related classes
  *
  * The details of the implementation are present in the paper
  * Implementation and evaluation of frequency division multiplexing of
  * numerologies for 5G new radio in ns-3, (B Bojovic, S Lagen, L Giupponi),
  * in Proceedings of the 10th Workshop on ns-3, 37-44.
  */

/**
  * \addtogroup gnb-mac g-Node-B MAC classes
  * \ingroup gnb
  *
  * \brief Grouping of the gNb MAC related classes
  */

/**
  * \addtogroup gnb-bwp g-Node-B BWP management classes
  * \ingroup gnb
  *
  * \brief Grouping of the gNb BWP management related classes
  *
  * The details of the implementation are present in the paper
  * Implementation and evaluation of frequency division multiplexing of
  * numerologies for 5G new radio in ns-3, (B Bojovic, S Lagen, L Giupponi),
  * in Proceedings of the 10th Workshop on ns-3, 37-44.
  */

/**
  * \addtogroup scheduler g-Node-B scheduler classes
  * \ingroup gnb
  *
  * \brief NR-enabled schedulers module
  *
  * This group contains the copy of FAPI documentation that was included
  * in the original "LTE MAC Scheduler Interface v1.11" document by FemtoForum
  * (Document number: FF_Tech_001_v1.11 , Date issued: 12-10-2010,
  * Document status: Document for public distribution).
  *
  * \section what Description of the module
  * This group specifies the MAC Scheduler interface and their implementation.
  * The goal of this interface specification is to allow the use of a wide range
  * of schedulers which can be plugged into the eNodeB and to allow for
  * standardized interference coordination interface to the scheduler.
  *
  * Not only the interface between the MAC and the scheduler is standardized,
  * but also the internals of the scheduler. The objective is to be able
  * to write new kind of schedulers with minimum efforts and the minimum
  * amount of code, re-using existing classes and methods.
  *
  * \section interface FAPI interface overview
  *
  * The MAC scheduler is part of MAC from a logical view and the MAC scheduler
  * should be independent from the PHY interface.
  *
  * The description in this interface does not foresee any specific
  * implementation of the interface. What is specified in the FAPI document is
  * the structure of the parameters. In order to describe the interface
  * in detail the following model is used:
  * - The interface is defined as a service access point offered by the MAC
  * scheduler to the remaining MAC functionality, as shown in Figure 1.
  * - A _REQ primitive is from MAC to the MAC scheduler.
  * - A _IND/_CNF primitives are from the MAC scheduler to the MAC.
  *
  * The description using primitives does not foresee any specific implementation
  * and is used for illustration purposes. Therefore an implementation could be
  * message-based or function-based interface. Timing constrains applicable to
  * the MAC scheduler are not yet specified.
  *
  * For the MAC scheduler interface specification a push-based concept is employed,
  * that is all parameters needed by the scheduler are passed to the scheduler
  * at specific times rather than using a pull-based concept (i.e. fetching the
  * parameters from different places as needed). The parameters specified are as
  * far as possible aligned with the 3GPP specifications.
  *
  * [Figure 1]
  *
  * Figure 1 shows the functionality split between the MAC scheduler and the
  * remaining MAC. For the purposes of describing the MAC scheduler interface
  * the MAC consists of a control block and a subframe block, which uses the
  * CSCHED and SCHED SAP respectively. The subframe block triggers the MAC
  * scheduler every TTI and receives the scheduler results. The control block
  * forwards control information to the MAC scheduler as necessary.
  *
  * \note CTTC implementation
  * The documentation of the implementation starts with class NrMacScheduler.
  * Please start your journey into the scheduler documentation from there.
  */

/**
  * \defgroup propagation Propagation models
  *
  * \brief Classes related to the propagation modeling
  *
  */

/**
  * \addtogroup spectrum Spectrum models
  * \ingroup propagation
  * \brief Physical layer spectrum modeling
  */

/**
 * \addtogroup error-models Error models
 * \ingroup propagation
 *
 * \brief Error models for the NR module
 *
 * The error models are used for calculating the error probability after a packet
 * is received (in the class NrSpectrumPhy) but also can be used for calculating the
 * best MCS to use before a transmission (in the class NrAmc). Please
 * take a look to the documentation of these classes if you wish to get more
 * information about their operations.
 *
 * The error model interface is defined in the class NrErrorModel. Each model
 * should implement the pure virtual functions defined there, to be ready
 * to work with the spectrum and the amc.
 *
 * The main output of an error model is a subclass of NrErrorModelOutput.
 * The spectrum will take care of creating a vector of all instances returned
 * by the error model for the same transmission (e.g., after a retransmission
 * of the original transmission), in order to create an "history" of the outputs
 * returned by the model.
 *
 * \see NrErrorModel
 * \see NrErrorModelOutput
 */

/**
 * \defgroup helper Helpers
 * \brief Helper code to setup and configure the NR module.
 *
 */

/**
 * \defgroup nru NR-U Classes
 * \brief Code to deal with NR-U regulation
 *
 */

/**
 * \defgroup utils Utils
 * \brief Classes that are used in different points of the code
 *
 */
#endif // GROUPS_H
