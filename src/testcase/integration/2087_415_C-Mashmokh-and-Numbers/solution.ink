// Translated from solution.cpp.

var n: dynamic = cpp_uninitialized();

var k: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(100005);

func main() -> dynamic
{
  ios_base.sync_with_stdio(0);
  cin.tie();
  cout.tie();
  read(n, k);
  if (((k < (n / 2)) || (((n == 1) && k))))
  {
    write(-1);
    return 0;
  }
  if (((n == 1) && (k == 0)))
  {
    write(1);
    return 0;
  }
  k = (k - (((n - 2)) / 2));
  write(k, " ", (k * 2), " ");
  k = ((k * 2) + 1);
  {
    var i: dynamic = 0;
    while ((i <= (n - 3)))
    {
      write((k + i), " ");
      i += 1;
    }
  }
  return 0;
}
