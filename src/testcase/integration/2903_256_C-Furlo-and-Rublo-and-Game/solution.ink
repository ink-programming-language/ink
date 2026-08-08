// Translated from solution.cpp.

var x: dynamic;

var a = [3, 15, 81, 6723, 50625, 2562991875];

var sg = [0, 1, 2, 0, 3, 1, 2];

var n: dynamic;

var ans = 0;

func main()
{
  read(n);
  while (cpp_update(n, "--"))
  {
    read(x);
    ans ^= sg[(lower_bound(a, (a + 6), x) - a)];
  }
  write((if (ans) "Furlo" else "Rublo"), "\n");
  return 0;
}
