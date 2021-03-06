#ifndef INTREPID2_HGRAD_TET_CN_FEM_ORTHDEF_HPP
#define INTREPID2_HGRAD_TET_CN_FEM_ORTHDEF_HPP
// @HEADER
// ************************************************************************
//
//                           Intrepid2 Package
//                 Copyright (2007) Sandia Corporation
//
// Under terms of Contract DE-AC04-94AL85000, there is a non-exclusive
// license for use of this work by or on behalf of the U.S. Government.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
// 1. Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
//
// 3. Neither the name of the Corporation nor the names of the
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY SANDIA CORPORATION "AS IS" AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL SANDIA CORPORATION OR THE
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Questions? Contact Kyungjoo Kim  (kyukim@sandia.gov), or
//                    Mauro Perego  (mperego@sandia.gov)
//
// ************************************************************************
// @HEADER

/** \file   Intrepid_HGRAD_TET_Cn_FEM_ORTHDef.hpp
    \brief  Definition file for FEM orthogonal basis functions of arbitrary degree 
    for H(grad) functions on TET.
    \author Created by R. Kirby
*/

namespace Intrepid2 {

  template<class Scalar, class ArrayScalar>
  Basis_HGRAD_TET_Cn_FEM_ORTH<Scalar,ArrayScalar>::Basis_HGRAD_TET_Cn_FEM_ORTH( ordinal_type degree )
  {
    this -> basisCardinality_  = (degree+1)*(degree+2)*(degree+3)/6;
    this -> basisDegree_       = degree;
    this -> basisCellTopology_ = shards::CellTopology(shards::getCellTopologyData<shards::Tetrahedron<4> >() );
    this -> basisType_         = BASIS_FEM_HIERARCHICAL;
    this -> basisCoordinates_  = COORDINATES_CARTESIAN;
    this -> basisTagsAreSet_   = false;

    initializeTags();
    this->basisTagsAreSet_ = true;
  }
  
  
  
  template<class Scalar, class ArrayScalar>
  void Basis_HGRAD_TET_Cn_FEM_ORTH<Scalar, ArrayScalar>::initializeTags() {
  
    // Basis-dependent initializations
    ordinal_type tagSize  = 4;        // size of DoF tag, i.e., number of fields in the tag
    ordinal_type posScDim = 0;        // position in the tag, counting from 0, of the subcell dim 
    ordinal_type posScOrd = 1;        // position in the tag, counting from 0, of the subcell ordinal
    ordinal_type posDfOrd = 2;        // position in the tag, counting from 0, of DoF ordinal relative to the subcell
  
    // An array with local DoF tags assigned to the basis functions, in the order of their local enumeration 
    ordinal_type *tags = new ordinal_type[tagSize * this->getCardinality()];
    for (ordinal_type i=0;i<this->getCardinality();i++) {
      tags[4*i] = 2;
      tags[4*i+1] = 0;
      tags[4*i+2] = i;
      tags[4*i+3] = this->getCardinality();
    }

    // Basis-independent function sets tag and enum data in tagToOrdinal_ and ordinalToTag_ arrays:
    Intrepid2::setOrdinalTagData(this -> tagToOrdinal_,
                                this -> ordinalToTag_,
                                tags,
                                this -> basisCardinality_,
                                tagSize,
                                posScDim,
                                posScOrd,
                                posDfOrd);
  }  
  


  template<class Scalar, class ArrayScalar> 
  void Basis_HGRAD_TET_Cn_FEM_ORTH<Scalar, ArrayScalar>::getValues(ArrayScalar &        outputValues,
                                                                  const ArrayScalar &  inputPoints,
                                                                  const EOperator      operatorType) const {
  
    // Verify arguments
#ifdef HAVE_INTREPID2_DEBUG
    Intrepid2::getValues_HGRAD_Args<Scalar, ArrayScalar>(outputValues,
                                                        inputPoints,
                                                        operatorType,
                                                        this -> getBaseCellTopology(),
                                                        this -> getCardinality() );
#endif
    const ordinal_type deg = this->getDegree();
  
    switch (operatorType) {
    case OPERATOR_VALUE:
      {
        TabulatorTet<Scalar,ArrayScalar,0>::tabulate( outputValues ,
                                                      deg ,
                                                      inputPoints );
      }
      break;
    case OPERATOR_GRAD:
    case OPERATOR_D1:
      {
        TabulatorTet<Scalar,ArrayScalar,1>::tabulate( outputValues ,
                                                      deg ,
                                                      inputPoints );
      }
      break;
    default:
      TEUCHOS_TEST_FOR_EXCEPTION( true , std::invalid_argument,
                          ">>> ERROR (Basis_HGRAD_TET_Cn_FEM_ORTH): invalid or unsupported operator" );
    }

    return;
  }
  
