#include <gtest/gtest.h>

#include "PathFollow.h"

float dot_quat(edF32VECTOR4* a, edF32VECTOR4* b);

TEST(PathFollowTimingTest, IdenticalUnitQuaternionsHaveZeroAngularDistance)
{
	edF32VECTOR4 rotation = { 1.0f, 0.0f, 0.0f, 0.0f };
	EXPECT_EQ(dot_quat(&rotation, &rotation), 1.0f);
}

TEST(PathFollowTimingTest, QuaternionDotProductDoesNotAssumeUnitLength)
{
	edF32VECTOR4 rotation = { 2.0f, 0.0f, 0.0f, 0.0f };
	EXPECT_FLOAT_EQ(dot_quat(&rotation, &rotation), 4.0f);
}

TEST(PathFollowTimingTest, RepeatedRotationKeepsIntermediateWaypointSeparateFromEndpoint)
{
	CPathFollow path;
	path.splinePointCount = 3;
	path.nbLeadInPoints = 0;
	edF32VECTOR4 rotations[3] = {
		{ 0.0f, 0.0f, 0.0f, 1.0f },
		{ 1.0f, 0.0f, 0.0f, 0.0f },
		{ 1.0f, 0.0f, 0.0f, 0.0f },
	};
	path.aSplineRotationsQuat = rotations;
	CPathFollowReaderAbsolute reader;
	reader.Create(0.3f, 0.0f, &path, 0, 0, 1, 0);

	EXPECT_FLOAT_EQ(reader.aSegmentDurations[0], 0.3f);
	EXPECT_FLOAT_EQ(reader.aSegmentDurations[1], 0.0011f);
	float destinationTime;
	float position;
	reader.GetClosestTimeToReachWaypoint(0.0f, 1, &destinationTime, &position);
	int pointA;
	int pointB;
	float fraction;
	EXPECT_NE(reader.ComputeSegment(destinationTime, &pointA, &pointB, &fraction), 2);
	EXPECT_EQ(pointB, 1);
	EXPECT_EQ(pointA, 2);
	EXPECT_NEAR(fraction, 0.0f, 0.0001f);

	// A second advance reaches the actual endpoint and can still fire the alarm.
	reader.GetClosestTimeToReachWaypoint(destinationTime, 2, &destinationTime, &position);
	EXPECT_EQ(reader.ComputeSegment(destinationTime, &pointA, &pointB, &fraction), 2);
}

TEST(PathFollowTimingTest, LevelThreeTargetRotationsStopBeforeAlarmEndpoint)
{
	// Raw Euler rotations from LEVEL_3/LEVEL.BNK path 237 (0x9cf1d0).
	edF32VECTOR4 rotations[3] = {
		{ -3.140893936f, 1.570796013f, 0.0f, 0.0f },
		{ -1.563392043f, -1.569403052f, -1.578904986f, 0.0f },
		{ -1.563392043f, -1.569403052f, -1.578904986f, 0.0f },
	};
	CPathFollow path;
	path.splinePointCount = 3;
	path.nbLeadInPoints = 0;
	path.aSplineRotationsEuler = rotations;
	CPathFollowReaderAbsolute reader;
	reader.Create(0.3f, 0.0f, &path, 0, 0, 1, 0);
	float destinationTime;
	float position;
	reader.GetClosestTimeToReachWaypoint(0.0f, 1, &destinationTime, &position);
	int pointA;
	int pointB;
	float fraction;
	EXPECT_NE(reader.ComputeSegment(destinationTime, &pointA, &pointB, &fraction), 2);
	EXPECT_EQ(pointB, 1);
	EXPECT_GT(reader.field_0x4 - destinationTime, 0.0001f);
}

TEST(PathFollowTimingTest, OrdinaryRotationPathRetainsOriginalTiming)
{
	edF32VECTOR4 rotations[3] = {
		{ 0.0f, 0.0f, 0.0f, 1.0f },
		{ 0.0f, 0.0f, 0.70710678f, 0.70710678f },
		{ 0.0f, 0.0f, 1.0f, 0.0f },
	};
	CPathFollow path;
	path.splinePointCount = 3;
	path.nbLeadInPoints = 0;
	path.aSplineRotationsQuat = rotations;
	CPathFollowReaderAbsolute reader;
	reader.Create(0.3f, 0.0f, &path, 0, 0, 1, 0);
	EXPECT_FLOAT_EQ(reader.field_0x4, 0.3f);
	EXPECT_NEAR(reader.aSegmentDurations[0], 0.15f, 0.000001f);
	EXPECT_NEAR(reader.aSegmentDurations[1], 0.1501f, 0.000001f);
}
