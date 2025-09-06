bool VisitCXXNewExpr(clang::CXXNewExpr* E){
	llvm::errs() 
    << E->getBeginLoc().printToString(Context.getSourceManager()) 
    << " Error: 'new' is not allowed in kernel, only value types are allowed.\n";
    
	reportError(E->getBeginLoc(), dynamicMemory);
	Valid = false;
	return true;
}