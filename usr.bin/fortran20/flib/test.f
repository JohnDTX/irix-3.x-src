	character*10 a
	i = 0
	a = a(1:i)
	end
	subroutine patranexit(addr, n)
	integer * 4 addr
	integer * 2 n
	write(*,*) 'addr = ',addr,' n = ',n
	return
	end
