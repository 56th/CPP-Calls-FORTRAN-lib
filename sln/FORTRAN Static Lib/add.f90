function add(x, y) bind(c, name = 'fortranAdd') result(z)
	use iso_c_binding
	real(c_double) :: x, y, z
    z = x + y
end function add