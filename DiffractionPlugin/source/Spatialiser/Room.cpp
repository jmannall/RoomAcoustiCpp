
#include "Spatialiser/Room.h"

using namespace Spatialiser;

void Room::UpdateListenerPosition(const vec3& position)
{
	mListenerPosition = position;
	lock_guard <mutex> wLock(mWallMutex);
	for (auto it = mWalls.begin(); it != mWalls.end(); it++)
	{
		it->second.SetRValid(it->second.ReflectPointInWall(mListenerPosition));
	}
	for (auto it = mEdges.begin(); it != mEdges.end(); it++)
	{
		it->second.SetRValid(!LineRoomIntersection(it->second.GetMidPoint(), mListenerPosition));
	}
}

size_t Room::AddWall(const Wall& wall)
{
	lock_guard <mutex> wLock(mWallMutex);
	if (!mEmptyWallSlots.empty()) // Assign source to an existing ID
	{
		size_t next = mEmptyWallSlots.back();
		mEmptyWallSlots.pop_back();
		mWalls.insert_or_assign(next, wall);
		InitEdges(next);
		return next;
	}
	else // Create a new ID
	{
		size_t next = mWalls.size();
		mWalls.insert_or_assign(next, wall);
		InitEdges(next);
		return next;
	}
}

#pragma region Edges

void Room::InitEdges(const size_t& id)
{
	auto itA = mWalls.find(id);
	if (itA == mWalls.end())
	{
		// Wall doesn't exist
	}
	else
	{
		for (auto itB = mWalls.begin(); itB != mWalls.end(); itB++)
		{
			if (itA->first != itB->first)
			{
				vec3 normalA = itA->second.GetNormal();
				vec3 normalB = itB->second.GetNormal();
				if (normalA != normalB)
				{
					if (normalA == -normalB)
					{
						ParallelFindEdges(itA->second, itB->second, itA->first, itB->first);
					}
					else
					{
						FindEdges(itA->second, itB->second, itA->first, itB->first);
					}
				}
			}
		}
	}
}

void Room::ParallelFindEdges(Wall& a, Wall& b, const size_t IDa, const size_t IDb)
{
	std::vector<vec3> verticesA = a.GetVertices();

	// walls must be coplanar
	if (b.PointWallPosition(verticesA[0]) == 0)
	{
		std::vector<vec3> verticesB = b.GetVertices();

		size_t numA = verticesA.size();
		size_t numB = verticesB.size();

		bool evenNum = numA % 2 == 0;
		for (int i = 0; i < numA; i += 2)
		{
			bool match = false;
			int j = 0;
			while (!match && j < numB)
			{
				match = verticesA[i] == verticesB[j];
				j++;
			}
			if (match)
			{
				j--;
				int idxA[2] = { i, (i + 1) % numA };
				bool validEdge = verticesA[idxA[1]] == verticesB[(j - 1) % numB];

				if (validEdge)
				{
					Edge edge = Edge(verticesA[idxA[0]], verticesA[idxA[1]], a.GetNormal(), b.GetNormal(), IDa, IDb);
					size_t id = AddEdge(edge);
					a.AddEdge(id);
					b.AddEdge(id);
				}

				if (i > 0 || evenNum)
				{
					idxA[1] = (i - 1) % numA;
					validEdge = verticesA[idxA[1]] == verticesB[(j + 1) % numB];

					if (validEdge)
					{
						Edge edge = Edge(verticesA[idxA[0]], verticesA[idxA[1]], a.GetNormal(), b.GetNormal(), IDa, IDb);
						size_t id = AddEdge(edge);
						a.AddEdge(id);
						b.AddEdge(id);
					}
				}
			}
		}
	}
}

void Room::FindEdges(Wall& a, Wall& b, const size_t IDa, const size_t IDb)
{
	std::vector<vec3> verticesA = a.GetVertices();
	std::vector<vec3> verticesB = b.GetVertices();

	size_t numA = verticesA.size();
	size_t numB = verticesB.size();

	for (int i = 0; i < numA; i += 2)
	{
		bool match = false;
		int j = 0;
		while (!match && j < numB)
		{
			match = verticesA[i] == verticesB[j];
			j++;
		}
		if (match)
		{
			j--;
			int idxA = (i + 1) % numA;
			bool validEdge = verticesA[idxA] == verticesB[(j - 1) % numB]; // Must be this way to ensure normals not twisted. (right hand rule) therefore one rotated up the edge one rotates down

			if (!validEdge)
			{
				idxA = (i - 1) % numA;
				validEdge = verticesA[idxA] == verticesB[(j + 1) % numB];
			}
			if (validEdge)
			{
				int check = 0;
				while (check == i || check == idxA)
				{
					check++;
				}
				float k = b.PointWallPosition(verticesA[check]);

				// K won't equal zero as then planes would be parallel
				bool reflexAngle = UnitVector(Cross(a.GetNormal(), b.GetNormal())) == UnitVector(verticesA[idxA] - verticesA[i]);
				if (k > 0 && !reflexAngle || k < 0 && reflexAngle) // Check returns correct angle type
				{
					Edge edge = Edge(verticesA[i], verticesA[idxA], a.GetNormal(), b.GetNormal(), IDa, IDb);
					size_t id = AddEdge(edge);
					a.AddEdge(id);
					b.AddEdge(id);
				}
				else
				{
					Edge edge = Edge(verticesA[i], verticesA[idxA], b.GetNormal(), a.GetNormal(), IDa, IDb);
					size_t id = AddEdge(edge);
					a.AddEdge(id);
					b.AddEdge(id);
				}
			}
		}
	}
}

