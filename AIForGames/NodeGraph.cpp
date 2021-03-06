#include "NodeGraph.h"
#include "raylib.h"
#include "Ray2D.h"
#include "Game.h"
#include "math.h"
bool NodeGraph::doesNodeHaveNeighbor(NavNode* node)
{
	if (node == nullptr)
	{
		return false;
	}
	return node->linkTop != nullptr || node->linkRight != nullptr || node->linkBottom != nullptr || node->linkLeft != nullptr;
}

void NodeGraph::prepareNodesForPath()
{
	for (int i = 0; i < getCount(); i++)
	{
		nodes[i].parent = nullptr;
		nodes[i].pathCost = INT16_MAX;
		closedList[i] = false;
	}
}


int NodeGraph::getIndexOfNode(NavNode* node)
{
	return ((int)roundf(node->pos.x / nodeSpacing) * nodesHeight) + (int)roundf(node->pos.y / nodeSpacing);
}

NodeGraph::NodeGraph(int nodesWide, int nodesHight, float nodeSpacing)
{
	nodesWidth = nodesWide;
	nodesHeight = nodesHight;
	this->nodeSpacing = nodeSpacing;
	nodes = new NavNode[nodesWidth * nodesHeight]{};
	closedList = new bool[nodesWidth * nodesHeight]{ false };

	//initializing nodes
	float xOffset = 0;
	float yOffset = 0;
	for (int x = 0; x < nodesWidth; x++)
	{
		for (int y = 0; y < nodesHeight; y++)
		{
			getNodeAt(x,y)->pos.x = xOffset;
			getNodeAt(x,y)->pos.y = yOffset;

			yOffset += nodeSpacing;
			if (yOffset / nodeSpacing >= nodesHeight)//sort of a modulus
			{
				yOffset = 0;
			}
		}
		xOffset += nodeSpacing;
		if (xOffset / nodeSpacing >= nodesWidth)//sort of a modulus
		{
			xOffset = 0;
		}
	}
	
}

NavNode* NodeGraph::getNodeAt(int x, int y)
{
	if (x < 0 || x >= nodesWidth || y < 0 || y >= nodesHeight)
	{
		return nullptr;
	}

	int index = (x * nodesHeight) + y;
	/*if the provided coords is out of range*/
	if (index > nodesWidth * nodesHeight || index < 0)
	{
		return nullptr;
	}

	return &nodes[index];
}

NavNode* NodeGraph::getNodeAtPos(float x, float y)
{
	NavNode* bestNodeResult = nullptr;
	//testing closest node to see if it is valid (if it can be navigated to), otherwise, find closest valid node
	bestNodeResult = getNodeAt((int)roundf(x / nodeSpacing), (int)roundf(y / nodeSpacing));
	
	if (!doesNodeHaveNeighbor(bestNodeResult))//if the current result isnt valid
	{
		bestNodeResult = getNodeAt((int)roundf(x / nodeSpacing) - 1, (int)roundf(y / nodeSpacing));//try the node to the left
	}

	return bestNodeResult;
}

void NodeGraph::linkNodes()
{
	NavNode* nodeA = nullptr;
	NavNode* nodeB = nullptr;
	for (int x = 0; x < nodesWidth; x++)
	{
		for (int y = 0; y < nodesHeight; y++)
		{
			nodeA = getNodeAt(x, y);
			//link top node
			if ((nodeB = getNodeAt(x, y + 1)) != nullptr)
			{
				if (canNodeLinkToNode(nodeA, nodeB))
				{
					nodeA->linkTop = nodeB;
				}
			}

			//link right node
			if ((nodeB = getNodeAt(x + 1, y)) != nullptr)
			{
				if (canNodeLinkToNode(nodeA, nodeB))
				{
					nodeA->linkRight = nodeB;
				}
			}

			//link bottom node
			if ((nodeB = getNodeAt(x, y - 1)) != nullptr)
			{
				if (canNodeLinkToNode(nodeA, nodeB))
				{
					nodeA->linkBottom = nodeB;
				}
			}
			

			//link left node
			if ((nodeB = getNodeAt(x - 1, y)) != nullptr)
			{
				if (canNodeLinkToNode(nodeA, nodeB))
				{
					nodeA->linkLeft = nodeB;
				}
			}

		}
	}
}

bool NodeGraph::canNodeLinkToNode(NavNode* nodeA, NavNode* nodeB)
{
	return !Game::get()->doesRayHitWall(Ray2D(nodeA->pos, Vector2Subtract(nodeB->pos, nodeA->pos)));
}

