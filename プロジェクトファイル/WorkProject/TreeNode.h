#pragma once
#include <vector>
#include <memory>

template <typename T>
class TreeNode 
{
public:
	T m_nodedata;
	TreeNode<T>* m_parent{};
	std::vector<std::unique_ptr<TreeNode<T>>> m_children;

	void Addchild(std::unique_ptr<TreeNode<T>> child) {
		
		m_children.push_back(std::move(child));
	}	
};