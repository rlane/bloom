PAGE_SIZE = 2**12

def bucketSingleProb(k,n,m)
	(1.0 - 1.0/m) ** (k*n)
end

def falsePositiveProb(k,n,m)
	(1.0 - bucketSingleProb(k,n,m)) ** k
end

def falsePositiveCount(k,n,m)
	n * falsePositiveProb(k,n,m)
end

# r: number of buckets
# k: number of 2nd level hash functions
# n: number of items
# m: bucket size in bits
def twoLevelFalsePositiveProb(r,k,n,m)
	falsePositiveProb(k,n.to_f/r,m)
end

def twoLevelFalsePositiveCount(r,k,n,m)
	n * twoLevelFalsePositiveProb(r,k,n,m)
end

def stdTwoLevelFalsePositiveCount(l,n,k=3)
	r = l / PAGE_SIZE
	m = PAGE_SIZE * 8
	twoLevelFalsePositiveCount(r,k,n,m)
end
