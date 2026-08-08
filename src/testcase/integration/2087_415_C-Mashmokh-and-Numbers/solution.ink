// Translated from solution.cpp.

var n: dynamic;

var k: dynamic;

var a = cpp_array(100005);

func main()
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
    var i = 0;
    while ((i <= (n - 3)))
    {
      write((k + i), " ");
      i += 1;
    }
  }
  return 0;
}
