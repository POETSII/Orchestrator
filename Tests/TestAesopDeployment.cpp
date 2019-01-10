/* Tests the deployment of Aesop for integrity. Does not test that the
   configuration in AesopDeployer.cpp is accurate. */

#define CATCH_CONFIG_MAIN

#include "catch.hpp"

#include "AesopDeployer.h"
#include "HardwareAddressFormat.h"
#include "HardwareModel.h"

TEST_CASE("Deployment to an empty engine and format does not capitulate", "[Aesop]")
{
    PoetsEngine engine("Engine000");
    HardwareAddressFormat addressFormat;
    AesopDeployer deployer;
    deployer.deploy(&engine, &addressFormat);
}
