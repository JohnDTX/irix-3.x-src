ip2mapio()
{
	return (syscall(71, 1, 0, 0, 0, 0));
}

ip2mapmem()
{
	return (syscall(71, 2, 0, 0, 0, 0));
}

ip2unmap()
{
	return (syscall(71, 0, 0, 0, 0, 0));
}
