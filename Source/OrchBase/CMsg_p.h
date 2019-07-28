#ifndef CMSG_P_H
#define CMSG_P_H

#include "PMsg_p.hpp"
#include "P_addr.h"

class CMsg_p : public PMsg_p
{
public:
      CMsg_p(MPI_Comm c=MPI_COMM_NULL);
      CMsg_p(byte * pb,int l,MPI_Comm c=MPI_COMM_NULL);
      CMsg_p(CMsg_p & r);
      CMsg_p(PMsg_p & r);

      virtual ~CMsg_p();

      void                                    Dump(FILE * = stdout);
      template <class T> void                 Dump(FILE * = stdout);
      template <class T> inline T *           Get(int k,int & cnt) {return k ? (T*)0 : PMsg_p::Get<T>(k,cnt);};
      pair<unsigned,P_addr_t> *               Get(int &);
      inline void                             Get(int k,string & s) {return;};
      template <class T> inline void          Get(int k,vector<T> & vT) {return;};
      void                                    Get(vector<pair<unsigned,P_addr_t> > &);
      inline void                             GetX(int k,vector<string> & vs) {return;};
      template <class T> inline int           Put() {return -1;};
      template <class T> void                 Put(int k,T * data,int cnt=1) {if (!k) PMsg_p::Put(k,data,cnt);};
      void                                    Put(pair<unsigned,P_addr_t> *,int=1);
      inline void                             Put(int k,string * data) {return;};
      template <class T> inline void          Put(int k,vector<T> * data) {return;};
      void                                    Put(vector<pair<unsigned,P_addr_t> > *);
      inline void                             PutX(int k,vector<string> * data) {return;};

};

template <class T> void CMsg_p::Dump(FILE * fp)
{
   fprintf(fp,"---------------------------------------------\n");
   fprintf(fp,"A CMsg_p message is not a generic container. Do not try to dump generic contents.\n\n");
   fprintf(fp,"---------------------------------------------\n");
   fflush(fp);
}

#endif // CMSG_P_H