size_t Room::AddEdge(const Edge& edge)
{
	if (!mEmptyEdgeSlots.empty()) // Assign source to an existing ID
	{
		size_t next = mEmptyEdgeSlots.back();
		mEmptyEdgeSlots.pop_back();
		mEdges.insert_or_assign(next, edge);
		return next;
	}
	else // Create a new ID
	{
		size_t next = mEdges.size();
		mEdges.insert_or_assign(next, edge);
		return next;
	}
}

#pragma endregion

FrequencyDependence Room::GetReverbTime(const float& volume)
{
	FrequencyDependence absorption = FrequencyDependence(0.0f, 0.0f, 0.0f, 0.0f, 0.0f);

	lock_guard <mutex> wLock(mWallMutex);
	for (auto wall : mWalls)
	{
		absorption += (1.0f - wall.second.GetAbsorption()) * wall.second.GetArea();
	}
	float temp[5];
	absorption.GetValues(&temp[0]);
	for (int i = 0; i < 5; i++)
	{
		Debug::Log("Absorption: " + FloatToStr(temp[i]) + " I: " + IntToStr(i), Color::White);
	}

	return 55.3f * volume / (SPEED_OF_SOUND * absorption);
}

#pragma region Geometry Checks

bool Room::LineRoomIntersection(const vec3& start, const vec3& end)
{
	return LineRoomIntersection(start, end, -1);
}

bool Room::LineRoomIntersection(const vec3& start, const vec3& end, size_t currentWallID)
{
	bool obstruction = false;
	LineRoomIntersection(start, end, currentWallID, obstruction);
	return obstruction;
}

void Room::LineRoomIntersection(const vec3& start, const vec3& end, size_t currentWallID, bool& obstruction)
{
	auto it = mWalls.begin();
	while (!obstruction && it != mWalls.end())
	{
		size_t id = it->first;
		Wall wall = it->second;
		if (id != currentWallID)
		{
			float kS = wall.PointWallPosition(start);
			float kE = wall.PointWallPosition(end);
			if (kS * kE < 0)
			{
				obstruction = wall.LineWallIntersection(start, end);
			}
		}
		it++;
	}
}

bool Room::LineRoomIntersection(const vec3& start, const vec3& end, size_t currentWallID1, size_t currentWallID2)
{
	bool obstruction = false;
	auto it = mWalls.begin();
	while (!obstruction && it != mWalls.end())
	{
		size_t id = it->first;
		Wall wall = it->second;
		if (id != currentWallID1 && id != currentWallID2)
		{
			float kS = wall.PointWallPosition(start);
			float kE = wall.PointWallPosition(end);
			if (kS * kE < 0)
			{
				obstruction = wall.LineWallIntersection(start, end);
			}
		}
		it++;
	}
	return obstruction;
}

#pragma endregion

#pragma region Image Source Method

void Room::UpdateISM()
{
	size_t oldEndPtr = mSources.size();
	auto endPtr = oldEndPtr;
	for (auto it = mSources.begin(); it != mSources.end(); it++)
	{
		{
			lock_guard <mutex> lLock(mLowPrioMutex);
			unique_lock <mutex> nLock(mNextMutex);
			lock_guard <mutex> rLock(mRemoveMutex);
			nLock.unlock();
			endPtr = mSources.size();

			if (endPtr != oldEndPtr)
			{
				return;
			}
			oldEndPtr = endPtr;

			std::vector<VirtualSourceData> vSources;
			vec3 position(it->second.position);

			bool visible = ReflectPointInRoom(position, vSources);
			Debug::Log("Source visible: " + BoolToStr(visible), Color::Blue);
			{
				lock_guard <mutex> iLock(it->second.mMutex);
				it->second.visible = visible;
				it->second.vSources = vSources;
			}

			// For debugging
			int count = 0;
			for (int i = 0; i < vSources.size(); i++)
			{
				if (vSources[i].visible)
					count++;
			}
			Debug::Log("Source " + IntToStr((int)it->first) + " has " + IntToStr(count) + " visible virtual sources", Color::Blue);
		}
	}
}

