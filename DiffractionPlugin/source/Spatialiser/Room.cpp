
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
	//for (auto it = mEdges.begin(); it != mEdges.end(); it++)
	//{
	//	it->second.SetRValid(mListenerPosition);
	//}
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

				if (k < 0) // Check angle greater than 180
				{
					// K won't equal zero as then planes would be parallel
					bool reflexAngle = UnitVector(Cross(a.GetNormal(), b.GetNormal())) == UnitVector(verticesA[idxA] - verticesA[i]);
					if (reflexAngle) // Check returns correct angle type
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
	float surfaceArea = 0.0f;

	lock_guard <mutex> wLock(mWallMutex);
	for (auto wall : mWalls)
	{
		absorption += (1.0f - wall.second.GetAbsorption() * wall.second.GetAbsorption()) * wall.second.GetArea();
		surfaceArea += wall.second.GetArea();
	}
	float temp[5];
	absorption.GetValues(&temp[0]);
	for (int i = 0; i < 5; i++)
	{
		Debug::Log("Absorption: " + FloatToStr(temp[i]) + " I: " + IntToStr(i), Color::White);
	}

	float factor = 24.0f * log(10) / SPEED_OF_SOUND;
	// Sabine
	// FrequencyDependence sabine = factor * volume / absorption;

	// eyring
	FrequencyDependence eyring = factor * volume / (-surfaceArea * (1 - absorption / surfaceArea).Log());
	return eyring;
}

#pragma region Geometry Checks

bool Room::LineRoomIntersection(const vec3& start, const vec3& end)
{
	return LineRoomIntersection(start, end, -1);
}

void Room::LineRoomIntersection(const vec3& start, const vec3& end, bool& obstruction)
{
	LineRoomIntersection(start, end, -1, obstruction);
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
			if (kS * kE < 0)	// point lies on plane when kS || kE == 0. Therefore not obstructed
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
		// if (true)
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

	if (mMaxOrder < 1)
		return lineOfSight;

	VirtualSourceMap sp;
	VirtualSourceMap edSp;
	VirtualSourceMap spEd;
	VirtualSourceMap ed;

	FirstOrderDiffraction(point, ed, vSources);

	// Reflections
	FirstOrderReflections(point, sp, vSources);

	if (mMaxOrder < 2)
		return lineOfSight;

	HigherOrderReflections(point, sp, vSources);

	// SpecularDiffraction(point, sp, edSp, spEd, vSources);

	HigherOrderSpecularDiffraction(point, sp, edSp, spEd, vSources);

	return lineOfSight;
}

#pragma endregion

#pragma region Diffraction

