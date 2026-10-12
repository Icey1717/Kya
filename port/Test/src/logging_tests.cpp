#include <gtest/gtest.h>
#include "log.h"
#include "logging.h"

TEST(Logging, NamedHandleSharesEntryAndHonorsRuntimeToggleAfterRehash)
{
	auto& log = Log::GetInstance();
	auto& entry = LogCategories::Event();
	EXPECT_EQ(&entry, &log.GetCategory("event"));
	const bool previousEnabled = entry.bEnabled;
	int evaluations = 0;
	auto emit = [&]() { MY_LOG_NAMED_CATEGORY(Event, LogLevel::Info, "{}", ++evaluations); };

	entry.bEnabled = false;
	emit();
	EXPECT_EQ(evaluations, 0);
	log.logs.rehash(log.logs.bucket_count() * 2);
	EXPECT_EQ(&LogCategories::Event(), &entry);
	entry.bEnabled = true;
	emit();
	EXPECT_EQ(evaluations, 1);
	entry.bEnabled = false;
	emit();
	EXPECT_EQ(evaluations, 1);
	entry.bEnabled = previousEnabled;
}

TEST(Logging, DynamicCategoryUsesCurrentNameAtSameCallSite)
{
	auto& log = Log::GetInstance();
	auto& first = log.GetCategory("LoggingDynamicFirstTest");
	auto& second = log.GetCategory("LoggingDynamicSecondTest");
	const bool previousFirst = first.bEnabled;
	const bool previousSecond = second.bEnabled;
	first.bEnabled = false;
	second.bEnabled = true;
	int evaluations = 0;
	auto emit = [&](const auto& category) { MY_LOG_CATEGORY(category, LogLevel::Info, "{}", ++evaluations); };
	std::string category = "LoggingDynamicFirstTest";
	emit(category);
	EXPECT_EQ(evaluations, 0);
	category = "LoggingDynamicSecondTest";
	emit(category);
	EXPECT_EQ(evaluations, 1);
	char mutableCategory[] = "LoggingDynamicFirstTest";
	emit(mutableCategory);
	EXPECT_EQ(evaluations, 1);
	mutableCategory[0] = 'X';
	emit(mutableCategory);
	EXPECT_EQ(evaluations, 2);
	first.bEnabled = previousFirst;
	second.bEnabled = previousSecond;
}

TEST(Logging, DisabledCategorySkipsArgumentsAndHonorsRuntimeToggle)
{
	auto& entry = Log::GetInstance().GetCategory("LoggingLazyArgumentsTest");
	const bool previousEnabled = entry.bEnabled;
	int categoryEvaluations = 0;
	int formatEvaluations = 0;
	int argumentEvaluations = 0;
	auto category = [&]() { ++categoryEvaluations; return "LoggingLazyArgumentsTest"; };
	auto format = [&]() { ++formatEvaluations; return "{}"; };
	auto argument = [&]() { ++argumentEvaluations; return "evaluated"; };
	auto emit = [&]() { MY_LOG_CATEGORY(category(), LogLevel::Info, format(), argument()); };

	entry.bEnabled = false;
	emit();
	EXPECT_EQ(categoryEvaluations, 1);
	EXPECT_EQ(formatEvaluations, 0);
	EXPECT_EQ(argumentEvaluations, 0);

	entry.bEnabled = true;
	emit();
	EXPECT_EQ(categoryEvaluations, 2);
	EXPECT_EQ(formatEvaluations, 1);
	EXPECT_EQ(argumentEvaluations, 1);

	entry.bEnabled = false;
	emit();
	EXPECT_EQ(categoryEvaluations, 3);
	EXPECT_EQ(formatEvaluations, 1);
	EXPECT_EQ(argumentEvaluations, 1);
	entry.bEnabled = previousEnabled;
}

TEST(Logging, MacroWithoutArgumentsIsSafeInIfElse)
{
	auto& entry = Log::GetInstance().GetCategory("LoggingIfElseTest");
	const bool previousEnabled = entry.bEnabled;
	entry.bEnabled = false;
	bool tookElse = false;
	if (true)
		MY_LOG_CATEGORY("LoggingIfElseTest", LogLevel::Info, "disabled");
	else
		tookElse = true;
	EXPECT_FALSE(tookElse);
	entry.bEnabled = previousEnabled;
}
