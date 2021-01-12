/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <fstream>
#include <codecvt>
#include <cmath>
#include <utility>
#include <stddef.h>

#include "bat/ads/internal/ml_tools/transformation/hashing_extractor.h"

#include "base/json/json_reader.h"
#include "base/strings/utf_string_conversions.h"
#include "base/values.h"

#include "bat/ads/internal/unittest_base.h"
#include "bat/ads/internal/unittest_util.h"

// npm run test -- brave_unit_tests --filter=BatAds*

namespace ads {
namespace ml_tools {

namespace {

const char kHashCheck[] = "hash_check.json";

}  // namespace

class BatAdsHashingExtractorTest : public UnitTestBase {
 protected:
  BatAdsHashingExtractorTest() = default;

  ~BatAdsHashingExtractorTest() override = default;
};

void run_hashing_extractor_test_case(std::string test_case_name) {
  const double kEps = 1e-7;

  const base::Optional<std::string> opt_value =
      ReadFileFromTestPathToString(kHashCheck);
  ASSERT_TRUE(opt_value.has_value());

  const std::string hash_check_json = opt_value.value();

  base::Optional<base::Value> root = base::JSONReader::Read(hash_check_json);
  ASSERT_TRUE(root);

  base::Value* case_params = root->FindKeyOfType(
      test_case_name,
      base::Value::Type::DICTIONARY);
  ASSERT_TRUE(case_params);

  base::Value* input = case_params->FindKey("input");
  ASSERT_TRUE(input);

  std::string input_value;
  EXPECT_TRUE(input->GetAsString(&input_value));

  std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;

  base::Value* idx = case_params->FindKey("idx");
  ASSERT_TRUE(idx->is_list());

  base::Value* count = case_params->FindKey("count");
  ASSERT_TRUE(count->is_list());

  transformation::HashVectorizer vectorizer;
  auto frequencies = vectorizer.GetFrequencies(input_value);
  EXPECT_EQ(idx->GetList().size(), frequencies.size());
  for (size_t i = 0; i < frequencies.size(); ++i) {
      const base::Value& idx_val = idx->GetList()[i];
      const base::Value& count_val = count->GetList()[i];
      EXPECT_TRUE(count_val.GetInt() - frequencies[idx_val.GetInt()] < kEps);
  }
}

TEST_F(BatAdsHashingExtractorTest, ValidJsonScheme) {
  base::Optional<base::Value> root = base::JSONReader::Read(
      "{"
      "  \"test\": {"
      "    \"foo\": true,"
      "    \"bar\": 3.14,"
      "    \"baz\": \"bat\","
      "    \"moo\": \"cow\""
      "  },"
      "  \"list\": ["
      "    \"a\","
      "    \"b\""
      "  ]"
      "}");

  ASSERT_TRUE(root);
  ASSERT_TRUE(root->is_dict());

  base::Value* dict = root->FindKeyOfType("test", base::Value::Type::DICTIONARY);
  ASSERT_TRUE(dict);

  base::Value* list = root->FindKeyOfType("list", base::Value::Type::LIST);
  ASSERT_TRUE(list);
}

TEST_F(BatAdsHashingExtractorTest, EmptyText) {
    run_hashing_extractor_test_case("empty");
}

TEST_F(BatAdsHashingExtractorTest, ShortText) {
    run_hashing_extractor_test_case("tiny");
}

TEST_F(BatAdsHashingExtractorTest, EnglishText) {
    run_hashing_extractor_test_case("english");
}

TEST_F(BatAdsHashingExtractorTest, GreekText) {
    run_hashing_extractor_test_case("greek");
}

TEST_F(BatAdsHashingExtractorTest, JapaneseText) {
    run_hashing_extractor_test_case("japanese");
}

}  // namespace ml_tools
}  // namespace ads