  template<class Scalar, class ArrayScalar>
  void Basis_HGRAD_TET_Cn_FEM_ORTH<Scalar, ArrayScalar>::getValues(ArrayScalar&           outputValues,
                                                                  const ArrayScalar &    inputPoints,
                                                                  const ArrayScalar &    cellVertices,
                                                                  const EOperator        operatorType) const {
    TEUCHOS_TEST_FOR_EXCEPTION( (true), std::logic_error,
                        ">>> ERROR (Basis_HGRAD_TET_Cn_FEM_ORTH): FEM Basis calling an FVD member function");
  }

  template<class Scalar, class ArrayScalar>
  void TabulatorTet<Scalar,ArrayScalar,0>::tabulate( ArrayScalar &outputValues ,
                                                    const ordinal_type deg ,
                                                    const ArrayScalar &z )
  {
    const ordinal_type np = z.dimension( 0 );
    ordinal_type idxcur;
  
    // each point needs to be transformed from Pavel's element
    // z(i,0) --> (2.0 * z(i,0) - 1.0)
    // z(i,1) --> (2.0 * z(i,1) - 1.0)
    // z(i,2) --> (2.0 * z(i,2) - 1.0)
  
    Teuchos::Array<Scalar> f1(np),f2(np),f3(np),f4(np),f5(np);
  
    for (ordinal_type i=0;i<np;i++) {
      f1[i] = 0.5 * ( 2.0 + 2.0*(2.0*z(i,0)-1.0) + (2.0*z(i,1)-1.0) + (2.0*z(i,2)-1.0) );
      Scalar foo =  0.5 * ( (2.0*z(i,1)-1.0) + (2.0*z(i,2)-1.0) );
      f2[i] = foo * foo;
      f3[i] = 0.5 * ( 1.0 + 2.0 * (2.0*z(i,1)-1.0) + (2.0*z(i,2)-1.0) );
      f4[i] = 0.5 * ( 1.0 - (2.0*z(i,2)-1.0) );
      f5[i] = f4[i] * f4[i];
    }

    // constant term
    idxcur = TabulatorTet<Scalar,ArrayScalar,0>::idx(0,0,0);
    for (ordinal_type i=0;i<np;i++) {
      outputValues(idxcur,i) = 1.0 + z(i,0) - z(i,0) + z(i,1) - z(i,1) + z(i,2) - z(i,2);
    }
  
    if (deg > 0) {

      // D^{1,0,0}
      idxcur = TabulatorTet<Scalar,ArrayScalar,0>::idx(1,0,0);
      for (ordinal_type i=0;i<np;i++) {
        outputValues(idxcur,i) = f1[i];
      }
  
      // p recurrence
      for (ordinal_type p=1;p<deg;p++) {
        Scalar a1 = (2.0 * p + 1.0) / ( p + 1.0);
        Scalar a2 = p / ( p + 1.0 );
        ordinal_type idxp = TabulatorTet<Scalar,ArrayScalar,0>::idx(p,0,0);
        ordinal_type idxpp1 = TabulatorTet<Scalar,ArrayScalar,0>::idx(p+1,0,0);
        ordinal_type idxpm1 = TabulatorTet<Scalar,ArrayScalar,0>::idx(p-1,0,0);
        for (ordinal_type i=0;i<np;i++) {
          outputValues(idxpp1,i) = a1 * f1[i] * outputValues(idxp,i) - a2 * f2[i] * outputValues(idxpm1,i);
        }
      }
      // q = 1
      for (ordinal_type p=0;p<deg;p++) {
        ordinal_type idx0 = TabulatorTet<Scalar,ArrayScalar,0>::idx(p,0,0);
        ordinal_type idx1 = TabulatorTet<Scalar,ArrayScalar,0>::idx(p,1,0);
        for (ordinal_type i=0;i<np;i++) {
          outputValues(idx1,i) = outputValues(idx0,i) * ( p * ( 1.0 + (2.0*z(i,1)-1.0) ) +
                                                          0.5 * ( 2.0 + 3.0 * (2.0*z(i,1)-1.0) + (2.0*z(i,2)-1.0) ) );
        }
      }
  
      // q recurrence
      for (ordinal_type p=0;p<deg-1;p++) {
        for (ordinal_type q=1;q<deg-p;q++) {
          Scalar aq,bq,cq;

	  TabulatorTet<Scalar,ArrayScalar,0>::jrc(2.0*p+1.0 ,0 ,q, aq, bq, cq);
          ordinal_type idxpqp1 = TabulatorTet<Scalar,ArrayScalar,0>::idx(p,q+1,0);
          ordinal_type idxpq = TabulatorTet<Scalar,ArrayScalar,0>::idx(p,q,0);
          ordinal_type idxpqm1 = TabulatorTet<Scalar,ArrayScalar,0>::idx(p,q-1,0);
          for (ordinal_type i=0;i<np;i++) {
            outputValues(idxpqp1,i) = ( aq * f3[i] + bq * f4[i] ) * outputValues(idxpq,i) 
              - ( cq * f5[i] ) * outputValues(idxpqm1,i);
          }
        }
      }
  
      // r = 1
      for (ordinal_type p=0;p<deg;p++) {
        for (ordinal_type q=0;q<deg-p;q++) {
          ordinal_type idxpq1 = TabulatorTet<Scalar,ArrayScalar,0>::idx(p,q,1);
          ordinal_type idxpq0 = TabulatorTet<Scalar,ArrayScalar,0>::idx(p,q,0);
          for (ordinal_type i=0;i<np;i++) {
            outputValues(idxpq1,i) = outputValues(idxpq0,i) * ( 1.0 + p + q + ( 2.0 + q + 
                                                                                p ) * (2.0*z(i,2)-1.0) );
          }
        }
      }
      // general r recurrence
      for (ordinal_type p=0;p<deg-1;p++) {
        for (ordinal_type q=0;q<deg-p-1;q++) {
          for (ordinal_type r=1;r<deg-p-q;r++) {
            Scalar ar,br,cr;
            ordinal_type idxpqrp1 = TabulatorTet<Scalar,ArrayScalar,0>::idx(p,q,r+1);
            ordinal_type idxpqr = TabulatorTet<Scalar,ArrayScalar,0>::idx(p,q,r);
            ordinal_type idxpqrm1 = TabulatorTet<Scalar,ArrayScalar,0>::idx(p,q,r-1);
            jrc(2.0*p+2.0*q+2.0, 0.0, r, ar, br, cr);
            for (ordinal_type i=0;i<np;i++) {
              outputValues(idxpqrp1,i) = (ar * (2.0*z(i,2)-1.0) + br) * outputValues( idxpqr , i ) - cr * outputValues(idxpqrm1,i);
            }
          }
        }
      }

    }  
    // normalize
    for (ordinal_type p=0;p<=deg;p++) {
      for (ordinal_type q=0;q<=deg-p;q++) {
        for (ordinal_type r=0;r<=deg-p-q;r++) {
          ordinal_type idxcur = TabulatorTet<Scalar,ArrayScalar,0>::idx(p,q,r);
          Scalar scal = sqrt( (p+0.5)*(p+q+1.0)*(p+q+r+1.5) );
          for (ordinal_type i=0;i<np;i++) {
            outputValues(idxcur,i) *= scal;
          }
        }
      }
    }
  
    return;
  
  }