bool Room::ReflectPointInRoom(const vec3& point, std::vector<VirtualSourceData>& vSources)
{
	// Direct sound
	bool lineOfSight = !LineRoomIntersection(point, mListenerPosition);

	FirstOrderDiffraction(point, vSources);

	if (mMaxRefOrder < 1)
		return lineOfSight;

	// Reflections
	FirstOrderReflections(point, vSources);

	if (mMaxRefOrder < 2)
		return lineOfSight;

	HigherOrderReflections(point, vSources);

	return lineOfSight;
}

void Room::FirstOrderDiffraction(const vec3& point, std::vector<VirtualSourceData>& vSources)
{
	bool feedsFDN = mMaxRefOrder == 1;
	bool valid;
	for (auto it : mEdges)
	{
		size_t id = it.first;
		Edge edge = it.second;

		VirtualSourceData vSource;
		vec3 position;
		vSource.AddEdgeID(id);

		bool sourceVisible = !LineRoomIntersection(edge.GetMidPoint(), point);

		if (sourceVisible) // source can see edge
		{
			// Valid diffraction path
			vSource.Valid();

			if (edge.GetRValid()) // receiver can see edge
			{
				// Visible diffraction path
				vSource.Visible(feedsFDN);
			}
		}
		vSource.SetTransform(position);
		vSources.push_back(vSource);
	}
}

void Room::FirstOrderReflections(const vec3& point, std::vector<VirtualSourceData>& vSources)
{
	bool feedsFDN = mMaxRefOrder == 1;
	bool valid;
	for (auto it : mWalls)
	{
		size_t id = it.first;
		Wall wall = it.second;

		VirtualSourceData vSource;
		vec3 position;
		vSource.AddWallID(id, wall.GetAbsorption());
		valid = wall.ReflectPointInWall(position, point);
		if (valid)
		{
			vSource.Valid();

			if (wall.GetRValid())
			{
				vec3 intersection;
				valid = wall.LineWallIntersection(intersection, mListenerPosition, position);

				if (valid)
				{
					bool obstruction = LineRoomIntersection(intersection, mListenerPosition, id);
					LineRoomIntersection(point, intersection, id, obstruction);
					if (!obstruction)
					{
						vSource.Visible(feedsFDN);
					}
				}
			}
		}
		vSource.SetTransform(position);
		vSources.push_back(vSource);
	}
}

void Room::HigherOrderReflections(const vec3& point, std::vector<VirtualSourceData>& vSources)
{
	int sourceStart = 0;
	int numSources = vSources.size();
	bool valid;
	for (int j = 1; j < mMaxRefOrder; j++)
	{
		int refOrder = j + 1;
		bool feedsFDN = mMaxRefOrder == refOrder;
		std::vector<vec3> intersections;
		size_t capacity = intersections.capacity();
		intersections.reserve(refOrder);
		std::fill_n(std::back_inserter(intersections), refOrder - capacity, vec3());

		for (auto it : mWalls)
		{
			size_t id = it.first;
			Wall wall = it.second;

			for (int k = sourceStart; k < numSources; k++)
			{
				if (vSources[k].valid && id != vSources[k].GetWallID(j - 1)) // Could add precomputed visibility between walls
				{
					VirtualSourceData vSource = VirtualSourceData(vSources[k]);
					vSource.Reset();
					vec3 position;
					vSource.AddWallID(id, wall.GetAbsorption());
					valid = wall.ReflectPointInWall(position, vSource.GetPosition(j - 1));
					if (valid)
					{
						vSource.Valid();

						if (wall.GetRValid())
						{
							// Check valid intersections
							valid = wall.LineWallIntersection(intersections[j], mListenerPosition, position);

							int n = 0;
							while (valid && n < j)
							{
								int idx = j - (n + 1); // Current bounce (j - n is previous bounce in code and next bounce in path)
								Wall previousWall = mWalls.find(vSource.GetWallID(idx))->second;
								valid = previousWall.LineWallIntersection(intersections[idx], intersections[j - n], vSource.GetPosition(idx));
								n++;
							}
							if (valid)
							{
								// Check for obstruction
								bool obstruction = LineRoomIntersection(intersections[j], mListenerPosition, id);
								LineRoomIntersection(intersections[0], point, vSource.GetWallID(0), obstruction);

								int p = 0;
								while (!obstruction && p < j)
								{
									obstruction = LineRoomIntersection(intersections[p], intersections[p + 1], vSource.GetWallID(p), vSource.GetWallID(p + 1));
									p++;
								}
								if (!obstruction)
								{
									vSource.Visible(feedsFDN);
								}
							}
						}
					}
					vSource.SetTransform(position);
					vSources.push_back(vSource);
				}
			}
		}
		sourceStart = numSources;
		numSources = vSources.size();
	}
}

#pragma endregion