void Room::HigherOrderSpecularDiffraction(const vec3& point, VirtualSourceMap& sp, VirtualSourceMap& edSp, VirtualSourceMap& spEd, VirtualSourceVec& vSources)
{
	for (int j = 1; j < mMaxOrder; j++) // only handle up to 1st order diffraction
	{
		int order = j + 1;
		int refIdx = j - 1;
		bool feedsFDN = mMaxOrder == order;
		std::vector<vec3> intersections;
		size_t capacity = intersections.capacity();
		intersections.reserve(j);
		std::fill_n(std::back_inserter(intersections), order - capacity, vec3());
		auto bIdx = sp.bucket(j);
		auto numReflectionPaths = sp.bucket_size(bIdx);

		auto vs = sp.begin(bIdx);
		for (int i = 0; i < numReflectionPaths; i++, vs++)
		{
			VirtualSourceData vSource = vs->second;
			VirtualSourceData vSourceB = vs->second;

			auto itW = mWalls.find(vSource.GetID());

			size_t idW = itW->first;
			Wall wall = itW->second;

			for (auto itE : mEdges)
			{
				size_t idE = itE.first;
				Edge edge = itE.second;

				if (!edge.AttachedToPlane(idW))
				{
					bool valid = wall.ReflectEdgeInWall(edge); // Check edge in front of wall

					if (valid)
					{
						// sped
						if (vSource.valid) // valid source route
						{
							vSource.Reset();
							// Check for sp - ed
							vec3 position = vSource.GetPosition();
							Diffraction::Path path = Diffraction::Path(position, mListenerPosition, edge);
							vSource.AddEdgeID(idE, path);

							if (path.valid)
							{
								vSource.Valid();

								if (mISMConfig.diffraction == DiffractionDepth::edSp && (path.inShadow || !mISMConfig.shadowOnly))
								{
									vSource.SetTransform(position, path.CalculateVirtualPostion());

									intersections[j] = path.GetApex();

									// Check valid intersections
									int n = 0;
									while (valid && n < j)
									{
										int idx = j - (n + 1); // Current bounce (j - n is previous bounce in code and next bounce in path)
										Wall previousWall = mWalls.find(vSource.GetID(idx))->second;
										valid = previousWall.LineWallIntersection(intersections[idx], intersections[j - n], vSource.GetPosition(idx));
										n++;
									}
									if (valid)
									{
										// Check for obstruction
										bool obstruction = LineRoomIntersection(mListenerPosition, intersections[j]);
										LineRoomIntersection(intersections[0], point, vSource.GetID(0), obstruction);

										int p = 0;
										while (!obstruction && p < j)
										{
											if (vSource.IsReflection(p))
											{
												if (vSource.IsReflection(p + 1))
													obstruction = LineRoomIntersection(intersections[p], intersections[p + 1], vSourceB.GetID(p), vSourceB.GetID(p + 1));
												else
													obstruction = LineRoomIntersection(intersections[p], intersections[p + 1], vSourceB.GetID(p));
											}
											else
												obstruction = LineRoomIntersection(intersections[p], intersections[p + 1], vSourceB.GetID(p + 1));
											p++;
										}
										if (!obstruction)
										{
											vSource.Visible(feedsFDN);
										}
									}
								}
							}
							if (path.rValid) // If !rValid, can't be any later paths and 1st order diffraction would already be invalid
								vSources.push_back(vSource);

							spEd.emplace(j, vSource);
						}

						// edsp
						if (vSourceB.rValid) // valid receiver route
						{
							vSourceB.Reset();
							// Check for ed - sp
							vec3 position = vSourceB.GetRPosition();
							Diffraction::Path path = Diffraction::Path(point, position, edge);
							vSourceB.AddEdgeIDToStart(idE, path);

							if (path.valid) // Would be more effcient to save sValid and rValid for each edge
							{
								vSourceB.RValid();
								if (mISMConfig.diffraction == DiffractionDepth::edSp && (path.inShadow || !mISMConfig.shadowOnly))
								{
									vec3 vPosition = path.CalculateVirtualPostion();
									for (int k = 1; k < order; k++)
									{
										auto temp = mWalls.find(vSourceB.GetID(k));
										if (temp != mWalls.end())
										{
											temp->second.ReflectPointInWall(vPosition, vPosition); // Won't reflect as vPosition behind wall...
										}
										else
										{
											// Wall does not exist
										}
									}
									vSourceB.SetRTransform(position, vPosition);

									intersections[j] = path.GetApex();

									int n = 0;
									valid = true; // In case changed in previous if statement
									while (valid && n < j)
									{
										int idx = j - (n + 1); // Current bounce (j - n is previous bounce in code and next bounce in path)
										Wall previousWall = mWalls.find(vSourceB.GetID(idx))->second;
										valid = previousWall.LineWallIntersection(intersections[idx], intersections[j - n], vSourceB.GetRPosition(idx));
										n++;
									}
									if (valid)
									{
										// Check for obstruction
										bool obstruction = LineRoomIntersection(point, intersections[j]);
										LineRoomIntersection(intersections[0], mListenerPosition, idW, obstruction);

										int p = 0;
										while (!obstruction && p < j)
										{
											if (vSource.IsReflection(p))
											{
												if (vSource.IsReflection(p + 1))
													obstruction = LineRoomIntersection(intersections[p], intersections[p + 1], vSourceB.GetID(p), vSourceB.GetID(p + 1));
												else
													obstruction = LineRoomIntersection(intersections[p], intersections[p + 1], vSourceB.GetID(p));
											}
											else
												obstruction = LineRoomIntersection(intersections[p], intersections[p + 1], vSourceB.GetID(p + 1));
											p++;
										}
										if (!obstruction)
										{
											vSourceB.Visible(feedsFDN);
										}
									}
								}
							}
							if (path.sValid) // If !sValid, can't be any later paths and 1st order diffraction would already be invalid
								vSources.push_back(vSourceB);

							edSp.emplace(j, vSourceB);
						}
					}
				}
			}
		}
		// spedsp paths
		for (int i = 1; i < j; i++)
		{
			auto idxSpEd = spEd.bucket(i);
			auto vsSpEd = spEd.begin(idxSpEd);
			auto numSpEd = spEd.bucket_size(idxSpEd);

			for (int x = 0; x < numSpEd; x++, vsSpEd++)
			{
				VirtualSourceData start = vsSpEd->second;

				if (start.mDiffractionPath.sValid)
				{
					auto idxEdSp = edSp.bucket(j - i);
					auto vsEdSp = edSp.begin(idxEdSp);
					auto numEdSp = edSp.bucket_size(idxEdSp);

					for (int y = 0; y < numEdSp; y++, vsEdSp++)
					{
						VirtualSourceData end = vsEdSp->second;

						if (end.mDiffractionPath.rValid)
						{
							if (start.GetID() == end.GetID(0))
							{
								VirtualSourceData vSource = start;
								bool recheckVisibility = vSource.AppendVSource(end, mListenerPosition);

								if (mISMConfig.diffraction == DiffractionDepth::edSp && (vSource.mDiffractionPath.inShadow || !mISMConfig.shadowOnly))
								{
									if (recheckVisibility)
									{
										Diffraction::Path path = vSource.mDiffractionPath;
										intersections[i] = path.GetApex();

										int n = 0;
										bool valid = true;
										while (valid && n < i)
										{
											int idx = i - (n + 1); // Current bounce (i - n is previous bounce in code and next bounce in path)
											Wall previousWall = mWalls.find(vSource.GetID(idx))->second;
											valid = previousWall.LineWallIntersection(intersections[idx], intersections[i - n], vSource.GetPosition(idx));
											n++;
										}

										n = 0;
										while (valid && n < j - i)
										{
											int idx = i + n + 1; // Current bounce (j - n is previous bounce in code and next bounce in path)
											Wall previousWall = mWalls.find(vSource.GetID(idx))->second;
											valid = previousWall.LineWallIntersection(intersections[idx], intersections[i + n], vSource.GetRPosition(j - idx));
											n++;
										}

										if (valid)
										{
											// Check for obstruction
											bool obstruction = LineRoomIntersection(point, intersections[0], vSource.GetID(0));
											LineRoomIntersection(mListenerPosition, intersections[j], vSource.GetID(j), obstruction);

											int p = 0;
											while (!obstruction && p < j)
											{
												if (vSource.IsReflection(p))
												{
													if (vSource.IsReflection(p + 1))
														obstruction = LineRoomIntersection(intersections[p], intersections[p + 1], vSource.GetID(p), vSource.GetID(p + 1));
													else
														obstruction = LineRoomIntersection(intersections[p], intersections[p + 1], vSource.GetID(p));
												}
												else // assume p + 1 is reflection
													obstruction = LineRoomIntersection(intersections[p], intersections[p + 1], vSource.GetID(p + 1));
												p++;
											}
											if (!obstruction)
												vSource.Visible(feedsFDN);
										}
									}
									else
									{
										if (vSource.visible)
											vSource.Visible(feedsFDN);
									}
								}
								else
									vSource.Invisible();

								if (start.mDiffractionPath.sValid) // No future paths if !sValid and already set in sped section
									vSources.push_back(vSource);
							}
						}
					}
				}
			}
		}
	}
}