  template<typename Scalar, typename ArrayScalar>
  void TabulatorTet<Scalar,ArrayScalar,1>::tabulate( ArrayScalar &outputValues ,
                                                    const ordinal_type deg ,
                                                    const ArrayScalar &z ) 
  {
    const ordinal_type np = z.dimension(0);
    const ordinal_type card = outputValues.dimension(0);
    FieldContainer<Sacado::Fad::SFad<Scalar,3> > dZ( z.dimension(0) , z.dimension(1) );
    for (ordinal_type i=0;i<np;i++) {
      for (ordinal_type j=0;j<3;j++) {
        dZ(i,j) = Sacado::Fad::SFad<Scalar,3>( z(i,j) );
        dZ(i,j).diff(j,3);
      }
    }
    FieldContainer<Sacado::Fad::SFad<Scalar,3> > dResult(card,np);

    TabulatorTet<Sacado::Fad::SFad<Scalar,3>,FieldContainer<Sacado::Fad::SFad<Scalar,3> >,0>::tabulate( dResult ,
                                                                                                    deg ,
                                                                                                    dZ );

    for (ordinal_type i=0;i<card;i++) {
      for (ordinal_type j=0;j<np;j++) {
        for (ordinal_type k=0;k<3;k++) {
          outputValues(i,j,k) = dResult(i,j).dx(k);
        }
      }
    }

    return;

  }


}// namespace Intrepid2


#endif