/*used for sorting open list when calculating dijkstras path*/
bool lessThanComparitor(NavNode* a, NavNode* b)
{
	return a->pathCost < b->pathCost;
}


void NodeGraph::processNodeLinkForPath(NavNode* currentNode, NavNode* connection, std::list<NavNode*>& openList)
{
	if (connection == nullptr)
		return;

	bool inOpened = std::find(openList.begin(), openList.end(), connection) != openList.end();
	bool inClosed = this->closedList[getIndexOfNode(connection)];

	int newPathcost = currentNode->pathCost + 1;//pathCost + 1 because there are no weights or edges.

	//if not already traversed (i.e, not in closed list) OR the new path cost is less than the path cost of the connection
	if (!inClosed || newPathcost < connection->pathCost)
	{
		connection->pathCost = newPathcost;
		connection->parent = currentNode;
	}

	//if not already traversed and is not in the open list
	if (!inClosed && !inOpened)
	{
		openList.push_back(connection);
	}
}

NavNode** NodeGraph::getShortestPathDijkstras(Vector2 startPos, Vector2 endPos, int& pathCount)
{
	NavNode* startNode = getNodeAtPos(startPos.x, startPos.y);
	NavNode* endNode = getNodeAtPos(endPos.x, endPos.y);

	/*handling invalid positions, just returns 0,0*/
	if (startNode == nullptr || endNode == nullptr)
	{
		pathCount = 1;
		NavNode** result = new NavNode * [1];
		result[0] = getNodeAt(0, 0);
		return result;
	}

	prepareNodesForPath();//prepare nodes for finding a path

	
	std::list<NavNode*> openList;//to be used as priority queue

	openList.push_back(startNode);//add start node

	while (!openList.empty())
	{
		openList.sort(lessThanComparitor);//sort by smallest path cost value

		NavNode* currentNode = openList.front();//get the first node in queue and remove from open list
		openList.pop_front();

		//stop all path finding apon reaching end node
		if (currentNode == endNode)
		{
			break;
		}

		this->closedList[getIndexOfNode(currentNode)] = true;

		//for each connection to the current node, process and add to open list
		processNodeLinkForPath(currentNode, currentNode->linkTop, openList);
		processNodeLinkForPath(currentNode, currentNode->linkRight, openList);
		processNodeLinkForPath(currentNode, currentNode->linkLeft, openList);
		processNodeLinkForPath(currentNode, currentNode->linkBottom, openList);
	}

	//construct result
	NavNode* current = endNode;
	int size = 0;
	//calculate size of path in nodes
	while (current != nullptr)
	{
		current = current->parent;
		size++;
	}
	pathCount = size;

	//construct result array
	NavNode** result = new NavNode * [size];
	current = endNode;
	while (current != nullptr)
	{
		result[size - 1] = current;
		current = current->parent;
		size--;
	}
	return result;
}

void NodeGraph::debugDrawNodes(Vector2 playerPos)
{
	//loop through each node and draw lines to their connected neighbors
	for (int i = 0; i < (nodesWidth * nodesHeight); i++)
	{
		NavNode* currentNode = &nodes[i];
		
		if (currentNode->linkTop != nullptr)
		{
			DrawLine((int)(currentNode->pos.x - 1), (int)currentNode->pos.y, (int)(currentNode->pos.x - 1), (int)(currentNode->pos.y + nodeSpacing), DARKPURPLE);
		}
		if (currentNode->linkRight != nullptr)
		{
			DrawLine((int)currentNode->pos.x, (int)(currentNode->pos.y - 1), (int)(currentNode->pos.x + nodeSpacing), (int)(currentNode->pos.y - 1), DARKPURPLE);
		}
		if (currentNode->linkBottom != nullptr)
		{
			DrawLine((int)(currentNode->pos.x + 1), (int)currentNode->pos.y, (int)(currentNode->pos.x + 1), (int)(currentNode->pos.y - nodeSpacing), DARKGREEN);
		}
		if (currentNode->linkLeft != nullptr)
		{
			DrawLine((int)currentNode->pos.x, (int)(currentNode->pos.y + 1), (int)(currentNode->pos.x - nodeSpacing), (int)(currentNode->pos.y + 1), DARKGREEN);
		}
		if (getNodeAtPos(playerPos.x, playerPos.y) == currentNode)
		{
			DrawRectangle((int)(currentNode->pos.x - 2), (int)(currentNode->pos.y - 2), 4, 4, GOLD);
		}
		else
		{
			DrawRectangle((int)(currentNode->pos.x - 2), (int)(currentNode->pos.y - 2), 4, 4, DARKGREEN);
		}
	}
}