void Room::FirstOrderDiffraction(const vec3& point, VirtualSourceMap& ed, std::vector<VirtualSourceData>& vSources)
{
	bool feedsFDN = mMaxOrder == 1;
	for (auto it : mEdges)
	{
		size_t id = it.first;
		Edge edge = it.second;

		Diffraction::Path path = Diffraction::Path(point, mListenerPosition, edge);
		vec3 position = path.CalculateVirtualPostion();

		VirtualSourceData vSource;
		vSource.AddEdgeID(id, path);
		vSource.SetTransform(position);

		if (path.valid)
		{
			// Valid diffraction path
			vSource.Valid();

			if (mISMConfig.diffraction != DiffractionDepth::none && (path.inShadow || !mISMConfig.shadowOnly))
			{
				bool obstruction = LineRoomIntersection(point, path.GetApex());
				LineRoomIntersection(path.GetApex(), mListenerPosition, obstruction);

				if (!obstruction) // source and receiver can see edge
				{
					// Visible diffraction path
					vSource.Visible(feedsFDN);
				}
			}
		}
		ed.emplace(1, vSource);
		vSources.push_back(vSource);
	}
}

#pragma endregion

#pragma region Reflection

void Room::FirstOrderReflections(const vec3& point, VirtualSourceMap& sp, VirtualSourceVec& vSources)
{
	bool feedsFDN = mMaxOrder == 1;
	for (auto it : mWalls)
	{
		size_t id = it.first;
		Wall wall = it.second;

		VirtualSourceData vSource;

		vSource.AddWallID(id, wall.GetAbsorption());

		vec3 position;
		bool valid = wall.ReflectPointInWall(position, point);

		vec3 rPosition;
		bool rValid = wall.ReflectPointInWall(rPosition, mListenerPosition);

		if (rValid)
		{
			vSource.RValid();
			vSource.SetRPosition(rPosition);
		}
		if (valid)
		{
			vSource.Valid();
			vSource.SetTransform(position);

			vec3 intersection;
			valid = wall.LineWallIntersection(intersection, mListenerPosition, position);

			if (valid)
			{
				bool obstruction = LineRoomIntersection(point, intersection, id);
				LineRoomIntersection(intersection, mListenerPosition, id, obstruction);
				if (!obstruction)
				{
					vSource.Visible(feedsFDN);
				}
			}
		}
		vSources.push_back(vSource); // Only care if valid (not if rValid)
		sp.emplace(1, vSource); // In theory only need to do this if either valid or rValid. Could then remove later checks for next vSource
	}
}

