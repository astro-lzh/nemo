// -*- C++ -*-                                                                 |
//-----------------------------------------------------------------------------+
//                                                                             |
// iact.h                                                                      |
//                                                                             |
// C++ code                                                                    |
//                                                                             |
// Copyright Walter Dehnen, 2000-2004                                          |
// e-mail:   walter.dehnen@astro.le.ac.uk                                      |
// address:  Department of Physics and Astronomy, University of Leicester      |
//           University Road, Leicester LE1 7RH, United Kingdom                |
//                                                                             |
//-----------------------------------------------------------------------------+
#ifndef falcON_included_iact_h
#define falcON_included_iact_h

#ifndef falcON_included_exit_h
#  include <public/exit.h>
#endif

#define WRITE_IACT_INFO
#undef  WRITE_IACT_INFO

#if defined(WRITE_IACT_INFO) && !defined(DEBUG)
#  define DEBUG
#endif

namespace nbdy {
  //////////////////////////////////////////////////////////////////////////////
  //                                                                          //
  // auxiliary stuff                                                          //
  //                                                                          //
  //////////////////////////////////////////////////////////////////////////////
  //----------------------------------------------------------------------------
  // struct iaction<>                                                           
  //   holds the left and right partner of an interaction                       
  //----------------------------------------------------------------------------
  template<typename A, typename B>
  struct iaction {
#ifdef WRITE_IACT_INFO
    int lev;                                       // level of iaction          
#endif
    A fst;                                         // first object              
    B snd;                                         // second object             
    iaction () : fst(0), snd(0)                    // constructor               
#ifdef WRITE_IACT_INFO
    ,lev(0)
#endif
    {}
    void set(A a, B b                              // set pair                  
#ifdef WRITE_IACT_INFO
	     , int l
#endif
	     ) {
      fst=a;  snd=b;
#ifdef WRITE_IACT_INFO
      lev=l;
#endif
    }
  };
  //----------------------------------------------------------------------------
  // struct iastack<>                                                           
  //   a stack of iaction<>s organised in a linked list                         
  //----------------------------------------------------------------------------
  template<typename A, typename B>
  class iastack {
  private:
    typedef iaction<A,B> iact;                     // type of stack objects     
    iact    *IA, *pi;                              // first & current element   
#ifdef DEBUG
    iact    *IEND;
#endif
    //--------------------------------------------------------------------------
  public:
    iastack (unsigned const&M)                     // constructor               
      : IA   ( falcON_New(iact,M) ),               //   allocate memory         
	pi   ( IA - 1 )                            //   set pter to activ       
#ifdef DEBUG
      , IEND ( IA + M )
#endif
    {}
    //--------------------------------------------------------------------------
    ~iastack () { delete[] IA; }                   // destructor: deallocate    
    bool is_empty() const   { return pi<IA;}       // if stack empty?           
    iact pop     ()         { return *(pi--); }    // give last:   pop          
    void push    (A a, B b                         // add element: push         
#ifdef WRITE_IACT_INFO
		  ,int l=0
#endif
		  ) {
      ++pi;
#ifdef DEBUG
      if(pi >= IEND) error("push()ing beyond end of iastack");
#endif
      pi->set(a,b
#ifdef WRITE_IACT_INFO
	      ,l
#endif
	      );
    }
  };
  //----------------------------------------------------------------------------
  // struct saction<>                                                           
  //   holds one partner of an interaction                                      
  //----------------------------------------------------------------------------
  template<typename A>
  struct saction {
#ifdef WRITE_IACT_INFO
    int lev;                                       // level of iaction          
#endif
    A obj;                                         // object                    
    saction () : obj(0)                            // constructor               
#ifdef WRITE_IACT_INFO
    ,lev(0)
#endif
    {}
    void set(A a                                   // set object                
#ifdef WRITE_IACT_INFO
	     , int l
#endif
	     ) {
      obj=a;
#ifdef WRITE_IACT_INFO
      lev=l;
#endif
    }
  };
  //----------------------------------------------------------------------------
  // struct sastack<>                                                           
  //   a stack of saction<>s organised in a linked list                         
  //----------------------------------------------------------------------------
  template<typename A>
  class sastack {
  private:
    typedef saction<A> sact;                       // type of stack objects     
    sact    *SA, *pi;                              // first & current element   
#ifdef DEBUG
    sact    *SEND;
#endif
    //--------------------------------------------------------------------------
  public:
    sastack (unsigned const&M)                     // constructor               
      : SA   ( falcON_New(sact,M) ),               //   allocate memory         
	pi   ( SA - 1 )                            //   set pter to activ       
#ifdef DEBUG
      , SEND ( SA + M )
#endif
    {}
    //--------------------------------------------------------------------------
    ~sastack () { delete[] SA; }                   // destructor: deallocate    
    bool is_empty() const   { return pi<SA;}       // if stack empty?           
    sact pop     ()         { return *(pi--); }    // give last:   pop          
    void push    (A a                              // add element: push         
#ifdef WRITE_IACT_INFO
		  ,int l=0
#endif
		  ) {
      ++pi;
#ifdef DEBUG
      if(pi >= SEND) error("push()ing beyond end of sastack");
#endif
      pi->set(a
#ifdef WRITE_IACT_INFO
	      ,l
#endif
	      );
    }
  };
  //////////////////////////////////////////////////////////////////////////////
  //                                                                          //
  // class nbdy::MutualInteractor<>                                           //
  //                                                                          //
  // encodes the mutual interaction algorithm.                                //
  // the order of nodes (first,second) is preserved when splitting a node     //
  //                                                                          //
  //////////////////////////////////////////////////////////////////////////////
  template<typename INTERACTOR> class MutualInteractor {
    //--------------------------------------------------------------------------
    // types                                                                    
    //--------------------------------------------------------------------------
    typedef typename INTERACTOR::cell_iter cell_iter;// iterator over cells     
    typedef typename INTERACTOR::leaf_iter leaf_iter;// iterator over leafs     
    //--------------------------------------------------------------------------
    typedef saction<cell_iter>           cx_iact;    // cell-self iaction       
    typedef iaction<cell_iter,cell_iter> cc_iact;    // cell-cell iaction       
    typedef iaction<cell_iter,leaf_iter> cl_iact;    // cell-leaf iaction       
    typedef iaction<leaf_iter,cell_iter> lc_iact;    // leaf-cell iaction       
    //--------------------------------------------------------------------------
    typedef sastack<cell_iter>           cx_stack;   // stack: cell-self iaction
    typedef iastack<cell_iter,cell_iter> cc_stack;   // stack: cell-cell iaction
    typedef iastack<cell_iter,leaf_iter> cl_stack;   // stack: cell-leaf iaction
    typedef iastack<leaf_iter,cell_iter> lc_stack;   // stack: leaf-cell iaction
    //--------------------------------------------------------------------------
    // data                                                                     
    //--------------------------------------------------------------------------
    const INTERACTOR *IA;                            // interactor              
    mutable cx_stack CX;                             // stack: cell-self iaction
    mutable cc_stack CC;                             // stack: cell-cell iaction
    mutable cl_stack CL;                             // stack: cell-leaf iaction
    mutable lc_stack LC;                             // stack: leaf-cell iaction
    //--------------------------------------------------------------------------
    // private methods                                                          
    //--------------------------------------------------------------------------
#define LoopCKids(C,A)							\
    LoopCellKids(typename cell_iter,C,A) if(INTERACTOR::take(A))
#define LoopLKids(C,A)							\
    LoopLeafKids(typename cell_iter,C,A) if(INTERACTOR::take(A))
#define LoopCPairs(C,A,B)						\
    LoopCellSecd(typename cell_iter,C,A,B) if(INTERACTOR::take(B))
#define LoopSPairs(C,A,B)						\
    LoopLKids(C,A)							\
      LoopLeafSecd(typename cell_iter,C,A+1,B) if(INTERACTOR::take(B))
    //--------------------------------------------------------------------------
    void clear_leaf_cell_stack() const {
      // clear the stack of leaf-cell interactions                              
      while(!LC.is_empty()) {                        // WHILE(LC non-empty)     
	lc_iact lc = LC.pop();                       //   pop new L-C iaction   
#ifdef WRITE_IACT_INFO
	std::cerr<<" LC "<<std::setw(2)<<lc.lev<<" :"
		 <<std::setw(6)<< lc.snd.my_tree()->index(lc.fst)<<' '
		 <<std::setw(6)<< lc.snd.index();
#endif
	if(!IA->interact(lc.fst,lc.snd)) {           //   IF(not performed)     
#ifdef WRITE_IACT_INFO
	  std::cerr<<": splitting\n";
#endif
	  LoopCKids(lc.snd,c2) LC.push  (lc.fst,c2   //     push    sub L-C     
#ifdef WRITE_IACT_INFO
					 ,lc.lev+1
#endif
					 );
	  LoopLKids(lc.snd,s2) leaf_leaf(lc.fst,s2); //     perform sub L-L     
	}                                            //   ENDIF                 
      }                                              // END WHILE               
    }
    //--------------------------------------------------------------------------
    void clear_cell_leaf_stack() const {
      // clear the stack of cell-leaf interactions                              
      while(!CL.is_empty()) {                        // WHILE(CL non-empty)     
	cl_iact cl = CL.pop();                       //   pop new C-L iaction   
#ifdef WRITE_IACT_INFO
	std::cerr<<" CL "<<std::setw(2)<<cl.lev<<" :"
		 <<std::setw(6)<< cl.fst.index()<<' '
		 <<std::setw(6)<< cl.fst.my_tree()->index(cl.snd);
#endif
	if(!IA->interact(cl.fst,cl.snd)) {           //   IF(not performed)     
#ifdef WRITE_IACT_INFO
	  std::cerr<<": splitting\n";
#endif
	  LoopCKids(cl.fst,c1) CL.push  (c1,cl.snd   //     push    sub C-L     
#ifdef WRITE_IACT_INFO
					 ,cl.lev+1
#endif
					 );
	  LoopLKids(cl.fst,s1) leaf_leaf(s1,cl.snd); //     perform sub L-L     
	}                                            //   ENDIF                 
      }                                              // END WHILE               
    }
    //--------------------------------------------------------------------------
    void split_cell_cell(cc_iact &cc) const {        // I:   iaction to be split
      // split cell-cell interaction and perform resulting leaf interactions    
      if(IA->split_first(cc.fst,cc.snd)) {           // IF(split 1st)           
	LoopCKids(cc.fst,c1) CC.push(c1,cc.snd       //   push sub C-C          
#ifdef WRITE_IACT_INFO
				     ,cc.lev+1
#endif
				     );
	if(has_leaf_kids(cc.fst)) {                  //   IF(leaf kids)         
	  LoopLKids(cc.fst,s1) LC.push(s1,cc.snd     //     push sub L-C        
#ifdef WRITE_IACT_INFO
				       ,cc.lev+1
#endif
				     );
	  clear_leaf_cell_stack();                   //     clear the LC stack  
	}                                            //   ENDIF                 
      } else {                                       // ELSE(split 2nd)         
	LoopCKids(cc.snd,c2) CC.push(cc.fst,c2       //   push sub C-C          
#ifdef WRITE_IACT_INFO
				     ,cc.lev+1
#endif
				     );
	if(has_leaf_kids(cc.snd)) {                  //   IF(leaf kids)         
	  LoopLKids(cc.snd,s2) CL.push(cc.fst,s2     //     push sub C-L        
#ifdef WRITE_IACT_INFO
				       ,cc.lev+1
#endif
				       );
	  clear_cell_leaf_stack();                   //     clear the CL stack  
	}                                            //   ENDIF                 
      }                                              // ENDIF                   
    }
    //--------------------------------------------------------------------------
    void clear_cell_cell_stack() const {
      // clear an stack of cell-cell interactions                               
      while(!CC.is_empty()) {                        // WHILE(CC non-empty)     
	cc_iact cc = CC.pop();                       //   pop new C-C iaction   
#ifdef WRITE_IACT_INFO
	std::cerr<<" CC "<<std::setw(2)<<cc.lev<<" :"
		 <<std::setw(6)<< cc.fst.index()<<' '
		 <<std::setw(6)<< cc.snd.index();
#endif
	if(!IA->interact(cc.fst,cc.snd)) {           //   IF(not performed)     
#ifdef WRITE_IACT_INFO
	  std::cerr<<": splitting\n";
#endif
	  split_cell_cell(cc);                       //     split               
	}
      }                                              // END WHILE               
    }
    //--------------------------------------------------------------------------
    void split_cell_self(cx_iact &cx) const {
      // split cell-self interaction and perform resulting leaf interactions    
      LoopCKids(cx.obj,c1) {                         // LOOP(cell kids)         
	CX.push(c1                                   //   push sub C-X          
#ifdef WRITE_IACT_INFO
		,cx.lev+1
#endif
		);
	LoopLKids (cx.obj,s2)      CL.push(c1,s2     //   push sub C-L          
#ifdef WRITE_IACT_INFO
					   ,cx.lev+1
#endif
					   );
	LoopCPairs(cx.obj,c1+1,c2) CC.push(c1,c2     //   push sub C-C          
#ifdef WRITE_IACT_INFO
					   ,cx.lev+1
#endif
					   );
      }                                              // END LOOP                
      LoopSPairs(cx.obj,s1,s2) leaf_leaf(s1,s2);     // perform sub L-L         
      clear_cell_leaf_stack();                       // clear the CL stack      
    }
    //--------------------------------------------------------------------------
#undef LoopCKids
#undef LoopLKids
#undef LoopCPairs
#undef LoopSPairs
    //--------------------------------------------------------------------------
    void clear_cell_self_stack() const {
      // clear an stack of cell-self interactions                               
      while(!CX.is_empty()) {                        // WHILE(CX non-empty)     
	cx_iact cx = CX.pop();                       //   pop new self iaction  
#ifdef WRITE_IACT_INFO
	std::cerr<<" CX "<<std::setw(2)<<cx.lev<<" :"
		 <<std::setw(6)<< cx.obj.index() <<"       ";
#endif
	if(!IA->interact(cx.obj)) {                  //   IF(not performed)     
#ifdef WRITE_IACT_INFO
	  std::cerr<<": splitting\n";
#endif
	  split_cell_self(cx);                       //     split               
	  clear_cell_cell_stack();                   //     clear the CC stack  
	}                                            //   ENDIF                 
      }                                              // END WHILE               
    }
    //--------------------------------------------------------------------------
    void work_cell_cell_stack(int      &i,
			      int const&M) const {
      // perform up to M cell-cell, perform all occuring leaf interactions      
      while(!CC.is_empty() && i<M) {                 // WHILE(CC non-empty)     
	cc_iact cc = CC.pop();                       //   pop new C-C iaction   
#ifdef WRITE_IACT_INFO
	std::cerr<<" CC "<<std::setw(2)<<cc.lev<<" :"
		 <<std::setw(6)<< cc.fst.index()<<' '
		 <<std::setw(6)<< cc.snd.index();
#endif
	if(IA->interact(cc.fst,cc.snd)) ++i;         //   IF(performed): count  
	else {                                       //   ELSE                  
#ifdef WRITE_IACT_INFO
	  std::cerr<<": splitting\n";
#endif
	  split_cell_cell(cc);                       //     split               
	}                                            //   ENDIF                 
      }                                              // END WHILE               
    }
    //--------------------------------------------------------------------------
    void work_cell_cell_stack(int const&M) const {
      // perform up to M cell-cell, perform all occuring leaf interactions      
      int i = 0;                                     // counter: performed C-C  
      work_cell_cell_stack(i,M);                     // work it out             
    }
    //--------------------------------------------------------------------------
    void work_cell_self_stack(int const&M) const {
      // perform up to M cell iactions, perform all occuring leaf iactions      
      int i = 0;                                     // counter: performed      
      work_cell_cell_stack(i,M);                     // work on C-C stack       
      while(!CX.is_empty() && i<M) {                 // WHILE(CX non-empty)     
	cx_iact cx = CX.pop();                       //   pop new self iaction  
#ifdef WRITE_IACT_INFO
	std::cerr<<" CX "<<std::setw(2)<<cx.lev<<" :"
		 <<std::setw(6)<< cx.obj.index() <<"       ";
#endif
	if(IA->interact(cx.obj))                     //   IF(not performed)     
	  ++i;                                       //     count               
	else {                                       //   ELSE:                 
#ifdef WRITE_IACT_INFO
	  std::cerr<<": splitting\n";
#endif
	  split_cell_self(cx);                       //     split               
	  work_cell_cell_stack(i,M);                 //     work on CC stack    
	}                                            //   ENDIF                 
      }                                              // END WHILE               
    }    
    //--------------------------------------------------------------------------
    // construction                                                             
    //--------------------------------------------------------------------------
    static unsigned n_x(unsigned const&d) { return Nsub*d+1; }
    static unsigned n_c(unsigned const&d) { return Nsub*(Nsub-1)/2+Nsub*d; }
    //--------------------------------------------------------------------------
  public:
    MutualInteractor(                                // constructor             
		     INTERACTOR*const&ia,            // I: interactor           
		     unsigned   const&d1) :          // I: depth of 1st tree    
      IA ( ia ),                                     //   initialize interactor 
      CX ( n_x(d1) ),                                //   initialize cell-self  
      CC ( n_c(2*d1) ),                              //   initialize cell-cell  
      CL ( n_c(2*d1) ),                              //   initialize cell-leaf  
      LC ( n_c(2*d1) ) {}                            //   initialize leaf-cell  
    //--------------------------------------------------------------------------
    MutualInteractor(                                // constructor             
		     INTERACTOR*const&ia,            // I: interactor           
		     unsigned   const&d1,            // I: depth of 1st tree    
		     unsigned   const&d2) :          // I: depth of 2nd tree    
      IA ( ia ),                                     //   initialize interactor 
      CX ( n_x(d1) ),                                //   initialize cell-self  
      CC ( n_c(d2? d1+d2 : 2*d1) ),                  //   initialize cell-cell  
      CL ( n_c(d2? d1+d2 : 2*d1) ),                  //   initialize cell-leaf  
      LC ( n_c(d2? d1+d2 : 2*d1) ) {}                //   initialize leaf-cell  
    //--------------------------------------------------------------------------
    // routines for interactions in ordinary (sequential) code                  
    //--------------------------------------------------------------------------
    void leaf_leaf(leaf_iter const&a,
		   leaf_iter const&b) const          // leaf-leaf interaction   
    {
      IA->interact(a,b);
    }
    //--------------------------------------------------------------------------
    void cell_self(cell_iter const&a) const          // cell-self interaction   
    {
      CX.push(a);                                    // initialize stack        
      clear_cell_self_stack();                       // work until stack empty  
    }
    //--------------------------------------------------------------------------
    void cell_cell(cell_iter const&a,
		   cell_iter const&b) const          // cell-cell interaction   
    {
      if(a==b)falcON_ErrorF("self-interaction","MutualInteractor::cell_cell()");
      CC.push(a,b);                                  // initialize stack        
      clear_cell_cell_stack();                       // work until stack empty  
    }
    //--------------------------------------------------------------------------
    void cell_leaf(cell_iter const&a,
		   leaf_iter const&b) const          // cell-leaf interaction   
    {
      CL.push(a,b);                                  // initialize stack        
      clear_cell_leaf_stack();                       // work until stack empty  
    }
    //--------------------------------------------------------------------------
    void leaf_cell(leaf_iter const&a,
		   cell_iter const&b) const          // leaf-cell interaction   
    {
      LC.push(a,b);                                  // initialize stack        
      clear_leaf_cell_stack();                       // work until stack empty  
    }
#ifdef falcON_MPI
    //--------------------------------------------------------------------------
    // routines for interactions in MPI parallel code                           
    //                                                                          
    // they allow to interrupt the clearing of the CX and/or CC stack, e.g., for
    // testing for the sending or receipt of an MPI message.                    
    // use init_..()   to add a new root-root interaction onto a stack          
    // use work_..()   to perform at most a pre-defined number of cell iactions 
    //                 (leaf-cell and leaf-leaf iaction are not counted)        
    //                 returns true if still some work is left.                 
    // use finish_..() to clear the stack                                       
    //--------------------------------------------------------------------------
    void init_cell_self(cell_iter const&a) const     // add cell-self iaction   
    {
      CX.push(a);                                    // initialize stack        
    }
    //--------------------------------------------------------------------------
    bool work_cell_self(                             // work on CX, performing  
			int const&M) const           // I: max # CX & CC iaction
    {
      work_cell_self_stack(M);                       // work at most M CC/CX    
      return !CX.is_empty();                         // still work left         
    }
    //--------------------------------------------------------------------------
    void finish_cell_self() const                    // clear CX stack          
    {
      clear_cell_self_stack();                       // work until stack empty  
    }
    //==========================================================================
    void init_cell_cell(cell_iter const&a,
			cell_iter const&b) const     // add cell-cell iaction   
    {
      CC.push(a,b);                                  // initialize stack        
    }
    //--------------------------------------------------------------------------
    bool work_cell_cell(                             // work on CC, performing  
			int const&M) const           // I: max # CC iaction     
    {
      work_cell_cell_stack(M);                       // work at most M CC       
      return !CC.is_empty();                         // still work left         
    }
    //--------------------------------------------------------------------------
    void finish_cell_cell() const                    // clear CC stack          
    {
      clear_cell_cell_stack();                       // work until stack empty  
    }
#endif  // falcON_MPI
    //--------------------------------------------------------------------------
  };
  //////////////////////////////////////////////////////////////////////////////
  //                                                                            
  // class nbdy::basic_iactor<ESTIMATOR>                                        
  //                                                                            
  // a base class for a possible template argument of MutualInteractor<>        
  //                                                                            
  // the template argument merely serves to provide the types cell_iterator and 
  // leaf_iterator.                                                             
  //                                                                            
  //////////////////////////////////////////////////////////////////////////////
  template<typename ESTIMATOR> class basic_iactor {
    //--------------------------------------------------------------------------
    // types of class basic_iactor                                              
    //--------------------------------------------------------------------------
  public:
    typedef ESTIMATOR                         estm_type;   // type of estimator 
    typedef typename estm_type::cell_iterator cell_iter;   // iterator over cell
    typedef typename estm_type::leaf_iterator leaf_iter;   // iterator over leaf
    //--------------------------------------------------------------------------
    // data of class basic_iactor                                               
    //--------------------------------------------------------------------------
  private:
    const int NCB, NCC, NCL;                         // params control direct   
    //--------------------------------------------------------------------------
    // abstract methods, MUST be provided by derived class                      
    //--------------------------------------------------------------------------
  protected:
    virtual bool many       (cell_iter const&, leaf_iter const&) const = 0;
    virtual bool many       (cell_iter const&, cell_iter const&) const = 0;
    virtual bool many       (cell_iter const&) const = 0;
    virtual void single     (leaf_iter const&, leaf_iter const&) const = 0;
    virtual bool discard    (cell_iter const&, leaf_iter const&) const = 0;
    virtual bool discard    (cell_iter const&, cell_iter const&) const = 0;
  public:
    virtual bool split_first(cell_iter const&, cell_iter const&) const = 0;
    //--------------------------------------------------------------------------
    // other methods                                                            
    //--------------------------------------------------------------------------
  protected:
    basic_iactor(const int dir[4] = Default::direct) :
      NCB(dir[1]), NCC(dir[2]), NCL(dir[3]) {}
    //--------------------------------------------------------------------------
  public:
    bool interact(cell_iter const&A, leaf_iter const&B) const {
      if(!is_active(A) && !is_active(B))return true;     // no actives -> no job
      if(discard(A,B))                  return true;     // try to discard      
      if(number(A) < NCB)               return many(A,B);// perform many single 
                                        return false;    // must split          
    }
    //--------------------------------------------------------------------------
    bool interact(leaf_iter const&B, cell_iter const&A) const {
      if(!is_active(A) && !is_active(B))return true;     // no actives -> no job
      if(discard(A,B))                  return true;     // try to discard      
      if(number(A) < NCB)               return many(A,B);// perform many single 
                                        return false;    // must split          
    }
    //--------------------------------------------------------------------------
    bool interact(cell_iter const&A, cell_iter const&B) const {
      if(!is_active(A) && !is_active(B))return true;     // no actives -> no job
      if(discard(A,B))                  return true;     // try to discard      
      if(number(A) < NCC &&
	 number(B) < NCC    )           return many(A,B);// perform many single 
                                        return false;    // must split          
    }
    //--------------------------------------------------------------------------
    bool interact(cell_iter const&A) const {
      if(!is_active(A))              return true;        // no actives -> no job
      if(number(A) < NCL)            return many(A);     // perform many single 
                                     return false;       // must split          
    }
    //--------------------------------------------------------------------------
    void interact(leaf_iter const&A, leaf_iter const&B) const {
      single(A,B);
    }
  };
  //////////////////////////////////////////////////////////////////////////////
  //                                                                          //
  // this code does not seem to be used anymore (WD 19/04/04)                 //
  //                                                                          //
  //////////////////////////////////////////////////////////////////////////////
  //                                                                          //
  // class nbdy::neighbour_counter                                            //
  //                                                                          //
  // for each active leaf: count # leafs with |R| < size(A);                  //
  // for individual sizes: requires:                                          //
  //     - each active leaf to hold its size and size^2.                      //
  //     - each active cell to hold its size                                  //
  // for global size, it is given as argument to the constructor              //
  //                                                                          //
  //////////////////////////////////////////////////////////////////////////////
  template<typename, bool>  class neighbour_counter;
  //////////////////////////////////////////////////////////////////////////////
  template<typename TREE>
  class neighbour_counter<TREE,true> : public basic_iactor<TREE> {
    //--------------------------------------------------------------------------
    // nbdy::neighbour_counter<true>                                            
    // MUST NOT be commented out via #ifdef falcON_INDI                         
    //--------------------------------------------------------------------------
  public:
    typedef typename TREE::leaf_iterator leaf_iter;
    typedef typename TREE::cell_iterator cell_iter;
  protected:
    //--------------------------------------------------------------------------
    void many(bool      const&all,
	      leaf_iter const&A,
	      leaf_iter const&B0,
	      leaf_iter const&BN) const {
      register real      Rq;
      register leaf_iter B;
      if(is_active(A))
	if(all)
	  for(B=B0; B<BN; B++) {
	    Rq = dist_sq(pos(A),pos(B));
	    if(Rq < sizeq(A)) A->inc();
	    if(Rq < sizeq(B)) B->inc();
	  }
	else
	  for(B=B0; B<BN; B++) {
	    Rq = dist_sq(pos(A),pos(B));
	    if(                Rq < sizeq(A)) A->inc();
	    if(is_active(B) && Rq < sizeq(B)) B->inc();
	  }
      else
	if(all)
	  for(B=B0; B<BN; B++) {
	    Rq = dist_sq(pos(A),pos(B));
	    if(Rq < sizeq(B)) B->inc();
	  }
	else
	  for(B=B0; B<BN; B++) if(is_active(B)) {
	    Rq = dist_sq(pos(A),pos(B));
	    if(Rq < sizeq(B)) B->inc();
	  }
    }
    //--------------------------------------------------------------------------
    void single(leaf_iter const&A, leaf_iter const&B) const {
      if(!is_active(A) && !is_active(B)) return;   // no actives -> no job      
      register real Rq = dist_sq(pos(A),pos(B));   // R^2 = distance^2          
      if(is_active(A) && Rq < sizeq(A)) A->inc();  // A is active && |R|<siza(A)
      if(is_active(B) && Rq < sizeq(B)) B->inc();  // B is active && |R|<siza(B)
    }
    //--------------------------------------------------------------------------
    bool discard(cell_iter const&A, leaf_iter const&B) const
    {
      return dist_sq(pos(A),pos(B)) >              // |xA-xB| >                 
	square(max(rmax(A)+size(B),size(A)));      // max(sA,rA+sB)  ?          
    }
    //--------------------------------------------------------------------------
    bool discard(cell_iter const&A, cell_iter const&B) const
    {
      return dist_sq(pos(A),pos(B)) >              // |xA-xB| >                 
	square(max(rmax(A)+size(B),rmax(B)+size(A))); // max(rb+sA,rA+sB)  ?    
    }
  public:
    //--------------------------------------------------------------------------
    neighbour_counter() {}
    //--------------------------------------------------------------------------
    bool split_first(cell_iter const&A, cell_iter const&B) const {
      return nbdy::is_twig(B) || size(A) > size(B);
    }
  };
  //////////////////////////////////////////////////////////////////////////////
  template<typename TREE>
  class neighbour_counter<TREE,false>
    : public basic_iactor<TREE> {
    //--------------------------------------------------------------------------
    // nbdy::neighbour_counter<false>                                           
    //--------------------------------------------------------------------------
  public:
    typedef typename TREE::leaf_iterator leaf_iter;
    typedef typename TREE::cell_iterator cell_iter;
  private:
    const real EPS,EPQ;
  protected:
    //--------------------------------------------------------------------------
    void many(bool      const&all,
	      leaf_iter const&A,
	      leaf_iter const&B0,
	      leaf_iter const&BN) const {
      register leaf_iter B;
      if(is_active(A))
	if(all)
	  for(B=B0; B<BN; B++) {
	    if(dist_sq(pos(A),pos(B)) < EPQ) { A->inc();
	    B->inc(); }
	  }
	else
	  for(B=B0; B<BN; B++) {
	    if(dist_sq(pos(A),pos(B)) < EPQ) {                  A->inc();
	    if(is_active(B)) B->inc(); }
	  }
      else
	if(all)
	  for(B=B0; B<BN; B++) {
	    if(dist_sq(pos(A),pos(B)) < EPQ) { B->inc(); }
	  }
	else
	  for(B=B0; B<BN; B++) 
	    if(is_active(B) && dist_sq(pos(A),pos(B)) < EPQ) { B->inc(); }
    }
    //--------------------------------------------------------------------------
    void single(leaf_iter const&A, leaf_iter const&B) const {
      if(!is_active(A) && !is_active(B)) return;   // no actives -> no job      
      if(dist_sq(pos(A),pos(B)) < EPQ) {           // distance^2 < eps^2        
	if(is_active(A)) A->inc();                 // A is active && |R|<siza(A)
	if(is_active(B)) B->inc();                 // B is active && |R|<siza(B)
      }
    }
    //--------------------------------------------------------------------------
    bool discard(cell_iter const&A, leaf_iter const&B) const
    {
      return dist_sq(pos(A),pos(B)) >                // |xA-xB| >               
	square(max(rmax(A)+EPS,size(A)));            // max(sA,rA+sB)  ?        
    }
    //--------------------------------------------------------------------------
    bool discard(cell_iter const&A, cell_iter const&B) const
    {
      return dist_sq(pos(A),pos(B)) >                // |xA-xB| >               
	square(max(rmax(A)+size(B),rmax(B)+size(A)));// max(rb+sA,rA+sB)  ?     
    }
  public:
    //--------------------------------------------------------------------------
    neighbour_counter(const real e) : EPS(e), EPQ(e*e) {}
    //--------------------------------------------------------------------------
    bool split_first(cell_iter const&A, cell_iter const&B) const {
      return is_twig(B) || size(A) > size(B); }
  };
  //////////////////////////////////////////////////////////////////////////////
}                                                  // END: namespace nbdy       
////////////////////////////////////////////////////////////////////////////////
#endif                                             // falcON_included_iact_h    
