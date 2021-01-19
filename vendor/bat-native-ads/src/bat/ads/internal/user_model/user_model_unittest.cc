/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <string>

#include "bat/ads/internal/user_model/user_model.h"

#include "bat/ads/internal/unittest_base.h"
#include "bat/ads/internal/unittest_util.h"

#include "base/files/file_path.h"

#include <cstdlib>
#include <fstream>
#include <sstream>

// npm run test -- brave_unit_tests --filter=BatAds*

namespace ads {
namespace ml_tools {  

namespace {

const char kHashingModel[] = "hashing_model_min.json";

const char kInvalidModel[] = "invalid_model.json";

const char kTextCMCCrash[] = "text_cmc_crash.txt";

}  // namespace

class BatAdsUserModelTest : public UnitTestBase {
 protected:
  BatAdsUserModelTest() = default;

  ~BatAdsUserModelTest() override = default;
};

TEST_F(BatAdsUserModelTest, InitValidModelTest) {
  user_model::UserModel user_model;

  const base::Optional<std::string> opt_value =
      ReadFileFromTestPathToString(kHashingModel);
  ASSERT_TRUE(opt_value.has_value());

  const std::string model_json = opt_value.value();
  EXPECT_TRUE(user_model.InitializePageClassifier(model_json));
}

TEST_F(BatAdsUserModelTest, InvalidModelTest) {
  user_model::UserModel user_model;

  const base::Optional<std::string> opt_value =
      ReadFileFromTestPathToString(kInvalidModel);
  ASSERT_TRUE(opt_value.has_value());

  const std::string model_json = opt_value.value();
  EXPECT_FALSE(user_model.InitializePageClassifier(model_json));
}

TEST_F(BatAdsUserModelTest, EmptyModelTest) {
  user_model::UserModel user_model;
  const std::string empty_model_json = "{}";
  EXPECT_FALSE(user_model.InitializePageClassifier(empty_model_json));
}

TEST_F(BatAdsUserModelTest, MissingModelTest) {
  user_model::UserModel user_model;
  const std::string missing_model_json = "";
  EXPECT_FALSE(user_model.InitializePageClassifier(missing_model_json));
}

TEST_F(BatAdsUserModelTest, TopPredUnitTest) {
  user_model::UserModel user_model;

  const base::Optional<std::string> opt_value =
      ReadFileFromTestPathToString(kHashingModel);
  ASSERT_TRUE(opt_value.has_value());

  const std::string model_json = opt_value.value();
  EXPECT_TRUE(user_model.InitializePageClassifier(model_json));

  std::string test_page = "ethereum bitcoin bat zcash crypto tokens!";
  auto preds = user_model.ClassifyPage(test_page);
  EXPECT_TRUE(preds.size());
  EXPECT_TRUE(preds.size() < 100);
  EXPECT_TRUE(preds.count("crypto-crypto"));
  for (auto const& pred: preds) {
    EXPECT_TRUE(pred.second <= preds["crypto-crypto"]);
  }
}

TEST_F(BatAdsUserModelTest, TextCMCCrashTest) {
  user_model::UserModel user_model;

  const base::Optional<std::string> opt_value =
      ReadFileFromTestPathToString(kHashingModel);
  ASSERT_TRUE(opt_value.has_value());

  const std::string model_json = opt_value.value();
  EXPECT_TRUE(user_model.InitializePageClassifier(model_json));

  const base::Optional<std::string> opt_text_value =
      ReadFileFromTestPathToString(kTextCMCCrash);
  ASSERT_TRUE(opt_text_value.has_value());
  const std::string bad_text = opt_text_value.value();

  auto preds = user_model.ClassifyPage(bad_text);
  EXPECT_TRUE(preds.size() < 100);
  EXPECT_TRUE(preds.size() > 2);
  EXPECT_TRUE(preds.count("personal finance-personal finance"));
  for (auto const& pred: preds) {
    EXPECT_TRUE(pred.second <= preds["personal finance-personal finance"]);
  }
}

}  // namespace ml_tools
}  // namespace ads
