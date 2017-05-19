/**
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */



#include <memory>
#include <string>
#include <core/RepositoryFactory.h>
#include "core/yaml/YamlConfiguration.h"
#include "../TestBase.h"

TEST_CASE("Test YAML Config Processing", "[YamlConfiguration]") {

  std::shared_ptr<core::Repository> testProvRepo = core::createRepository("provenancerepository", true);
  std::shared_ptr<core::Repository> testFlowFileRepo = core::createRepository("flowfilerepository", true);
  std::shared_ptr<minifi::Configure> configuration = std::make_shared<minifi::Configure>();
  //configuration->set(minifi::Configure::nifi_flow_configuration_file, test_file_location);
  std::shared_ptr<minifi::io::StreamFactory> streamFactory = std::make_shared<minifi::io::StreamFactory>(configuration);
  core::YamlConfiguration *yamlConfig = new core::YamlConfiguration(testProvRepo, testFlowFileRepo, streamFactory, configuration);

  SECTION("loading YAML without optional component IDs works") {
    std::string testFile = RESOURCE_PATH + "/TestYAMLConfigV1_ComponentIdsNotSpecified.yml";
    std::unique_ptr<core::ProcessGroup> rootFlowConfig = yamlConfig->getRoot(testFile);

    REQUIRE(rootFlowConfig);
    REQUIRE(rootFlowConfig->findProcessor("TailFile"));
    REQUIRE(NULL != rootFlowConfig->findProcessor("TailFile")->getUUID());
    REQUIRE(!rootFlowConfig->findProcessor("TailFile")->getUUIDStr().empty());
    REQUIRE(1 == rootFlowConfig->findProcessor("TailFile")->getMaxConcurrentTasks());
    REQUIRE(core::SchedulingStrategy::TIMER_DRIVEN == rootFlowConfig->findProcessor("TailFile")->getSchedulingStrategy());
    REQUIRE(1 == rootFlowConfig->findProcessor("TailFile")->getMaxConcurrentTasks());
    REQUIRE(1*1000*1000*1000 == rootFlowConfig->findProcessor("TailFile")->getSchedulingPeriodNano());
    REQUIRE(30*1000 == rootFlowConfig->findProcessor("TailFile")->getPenalizationPeriodMsec());
    REQUIRE(1*1000 == rootFlowConfig->findProcessor("TailFile")->getYieldPeriodMsec());
    REQUIRE(0 == rootFlowConfig->findProcessor("TailFile")->getRunDurationNano());

    std::map<std::string, std::shared_ptr<minifi::Connection>> connectionMap;
    rootFlowConfig->getConnections(connectionMap);
    REQUIRE(1 == connectionMap.size());
    // This is a map of UUID->Connection, and we don't know UUID, so just going to loop over it
    for(
        std::map<std::string,std::shared_ptr<minifi::Connection>>::iterator it = connectionMap.begin();
        it != connectionMap.end();
        ++it) {
      REQUIRE(it->second);
      REQUIRE(!it->second->getUUIDStr().empty());
      REQUIRE(it->second->getDestination());
      REQUIRE(it->second->getSource());
    }
  }
  SECTION("missing required field in YAML throws exception") {
    std::string testFile = RESOURCE_PATH + "/TestYAMLConfigV1_MissingRequiredField.yml";
    yamlConfig->getRoot(testFile);
    REQUIRE_THROWS_AS(yamlConfig->getRoot(testFile), std::invalid_argument);
  }
}