void Room::HigherOrderReflections(const vec3& point, VirtualSourceMap& sp, VirtualSourceVec& vSources)
{
	for (int j = 1; j < mMaxOrder; j++)
	{
		int refOrder = j + 1;
		bool feedsFDN = mMaxOrder == refOrder;
		std::vector<vec3> intersections;
		size_t capacity = intersections.capacity();
		intersections.reserve(refOrder);
		std::fill_n(std::back_inserter(intersections), refOrder - capacity, vec3());
		auto n = sp.bucket(j);
		auto m = sp.bucket_size(n);
		for (auto it : mWalls)
		{
			size_t id = it.first;
			Wall wall = it.second;

			n = sp.bucket(j);
			auto vs = sp.begin(n);
			for (int i = 0; i < m; i++, vs++)
			{
				VirtualSourceData vSource = vs->second;

				if (id != vSource.GetID(j - 1) && (vSource.valid || vSource.rValid))
				{
					vSource.AddWallID(id, wall.GetAbsorption());

					bool r = vSource.rValid;
					bool s = vSource.valid;
					vSource.Reset();
					if (r)
					{
						vec3 rPosition;
						bool rValid = wall.ReflectPointInWall(rPosition, vSource.GetRPosition(j - 1)); // Need to add rValid checks for ed - sp
						if (rValid)
						{
							vSource.RValid();
							vSource.SetRPosition(rPosition);
						}
					}
					if (s) // Could add precomputed visibility between walls
					{
						// vSource.Reset(); // Resets validity and visibility

						vec3 position;
						bool valid = wall.ReflectPointInWall(position, vSource.GetPosition(j - 1));

						if (valid)
						{
							vSource.Valid();
							vSource.SetTransform(position);

							if (wall.GetRValid()) // wall.GetRValid() returns if mListenerPosition in front of current wall
							{
								// Check valid intersections
								valid = wall.LineWallIntersection(intersections[j], mListenerPosition, position);

								int n = 0;
								while (valid && n < j)
								{
									int idx = j - (n + 1); // Current bounce (j - n is previous bounce in code and next bounce in path)
									Wall previousWall = mWalls.find(vSource.GetID(idx))->second;
									valid = previousWall.LineWallIntersection(intersections[idx], intersections[j - n], vSource.GetPosition(idx));
									n++;
								}
								if (valid)
								{
									// Check for obstruction
									bool obstruction = LineRoomIntersection(intersections[j], mListenerPosition, id);
									LineRoomIntersection(intersections[0], point, vSource.GetID(0), obstruction);

									int p = 0;
									while (!obstruction && p < j)
									{
										obstruction = LineRoomIntersection(intersections[p], intersections[p + 1], vSource.GetID(p), vSource.GetID(p + 1));
										p++;
									}
									if (!obstruction)
									{
										vSource.Visible(feedsFDN);
									}
								}
							}
						}
						vSources.push_back(vSource); // Only added if previous source was valid (not if rValid)
					}
					if (s || r) // In theory this only needs to be if current source valid or rValid? - could then remove checks when finding next vSource
					{
						sp.emplace(refOrder, vSource); // Need rValid for edsp paths
					}
				}
			}
		}
	}
}

#pragma endregion