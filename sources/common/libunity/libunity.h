/**
 * @file libunity.h
 * @author Shang Huang
 * @brief Customized Unity Framework for μEDP Testing with μEDP coding style and conventions
 * @version 0.1
 * @date 2026-11-08
 * @copyright MIT License
 */
#ifndef __LUNITY_H__
	#define __LUNITY_H__

  // ANCHOR - Khai báo các thư viện sử dụng
  #include "unity_config.h"
  #include "unity.h"

  //ANCHOR - Test unit setup / teardown
  #define TEST_SETUP() setUp()
  #define TEST_TEARDOWN() tearDown()

  //ANCHOR - Test suite setup / teardown
  #define TESTSUITE_SETUP() suiteSetUp()
  #define TESTSUITE_TEARDOWN(num_failures) suiteTearDown(num_failures)

  //ANCHOR - Test Reset and Verify
  #define TEST_RESET() resetTest()
  #define TEST_VERIFY() verifyTest()

#endif