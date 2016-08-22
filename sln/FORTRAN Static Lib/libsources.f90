subroutine square(n, m) bind(c, name = 'fortranSquare') 
	use iso_c_binding
	integer(c_size_t) :: n, m
	m = n * n
end subroutine square