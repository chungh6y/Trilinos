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


/** \file
    \brief  Unit test (CubatureDirect,CubatureTensor): 
            correctness of volume computations for reference cells.
    \author Created by P. Bochev, D. Ridzal and Kyungjoo Kim
*/

#include "Intrepid2_config.h"

#ifdef HAVE_INTREPID2_DEBUG
#define INTREPID2_TEST_FOR_DEBUG_ABORT_OVERRIDE_TO_CONTINUE
#endif

#include "Intrepid2_Types.hpp"
#include "Intrepid2_Utils.hpp"

#include "Intrepid2_CubatureDirectLineGauss.hpp"
//#include "Intrepid2_CubatureTensor.hpp"

//#include "Intrepid2_CubatureDirectLineGaussJacobi20.hpp"
//#include "Intrepid2_CubatureDirectTriDefault.hpp"
//#include "Intrepid2_CubatureDirectTetDefault.hpp"
//#include "Intrepid2_CubatureTensorPyr.hpp"

#include "Teuchos_oblackholestream.hpp"
#include "Teuchos_RCP.hpp"

namespace Intrepid2 {

  namespace Test {
#define INTREPID2_TEST_ERROR_EXPECTED( S, nthrow, ncatch )              \
    try {                                                               \
      ++nthrow;                                                         \
      S ;                                                               \
    } catch (std::logic_error err) {                                    \
      ++ncatch;                                                         \
      *outStream << "Expected Error ----------------------------------------------------------------\n"; \
      *outStream << err.what() << '\n';                                 \
      *outStream << "-------------------------------------------------------------------------------" << "\n\n"; \
    };                                                                  \

    template<typename cubWeightViewType>
    typename cubWeightViewType::value_type 
    computeRefVolume(const ordinal_type      numPoints, 
                     const cubWeightViewType cubWeights) {
      typename cubWeightViewType::value_type r_val = 0.0;
      for (auto i=0;i<numPoints;++i)
        r_val += cubWeights(i);

      return r_val;
    }

