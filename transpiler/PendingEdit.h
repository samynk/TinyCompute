#pragma once
#include <clang/Basic/SourceLocation.h>
#include <clang/Rewrite/Core/Rewriter.h>

struct PendingEdit { 

	PendingEdit(clang::SourceRange range, const std::string& replacement, bool insert = false, bool insertAfter=false)
		:m_Range{range},m_Replacement{replacement},m_Insert{insert},m_InsertAfter{insertAfter}
	{

	}

	void replace(clang::Rewriter& rewriteObj) {
		if (m_Insert) {
			rewriteObj.InsertText(m_Range.getBegin(),m_Replacement,m_InsertAfter);
		}
		else {
			rewriteObj.ReplaceText(m_Range, m_Replacement);
		}
	}

	clang::SourceRange m_Range; 
	std::string m_Replacement; 
	bool m_Insert;
	bool m_InsertAfter;
};


