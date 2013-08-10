#ifdef ENABLE_MPI

#ifndef FAU_MPI_H
#define FAU_MPI_H

#ifndef SWIG
#include <faunus/common.h>
#include <faunus/point.h>
#include <faunus/slump.h>
#include <faunus/textio.h>
#include <mpi.h>
#endif

namespace Faunus {

  /**
   * @brief Namespace for Message Parsing Interface (MPI) functionality
   */
  namespace MPI {

    /**
     * @brief Main controller for MPI calls
     *
     * This is the MPI controller required for all MPI programs.
     *
     *     MPIController mpi; // call this very first thing in your program
     *     std::cout << "I'm rank " << mpi.rank << " out of " << mpi.nproc;
     *     mpi.cout << "This will go to a file called mpi%r.stdout where %r is my rank"
     *     if (mpi.isMaster())
     *       cout << "I'm the master!";
     *
     * When MPIController is instantiated the textio::prefix variable is automatically
     * set to `mpi%j`. Which can be used to prefix input and output files. For example:
     *
     *     InputMap mcp(textio::prefix+"input"); // tries to load "mpi%r.input" where %r is the rank
     *
     * @date Lund 2012
     */
    class MPIController {
      public:
        MPIController(MPI_Comm=MPI_COMM_WORLD); //!< Constructor
        ~MPIController(); //!< End of all MPI calls!
        MPI_Comm comm;    //!< Communicator (Default: MPI_COMM_WORLD)
        int nproc();      //!< Number of processors in communicator
        int rank();       //!< Rank of process
        int rankMaster(); //!< Rank number of the master
        bool isMaster();  //!< Test if current process is master
        slump random;     //!< Random number generator for MPI calls
        string id;        //!< Unique name associated with current rank
        std::ofstream cout; //!< Redirect stdout to here for rank-based file output

        inline string info() {
          std::ostringstream o;
          o << textio::header("Message Parsing Interface (MPI)")
            << textio::pad(textio::SUB, 25, "Number of processors") << nproc() << endl
            << textio::pad(textio::SUB, 25, "Current rank") << rank() << endl
            << textio::pad(textio::SUB, 25, "Master rank") << rankMaster() << endl;
          return o.str();
        }

      private:
        int _nproc;        //!< Number of processors in communicator
        int _rank;         //!< Rank of process
        int _master;       //!< Rank number of the master
    };

    /**
     * @brief Split N items into nproc parts
     *
     * This returns a pair with the first and last
     * item for the current rank.
     */
    template<class T=int>
      std::pair<T,T> splitEven(MPIController &mpi, T N) {
        T M = mpi.nproc();
        T i = mpi.rank();
        T beg=(N*i)/M;
        T end=(N*i+N)/M-1;
        return std::pair<T,T>(beg,end);
      }

    /**
     * @brief Reduced sum
     *
     * Each rank sends "local" to master who sums them up.
     * Master sends back (broadcasts) sum to all ranks.
     */
    inline double reduceDouble(MPIController &mpi, double local) {
      double sum;
      MPI_Allreduce(&local,&sum,1,MPI_DOUBLE,MPI_SUM,mpi.comm);
      return sum;
    }

    /*!
     * \brief Class for transmitting floating point arrays over MPI
     * \note If you change the floatp typedef, remember also to change to change to/from
     *       MPI_FLOAT or MPI_DOUBLE.
     */
    class FloatTransmitter {
      private:
        MPI_Request sendReq, recvReq;
        MPI_Status sendStat, recvStat;
        int tag;
      public:
        typedef double floatp;   //!< Transmission precision
        FloatTransmitter();
        vector<floatp> swapf(MPIController&, vector<floatp>&, int); //!< Swap data with another process
        void sendf(MPIController&, vector<floatp>&, int); //!< Send vector of floats
        void recvf(MPIController&, int, vector<floatp>&); //!< Receive vector of floats
        void waitsend(); //!< Wait for send to finish              
        void waitrecv(); //!< Wait for reception to finish
    };

    /**
     * @brief Class for sending/receiving particle vectors over MPI.
     *
     * This will take a particle vector and send selected information though MPI. It is
     * possible to send only coordinates using the dataformat `XYZ` or, if charges should be
     * send too, `XYZQ`.
     *
     * Besides particle data it is possible to send extra floats by adding
     * these to the `sendExtra` vector; received extras will be stored in `recvExtra`. Before
     * transmitting extra data, make sure that `recvExtra` and `sendExtra` have the
     * same size.
     *
     *     int dst_rank = 1;
     *     floatp extra1 = 2.34, extra2 = -1.23
     *     p_vec myparticles(200); // we have 200 particles
     *    
     *     Faunus::MPI::MPIController mpi;
     *     Faunus::MPI::ParticleTransmitter pt;
     *    
     *     pt.sendExtra.push_back(extra1);
     *     pt.sendExtra.push_back(extra2);
     *    
     *     pt.send(mpi, myparticles, dst_rank);
     *     pt.waitsend();
     *
     * @date Lund 2012
     *
     */
    class ParticleTransmitter : public FloatTransmitter {
      public:
        enum dataformat {XYZ=3, XYZQ=4, XYZQI=5};
        vector<floatp> sendExtra;                      //!< Put extra data to send here.
        vector<floatp> recvExtra;                      //!< Received extra data will be stored here

        ParticleTransmitter();
        void send(MPIController&, const p_vec&, int); //!< Send particle vector to another node 
        void recv(MPIController&, int, p_vec&);       //!< Receive particle vector from another node
        void waitrecv();
        void setFormat(dataformat);
        void setFormat(string);
        dataformat getFormat();

      private:
        dataformat format;                             //!< Data format to send/receive - default is XYZQ
        vector<floatp> sendBuf, recvBuf;
        p_vec *dstPtr;  //!< pointer to receiving particle vector
        void pvec2buf(const p_vec&); //!< Copy source particle vector to send buffer
        void buf2pvec(p_vec&);       //!< Copy receive buffer to target particle vector
    };

  } //end of mpi namespace
}//end of faunus namespace

#endif
#endif