    template<typename ValueType, typename DeviceSpaceType>
    int Integration_Test01(const bool verbose) {
      typedef ValueType value_type;

      Teuchos::RCP<std::ostream> outStream;
      Teuchos::oblackholestream bhs; // outputs nothing

      if (verbose)
        outStream = Teuchos::rcp(&std::cout, false);
      else
        outStream = Teuchos::rcp(&bhs, false);

      Teuchos::oblackholestream oldFormatState;
      oldFormatState.copyfmt(std::cout);

      typedef typename
        Kokkos::Impl::is_space<DeviceSpaceType>::host_mirror_space::execution_space HostSpaceType ;

      *outStream << "DeviceSpace::  "; DeviceSpaceType::print_configuration(*outStream, false);
      *outStream << "HostSpace::    ";   HostSpaceType::print_configuration(*outStream, false);

      *outStream 
        << "===============================================================================\n"
        << "|                                                                             |\n"
        << "|                  Unit Test (CubatureDirect,CubatureTensor)                  |\n"
        << "|                                                                             |\n"
        << "|     1) Computing volumes of reference cells                                 |\n"
        << "|                                                                             |\n"
        << "|  Questions? Contact  Pavel Bochev (pbboche@sandia.gov) or                   |\n"
        << "|                      Denis Ridzal (dridzal@sandia.gov) or                   |\n"
        << "|                      Kyungjoo Kim (kyukim@sandia.gov).                      |\n"
        << "|                                                                             |\n"
        << "|  Intrepid's website: http://trilinos.sandia.gov/packages/intrepid           |\n"
        << "|  Trilinos website:   http://trilinos.sandia.gov                             |\n"
        << "|                                                                             |\n"
        << "===============================================================================\n";
      
      //typedef FunctionSpaceTools<DeviceSpaceType> fst;
      typedef Kokkos::DynRankView<value_type,DeviceSpaceType> DynRankView;
#define ConstructWithLabel(obj, ...) obj(#obj, __VA_ARGS__)

      const auto tol = 100.0 * Parameters::Tolerence;
      
      int errorFlag  = 0;
      
      *outStream                               
        << "\n"
        << "===============================================================================\n"
        << "| TEST 1: exception                                                           |\n"
        << "===============================================================================\n";

      try {
        ordinal_type nthrow = 0, ncatch = 0;
#ifdef HAVE_INTREPID2_DEBUG
        *outStream << "-> Line testing\n\n";
        {
          INTREPID2_TEST_ERROR_EXPECTED( CubatureDirectLineGauss<DeviceSpaceType>(-1), nthrow, ncatch );
          INTREPID2_TEST_ERROR_EXPECTED( CubatureDirectLineGauss<DeviceSpaceType>(Parameters::MaxCubatureDegreeEdge+1), nthrow, ncatch );
        }
        
        *outStream << "-> Triangle testing\n\n";
        // {
        //   INTREPID2_TEST_ERROR_EXPECTED( CubatureDirectTriDefault<DeviceSpaceType> triCub(-1), nthrow, ncatch );
        //   INTREPID2_TEST_ERROR_EXPECTED( CubatureDirectTriDefault<DeviceSpaceType> triCub(Parameters::MaxCubatureDegreeTri+1), nthrow, ncatch );
        // }

        *outStream << "-> Tetrahedron testing\n\n";
        // {
        //   INTREPID2_TEST_ERROR_EXPECTED( CubatureDirectTetDefault<DeviceSpaceType> tetCub(-1), nthrow, ncatch );
        //   INTREPID2_TEST_ERROR_EXPECTED( CubatureDirectTetDefault<DeviceSpaceType> tetCub(Parameters::MaxCubatureDegreeTet+1), nthrow, ncatch );
        // }
#endif 
        if (nthrow != ncatch) {
          errorFlag++;
          *outStream << std::setw(70) << "^^^^----FAILURE!" << "\n";
          *outStream << "# of catch ("<< ncatch << ") is different from # of throw (" << ncatch << ")\n";
        }

      } catch (std::logic_error err) {
        *outStream << err.what() << "\n";
        errorFlag = -1000;
      };


      // *outStream                               
      //   << "\n"
      //   << "===============================================================================\n"
      //   << "| TEST 2: basic functionalities                                               |\n"
      //   << "===============================================================================\n";

      // try {

      //   *outStream << "-> Line testing\n\n";
      //   {
      //     CubatureDirectLineGauss<DeviceSpaceType> lineCub(4);
      //     INTREPID2_TEST_FOR_EXCEPTION( lineCub.getDimension() != 1, std::logic_error,
      //                                   ">>> ERROR (Integration::Test01): line cubature must have 1 dimension.");
      //     INTREPID2_TEST_FOR_EXCEPTION( lineCub.getAccuracy() != 4, std::logic_error,
      //                                   ">>> ERROR (Integration::Test01): line cubature reports wrong accuracy.");
      //   }
        
      //   *outStream << "-> Triangle testing\n\n";
      //   // {
      //   //   CubatureDirectTriDefault<DeviceSpaceType> triCub(17);
      //   //   INTREPID2_TEST_FOR_EXCEPTION( triCub.getNumPoints() != 61, std::logic_error, 
      //   //                                 ">>> ERROR (Integration::Test01): triangle cubature reports a wrong number of points.");
      //   //   INTREPID2_TEST_FOR_EXCEPTION( triCub.getDimension() != 2, std::logic_error, 
      //   //                                 ">>> ERROR (Integration::Test01): triangle cubature reports a wrong dimension.");
      //   // }

      //   *outStream << "-> Tetrahedron testing\n\n";
      //   // {
      //   //   CubatureDirectTetDefault<DeviceSpaceType> tetCub(17);
      //   //   INTREPID2_TEST_FOR_EXCEPTION( tetCub.getNumPoints() != 495, std::logic_error, 
      //   //                                 ">>> ERROR (Integration::Test01): tetrahedron cubature reports a wrong number of points.");
      //   //   INTREPID2_TEST_FOR_EXCEPTION( tetCub.getDimension() != 3, std::logic_error,
      //   //                                 ">>> ERROR (Integration::Test01): tetrahedron cubature reports a wrong dimension.");
      //   // }


      //   *outStream << "-> Quad testing\n\n";                
      //   // {
      //   //   typedef CubatureDirectLineGauss<DeviceSpaceType> lineCubatureType;
      //   //   auto quadCub = createCubatureTensor( lineCubatureType(3), lineCubatureType(7) );

      //   //   INTREPID2_TEST_FOR_EXCEPTION( quadCub.getDimension != 2, std::logic_error,
      //   //                                 ">>> ERROR (Integration:_Test01): quad cubature must have 2 dimension.");

      //   //   ordinal_type accuracy[Parameters::MaxDimension];
      //   //   quadCub.getAccuracy( accuracy );
      //   //   INTREPID2_TEST_FOR_EXCEPTION( accuracy[0] != 3 || accuracy[1] != 7, std::logic_error,
      //   //                                 ">>> ERROR (Integration:_Test01): quad cubature reports wrong accuracy.");

      //   // }

      //   // {
      //   //   Teuchos::RCP< Cubature<DeviceSpaceType> > lineCubX = Teuchos::rcp(new CubatureDirectLineGauss<DeviceSpaceType>(3));
      //   //   Teuchos::RCP< Cubature<DeviceSpaceType> > lineCubY = Teuchos::rcp(new CubatureDirectLineGauss<DeviceSpaceType>(7));
          
      //   //   CubatureTensor quadCub( lineCubX, lineCubY );
      //   //   INTREPID2_TEST_FOR_EXCEPTION( quadCub.getDimension != 2, std::logic_error,
      //   //                                 ">>> ERROR (Integration:_Test01): quad cubature must have 2 dimension.");

      //   //   ordinal_type accuracy[Parameters::MaxDimension];
      //   //   quadCub.getAccuracy( accuracy );
      //   //   INTREPID2_TEST_FOR_EXCEPTION( accuracy[0] != 3 || accuracy[1] != 7, std::logic_error,
      //   //                                 ">>> ERROR (Integration:_Test01): quad cubature reports wrong accuracy.");

      //   // }

      //   //
      //   // Hex
      //   // 
      //   // {
      //   //   Teuchos::RCP< Cubature<DeviceSpaceType> > lineCubX = Teuchos::rcp(new CubatureDirectLineGauss<DeviceSpaceType>(1));
      //   //   Teuchos::RCP< Cubature<DeviceSpaceType> > lineCubY = Teuchos::rcp(new CubatureDirectLineGauss<DeviceSpaceType>(4));
      //   //   Teuchos::RCP< Cubature<DeviceSpaceType> > lineCubZ = Teuchos::rcp(new CubatureDirectLineGauss<DeviceSpaceType>(2));
          
      //   //   CubatureTensor hexCub( lineCubX, lineCubY, lineCubZ );
      //   //   INTREPID2_TEST_FOR_EXCEPTION( hexCub.getDimension != 3, std::logic_error,
      //   //                                 ">>> ERROR (Integration:_Test01): hex cubature must have 2 dimension.");

      //   //   ordinal_type accuracy[Parameters::MaxDimension];
      //   //   hexCub.getAccuracy( accuracy );
      //   //   INTREPID2_TEST_FOR_EXCEPTION( accuracy[0] != 1 || accuracy[1] != 4 || accuracy[2] != 2, std::logic_error,
      //   //                                 ">>> ERROR (Integration:_Test01): hex cubature reports wrong accuracy.");

      //   // }

      //   //
      //   // Prism
      //   // 
      //   // {
      //   //   Teuchos::RCP< Cubature<DeviceSpaceType> > lineCubXY = Teuchos::rcp(new CubatureDirectTriGauss<DeviceSpaceType>(4));
      //   //   Teuchos::RCP< Cubature<DeviceSpaceType> > lineCubZ = Teuchos::rcp(new CubatureDirectLineGauss<DeviceSpaceType>(3));
          
      //   //   CubatureTensor prismCub( lineCubXY, lineCubZ );
      //   //   INTREPID2_TEST_FOR_EXCEPTION( prismCub.getDimension != 3, std::logic_error,
      //   //                                 ">>> ERROR (Integration:_Test01): prism cubature must have 2 dimension.");

      //   //   ordinal_type accuracy[Parameters::MaxDimension];
      //   //   prismCub.getAccuracy( accuracy );
      //   //   INTREPID2_TEST_FOR_EXCEPTION( accuracy[0] != 4 || accuracy[1] != 2, std::logic_error,
      //   //                                 ">>> ERROR (Integration:_Test01): prism cubature reports wrong accuracy.");

      //   // }

                          
      //   // INTREPID2_TEST_ERROR_EXPECTED( std::vector< Teuchos::RCP< Cubature<DeviceSpaceType> > > miscCubs(3);
      //   //                                miscCubs[0] = Teuchos::rcp(new CubatureDirectLineGauss<DeviceSpaceType>(3));
      //   //                                miscCubs[1] = Teuchos::rcp(new CubatureDirectLineGauss<DeviceSpaceType>(7));
      //   //                                miscCubs[2] = Teuchos::rcp(new CubatureDirectLineGauss<DeviceSpaceType>(5));
      //   //                                CubatureTensor<DeviceSpaceType> tensorCub(miscCubs);
      //   //                                FieldContainer<DeviceSpaceType> points(tensorCub.getNumPoints(), tensorCub.getDimension());
      //   //                                FieldContainer<DeviceSpaceType> weights(tensorCub.getNumPoints());
      //   //                                tensorCub.getCubature(points, weights)
      //   //                                );
        
          
      //   // INTREPID2_TEST_ERROR_EXPECTED( Teuchos::RCP< CubatureDirect<DeviceSpaceType> > lineCub = Teuchos::rcp(new CubatureDirectLineGauss<DeviceSpaceType>(15));
      //   //                                Teuchos::RCP< CubatureDirect<DeviceSpaceType> > triCub = Teuchos::rcp(new CubatureDirectTriDefault<DeviceSpaceType>(12));
      //   //                                CubatureTensor<DeviceSpaceType> tensorCub(lineCub, triCub);
      //   //                                std::vector<int> a(2); a[0] = 15; a[1] = 12;
      //   //                                std::vector<int> atest(2);
      //   //                                tensorCub.getAccuracy(atest);
      //   //                                TEUCHOS_TEST_FOR_EXCEPTION( (tensorCub.getDimension() != 3) || (a != atest),
      //   //                                                            std::logic_error,
      //   //                                                            "Check constructormembers dimension and getAccuracy!" ) );
      //   // INTREPID2_TEST_ERROR_EXPECTED( Teuchos::RCP< CubatureDirect<DeviceSpaceType> > lineCub = Teuchos::rcp(new CubatureDirectLineGauss<DeviceSpaceType>(15));
      //   //                                Teuchos::RCP< CubatureDirect<DeviceSpaceType> > triCub = Teuchos::rcp(new CubatureDirectTriDefault<DeviceSpaceType>(12));
      //   //                                CubatureTensor<DeviceSpaceType> tensorCub(triCub, lineCub, triCub);
      //   //                                std::vector<int> a(3); a[0] = 12; a[1] = 15; a[2] = 12;
      //   //                                std::vector<int> atest(3);
      //   //                                tensorCub.getAccuracy(atest);
      //   //                                TEUCHOS_TEST_FOR_EXCEPTION( (tensorCub.getDimension() != 5) || (a != atest),
      //   //                                                            std::logic_error,
      //   //                                                            "Check constructor and members dimension and getAccuracy!" ) );
        
      //   // INTREPID2_TEST_ERROR_EXPECTED( Teuchos::RCP< CubatureDirect<DeviceSpaceType> > triCub = Teuchos::rcp(new CubatureDirectTriDefault<DeviceSpaceType>(12));
      //   //                                CubatureTensor<DeviceSpaceType> tensorCub(triCub, 5);
      //   //                                std::vector<int> a(5); a[0] = 12; a[1] = 12; a[2] = 12; a[3] = 12; a[4] = 12;
      //   //                                std::vector<int> atest(5);
      //   //                                tensorCub.getAccuracy(atest);
      //   //                                TEUCHOS_TEST_FOR_EXCEPTION( (tensorCub.getDimension() != 10) || (a != atest),
      //   //                                                            std::logic_error,
      //   //                                                            "Check constructor and members dimension and getAccuracy!" ) );
      // } catch (std::logic_error err) {
      //   *outStream << err.what() << "\n";
      //   errorFlag = -1000;
      // };
 
      // *outStream
      //   << "===============================================================================\n"
      //   << "| TEST 3: volume computations                                                 |\n"
      //   << "===============================================================================\n";

      // try {
        

      //   *outStream << "-> Line testing\n\n";
      //   {
      //     for (auto deg=0;deg<=Parameters::MaxCubatureDegreeEdge;++deg) {
      //       CubatureDirectLineGauss<DeviceSpaceType> cub(deg);
      //       cub.getCubature(cubPoints, cubWeights);
      //       const auto testVol = computeRefVolume(line, deg);
      //       const auto refVol  = 2.0;
      //       if (std::abs(testVol - refVol) > tol) {
      //         *outStream << std::setw(30) << "Line volume --> " << std::setw(10) << std::scientific << testVol <<
      //           std::setw(10) << "diff = " << std::setw(10) << std::scientific << std::abs(testVol - refVol) << "\n";
      //         ++errorFlag;
      //         *outStream << std::setw(70) << "^^^^----FAILURE!" << "\n";
      //       }
      //     }
      //   }

      //   *outStream << "-> Triangle testing\n\n";
      //   // {
      //   //   shards::CellTopology tri(shards::getCellTopologyData< shards::Triangle<> >());
      //   //   for (auto deg=0;deg<=Parameters::MaxCubatureDegreeTri;++deg) {
      //   //     const auto testVol = computeRefVolume(tri, deg);
      //   //     const auto refVol  = 0.5;
      //   //     if (std::abs(testVol - refVol) > tol) {
      //   //       *outStream << std::setw(30) << "Triangle volume --> " << std::setw(10) << std::scientific << testVol <<
      //   //         std::setw(10) << "diff = " << std::setw(10) << std::scientific << std::abs(testVol - refVol) << "\n";
      //   //       ++errorFlag;
      //   //       *outStream << std::setw(70) << "^^^^----FAILURE!" << "\n";
      //   //     }
      //   //   }
      //   // }
        
      //   *outStream << "-> Quad testing\n\n";
      //   {
      //     shards::CellTopology quad(shards::getCellTopologyData< shards::Quadrilateral<> >());
      //     for (auto y_deg=0;y_deg<=Parameters::MaxCubatureDegreeEdge;++y_deg) 
      //       for (auto x_deg=0;x_deg<=Parameters::MaxCubatureDegreeEdge;++x_deg) {
      //         const auto testVol = computeRefVolume(quad, deg);
      //         const auto refVol  = 4.0;
      //         if (std::abs(testVol - refVol) > tol) {
      //           *outStream << std::setw(30) << "Quadrilateral volume --> " << std::setw(10) << std::scientific << testVol <<
      //             std::setw(10) << "diff = " << std::setw(10) << std::scientific << std::abs(testVol - refVol) << "\n";
                
      //           ++errorFlag;
      //           *outStream << std::setw(70) << "^^^^----FAILURE!" << "\n";
      //         }
      //       }
      //   }

      //   *outStream << "-> Tetrahedron testing\n\n";
      //   //  {
      //   //   shards::CellTopology tet(shards::getCellTopologyData< shards::Tetrahedron<> >());
      //   //   for (auto deg=0;deg<=Parameters::MaxCubatureDegreeTet;++deg) {
      //   //     const auto testVol = computeRefVolume(tet, deg);
      //   //     const auto refVol  = 1.0/6.0;
      //   //     if (std::abs(testVol - refVol) > tol) {
      //   //       *outStream << std::setw(30) << "Tetrahedron volume --> " << std::setw(10) << std::scientific << testVol <<
      //   //         std::setw(10) << "diff = " << std::setw(10) << std::scientific << std::abs(testVol - refVol) << "\n";
              
      //   //       ++errorFlag;
      //   //       *outStream << std::setw(70) << "^^^^----FAILURE!" << "\n";
      //   //     }
      //   //   }
      //   // }

      //   *outStream << "-> Hexahedron testing\n\n";
      //   {
      //     shards::CellTopology hex(shards::getCellTopologyData< shards::Hexahedron<> >());
      //     for (auto z_deg=0;z_deg<=Parameters::MaxCubatureDegreeEdge;++z+deg) 
      //       for (auto y_deg=0;y_deg<=Parameters::MaxCubatureDegreeEdge;++y_deg) 
      //         for (auto x_deg=0;x_deg<=Parameters::MaxCubatureDegreeEdge;++x_deg) {
      //           const auto testVol = computeRefVolume(hex, deg);
      //           const auto refVol  = 8.0;
      //           if (std::abs(testVol - refVol) > tol) {
      //             *outStream << std::setw(30) << "Hexahedron volume --> " << std::setw(10) << std::scientific << testVol <<
      //               std::setw(10) << "diff = " << std::setw(10) << std::scientific << std::abs(testVol - refVol) << "\n";
      //             ++errorFlag;
      //             *outStream << std::setw(70) << "^^^^----FAILURE!" << "\n";
      //           }
      //         }
      //   }

      //   *outStream << "-> Prism testing\n\n";
      //   // {
      //   //   shards::CellTopology wedge(shards::getCellTopologyData< shards::Wedge<> >());
      //   //   for (auto z_deg=0;z_deg<=Parameters::MaxCubatureDegreeEdge;++z_deg) 
      //   //     for (auto xy_deg=0;xy_deg<=Parameters::MaxCubatureDegreeTri;++xy_deg) {
      //   //       const auto testVol = computeRefVolume(wedge, deg);
      //   //       const auto refVol  = 1.0;
      //   //       if (std::abs(testVol - refVol) > tol) {
      //   //         *outStream << std::setw(30) << "Wedge volume --> " << std::setw(10) << std::scientific << testVol <<
      //   //           std::setw(10) << "diff = " << std::setw(10) << std::scientific << std::abs(testVol - refVol) << "\n";
      //   //         ++errorFlag;
      //   //         *outStream << std::setw(70) << "^^^^----FAILURE!" << "\n";
      //   //       }
      //   //     }
      //   // }

      //   *outStream << "-> Pyramid testing\n\n";
      //   // {
      //   //   shards::CellTopology pyr(shards::getCellTopologyData< shards::Pyramid<> >());
      //   //   for (int deg=0; deg<=std::min(INTREPID2_CUBATURE_LINE_GAUSS_MAX,INTREPID2_CUBATURE_LINE_GAUSSJACOBI20_MAX); deg++) {
      //   //     const auto testVol = computeRefVolume(pyr, deg);
      //   //     const auto refVol  = 4.0/3.0;
      //   //     if (std::abs(testVol - refVol) > tol) {
      //   //       *outStream << std::setw(30) << "Pyramid volume --> " << std::setw(10) << std::scientific << testVol <<
      //   //         std::setw(10) << "diff = " << std::setw(10) << std::scientific << std::abs(testVol - refVol) << "\n";
      //   //       ++errorFlag;
      //   //       *outStream << std::setw(70) << "^^^^----FAILURE!" << "\n";
      //   //     }
      //   //   }
      //   // }
        
      //   *outStream << "-> Hypercube testing\n\n";
      //   // later.... refVol = 32
      //   // for (int deg=0; deg<=20; deg++) {
      //   //   Teuchos::RCP<CubatureDirectLineGauss<DeviceSpaceType> > lineCub = Teuchos::rcp(new CubatureDirectLineGauss<DeviceSpaceType>(deg));
      //   //   CubatureTensor<DeviceSpaceType> hypercubeCub(lineCub, 5);
      //   //   int numCubPoints = hypercubeCub.getNumPoints();
      //   //   FieldContainer<DeviceSpaceType> cubPoints( numCubPoints, hypercubeCub.getDimension() );
      //   //   FieldContainer<DeviceSpaceType> cubWeights( numCubPoints );
      //   //   hypercubeCub.getCubature(cubPoints, cubWeights);
      //   //   testVol = 0;
      //   //   for (int i=0; i<numCubPoints; i++)
      //   //     testVol += cubWeights(i);
      //   //   *outStream << std::setw(30) << "5-D Hypercube volume --> " << std::setw(10) << std::scientific << testVol <<
      //   //     std::setw(10) << "diff = " << std::setw(10) << std::scientific << std::abs(testVol - volumeList[8]) << "\n";
      //   //   if (std::abs(testVol - volumeList[8])/std::abs(testVol) > tol) {
      //   //     errorFlag = 1;
      //   //     *outStream << std::setw(70) << "^^^^----FAILURE!" << "\n";
      //   //   }
      //   // }
      // }  catch (std::logic_error err) {
      //   *outStream << err.what() << "\n";
      //   errorFlag = -1;
      // };


      if (errorFlag != 0)
        std::cout << "End Result: TEST FAILED\n";
      else
        std::cout << "End Result: TEST PASSED\n";

      // reset format state of std::cout
      std::cout.copyfmt(oldFormatState);
      Kokkos::finalize();
      return errorFlag;
    }



  } // end of namespace test 
} // end of namespace intrepid2