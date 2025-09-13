#pragma once
#include <clang/Basic/SourceLocation.h>
#include <clang/Rewrite/Core/Rewriter.h>

struct PendingEdit { 
	PendingEdit(clang::SourceLocation charLoc, const std::string& replacement)
		:m_Range{charLoc},
		m_Replacement{replacement},
		m_Insert{false},
		m_Single{true}
	{
	}


	PendingEdit(clang::SourceRange range, const std::string& replacement, bool insert = false, bool insertAfter=false)
		:m_Range{range},m_Replacement{replacement},m_Insert{insert},m_InsertAfter{insertAfter},m_Single{false}
	{

	}

	void replace(clang::Rewriter& rewriteObj) {
		if (m_Insert) {
			rewriteObj.InsertText(m_Range.getBegin(),m_Replacement,m_InsertAfter);
		}
		else if (m_Single) {
			rewriteObj.ReplaceText(m_Range.getBegin(), 1, m_Replacement);
		}
		else {
			rewriteObj.ReplaceText(m_Range, m_Replacement);
		}
		
	}

	clang::SourceRange m_Range; 
	std::string m_Replacement; 
	bool m_Insert;
	bool m_InsertAfter;
	bool m_Single;
};


