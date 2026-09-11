// Translated from solution.cpp.

var x: dynamic = cpp_uninitialized();

var a: dynamic = [3, 15, 81, 6723, 50625, 2562991875];

var sg: dynamic = [0, 1, 2, 0, 3, 1, 2];

var n: dynamic = cpp_uninitialized();

var ans: dynamic = 0;

func main() -> dynamic
{
  read(n);
  while (cpp_update(n, "--"))
  {
    read(x);
    ans ^= sg[(lower_bound(a, (a + 6), x) - a)];
  }
  write(( (ans) ? "Furlo" : "Rublo"), "\n");
  return 0;
}
