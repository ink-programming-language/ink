// Translated from solution.cpp.

var n: dynamic = cpp_uninitialized();

var s: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  read(n, s, s);
  write(( ((s == "week")) ? ( (((n == 6) || (n == 5))) ? 53 : 52) : ( ((n == 31)) ? 7 : ( ((n == 30)) ? 11 : 12))));
  return 0;
}
